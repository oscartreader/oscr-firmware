/**
 * @file
 * @brief Dummy file for Arduino IDE.
 *
 * This file exists purely to allow compilation and updating via the
 * Arduino IDE. You can no longer easily develop this project using
 * the Arduino IDE.
 *
 * If you are a developer, you should switch to Visual Studio Code
 * with the PlatformIO extension.
 *
 * If you are just trying to update your OSCR, you can use the web
 * updater located at https://oscr.tools/ without having to install
 * additional software.
 *
 * If you still wish to use the Arduino IDE, you must create a config
 * file before you will be able to compile this project.
 *
 * It is typically located in one of the following locations:
 * - Windows: `C:\Users\<user>\AppData\Local\Arduino15\packages\arduino\hardware\avr\<version>\`
 * - Linux: `~/.arduino15/packages/arduino/hardware/avr/<version>/`
 *
 * You will know it is the correct location if it contains the files
 * named `platform.txt` and `boards.txt`.
 *
 * Create and open `platform.local.txt` and add the following to it:
 *
 * ```
 * compiler.c.extra_flags=-mcall-prologues -flto=auto
 * compiler.cpp.extra_flags=-std=gnu++1z -mcall-prologues -flto=auto -I{build.path}/sketch/include/
 * compiler.c.elf.extra_flags=-Wl,--relax -flto=auto
 * ```
 *
 * After saving it, click on `Tools > Reload Board Data` or restart
 * the Arduino IDE. After that you should be able to use the IDE to
 * compile the firmware and update your OSCR.
 *
 * @sa ArduinoConfig.h
 */

extern void setup();
extern void loop();
