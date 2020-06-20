#ifndef EC
#define EC

#include <stdint.h>
#include "return.h"

#define EC_RX_BUFF_SIZE 128
#define EC_RECV_BUFFER_SIZE 48
#define EC_CALIB_STRINGS 10
#define EC_CALIB_LENGTH 14

typedef enum ec_return_e {
    EC_RET_SUCCESS,
    EC_RET_ERR_RESPONSE,
    EC_RET_READ_TIMEOUT,
    EC_RET_BUFFER_OVERFLOW,
    EC_RET_UNEXPECTED_RESPONSE
} ec_return_t;

ec_return_t ec_read_line(char *string_out);
ec_return_t ec_disable_continuous_reading();
void ec_init();
return_status_t ec_read_ec(uint32_t *value);
return_status_t ec_calibration_dry();
return_status_t ec_calibration_low();
return_status_t ec_calibration_high();
return_status_t ec_calibration_clear();
return_status_t ec_calibration_format(uint8_t *n_strings, uint8_t *n_bytes);
return_status_t ec_calibration_export(uint8_t *calib_data);
return_status_t ec_temperature_compensation(float temperature);

#endif /* EC */
