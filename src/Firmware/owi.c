/**
 * @file owi.c
 * @author Thies Lennart Alff (you@domain.com)
 * @version 0.1
 * @date 2020-05-06
 *
 * @copyright Copyright (c) 2020
 *
 */
#include "owi.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>

#define OWI_READ_ROM_CMD 0x33
#define OWI_SEARCH_ROM_CMD 0xF0
#define OWI_MATCH_ROM_CMD 0x55
#define OWI_SKIP_ROM_CMD 0xCC
#define OWI_CONVERT_TEMP_CMD 0x44
#define OWI_SCRATCHPAD_READ_CMD 0xBE
#define OWI_SCRATCHPAD_WRITE_CMD 0x4E
#define OWI_SCRATCHPAD_COPY_CMD 0x48

#define OWI_RES9_BYTE 0x1F
#define OWI_RES10_BYTE 0x3F
#define OWI_RES11_BYTE 0x5F
#define OWI_RES12_BYTE 0x7F

#define CONV_TIME_9_MS 100
#define CONV_TIME_10_MS 200
#define CONV_TIME_11_MS 400
#define CONV_TIME_12_MS 800
#define CONV_TIME_MAX 800

static volatile uint8_t *port;
static uint8_t pinNumber = 0;

static uint8_t rom[8] = {0};
static uint8_t last_discrepancy;
static uint8_t last_family_discrepancy;
static uint8_t last_device_flag;
static uint8_t crc8;

/**
 * @brief Holds the resolution set by @ref owi_set_resolution_all.
 *
 */
static owi_resolution_t resolution_all = OWI_RES_9;

/**
 * @brief Lookup table for Maxim Integrated's OWI CRC.
 *
 */
static uint8_t dscrc_table[] = {
    0,   94,  188, 226, 97,  63,  221, 131, 194, 156, 126, 32,  163, 253, 31,
    65,  157, 195, 33,  127, 252, 162, 64,  30,  95,  1,   227, 189, 62,  96,
    130, 220, 35,  125, 159, 193, 66,  28,  254, 160, 225, 191, 93,  3,   128,
    222, 60,  98,  190, 224, 2,   92,  223, 129, 99,  61,  124, 34,  192, 158,
    29,  67,  161, 255, 70,  24,  250, 164, 39,  121, 155, 197, 132, 218, 56,
    102, 229, 187, 89,  7,   219, 133, 103, 57,  186, 228, 6,   88,  25,  71,
    165, 251, 120, 38,  196, 154, 101, 59,  217, 135, 4,   90,  184, 230, 167,
    249, 27,  69,  198, 152, 122, 36,  248, 166, 68,  26,  153, 199, 37,  123,
    58,  100, 134, 216, 91,  5,   231, 185, 140, 210, 48,  110, 237, 179, 81,
    15,  78,  16,  242, 172, 47,  113, 147, 205, 17,  79,  173, 243, 112, 46,
    204, 146, 211, 141, 111, 49,  178, 236, 14,  80,  175, 241, 19,  77,  206,
    144, 114, 44,  109, 51,  209, 143, 12,  82,  176, 238, 50,  108, 142, 208,
    83,  13,  239, 177, 240, 174, 76,  18,  145, 207, 45,  115, 202, 148, 118,
    40,  171, 245, 23,  73,  8,   86,  180, 234, 105, 55,  213, 139, 87,  9,
    235, 181, 54,  104, 138, 212, 149, 203, 41,  119, 244, 170, 72,  22,  233,
    183, 85,  11,  136, 214, 52,  106, 43,  117, 151, 201, 74,  20,  246, 168,
    116, 42,  200, 150, 21,  75,  169, 247, 182, 232, 10,  84,  215, 137, 107,
    53};

/**
 * @brief Performs CRC computation.
 *
 * @param value Input for CRC generation.
 * @return uint8_t Returns the crc checksum.
 */
