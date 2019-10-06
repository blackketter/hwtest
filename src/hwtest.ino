/*
  hwtest - Hardware Test
 */

#include "Console.h"
#include "Clock.h"

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

  Serial6.begin(115200);

}

void loop() {
  console.idle();

  if (Serial6.available() > 0) {
    uint8_t incomingByte = Serial6.read();
    console.print((char)incomingByte);
	}
}


