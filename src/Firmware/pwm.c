#include "pwm.h"

#include <stdint.h>

#include "common.h"
#include "twi.h"
#include "serial.h"

#define TWI_ADDRESS 0x40

#define PWM_0_REG 0x06
#define REG_PER_PWM 0x04

#define MODE1_REG 0x00
#define MODE1_AI 0x05
#define MODE1_ALLCALL 0x00

#define EN_PORT PORTE
#define EN_PIN PE2

void pwm_init() {
    DDR_REGISTER(EN_PORT) |= (1 << EN_PIN);
    EN_PORT &= ~(1 << EN_PIN);
    twi_init();
    twi_start_write(TWI_ADDRESS);
    twi_write_byte(MODE1_REG);
    twi_write_byte((1 << MODE1_AI) | (1 << MODE1_ALLCALL));
    twi_stop();
}

void pwm_set(uint8_t index, uint16_t pwm) {
    uint8_t reg = PWM_0_REG + REG_PER_PWM * index;
    // skip on regs
    reg += 2;
    if (twi_start_write(TWI_ADDRESS)) {
        serial_error(SERIAL_SRC_GENERAL,
                     "Could not communicate with slave on address %x",
                     TWI_ADDRESS);
    }
    twi_write_byte(reg);
    twi_write_byte(LOWER_BYTE(pwm));
    twi_write_byte(UPPER_BYTE(pwm));
    twi_stop();
}

void pwm_get(uint8_t index, uint16_t *pwm) {
    uint8_t buffer[sizeof(*pwm)];
    uint8_t reg = PWM_0_REG + REG_PER_PWM * index;
    reg += 2;
    twi_start_write(TWI_ADDRESS);
    twi_write_byte(reg);
    twi_start_read(TWI_ADDRESS);
    twi_read(buffer, sizeof(*pwm));
    twi_stop();
    *pwm = (uint16_t)buffer[0] | (buffer[1] << 8);
}