static uint8_t do_crc8(uint8_t value) {
    crc8 = dscrc_table[crc8 ^ value];
    return crc8;
}

/**
 * @brief Helper function that initialized communication with
 * #OWI_MATCH_ROM_CMD.
 *
 * This function should be followed by the actual command that is to be
 * executed.
 *
 * @param device Pointer to the OWI device that holds the ROM address. \see
 * #ds18b20_s
 */
static void owi_match_rom(ds18b20_t *device) {
    owi_reset();
    owi_write_byte(OWI_MATCH_ROM_CMD);
    for (uint8_t i = 0; i < 8; i++) {
        owi_write_byte(device->rom[i]);
    }
    _delay_us(10);
}

/**
 * @brief Initializes the OWI module.
 *
 * @param owi_port Port of the data GPIO pin used for OWI communication.
 * @param owi_pinNumber Identifies the GPIO pin of the port used for OWI
 * communication.
 */
void owi_init(volatile uint8_t *owi_port, uint8_t owi_pinNumber) {
    port = owi_port;
    pinNumber = owi_pinNumber;
    owi_set_resolution_all(OWI_RES_12);
}

/**
 * @brief Resets all devices on the OWI bus.
 *
 * Holds down the OWI data line to reset all connected devices. Devices will
 * answer with a presence pulse indicating their existence.
 *
 * @return Returns one of the following exit codes specified by @ref
 * return_status_t
 * - @ref RET_SUCCESS indicating that at least one device is available
 * - @ref RET_OWI_NO_PRESENCE indicating that **no** device is available
 */
return_status_t owi_reset() {
    uint8_t response = 0;
    *port &= ~(1 << pinNumber);  // set output to 0 for reset pulse
    DDR_REGISTER(*port) |=
        (1 << pinNumber);  // set the pin for 1-wire as output
    _delay_us(500);
    DDR_REGISTER(*port) &=
        ~(1 << pinNumber);  // set pin to input to read the presence pulse
    *port |= (1 << pinNumber);
    _delay_us(70);  // wait for presence pulse
    response = PIN_REGISTER(*port) & (1 << pinNumber);
    _delay_us(200);
    *port |= (1 << pinNumber);
    DDR_REGISTER(*port) |= (1 << pinNumber);
    _delay_us(600);
    return response ? RET_OWI_NO_PRESENCE : RET_SUCCESS;
}

/**
 * @brief Writes a single bit to the OWI bus.
 *
 * @param bit Writes a logical 1 if @p bit evaluates to `true`. Otherwise a
 * logical 0 will be written.
 */
void owi_write_bit(uint8_t bit) {
    *port |= (1 << pinNumber);                // set pin high
    DDR_REGISTER(*port) |= (1 << pinNumber);  // set pin as output
    *port &= ~(1 << pinNumber);  // set output low to start write slot
    if (bit) {
        _delay_us(8);
    } else {
        _delay_us(80);
    }
    DDR_REGISTER(*port) &=
        ~(1 << pinNumber);  // set pin as input to release the bus
    *port |= (1 << pinNumber);
    if (bit) {
        _delay_us(80);
    } else {
        _delay_us(2);
    }
}

/**
 * @brief Writes Byte by repeatingly calling @ref owi_write_bit().
 *
 * @param byte The data to write.
 */
void owi_write_byte(uint8_t byte) {
    for (uint8_t mask = 0x01; mask != 0; mask <<= 1) {
        owi_write_bit(byte & mask);
    }
}

/**
 * @brief Reads a single bit from the OWI bus.
 *
 * @return uint8_t Returns 1 if OWI bus signals a logical 1, returns 0
 * otherwise.
 */
uint8_t owi_read_bit() {
    uint8_t bit = 0;
    *port |= (1 << pinNumber);                // set pin high
    DDR_REGISTER(*port) |= (1 << pinNumber);  // configure pin as output
    *port &= ~(1 << pinNumber);               // start read slot by pulling low
    _delay_us(2);
    DDR_REGISTER(*port) &=
        ~(1 << pinNumber);  // release the bus by setting pin as input
    *port |= (1 << pinNumber);
    _delay_us(5);
    bit = (PIN_REGISTER(*port) & (1 << pinNumber)) ? true
                                                   : false;  // read the input
    _delay_us(60);
    return bit;
}

