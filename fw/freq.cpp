#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <math.h>

#include "freq.h"

// Common gate timing (250 ticks per ms)
#if F_CPU == 16000000UL
  #define FREQ_TIMER2_TCCR2B _BV(CS22)
#elif F_CPU == 8000000UL
  #define FREQ_TIMER2_TCCR2B (_BV(CS21) | _BV(CS20))
#else
  #error "Frequency counter requires an 8 MHz or 16 MHz CPU clock."
#endif

#define FREQ_TIMER2_TICKS_PER_MS 250U
#define FREQ_TIMER2_TICKS_PER_S 250000U

static uint16_t freq_gate_ms = 0U;
static uint16_t freq_gate_index = 0U;
static volatile uint16_t freq_overflows = 0U;

static uint32_t freq_last_count = 0U;
static uint8_t freq_last_phase = 0U;
static bool freq_has_history = false;

static volatile uint32_t freq_sample_count = 0U;
static volatile uint32_t freq_sample_ticks = 0U;
static volatile bool freq_is_ready = false;

bool freq_init(uint16_t gate_ms)
{
  freq_stop();
  freq_gate_ms = gate_ms;
  pinMode(5, INPUT);

  return (freq_gate_ms > 0U);
}

void freq_start(void)
{
  freq_stop();

  if (freq_gate_ms == 0U)
  {
    return;
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    TCCR1A = 0U;
    TCNT1 = 0U;
    TIFR1 = _BV(TOV1);

    ASSR = 0U;
    TCCR2A = _BV(WGM21);
    TCCR2B = 0U;
    TCNT2 = 0U;
    OCR2A = 249U;
    TIFR2 = _BV(OCF2A) | _BV(OCF2B) | _BV(TOV2);
    GTCCR = _BV(PSRASY);

    freq_overflows = 0U;
    freq_gate_index = freq_gate_ms - 1U;
    freq_has_history = false;

    TIMSK1 = _BV(TOIE1);
    TIMSK2 = _BV(OCIE2A);

    // T1 counts rising edges on D5.
    TCCR1B = _BV(CS12) | _BV(CS11) | _BV(CS10);
    TCCR2B = FREQ_TIMER2_TCCR2B;
  }
}

void freq_stop(void)
{
  TIMSK2 = 0U;
  TCCR2B = 0U;
  TIMSK1 = 0U;
  TCCR1B = 0U;
  freq_is_ready = false;
}

bool freq_ready(void)
{
  return freq_is_ready;
}

double freq_read(void)
{
  uint32_t count = 0U;
  uint32_t ticks = 0U;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if (freq_is_ready)
    {
      count = freq_sample_count;
      ticks = freq_sample_ticks;
      freq_is_ready = false;
    }
  }

  if (ticks == 0U)
  {
    return NAN;
  }

  return (double)count * ((double)FREQ_TIMER2_TICKS_PER_S / ticks);
}

ISR(TIMER1_OVF_vect)
{
  freq_overflows++;
}

ISR(TIMER2_COMPA_vect)
{
  if (++freq_gate_index < freq_gate_ms)
  {
    return;
  }

  freq_gate_index = 0U;

  uint16_t overflows = freq_overflows;
  uint16_t counter = TCNT1;

  if (TIFR1 & _BV(TOV1))
  {
    counter = TCNT1;
    overflows++;
  }

  const uint8_t phase = TCNT2;
  const uint32_t count = ((uint32_t)overflows << 16) | counter;

  if (freq_has_history)
  {
    freq_sample_count = count - freq_last_count;
    freq_sample_ticks = (uint32_t)freq_gate_ms * FREQ_TIMER2_TICKS_PER_MS +
                        phase - freq_last_phase;
    freq_is_ready = true;
  }

  freq_last_count = count;
  freq_last_phase = phase;
  freq_has_history = true;
}
