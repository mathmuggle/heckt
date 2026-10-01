#include "lc.h"
#include "gui.h"
#include "config.h"

static gui_t current_page = GUI_CALIBRATION_INFO;

void setup()
{
  lc_init();

  if (!gui_init())
  {
    while(1){}
  }

  const uint32_t start_ms = millis();

  // Forget calibration at startup
  while (digitalRead(HECKT_PIN_BUTTON) == LOW)
  {
    if (millis() - start_ms >= HECKT_TIME_CALIB_RESET_HOLD_MS)
    {
      lc_calibration_reset();
      break;
    }

    delay(1);
  }

  gui_render_page(GUI_TITLE);

  // Startup press should not start calibration.
  while (digitalRead(HECKT_PIN_BUTTON) == LOW)
  {
    delay(1);
  }

  if (lc_is_calibrated())
  {
    current_page = lc_dut_iscap() ? GUI_CAPACITANCE : GUI_INDUCTANCE;
    lc_meas_start();
  }

  gui_render_page(current_page);
}

void loop()
{
  const bool capacitance_mode = lc_dut_iscap();

  if (gui_is_pressed())
  {
    switch (current_page)
    {
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
        current_page = capacitance_mode ? GUI_CAPACITANCE : GUI_INDUCTANCE;
        lc_meas_start();
        gui_render_page(current_page);
        break;
      }

      default:
      {
        lc_meas_stop();
        current_page = GUI_CALIBRATION_INFO;
        gui_render_page(current_page);
        break;
      }
    }

    return;
  }

  if (current_page != GUI_CAPACITANCE && current_page != GUI_INDUCTANCE)
  {
    return;
  }

  const gui_t measurement_page =
    capacitance_mode ? GUI_CAPACITANCE : GUI_INDUCTANCE;

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