/**
 * @brief Reads a byte from the OWI bus starting with the least-significant bit
 * by repeatingly calling @ref owi_read_bit().
 *
 * @return uint8_t Returns the byte read from the OWI bus.
 */
uint8_t owi_read_byte() {
    uint8_t byte = 0;
    for (uint8_t mask = 0x01; mask != 0; mask <<= 1) {
        byte |= (owi_read_bit() * mask);
    }
    return byte;
}

/**
 * @brief Reads a ROM address by performing a complete communication cycle.
 *
 * @attention Only use with a single device connected to the bus. Otherwise data
 * collision will occure.
 *
 * @param romBuffer
 */
void owi_read_rom(uint8_t *romBuffer) {
    owi_reset();
    owi_write_byte(OWI_READ_ROM_CMD);
    for (uint8_t i = 0; i < 8; i++) {
        romBuffer[i] = owi_read_byte();
    }
}

/**
 * @brief Resets the stored search state to initial values and performs a single
 * search command by calling @ref owi_search().
 *
 * @return Returns one of the following exit codes defined in @ref return_status_t
 * - Return value of @ref owi_search().
 */
return_status_t owi_search_first() {
    last_discrepancy = 0;
    last_device_flag = false;
    last_family_discrepancy = 0;

    return owi_search();
}

/**
 * @brief Convinience function that invokes @ref owi_search().
 *
 * @return Returns one of the following exit codes defined in @ref return_status_t.
 * - Return value of @ref owi_search()
 */
return_status_t owi_search_next() { return owi_search(); }

/**
 * @brief Performs a single search cycle.
 *
 * After each call the newly detected ROM address is stored in @ref rom and can
 * be accessed by calling @ref owi_get_buffered_rom().
 *
 * @return Returns one of the following exit codes defined in @ref return_status_t
 * - @ref RET_SUCCESS if new device was detected and it's ROM address was
 * stored
 * - @ref RET_OWI_SEARCH_LAST_DEVICE if all devices has been detected already. A
 * new search can be started by calling @ref owi_search_first().
 * - Return value of @ref owi_reset() if it's call was not successfull.
 * - @ref RET_OWI_CRC_ERR if the CRC checksum of the read address is invalid.
 * - @ref RET_OWI_INVALID_FAMILY_CODE
 */
return_status_t owi_search() {
    uint8_t bit_index = 1;  // count bits starting from 1 (!!!)
    uint8_t byte_index = 0;
    uint8_t last_zero = 0;
    uint8_t response_bit, inv_response_bit;
    uint8_t byte_mask = 1;
    uint8_t direction;

    return_status_t status = RET_OWI_SEARCH_LAST_DEVICE;

    crc8 = 0;

    if (!last_device_flag) {
        // send reset pulse and make sure, that sensors are available
        status = owi_reset();
        if (!(status == RET_SUCCESS)) {
            last_discrepancy = 0;
            last_device_flag = false;
            last_family_discrepancy = 0;
            return status;
        }

        // send search command
        owi_write_byte(OWI_SEARCH_ROM_CMD);

        do {
            response_bit = owi_read_bit();
            inv_response_bit = owi_read_bit();

            // both bits = 1 means no device is responding
            if ((response_bit == 1) && (inv_response_bit == 1)) {
                break;
            } else {
                // all devices have the same bit
                if (response_bit != inv_response_bit)
                    direction = response_bit;
                else {
                    if (bit_index < last_discrepancy)
                        direction = ((rom[byte_index] & byte_mask) > 0);
                    else
                        direction = (bit_index == last_discrepancy);

                    if (direction == 0) {
                        last_zero = bit_index;
                        if (last_zero < 9) last_family_discrepancy = last_zero;
                    }
                }

                if (direction == 1)
                    rom[byte_index] |= byte_mask;
                else
                    rom[byte_index] &= ~byte_mask;

                // tell the devices the search direction
                owi_write_bit(direction);

                bit_index++;
                byte_mask <<= 1;

                if (byte_mask == 0) {
                    do_crc8(rom[byte_index]);
                    byte_index++;
                    byte_mask = 1;
                }
            }
        } while (byte_index < 8);  // rom bytes are 0-7

        if (!((bit_index < 65) || (crc8 != 0))) {
            last_discrepancy = last_zero;

            if (last_discrepancy == 0) last_device_flag = true;

            status = RET_SUCCESS;
        } else {
            status = RET_OWI_CRC_ERR;
        }
    }

    if (!(status == RET_SUCCESS) || !rom[0]) {
        if (!rom[0]) {
            status = RET_OWI_INVALID_FAMILY_CODE;
        }
        last_discrepancy = 0;
        last_device_flag = false;
        last_family_discrepancy = 0;
    }

    return status;
}

