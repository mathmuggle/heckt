#include "gui.h"

void setup()
{
  Serial.begin(115200);
  gui_init();
}

void loop()
{
  gui_debug(1500);
}
