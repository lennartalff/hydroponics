#include "led.h"

#include <util/delay.h>

#include "common.h"

void led_init() {
    // set LED port as output
    DDR_REGISTER(LED_PORT) = 0xFF;
}

void led_set_state_all(uint8_t state) { LED_PORT = state; }

void led_toggle_all() { LED_PORT ^= 0xFF; }

void led_on(led_t led) { LED_PORT |= (1 << led); }

void led_off(led_t led) { LED_PORT &= ~(1 << led); }

void led_toggle(led_t led) { LED_PORT ^= (1 << led); }

void led_boot_sequence() {
    led_set_state_all(0x00);
    for (uint8_t i = 0; i < 8; i++) {
        led_on(i);
        _delay_ms(50);
    }
    for (uint8_t i = 0; i < 8; i++) {
        led_off(i);
        _delay_ms(50);
    }
    led_set_state_all(0xFF);
    for (uint8_t i = 0; i < 20; i++) {
        led_toggle_all();
        _delay_ms(10);
        led_toggle_all();
        _delay_ms(2);
    }
    led_set_state_all(0x00);
}