#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>

static void (*uart_0_receive_callback)(char) = NULL;
static void (*uart_1_receive_callback)(char) = NULL;
static void (*uart_2_receive_callback)(char) = NULL;
static void (*uart_3_receive_callback)(char) = NULL;

uint8_t uart_init(uint8_t uart_id, uint32_t baud) {
    // 1. set baud rate
    // 2. enable transmitter and receiver
    // 3. set 8bit n1 mode
    uint16_t baud_register = F_CPU / (16 * baud) - 1;
    switch (uart_id) {
        case 0:
            UBRR0 = baud_register;
            UCSR0B = (1 << RXEN0) | (1 << TXEN0);
            UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
            break;
        case 1:
            UBRR1 = baud_register;
            UCSR1B = (1 << RXEN1) | (1 << TXEN1);
            UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
            break;
        case 2:
            UBRR2 = baud_register;
            UCSR2B = (1 << RXEN2) | (1 << TXEN2);
            UCSR2C = (1 << UCSZ21) | (1 << UCSZ20);
            break;
        case 3:
            UBRR3 = baud_register;
            UCSR3B = (1 << RXEN3) | (1 << TXEN3);
            UCSR3C = (1 << UCSZ31) | (1 << UCSZ30);
            break;
        default:
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void uart_0_set_receive_callback(void (*receive_callback)(char)) {
    // enable receive interrupts
    UCSR0B |= (1 << RXCIE0);
    uart_0_receive_callback = receive_callback;
}

void uart_1_set_receive_callback(void (*receive_callback)(char)) {
    // enable receive interrupts
    UCSR1B |= (1 << RXCIE1);
    uart_1_receive_callback = receive_callback;
}

void uart_2_set_receive_callback(void (*receive_callback)(char)) {
    // enable receive interrupts
    UCSR2B |= (1 << RXCIE2);
    uart_2_receive_callback = receive_callback;
}

void uart_3_set_receive_callback(void (*receive_callback)(char)) {
    // enable receive interrupts
    UCSR3B |= (1 << RXCIE3);
    uart_3_receive_callback = receive_callback;
}

void uart_0_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = c;
}

void uart_1_putc(char c) {
    while (!(UCSR1A & (1 << UDRE1)))
        ;
    UDR1 = c;
}

void uart_2_putc(char c) {
    while (!(UCSR2A & (1 << UDRE2)))
        ;
    UDR2 = c;
}

void uart_3_putc(char c) {
    while (!(UCSR3A & (1 << UDRE3)))
        ;
    UDR3 = c;
}

char uart_0_getc() {
    // wait for data
    while (!(UCSR0A & (1 << RXC0)))
        ;
    return UDR0;
}

char uart_1_getc() {
    // wait for data
    while (!(UCSR1A & (1 << RXC1)))
        ;
    return UDR1;
}

char uart_2_getc() {
    // wait for data
    while (!(UCSR2A & (1 << RXC2)))
        ;
    return UDR2;
}

char uart_3_getc() {
    // wait for data
    while (!(UCSR3A & (1 << RXC3)))
        ;
    return UDR3;
}

char uart_0_getc_timeout(uint16_t timeout, uint8_t *success) {
    for (uint16_t i = 0; i < timeout; i++) {
        if (UCSR0A & (1 << RXC0)) {
            *success = true;
            return UDR0;
        }
        _delay_ms(1);
    }
    *success = false;
    return 0;
}

char uart_1_getc_timeout(uint16_t timeout, uint8_t *success) {
    for (uint16_t i = 0; i < timeout; i++) {
        if (UCSR1A & (1 << RXC1)) {
            *success = true;
            return UDR1;
        }
        _delay_ms(1);
    }
    *success = false;
    return 0;
}

char uart_2_getc_timeout(uint16_t timeout, uint8_t *success) {
    for (uint16_t i = 0; i < timeout; i++) {
        if (UCSR2A & (1 << RXC2)) {
            *success = true;
            return UDR2;
        }
        _delay_ms(1);
    }
    *success = false;
    return 0;
}

char uart_3_getc_timeout(uint16_t timeout, uint8_t *success) {
    for (uint16_t i = 0; i < timeout; i++) {
        if (UCSR3A & (1 << RXC3)) {
            *success = true;
            return UDR3;
        }
        _delay_ms(1);
    }
    *success = false;
    return 0;
}

void uart_0_puts(char *string) {
    uint8_t i = 0;
    while (string[i] != '\0' && i < 0xFF) {
        uart_0_putc(string[i++]);
    }
}

void uart_1_puts(char *string) {
    uint8_t i = 0;
    while (string[i] != '\0' && i < 0xFF) {
        uart_1_putc(string[i++]);
    }
}

void uart_2_puts(char *string) {
    uint8_t i = 0;
    while (string[i] != '\0' && i < 0xFF) {
        uart_2_putc(string[i++]);
    }
}

void uart_3_puts(char *string) {
    uint8_t i = 0;
    while (string[i] != '\0' && i < 0xFF) {
        uart_3_putc(string[i++]);
    }
}

ISR(USART0_RX_vect) {
    char data = UDR0;
    if (uart_0_receive_callback != NULL) {
        uart_0_receive_callback(data);
    }
}

ISR(USART1_RX_vect) {
    char data = UDR1;
    if (uart_1_receive_callback != NULL) {
        uart_1_receive_callback(data);
    }
}

ISR(USART2_RX_vect) {
    char data = UDR2;
    if (uart_2_receive_callback != NULL) {
        uart_2_receive_callback(data);
    }
}

ISR(USART3_RX_vect) {
    char data = UDR3;
    if (uart_3_receive_callback != NULL) {
        uart_3_receive_callback(data);
    }
}
