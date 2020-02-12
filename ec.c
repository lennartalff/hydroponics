#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "common.h"
#include "ec.h"
#include "uart.h"

#define EC_RECV_BUFFER_SIZE 48
#define EC_UART_NO 3
#define PASTER(x,y,z) uart_##y##_##x(z)
#define EVALUATOR(x, y, z) PASTER(x, y, z)
#define UART_PUTC(x) EVALUATOR(putc, EC_UART_NO, x)
#define UART_PUTS(x) EVALUATOR(puts, EC_UART_NO, x)
#define UART_SET_CB(x) EVALUATOR(set_receive_callback, EC_UART_NO, x)
#define EC_BAUD 9600UL

typedef struct {
    uint8_t data[EC_RECV_BUFFER_SIZE];
    uint8_t index;
    uint8_t receiving;
} ec_buffer_t;

ec_buffer_t ec_receive_buffer;

void ec_receive_callback(char data)
{
    if (!ec_receive_buffer.receiving) {
        ec_receive_buffer.receiving = 1;
    }

    ec_receive_buffer.data[ec_receive_buffer.index++] = data;

    if (data == '\r') {
        ec_receive_buffer.data[ec_receive_buffer.index] = '\0';
        ec_receive_buffer.receiving = 0;
        ec_receive_buffer.index = 0;
    }
}

void ec_init(uint32_t baud)
{
    uart_init(EC_UART_NO, EC_BAUD);

    ec_receive_buffer.index = 0;
    ec_receive_buffer.receiving = 0;

    UART_SET_CB(ec_receive_callback);
}

void ec_write_line(char *string)
{
    UART_PUTS(string);
    UART_PUTC('\r');
}