/**
 * @brief Reads the currently buffered ROM address into @p buffer.
 *
 * The buffer holds the ROM address of the last scanned device by @ref
 * owi_search_first() or owi_search_next().
 *
 * \par Example
 * @code{.c}
 * // start a new search and store the first address in the buffer.
 * owi_search_first();
 * // store the buffered ROM address in an array.
 * owi_get_buffered_rom(first_rom);
 * // get the next address
 * owi_search_next();
 * // store the buffered ROM again.
 * owi_get_buffered_rom(second_rom);
 * @endcode
 *
 * @param[out] buffer
 * @attention Make sure that the length of the array referenced by @p buffer is
 * approprietly sized to hold the ROM address.
 * @return Returns one of the following exit codes defined in @ref
 * return_status_t.
 * - @ref RET_SUCCESS
 */
return_status_t owi_get_buffered_rom(uint8_t *buffer) {
    for (uint8_t i = 0; i < 8; i++) {
        buffer[i] = rom[i];
    }
    return RET_SUCCESS;
}

/**
 * @brief Unused
 *
 * @return uint8_t
 */
uint8_t owi_verify_device() {
    uint8_t rom_copy[8];
    uint8_t ld_copy, lfd_copy, ldf_copy, result;

    for (uint8_t i = 0; i < 8; i++) {
        rom_copy[i] = rom[i];
    }

    ld_copy = last_discrepancy;
    lfd_copy = last_family_discrepancy;
    ldf_copy = last_device_flag;

    last_discrepancy = 64;
    last_device_flag = false;

    if (owi_search()) {
        result = true;
        for (uint8_t i = 0; i < 8; i++) {
            if (rom_copy[i] != rom[i]) {
                result = false;
                break;
            }
        }
    }

    else
        result = false;

    for (uint8_t i = 0; i < 8; i++) {
        rom[i] = rom_copy[i];
    }

    last_discrepancy = ld_copy;
    last_family_discrepancy = lfd_copy;
    last_device_flag = ldf_copy;

    return result;
}

/**
 * @brief Get a list of all availabe OWI devices.
 *
 * Only the ROM-Address of a device is stored. All other attributes stay
 * uninitialized.
 *
 * @param[in] devices Pointer to an array of OWI devices.
 * @param array_size Maximum length of the array referenced by \p devices.
 * @param[out] count The number of devices available on the bus.
 * @return Returns one of the following exit codes defined in
 * #return_status_t.
 * - @ref RET_SUCCESS
 * - Return value of @ref owi_search_first()
 * - Return value of @ref owi_search_next()
 */
