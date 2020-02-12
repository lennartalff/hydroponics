#ifndef UART_H
#define UART_H

#include <stdio.h>

#define NO_OF_UARTS 4

uint8_t uart_init(uint8_t uart_id, uint32_t baud);

void uart_0_set_receive_callback(void *receive_callback);
void uart_1_set_receive_callback(void *receive_callback);
void uart_2_set_receive_callback(void *receive_callback);
void uart_3_set_receive_callback(void *receive_callback);

void uart_0_putc(char c);
void uart_1_putc(char c);
void uart_2_putc(char c);
void uart_3_putc(char c);

char uart_0_getc_timeout(uint16_t min_timeout_ms, uint8_t *success);
char uart_1_getc_timeout(uint16_t min_timeout_ms, uint8_t *success);
char uart_2_getc_timeout(uint16_t min_timeout_ms, uint8_t *success);
char uart_3_getc_timeout(uint16_t min_timeout_ms, uint8_t *success);

char uart_0_getc();
char uart_1_getc();
char uart_2_getc();
char uart_3_getc();

void uart_0_puts(char *string);
void uart_1_puts(char *string);
void uart_2_puts(char *string);
void uart_3_puts(char *string);

#endif
