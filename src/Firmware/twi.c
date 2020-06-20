#include "twi.h"

#include <avr/io.h>
#include <util/twi.h>

#include "serial.h"

return_status_t twi_sla_w(uint8_t slave_address);
return_status_t twi_sla_r(uint8_t slave_address);
return_status_t twi_read_ack(uint8_t *data);
return_status_t twi_read_nack(uint8_t *data);

void twi_init() { TWBR = 72; }

return_status_t twi_start_write(uint8_t slave_address) {
    serial_debug(SERIAL_SRC_TWI, "Start transmitting");
    uint8_t status;
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    loop_until_bit_is_set(TWCR, TWINT);

    status = TW_STATUS;

    if (!(status == TW_START || status == TW_REP_START)) {
        return RET_TWI_START_ERR;
    }

    status = twi_sla_w(slave_address);
    if (status != RET_SUCCESS) {
        serial_debug(
            SERIAL_SRC_TWI,
            "Could not start master transmitter. Error code: %d TWSTATUS: %02x",
            status, TW_STATUS);
    }

    ASSERT_SUCCESS(status);

    return RET_SUCCESS;
}

return_status_t twi_start_read(uint8_t slave_address) {
    serial_debug(SERIAL_SRC_TWI, "Start receiving");
    uint8_t status;
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    loop_until_bit_is_set(TWCR, TWINT);

    status = TW_STATUS;

    if (!(status == TW_START || status == TW_REP_START))
        return RET_TWI_START_ERR;

    status = twi_sla_r(slave_address);

    if (status != RET_SUCCESS) {
        serial_debug(
            SERIAL_SRC_TWI,
            "Could not start master receiver. Error code: %d TWSTATUS: %02x",
            status, TW_STATUS);
    }

    ASSERT_SUCCESS(status);

    return RET_SUCCESS;
}

return_status_t twi_write(uint8_t *data, uint8_t length) {
    return_status_t status;
    for (uint8_t i = 0; i < length; i++) {
        status = twi_write_byte(data[i]);
        if (status != RET_SUCCESS) return status;
    }
    return RET_SUCCESS;
}

return_status_t twi_read(uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length - 1; i++) {
        twi_read_ack(data + i);
    }
    twi_read_nack(data + length - 1);
    return RET_SUCCESS;
}

void twi_stop() { TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); }

return_status_t twi_write_byte(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    loop_until_bit_is_set(TWCR, TWINT);
    if (!((TW_STATUS == TW_MT_DATA_ACK) || (TW_STATUS == TW_MT_SLA_ACK) ||
          (TW_STATUS == TW_MR_SLA_ACK))) {
        return RET_TWI_NO_ACK;
    }
    return RET_SUCCESS;
}

return_status_t twi_sla_w(uint8_t slave_address) {
    uint8_t byte = (uint8_t)(slave_address << 1) | TW_WRITE;
    return twi_write_byte(byte);
}

return_status_t twi_sla_r(uint8_t slave_address) {
    uint8_t byte = (uint8_t)(slave_address << 1) | TW_READ;
    return twi_write_byte(byte);
}

return_status_t twi_read_ack(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    loop_until_bit_is_set(TWCR, TWINT);
    *data = TWDR;
    return RET_SUCCESS;
}

return_status_t twi_read_nack(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    loop_until_bit_is_set(TWCR, TWINT);
    *data = TWDR;
    return RET_SUCCESS;
}
