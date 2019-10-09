/*
  hwtest - Hardware Test
 */

#include "Clock.h"
#include "Console.h"

#include "USBHostCommand.h"

#include <SD.h>
#include <SPI.h>

Console console;

void setup() {
  console.begin();

  console.debugln("Setup begin");

#if defined(HAS_SD) || defined(BUILTIN_SDCARD)
  if (!SD.begin(BUILTIN_SDCARD)) {
    console.debugln("SD card not found");
  }
#endif

  console.println("Setup done");
}

void loop() {
  console.idle();
}

