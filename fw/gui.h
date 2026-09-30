#ifndef _HECKT_GUI_H_
#define _HECKT_GUI_H_

#include <inttypes.h>

typedef enum
{
  GUI_TITLE,
  GUI_CALIBRATION_INFO,
  GUI_CALIBRATION_RESULT,
  GUI_CALIBRATION_LOADING,
  GUI_INDUCTANCE,
  GUI_CAPACITANCE,
  GUI_COUNT
} gui_t;

typedef enum
{
  LC_ERROR_NONE,
  LC_ERROR_CALIBRATION,
  LC_ERROR_COUNT
} lc_error_t;

bool gui_init(void);
bool gui_is_pressed(void);
void gui_render_page(gui_t page);
void gui_render_page(lc_error_t error);
void gui_render_debug(uint32_t ms);

#endif
