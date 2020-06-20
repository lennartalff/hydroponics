/**
 * @file owi.h
 * @author Thies Lennart Alff
 * @version 0.1
 * @date 2020-05-06
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef OWI_H_
#define OWI_H_

#include "common.h"

enum owi_resolution_e { OWI_RES_12, OWI_RES_11, OWI_RES_10, OWI_RES_9 };

typedef enum owi_resolution_e owi_resolution_t;

struct ds18b20_s {
    uint8_t rom[8];
    uint8_t available;
    uint16_t temperature;
    uint8_t config_register;
    uint8_t alarm_high_register;
    uint8_t alarm_low_register;
    owi_resolution_t resolution;
};

typedef struct ds18b20_s ds18b20_t;

void owi_init(volatile uint8_t *owi_port, uint8_t owi_pinNumber);

return_status_t owi_reset();

void owi_write_bit(uint8_t bit);

void owi_write_byte(uint8_t byte);

uint8_t owi_read_bit();

uint8_t owi_read_byte();

void owi_read_rom(uint8_t *romBuffer);

return_status_t owi_search();

return_status_t owi_search_first();

return_status_t owi_search_next();

return_status_t owi_get_buffered_rom(uint8_t *buffer);

uint8_t owi_verify_device();

return_status_t owi_get_devices(ds18b20_t *devices, uint8_t array_size,
                             uint8_t *count);

return_status_t owi_set_resolution_all(owi_resolution_t resolution);

return_status_t owi_get_resolution_all(owi_resolution_t *resolution);

void owi_set_resolution(ds18b20_t *device, owi_resolution_t resolution);

return_status_t owi_read_temperature(ds18b20_t *device);

void owi_read_scratchpad(ds18b20_t *device, uint8_t *buffer);

return_status_t owi_start_conversion();

return_status_t owi_wait_conversion(ds18b20_t *device);

#endif /* AVR_LIB_OWI_H_ */
