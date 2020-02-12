#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

#define LED_PORT PORTK

int main()
{
    uart_init(0, 57600);
    DDRK = 0xFF;
    
    while(1)
    {
        _delay_ms(1000);
        LED_PORT ^= 0xFF;
    }
    
}
