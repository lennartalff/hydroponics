#ifndef PH
#define PH

#include "common.h"

void ph_init();
return_status_t ph_read_ph(uint32_t *value);
return_status_t ph_calibration_mid();
return_status_t ph_calibration_low();
return_status_t ph_calibration_high();
return_status_t ph_calibration_clear();
return_status_t ph_calibration_format(uint8_t *n_strings, uint8_t *n_bytes);
return_status_t ph_calibration_export(uint8_t *calib_data);
return_status_t ph_temperature_compensation(float temperature);
#endif /* PH */
