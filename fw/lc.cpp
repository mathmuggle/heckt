#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>

#include "lc.h"
#include "freq.h"
#include "config.h"

static double eff_f = 0.0; // Tank resonant frequency [Hz]: relay OFF, no DUT
static double eff_c = 0.0; // Calibrated effective tank capacitance [F]
static double eff_i = 0.0; // Calibrated effective tank inductance [H]
static double off_c = 0.0; // Open-lead capacitance offset [F]
static double off_i = 0.0; // Shorted-lead inductance offset [H]

static bool is_calib = false; // True for successful calibration
static double freqency = NAN; // Latest measured frequency; NAN if unavailable
static bool freq_reject = true; // Discard 1st reading after measurement start

static double lc_wait_freq_ready(uint16_t gate_ms, bool is_cap);
static double lc_sample_frequency(bool relay_on);
static double lc_get_raw(double frequency, double eff_val);

void lc_init(void)
{
  digitalWrite(HECKT_PIN_RELAY, LOW);
  pinMode(HECKT_PIN_RELAY, OUTPUT);
  pinMode(HECKT_PIN_DUT, INPUT_PULLUP);
  freq_init(HECKT_TIME_FREQ_GATE_MS);
  lc_meas_stop();
  is_calib = false;

#ifdef HECKT_FRESH_START
  EEPROM.update(HECKT_EEPROM_ADDR_CALIB_FLAG, HECKT_EEPROM_CALIB_FLAG_INVALID);
  EEPROM.update(HECKT_EEPROM_ADDR_OFF_C_FLAG, HECKT_EEPROM_OFFSET_FLAG_INVALID);
  EEPROM.update(HECKT_EEPROM_ADDR_OFF_I_FLAG, HECKT_EEPROM_OFFSET_FLAG_INVALID);
#endif

  if (EEPROM.read(HECKT_EEPROM_ADDR_CALIB_FLAG) == HECKT_EEPROM_CALIB_FLAG_VALID)
  {
    EEPROM.get(HECKT_EEPROM_ADDR_EFF_F, eff_f);
    EEPROM.get(HECKT_EEPROM_ADDR_EFF_C, eff_c);
    EEPROM.get(HECKT_EEPROM_ADDR_EFF_L, eff_i);

    is_calib = isfinite(eff_f) && eff_f > 0.0 &&
               isfinite(eff_c) && eff_c > 0.0 &&
               isfinite(eff_i) && eff_i > 0.0;
  }

  if (is_calib)
  {
    if (EEPROM.read(HECKT_EEPROM_ADDR_OFF_C_FLAG) == HECKT_EEPROM_OFFSET_FLAG_VALID)
    {
      EEPROM.get(HECKT_EEPROM_ADDR_OFF_C, off_c);
      if (!isfinite(off_c))
      {
        off_c = 0.0;
      }
    }

    if (EEPROM.read(HECKT_EEPROM_ADDR_OFF_I_FLAG) == HECKT_EEPROM_OFFSET_FLAG_VALID)
    {
      EEPROM.get(HECKT_EEPROM_ADDR_OFF_I, off_i);
      if (!isfinite(off_i))
      {
        off_i = 0.0;
      }
    }
  }
  else
  {
    eff_f = 0.0;
    eff_c = 0.0;
    eff_i = 0.0;
  }
}

bool lc_is_calibrated(void)
{
  return is_calib;
}

void lc_meas_start(void)
{
  lc_meas_stop();
  freq_start();
}

void lc_meas_stop(void)
{
  freq_stop();
  freqency = NAN;
  freq_reject = true;
}

bool lc_update(void)
{
  if (!freq_ready())
  {
    return false;
  }

  const double f = freq_read();

  if (freq_reject)
  {
    freq_reject = false;
    return false;
  }

  if (!isfinite(f) || f <= 0.0)
  {
    freqency = NAN;
    return true;
  }

  freqency = f;

  return true;
}

bool lc_zero(bool is_cap)
{
  if (!is_calib || lc_dut_iscap() != is_cap)
  {
    return false;
  }

  freq_start();
  const double freq = lc_wait_freq_ready(HECKT_TIME_FREQ_GATE_MS, is_cap);
  freq_stop();

  const double offset = lc_get_raw(freq, is_cap ? eff_c : eff_i);

  if (!isfinite(offset))
  {
    return false;
  }

  const int flag_addr = is_cap ? HECKT_EEPROM_ADDR_OFF_C_FLAG : HECKT_EEPROM_ADDR_OFF_I_FLAG;
  const int offset_addr = is_cap ? HECKT_EEPROM_ADDR_OFF_C : HECKT_EEPROM_ADDR_OFF_I;
  EEPROM.update(flag_addr, HECKT_EEPROM_OFFSET_FLAG_INVALID);
  EEPROM.put(offset_addr, offset);
  EEPROM.update(flag_addr, HECKT_EEPROM_OFFSET_FLAG_VALID);
  (is_cap ? off_c : off_i) = offset;

  return true;
}

