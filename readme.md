# heckt

Software for a digital, self-calibrating LC meter built with an Arduino (ATmega328P), based on Neil Wayne Heckt's design published in the June 1996 issue of *Electronics Now* [1].

This project grew out of my collaboration with [Tobias (KK7BCO/2M0TFF)](https://kk7bco.com/), who first introduced me to the design through Phil Rice's LC meter projects [2]. It also serves as the fifth build in [Red Summit RF](https://www.youtube.com/@RedSummitRF)'s *Beginner Homebrew Builder Series*.

If you've always wanted an LC meter for your electronics hobby or radio projects, it's time to homebrew your own! Heckt himself offers some encouragement [1]:
> It's really a "put it together and it works" project.

## instructions

- **First use:** If no valid calibration is saved in EEPROM, the device opens the calibration page after startup. Set the SPDT switch to **C**, leave the DUT terminals open, and short-press the button to start calibration.
- **Normal startup:** Power (without pressing the button) to load the saved calibration and open the measurement page selected by the SPDT switch (**C** for capacitance or **L** for inductance).
- **Calibration:** Hold the button while powering on until the title screen appears, then release it. Short-press on the calibration page to start. A successful calibration replaces the saved calibration and clears both zero offsets.
- **Zeroing:** Short-press while measuring. Leave the leads open in **C** mode or short them together in **L** mode, then short-press again. On success, the selected offset is saved to EEPROM. Re-zero the affected mode if you change the leads or fixture.
- **Frequency measurement:** Press and hold the button during normal operation or from the calibration page to display the oscillator frequency. Short-press to exit.

## files

- **[fw.ino](fw/fw.ino):** Runs the calibration, zeroing, and measurement flow and handles mode changes.
- **[gui.h](fw/gui.h) / [gui.cpp](fw/gui.cpp):** Reads the button and renders the OLED's calibration, measurement, and error pages.
- **[freq.h](fw/freq.h) / [freq.cpp](fw/freq.cpp):** Measures frequency on D5 using Timer1 pulse counting and a Timer2 timed gate.
- **[lc.h](fw/lc.h) / [lc.cpp](fw/lc.cpp):** Controls the relay, stores calibration in EEPROM, and calculates inductance or capacitance from the measured frequency.

## dependencies

Use **Tools → Manage Libraries…** in the Arduino IDE to install:

- **[Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306):** OLED display driver.
- **[Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library):** Text and graphics library for the OLED.

## references

1. Neil Wayne Heckt - *Build this self-calibrating LC meter to measure capacitance and inductance*. Electronics Now (June 1996)
2. Phil Rice - *A surprisingly accurate digital LC meter*. Amateur Radio (April 2004)
