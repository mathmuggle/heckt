#include <Arduino.h>
#include <math.h>

#include "lc.h"
#include "freq.h"
#include "version.h"

static double eff_f = 0.0; // Tank resonant frequency [Hz]: relay OFF, no DUT
static double eff_c = 0.0; // Calibrated effective tank capacitance [F]
static double eff_i = 0.0; // Calibrated effective tank inductance [H]

static bool is_calib = false; // True for successful calibration
static double freqency = NAN; // Latest measured frequency; NAN if unavailable
static bool freq_reject = true; // Discard 1st reading after measurement start

static double lc_wait_freq_ready(uint16_t gate_ms);
static double lc_sample_frequency(bool relay_on);

void lc_init(void)
{
  digitalWrite(HECKT_PIN_RELAY, LOW);
  pinMode(HECKT_PIN_RELAY, OUTPUT);
  pinMode(HECKT_PIN_DUT, INPUT_PULLUP);
  freq_init(HECKT_TIME_FREQ_GATE_MS);
}

void lc_meas_start(void)
{
  lc_meas_stop();

  if (is_calib)
  {
    freq_start();
  }
}

void lc_meas_stop(void)
{
  freq_stop();
  freqency = NAN;
  freq_reject = true;
}

bool lc_update(void)
{
  if (!is_calib || !freq_ready())
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

double lc_get_c(void)
{
  if (!is_calib || !isfinite(freqency) || freqency <= 0.0)
  {
    return NAN;
  }

  const double freq_ratio = eff_f / freqency;

  return eff_c * (freq_ratio * freq_ratio - 1.0);
}

double lc_get_l(void)
{
  if (!is_calib || !isfinite(freqency) || freqency <= 0.0)
  {
    return NAN;
  }

  const double freq_ratio = eff_f / freqency;

  return eff_i * (freq_ratio * freq_ratio - 1.0);
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
  is_calib = false;
  eff_f = 0.0;
  eff_c = 0.0;
  eff_i = 0.0;
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

  eff_c = HECKT_CAP_CALIBRATION_FARAD / (freq_ratio_sq - 1.0);
  eff_i = 1.0 / (4.0 * M_PI * M_PI * freq_off * freq_off * eff_c);

  if (!isfinite(eff_c) || !isfinite(eff_i) || eff_c <= 0.0 || eff_i <= 0.0)
  {
    return false;
  }

  eff_f = freq_off;
  is_calib = true;

  return true;
}

static double lc_wait_freq_ready(uint16_t gate_ms)
{
  const uint32_t start_ms = millis();

  while (!freq_ready())
  {
    if (digitalRead(HECKT_PIN_DUT) != HECKT_DUT_CAPACITANCE_LEVEL ||
        millis() - start_ms > (uint32_t)gate_ms + 500U)
    {
      return NAN;
    }
  }

  if (digitalRead(HECKT_PIN_DUT) != HECKT_DUT_CAPACITANCE_LEVEL)
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
  if (!isfinite(lc_wait_freq_ready(HECKT_TIME_FREQ_GATE_MS)))
  {
    freq_stop();
    return NAN;
  }

  freq_init(HECKT_TIME_RELAY_SAMPLE_MS);
  freq_start();

  const double frequency = lc_wait_freq_ready(HECKT_TIME_RELAY_SAMPLE_MS);

  freq_init(HECKT_TIME_FREQ_GATE_MS);

  return frequency;
}
