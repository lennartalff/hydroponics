#ifndef RELAYS
#define RELAYS

#include <stdint.h>

typedef enum { RELAYS_RED, RELAYS_BLUE, RELAYS_WHITE } RELAYS_COLOR_t;

void relays_init();
void relays_all_on();
void relays_all_off();
void relays_on(RELAYS_COLOR_t color);
void relays_off(RELAYS_COLOR_t color);
void relays_set(RELAYS_COLOR_t color, uint8_t state);
#endif /* RELAYS */
