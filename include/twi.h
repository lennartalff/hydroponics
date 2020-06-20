#ifndef TWI_H_
#define TWI_H_

#include "common.h"

void twi_init();
return_status_t twi_start_write(uint8_t slave_address);
return_status_t twi_start_read(uint8_t slave_address);
return_status_t twi_write(uint8_t *data, uint8_t length);
return_status_t twi_write_byte(uint8_t data);
return_status_t twi_read(uint8_t *data, uint8_t length);
return_status_t twi_read_ack(uint8_t *data);
return_status_t twi_read_nack(uint8_t *data);
void twi_stop();

#endif /* TWI_H_ */
