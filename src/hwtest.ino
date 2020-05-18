/*
  hwtest - Hardware Test
 */

#include <SD.h>
#include <SPI.h>

#include "Clock.h"
#include "Console.h"

#include "Commands/DateCommand.h"
#include "Commands/SerialCommand.h"

SerialCommand theSerialCommand;
Console console;
Clock clock;

void setup() {
  console.begin();

  console.debugln("Setup begin");

#if defined(HAS_SD) || defined(BUILTIN_SDCARD)
  if (!SD.begin(BUILTIN_SDCARD)) {
    console.debugln("SD card not found");
  } else {
    console.debugln("SD card found");
  }
#endif

  console.println("Setup done");

  console.executeCommandLine("pin blink 13");

  clock.updateTime();

}

void loop() {
  console.idle();
}