return_status_t owi_get_devices(ds18b20_t *devices, uint8_t array_size,
                                uint8_t *count) {
    return_status_t status;

    status = owi_search_first();

    while (*count < array_size) {
        if (status == RET_SUCCESS) {
            for (uint8_t i = 0; i < 8; i++) {
                devices[*count].rom[i] = rom[i];
            }
            devices[*count].available = true;
            (*count)++;
        } else
            break;

        status = owi_search_next();
    }

    return RET_SUCCESS;
}

/**
 * @brief Gets the globally set resolution.
 *
 * @param[out] resolution Resolution is written to @p resolution.
 * @return Returns one of the following exit codes defined in @ref
 * return_status_t.
 * - @ref RET_SUCCESS
 */
return_status_t owi_get_resolution_all(owi_resolution_t *resolution) {
    *resolution = resolution_all;
    return RET_SUCCESS;
}

/**
 * @brief Sets the resolution for all devices on the bus.
 *
 * Stores the resolution in #resolution_all.
 *
 * @param resolution One of the resolutons available in @ref return_status_t.
 * @return Returns one of the following exit codes defined in @ref
 * owi_return_e.
 * - @ref RET_SUCCESS
 * - @ref RET_OWI_UNKNOWN_RES
 * - One of the return values of @ref owi_reset()
 */
return_status_t owi_set_resolution_all(owi_resolution_t resolution) {
    return_status_t status;
    uint8_t resolution_byte;
    switch (resolution) {
        case OWI_RES_9:
            resolution_byte = OWI_RES9_BYTE;
            break;
        case OWI_RES_10:
            resolution_byte = OWI_RES10_BYTE;
            break;
        case OWI_RES_11:
            resolution_byte = OWI_RES11_BYTE;
            break;
        case OWI_RES_12:
            resolution_byte = OWI_RES12_BYTE;
            break;
        default:
            return RET_OWI_UNKNOWN_RES;
    }
    resolution_all = resolution;

    status = owi_reset();
    if (!(status == RET_SUCCESS)) {
        return status;
    }
    owi_write_byte(OWI_SKIP_ROM_CMD);
    owi_write_byte(OWI_SCRATCHPAD_WRITE_CMD);
    // first two byte are for temperature alarm -> ignore them
    owi_write_byte(0);
    owi_write_byte(0);
    owi_write_byte(resolution_byte);
    return RET_SUCCESS;
}

/**
 * @brief Sets the resolution for a specified device.
 *
 * @param device Pointer to @ref ds18b20_t sensor.
 * @param resolution Available resolutions are defined in @ref owi_resolution_e.
 */
void owi_set_resolution(ds18b20_t *device, owi_resolution_t resolution) {
    switch (resolution) {
        case OWI_RES_9:
            device->config_register = OWI_RES9_BYTE;
            break;
        case OWI_RES_10:
            device->config_register = OWI_RES10_BYTE;
            break;
        case OWI_RES_11:
            device->config_register = OWI_RES11_BYTE;
            break;
        case OWI_RES_12:
            device->config_register = OWI_RES12_BYTE;
            break;
    }

    device->resolution = resolution;

    owi_reset();
    owi_write_byte(OWI_MATCH_ROM_CMD);
    for (uint8_t i = 0; i < 8; i++) {
        owi_write_byte(device->rom[i]);
    }

    owi_write_byte(OWI_SCRATCHPAD_WRITE_CMD);
    owi_write_byte(device->alarm_high_register);
    owi_write_byte(device->alarm_low_register);
    owi_write_byte(device->config_register);

    owi_reset();
    owi_write_byte(OWI_MATCH_ROM_CMD);
    for (uint8_t i = 0; i < 8; i++) {
        owi_write_byte(device->rom[i]);
    }

    owi_write_byte(OWI_SCRATCHPAD_COPY_CMD);
    _delay_ms(10);
}

/**
 * @brief Starts a temperature conversion for all devices on the bus.
 *
 * This function call should be followed by @ref owi_wait_conversion() call
 * before reading the temperature by calling @ref owi_read_temperature().
 *
 * @par Example
 * @code{.c}
 * owi_start_conversion();
 * owi_wait_conversion(NULL);
 * owi_read_temperature(ptr_to_device);
 * @endcode
 *
 * @return Returns one of the following exit codes defined in @ref return_status_t.
 * - @ref RET_SUCCESS
 * - Return value of @ref owi_reset() if an error occured.
 */
