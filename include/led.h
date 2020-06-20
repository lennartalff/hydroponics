#ifndef LED
#define LED

#include <avr/io.h>
#include <stdint.h>

typedef enum { LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7 } led_t;

void led_init();
void led_set_state_all(uint8_t state);
void led_toggle_all();
void led_on(led_t led);
void led_off(led_t led);
void led_toggle(led_t led);
void led_boot_sequence();

#define LED_PORT PORTK
#endif /* LED */