double lc_get_c(void)
{
  return lc_get_raw(freqency, eff_c) - off_c;
}

double lc_get_l(void)
{
  return lc_get_raw(freqency, eff_i) - off_i;
}

static double lc_get_raw(double freq, double eff_val)
{
  if (!is_calib || !isfinite(freq) || freq <= 0.0)
  {
    return NAN;
  }

  const double freq_ratio = eff_f / freq;

  return eff_val * (freq_ratio * freq_ratio - 1.0);
}

double lc_get_f(void)
{
  return freqency;
}

double lc_get_ec(void)
{
  return eff_c;
}

double lc_get_ef(void)
{
  return eff_f;
}

double lc_get_el(void)
{
  return eff_i;
}

void lc_relay_on(void)
{
  digitalWrite(HECKT_PIN_RELAY, HIGH);
}

void lc_relay_off(void)
{
  digitalWrite(HECKT_PIN_RELAY, LOW);
}

bool lc_dut_iscap(void)
{
  return (digitalRead(HECKT_PIN_DUT) == HECKT_DUT_CAPACITANCE_LEVEL);
}

bool lc_calibrate(void)
{
  lc_meas_stop();
  lc_relay_off();

  if (digitalRead(HECKT_PIN_DUT) != HECKT_DUT_CAPACITANCE_LEVEL)
  {
    return false;
  }

  const double freq_off = lc_sample_frequency(false);
  double freq_on = NAN;

  if (isfinite(freq_off) && freq_off > 0.0)
  {
    freq_on = lc_sample_frequency(true);
  }

  lc_relay_off();
  delay(HECKT_TIME_RELAY_SETTLE_MS);

  if (!isfinite(freq_off) || !isfinite(freq_on) ||
      freq_off <= 0.0 || freq_on <= 0.0 ||
      freq_on >= freq_off ||
      digitalRead(HECKT_PIN_DUT) != HECKT_DUT_CAPACITANCE_LEVEL)
  {
    return false;
  }

  const double freq_ratio = freq_off / freq_on;
  const double freq_ratio_sq = freq_ratio * freq_ratio;

  const double new_eff_c = HECKT_CAP_CALIBRATION_FARAD / (freq_ratio_sq - 1.0);
  const double new_eff_i = 1.0 / (4.0 * M_PI * M_PI * freq_off * freq_off * new_eff_c);

  if (!isfinite(new_eff_c) || !isfinite(new_eff_i) ||
      new_eff_c <= 0.0 || new_eff_i <= 0.0)
  {
    return false;
  }

  eff_f = freq_off;
  eff_c = new_eff_c;
  eff_i = new_eff_i;

  EEPROM.update(HECKT_EEPROM_ADDR_CALIB_FLAG, HECKT_EEPROM_CALIB_FLAG_INVALID);
  EEPROM.put(HECKT_EEPROM_ADDR_EFF_F, eff_f);
  EEPROM.put(HECKT_EEPROM_ADDR_EFF_C, eff_c);
  EEPROM.put(HECKT_EEPROM_ADDR_EFF_L, eff_i);
  EEPROM.update(HECKT_EEPROM_ADDR_OFF_C_FLAG, HECKT_EEPROM_OFFSET_FLAG_INVALID);
  EEPROM.update(HECKT_EEPROM_ADDR_OFF_I_FLAG, HECKT_EEPROM_OFFSET_FLAG_INVALID);
  EEPROM.update(HECKT_EEPROM_ADDR_CALIB_FLAG, HECKT_EEPROM_CALIB_FLAG_VALID);

  off_c = 0.0;
  off_i = 0.0;
  is_calib = true;

  return true;
}

static double lc_wait_freq_ready(uint16_t gate_ms, bool is_cap)
{
  const uint32_t start_ms = millis();

  while (!freq_ready())
  {
    if (lc_dut_iscap() != is_cap ||
        millis() - start_ms > (uint32_t)gate_ms + 500U)
    {
      return NAN;
    }
  }

  if (lc_dut_iscap() != is_cap)
  {
    return NAN;
  }

  return freq_read();
}

static double lc_sample_frequency(bool relay_on)
{
  freq_stop();

  if (relay_on)
  {
    lc_relay_on();
  }
  else
  {
    lc_relay_off();
  }

  delay(HECKT_TIME_RELAY_SETTLE_MS);
  freq_start();

  // Discard the first gate after switching, as in the tested sketch.
  if (!isfinite(lc_wait_freq_ready(HECKT_TIME_FREQ_GATE_MS, true)))
  {
    freq_stop();
    return NAN;
  }

  freq_init(HECKT_TIME_RELAY_SAMPLE_MS);
  freq_start();
  const double freq = lc_wait_freq_ready(HECKT_TIME_RELAY_SAMPLE_MS, true);
  freq_init(HECKT_TIME_FREQ_GATE_MS);

  return freq;
}
