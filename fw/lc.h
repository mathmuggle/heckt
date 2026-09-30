#ifndef _HECKT_LC_H_
#define _HECKT_LC_H_

void lc_init(void);

// Measurements control
void lc_meas_start(void);
void lc_meas_stop(void);
bool lc_update(void);

// DUT measurements
double lc_get_c(void);
double lc_get_l(void);
double lc_get_f(void);

// Effective tank parameters
double lc_get_ec(void);
double lc_get_el(void);
double lc_get_ef(void);

// Hardware and calibration
void lc_relay_on(void);
void lc_relay_off(void);
bool lc_dut_iscap(void);
bool lc_calibrate(void);
bool lc_is_calibrated(void);
void lc_calibration_reset(void);

#endif
