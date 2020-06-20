#ifndef PWM
#define PWM

#include <stdint.h>

void pwm_init();
void pwm_set(uint8_t index, uint16_t pwm);
void pwm_get(uint8_t index, uint16_t *pwm);
#endif /* PWM */