return_status_t owi_start_conversion() {
    return_status_t status;
    status = owi_reset();

    if (!(status == RET_SUCCESS)) {
        return status;
    }
    owi_write_byte(OWI_SKIP_ROM_CMD);
    owi_write_byte(OWI_CONVERT_TEMP_CMD);
    return RET_SUCCESS;
}

/**
 * @brief Waits for a specified amount of time it takes for a temperature
 * conversion to complete.
 *
 * If @p device is NULL, the time to wait is specified by the most recent
 * call of #owi_set_resolution_all. Otherwise @ref ds18b20_s.resolution
 * referenced by @p device will be used.
 *
 * @param device Pointer to a @ref ds18b20_t sensor.
 * @return Returns one of the following exit codes specified in
 * @ref return_status_t.
 * - @ref RET_SUCCESS
 * - @ref RET_OWI_UNKNOWN_RES
 */
return_status_t owi_wait_conversion(ds18b20_t *device) {
    owi_resolution_t resolution;
    if (device == NULL) {
        resolution = resolution_all;
    } else {
        resolution = device->resolution;
    }
    switch (resolution_all) {
        case OWI_RES_9:
            _delay_ms(CONV_TIME_9_MS);
            break;
        case OWI_RES_10:
            _delay_ms(CONV_TIME_10_MS);
            break;
        case OWI_RES_11:
            _delay_ms(CONV_TIME_11_MS);
            break;
        case OWI_RES_12:
            _delay_ms(CONV_TIME_12_MS);
            break;
        default:
            _delay_ms(CONV_TIME_12_MS);
            return RET_OWI_UNKNOWN_RES;
    }
    return RET_SUCCESS;
}

/**
 * @brief Reads the temperature from the scratchpad memory of the @p device.
 *
 * For example usage of a complete temperature measurement see @ref
 * owi_start_conversion().
 *
 * @param[in, out] device Pointer to the sensor. The selected resolution is read
 * from it and the temperature is written to it.
 * @return Returns one of the following exit codes defined in @ref return_status_t.
 * - @ref RET_SUCCESS
 * - @ref RET_OWI_UNKNOWN_RES
 */
return_status_t owi_read_temperature(ds18b20_t *device) {
    uint8_t scratchpad_buffer[9];

    owi_read_scratchpad(device, scratchpad_buffer);
    device->temperature = scratchpad_buffer[0] | (scratchpad_buffer[1] << 8);

    // mask out bits that are undefined in certain resolution modes
    switch (device->resolution) {
        case OWI_RES_9:
            device->temperature &= 0xFFF8;
            break;
        case OWI_RES_10:
            device->temperature &= 0xFFFC;
            break;
        case OWI_RES_11:
            device->temperature &= 0xFFFE;
            break;
        case OWI_RES_12:
            break;
        default:
            return RET_OWI_UNKNOWN_RES;
    }

    // sign bit is set
    if (device->temperature & 0x8000) {
        // mask out the sign bits
        device->temperature &= 0x07FF;
        // twos complement
        device->temperature = ((~device->temperature) + 1);
    }

    return RET_SUCCESS;
}

/**
 * @brief Reads the scratchpad of a sensor.
 *
 * @param[in] device Holding the ROM address of the sensor.
 * @param[out] buffer Pointer to the buffer.
 * @attention Length of @p buffer needs to be at least 9.
 */
void owi_read_scratchpad(ds18b20_t *device, uint8_t *buffer) {
    owi_match_rom(device);
    owi_write_byte(OWI_SCRATCHPAD_READ_CMD);
    for (uint8_t i = 0; i < 9; i++) {
        buffer[i] = owi_read_byte();
    }
}
