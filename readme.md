# heckt

Software for a digital, self-calibrating LC meter built with an Arduino (ATmega328P), based on Neil Wayne Heckt's design published in the June 1996 issue of *Electronics Now* [1].

This project grew out of my collaboration with [Tobias (KK7BCO/2M0TFF)](https://kk7bco.com/), who first introduced me to the design through Phil Rice's LC meter projects ([Version 1](https://sites.google.com/site/vk3bhr/home/lcm1) and [Version 2](https://sites.google.com/site/vk3bhr/home/index2-html)).

If you've always wanted an LC meter for your electronics hobby or radio projects, it's time to homebrew your own! Heckt himself offers some encouragement [1]:
> It's really a "put it together and it works" project.

## instructions

- **First use:** If no valid calibration is saved, the device opens the calibration page after startup.
- **Calibration:** Set the SPDT switch to **C**, leave the DUT terminals open, and press the button to start calibration.
- **Save calibration:** After successful calibration, the parameters are saved to EEPROM and retained even after the power is switched off. Press the button again to begin measuring.
- **Normal startup:** On subsequent startups, the device loads the saved calibration and opens the measurement page selected by the SPDT switch: **C** for capacitance or **L** for inductance.
- **Reset calibration:** Hold the button while powering on and keep it held until the title screen appears. Then release it; the calibration page will open.

## files

- **[fw.ino](fw/fw.ino):** Runs the calibration and measurement state machine, handles mode changes, and supports resetting calibration at startup.
- **[gui.h](fw/gui.h) / [gui.cpp](fw/gui.cpp):** Reads the button and renders the OLED's calibration, measurement, and error pages.
- **[freq.h](fw/freq.h) / [freq.cpp](fw/freq.cpp):** Measures frequency on D5 using Timer1 pulse counting and a Timer2 timed gate.
- **[lc.h](fw/lc.h) / [lc.cpp](fw/lc.cpp):** Controls the relay, stores calibration in EEPROM, and calculates inductance or capacitance from the measured frequency.

## dependencies

Use **Tools → Manage Libraries…** in the Arduino IDE to install:

- **[Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306):** OLED display driver.
- **[Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library):** Text and graphics for the OLED.

Select **Install all** if prompted to install supporting libraries. `Wire` and `EEPROM` come with Arduino AVR board support. Select **Arduino Pro or Pro Mini (ATmega328P)** with the clock setting matching your board.

## references

1. Heckt - *Build this self-calibrating LC meter to measure capacitance and inductance* (1996)
