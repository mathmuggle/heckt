#include "gui.h"
#include "version.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

float freq = 235.34;
float effl = 104.5;
float effc = 1.5;
float measl = 104.5;
float measc = 1.5;
float measf = 234.5;

#define OLED_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1

#define OLED_TEXT_WIDTH 6
#define OLED_TEXT_HEIGHT 8

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

bool gui_init(void)
{
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
  {
    return false;
  }

  return true;
}

void gui_debug(uint32_t delay_ms)
{
  for (uint8_t page = 0U; page < GUI_COUNT; page++)
  {
    gui_t current_page = (gui_t)(page);
    gui_render_page(current_page);
    delay(delay_ms);
  }

  for (uint8_t error = LC_ERROR_NONE+1U; error < LC_ERROR_COUNT; error++)
  {
    gui_render_page((lc_error_t)(error));
    delay(delay_ms);
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
      display.clearDisplay();
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
      display.setCursor(9, 26);
      display.print(F("L : "));
      display.print(measl, 2);
      display.print(F(" mH"));
      display.setCursor(9, 45);
      display.print(F("f : "));
      display.print(measf, 2);
      display.print(F(" KHz"));
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
      display.setCursor(9, 26);
      display.print(F("C : "));
      display.print(measl, 2);
      display.print(F(" nF"));
      display.setCursor(9, 45);
      display.print(F("f : "));
      display.print(measf, 2);
      display.print(F(" KHz"));
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
      display.print(effl, 2);
      display.print(F(" mH"));
      display.setCursor(7, 36);
      display.print(F("C-EFF : "));
      display.print(effc, 2);
      display.print(F(" nF"));
      display.setCursor(7, 50);
      display.print(F("f-eff : "));
      display.print(freq, 2);
      display.print(F(" KHz"));
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
