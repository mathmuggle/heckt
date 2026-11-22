#include "lc.h"
#include "gui.h"
#include "config.h"

static gui_t last_page = GUI_CALIBRATION_INFO;
static gui_t current_page = GUI_CALIBRATION_INFO;

void setup()
{
  lc_init();

  if (!gui_init())
  {
    while(1){}
  }

  const uint32_t start_ms = millis();
  bool calibrate_at_startup = false;

  // Open calibration at startup
  while (digitalRead(HECKT_PIN_BUTTON) == LOW)
  {
    if (millis() - start_ms >= HECKT_TIME_CALIB_HOLD_MS)
    {
      calibrate_at_startup = true;
      break;
    }

    delay(1);
  }

  gui_render_page(GUI_TITLE);

  if (lc_is_calibrated() && !calibrate_at_startup)
  {
    current_page = lc_dut_iscap() ? GUI_CAPACITANCE : GUI_INDUCTANCE;
    lc_meas_start();
  }

  gui_render_page(current_page);

  // Startup press should not start calibration.
  while (digitalRead(HECKT_PIN_BUTTON) == LOW)
  {
    delay(1);
  }
}

void loop()
{
  const bool is_cap = lc_dut_iscap();
  const gui_button_event_t button_event = gui_poll_button();
  const gui_t measurement_page = is_cap ? GUI_CAPACITANCE : GUI_INDUCTANCE;
  const gui_t zero_page = is_cap ? GUI_ZERO_C : GUI_ZERO_L;
  const bool zero_pending = current_page == GUI_ZERO_C || current_page == GUI_ZERO_L;

  if (button_event == GUI_BUTTON_LONG_PRESS)
  {
    if (current_page != GUI_FREQUENCY)
    {
      last_page = zero_pending ? measurement_page : current_page;
      lc_relay_off();
      lc_meas_start();
      current_page = GUI_FREQUENCY;
      gui_render_page(current_page);
    }

    return;
  }

  if (zero_pending && current_page != zero_page)
  {
    current_page = measurement_page;
    lc_meas_start();
    gui_render_page(current_page);
    return;
  }

  if (button_event == GUI_BUTTON_SHORT_PRESS)
  {
    switch (current_page)
    {
      case GUI_FREQUENCY:
      {
        lc_meas_stop();
        current_page = last_page;

        if (current_page == GUI_CAPACITANCE || current_page == GUI_INDUCTANCE)
        {
          lc_meas_start();
        }

        gui_render_page(current_page);
        break;
      }

      case GUI_CALIBRATION_INFO:
      {
        gui_render_page(GUI_CALIBRATION_LOADING);

        if (lc_calibrate())
        {
          current_page = GUI_CALIBRATION_RESULT;
          gui_render_page(current_page);
        }
        else
        {
          gui_render_page(LC_ERROR_CALIBRATION);
        }

        break;
      }

      case GUI_CALIBRATION_RESULT:
      {
        current_page = measurement_page;
        lc_meas_start();
        gui_render_page(current_page);
        break;
      }

      case GUI_CAPACITANCE:
      case GUI_INDUCTANCE:
      {
        lc_meas_stop();
        current_page = zero_page;
        gui_render_page(current_page);
        break;
      }

      case GUI_ZERO_C:
      case GUI_ZERO_L:
      {
        lc_zero(is_cap);

        current_page = lc_dut_iscap() ? GUI_CAPACITANCE : GUI_INDUCTANCE;
        lc_meas_start();
        gui_render_page(current_page);
        break;
      }

      default: break;
    }

    return;
  }

  if (current_page == GUI_FREQUENCY)
  {
    if (lc_update())
    {
      gui_render_page(current_page);
    }

    return;
  }

  if (current_page != GUI_CAPACITANCE && current_page != GUI_INDUCTANCE)
  {
    return;
  }

  if (current_page != measurement_page)
  {
    lc_meas_start();
    current_page = measurement_page;
    gui_render_page(current_page);
  }

  if (lc_update())
  {
    gui_render_page(current_page);
  }
}
