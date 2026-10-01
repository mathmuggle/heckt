#include "gui.h"
#include "lc.h"
#include "config.h"

#include <math.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1

#define OLED_TEXT_HEIGHT 8

#define SCALE_MICRO 1.0e6
#define SCALE_NANO 1.0e9
#define SCALE_PICO 1.0e12

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
static void gui_print_value(double value, double scale, const char *unit);
static void gui_print_capacitance(double capacitance);
static void gui_print_inductance(double inductance);
static void gui_print_centered(void (*printer)(double), double value);

bool gui_init(void)
{
  pinMode(HECKT_PIN_BUTTON, INPUT_PULLUP);

  return display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
}

// Detect a new button press
bool gui_is_pressed(void)
{
  static bool was_pressed = false;
  const bool pressed = (digitalRead(HECKT_PIN_BUTTON) == LOW);
  const bool new_press = pressed && !was_pressed;
  was_pressed = pressed;

  return new_press;
}

void gui_render_page(gui_t page)
{
  switch(page)
  {
    case GUI_TITLE:
    {
      display.clearDisplay();
      display.setFont();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
      display.setCursor(31, 6);
      display.print(F("HECKT-DUINO"));
      display.setCursor(40, 18);
      display.print(F("LC METER"));
      display.setCursor(6, 40);
      display.print(F(HECKT_VERSION));
      display.setCursor(6, 52);
      display.print(F(HECKT_RELEASE_DATE));
      display.display();
      delay(1500);
      break;
    }

    case GUI_CALIBRATION_INFO:
    {
      display.clearDisplay();
      display.setFont();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setTextWrap(false);
      display.drawRect(0, 0, OLED_WIDTH, OLED_TEXT_HEIGHT+7, SSD1306_WHITE);
      display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
      display.setCursor(31, 4);
      display.println(F("CALIBRATION"));
      display.setCursor(7, 21);
      display.print(F("1. KEEP LEADS OPEN"));
      display.setCursor(7, 36);
      display.print(F("2. SET SPDT TO \"C\""));
      display.setCursor(7, 50);
      display.print(F("3. PRESS TO START"));
      display.display();
      break;
    }

    case GUI_INDUCTANCE:
    {
      display.clearDisplay();
      display.setFont();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setTextWrap(false);
      display.drawRect(0, 0, OLED_WIDTH, OLED_TEXT_HEIGHT+7, SSD1306_WHITE);
      display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
      display.setCursor(34, 4);
      display.println(F("INDUCTANCE"));
      gui_print_centered(gui_print_inductance, lc_get_l());
      display.display();
      break;
    }

    case GUI_CAPACITANCE:
    {
      display.clearDisplay();
      display.setFont();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setTextWrap(false);
      display.drawRect(0, 0, OLED_WIDTH, OLED_TEXT_HEIGHT+7, SSD1306_WHITE);
      display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
      display.setCursor(31, 4);
      display.println(F("CAPACITANCE"));
      gui_print_centered(gui_print_capacitance, lc_get_c());
      display.display();
      break;
    }

    case GUI_CALIBRATION_RESULT:
    {
      display.clearDisplay();
      display.setFont();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setTextWrap(false);
      display.drawRect(0, 0, OLED_WIDTH, OLED_TEXT_HEIGHT+7, SSD1306_WHITE);
      display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
      display.setCursor(22, 4);
      display.println(F("* CALIBRATED *"));
      display.setCursor(7, 21);
      display.print(F("L-EFF : "));
      gui_print_inductance(lc_get_el());
      display.setCursor(7, 36);
      display.print(F("C-EFF : "));
      gui_print_capacitance(lc_get_ec());
      display.setCursor(7, 50);
      display.print(F("F-EFF : "));
      gui_print_value(lc_get_ef(), 1.0e-3, "kHz");
      display.display();
      break;
    }

    case GUI_CALIBRATION_LOADING:
    {
      display.clearDisplay();
      display.setFont();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setTextWrap(false);
      display.drawRect(0, 0, OLED_WIDTH, OLED_TEXT_HEIGHT+7, SSD1306_WHITE);
      display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
      display.setCursor(31, 4);
      display.print(F("CALIBRATION"));
      display.setCursor(34, 35);
      display.print(F("LOADING..."));
      display.display();
      break;
    }

    default: break;
  }
}

void gui_render_page(lc_error_t error)
{
  display.clearDisplay();
  display.setFont();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.drawRect(0, 0, OLED_WIDTH, OLED_TEXT_HEIGHT+7, SSD1306_WHITE);
  display.drawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
  display.setCursor(37, 4);
  display.println(F("* ERROR *"));

  switch(error)
  {
    case LC_ERROR_NONE:
    {
      break;
    }

    case LC_ERROR_CALIBRATION:
    {
      display.setCursor(19, 26);
      display.print(F("SET SPDT TO \"C\""));
      display.setCursor(25, 45);
      display.print(F("AND TRY AGAIN"));
      break;
    }

    default:
    {
      display.setCursor(31, 34);
      display.print(F("UNKNOWN ERROR"));
      break;
    }
  }

  display.display();
}

void gui_render_debug(uint32_t delay_ms)
{
  for (uint8_t page = 0U; page < GUI_COUNT; page++)
  {
    gui_render_page((gui_t)(page));
    delay(delay_ms);
  }

  for (uint8_t error = LC_ERROR_NONE+1U; error < LC_ERROR_COUNT; error++)
  {
    gui_render_page((lc_error_t)(error));
    delay(delay_ms);
  }
}

static void gui_print_value(double value, double scale, const char *unit)
{
  if (!isfinite(value))
  {
    display.print(F("NaN"));
    return;
  }

  display.print(value * scale, 2);
  display.print(" ");
  display.print(unit);
}

static void gui_print_capacitance(double capacitance)
{
  const double absolute_value = fabs(capacitance);

  if (absolute_value >= 1.0e-6)
  {
    gui_print_value(capacitance, SCALE_MICRO, "uF");
  }
  else if (absolute_value >= 1.0e-9)
  {
    gui_print_value(capacitance, SCALE_NANO, "nF");
  }
  else
  {
    gui_print_value(capacitance, SCALE_PICO, "pF");
  }
}

static void gui_print_inductance(double inductance)
{
  const double absolute_value = fabs(inductance);

  if (absolute_value >= 1.0)
  {
    gui_print_value(inductance, 1.0, "H");
  }
  else if (absolute_value >= 1.0e-3)
  {
    gui_print_value(inductance, 1.0e3, "mH");
  }
  else if (absolute_value >= 1.0e-6)
  {
    gui_print_value(inductance, SCALE_MICRO, "uH");
  }
  else
  {
    gui_print_value(inductance, SCALE_NANO, "nH");
  }
}

static void gui_print_centered(void (*printer)(double), double value)
{
  display.setFont();
  display.setTextWrap(false);
  display.setTextSize(2);

  display.setCursor(0, OLED_HEIGHT);
  printer(value);
  int16_t width = display.getCursorX();

  uint8_t size = 2;
  if (width > OLED_WIDTH - 2)
  {
    size = 1;
    width /= 2;
  }

  const uint8_t top = OLED_TEXT_HEIGHT + 7;
  const uint8_t height = OLED_HEIGHT - top - 1;

  display.setTextSize(size);
  display.setCursor((OLED_WIDTH - width) / 2,
                    top + (height - OLED_TEXT_HEIGHT * size) / 2);
  printer(value);
}
