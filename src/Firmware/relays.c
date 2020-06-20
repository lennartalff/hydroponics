#include "relays.h"

#include <avr/io.h>
#include <util/delay.h>

#include "common.h"

#define RELAYS_PORT PORTA

#define LED_RED_PIN PA0
#define LED_BLUE_PIN PA1
#define LED_WHITE_PIN PA2

void relays_init() {
    DDR_REGISTER(RELAYS_PORT) = 0xFF;

    relays_all_off();
}

void relays_all_on() { RELAYS_PORT = 0x00; }

void relays_all_off() { RELAYS_PORT = 0xFF; }

void relays_on(RELAYS_COLOR_t color) {
    switch (color) {
        case RELAYS_RED:
            RELAYS_PORT &= ~(1 << LED_RED_PIN);
            break;
        case RELAYS_BLUE:
            RELAYS_PORT &= ~(1 << LED_BLUE_PIN);
            break;
        case RELAYS_WHITE:
            RELAYS_PORT &= ~(1 << LED_WHITE_PIN);
            break;
    }
    _delay_ms(100);
}

void relays_off(RELAYS_COLOR_t color) {
    switch (color) {
        case RELAYS_RED:
            RELAYS_PORT |= (1 << LED_RED_PIN);
            break;
        case RELAYS_BLUE:
            RELAYS_PORT |= (1 << LED_BLUE_PIN);
            break;
        case RELAYS_WHITE:
            RELAYS_PORT |= (1 << LED_WHITE_PIN);
            break;
    }
}

void relays_set(RELAYS_COLOR_t color, uint8_t state) {
    if (state) {
        relays_on(color);
    } else {
        relays_off(color);
    }
}
