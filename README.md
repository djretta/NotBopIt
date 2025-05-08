README NotBopIt Final Project ECE167 Winter 2025: Sensing and Sensor Technologies UCSC

SYNOPSIS

NotBopIt is an enhanced version of the game BopIt. Like BopIt, NotBopIt incorporates several sensors into a housing, of which the player is expected to interact with correctly to proceed to the next trial, eventually level up, and, after completing all of the levels, win.

To correctly interact with a sensor, the player must observe an auditory or visual cue indicating which sensor is the one they must activate. The player has a limited time window in which to activate the sensor and failure to do so results in a loss and resetting of the game. As the player progresses through the levels, this time window decreases.

Unlike BopIt's clear and loud sensor indications (e.g. "TwistIt!, "PullIt!"), NotBopIt simply indicates with a flat tone and a color output to an RGB LED. These cue-sensor associations must be recalled by the player, or initially read off of a list.

COMPILATION

The following files, written in C, are intended for compilation using Visual Studio Code running on Windows, with the PlatformIO IDE extension installed.

file description

NotBopIt.c state machine and sound/RGB controls light.c/.h encoder RGB output functions sound.c/.h speaker output functions sensors.c/.h sensor interpretting functions PING.c/.h utlrasonic ping sensor distance functions QEI.c/.h relative rotary coder current position in degrees

Additionally, the file pack provided by the course ("Common" folder) which implements a variety of functionality for the Nucleo STM32F411RE and UCSC Nucleo development shield is required.

The following platformio.ini is also required:

[env:nucleo_f411re] platform = ststm32 board = nucleo_f411re framework = stm32cube lib_deps = ../Common lib_archive = no monitor_speed = 115200 build_flags = -Wl,-u_printf_float

see: https://github.com/djretta/NotBopIt/tree/main/NotBopIt/src

