**NotBopIt Final Project ECE167 Winter 2025: Sensing and Sensor Technologies UCSC**

Key game source files are located at: https://github.com/djretta/NotBopIt/tree/main/NotBopIt/src

**Synopsis**

NotBopIt is an enhanced version of the game BopIt. Like BopIt, NotBopIt incorporates several sensors into a housing, of which the player is expected to interact with correctly to proceed to the next trial, eventually level up, and, after completing all of the levels, win.

To correctly interact with a sensor, the player must observe an auditory or visual cue indicating which sensor is the one they must activate. The player has a limited time window in which to activate the sensor and failure to do so results in a loss and resetting of the game. As the player progresses through the levels, this time window decreases.

Unlike BopIt's clear and loud sensor indications (e.g. "TwistIt!, "PullIt!"), NotBopIt simply indicates with a flat tone and a color output to an RGB LED. These cue-sensor associations must be recalled by the player, initially read off of a list.

Additionally, to interact with a sensor, the sensor must be oriented upwards, as detected by an IMU.

**Compilation**

The following files, written in C, are intended for compilation using Visual Studio Code running on Windows, with the PlatformIO IDE extension installed and configured along with the following _platformio.ini_:

```ini
[env:nucleo_f411re]
platform = ststm32
board = nucleo_f411re
framework = stm32cube
lib_deps = ../Common
lib_archive = no
monitor_speed = 115200
build_flags = -Wl,-u_printf_float
```

The required course-provided file package ("Common" folder) implements additional support for the Nucleo STM32F411RE when coupled with the UCSC Nucleo development shield.

| File           | Description                                           |
|----------------|-------------------------------------------------------|
| `NotBopIt.c`   | State machine and sound/RGB controls                  |
| `light.c/.h`   | RGB output functions                                  |
| `sound.c/.h`   | Speaker output functions                              |
| `sensors.c/.h` | Sensor interpreting functions                         |
| `PING.c/.h`    | Ultrasonic ping sensor distance functions             |
| `QEI.c/.h`     | Relative rotary encoder current position in degrees   |
