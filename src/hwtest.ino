/*
  hwtest - Hardware Test
 */

#include <SD.h>
#include <SPI.h>

#include "Clock.h"
#include "Console.h"

#include "USBHostCommand.h"
#include "Commands/SerialCommand.h"
SerialCommand theSerialCommand;
Console console;

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
}

void loop() {
  console.idle();
}

class ESPToolCommand : public SerialCommand {
  public:
    const char* getName() { return "esp"; }
    const char* getHelp() { return ("turn on esp32 sidecar passthrough"); }

    void execute(Stream* c, uint8_t paramCount, char** params) {
      openSerial(c, RX_PIN,TX_PIN,BAUD);
    }

    void idle(Stream*c) override {
      SerialCommand::idle(c);
      if (isRunning()) {
        bool dtr = Serial.dtr();
        bool rts = Serial.rts();
        if (_lastDTR != dtr) {
          _lastDTR = dtr;
          if (dtr) {
            pinMode(EN_PIN, OUTPUT);
            digitalWrite(EN_PIN, LOW);
          } else {
            pinMode(EN_PIN, INPUT);
          }
        }
        if (_lastRTS != rts) {
          _lastRTS = rts;
          pinMode(GPIO0_PIN, OUTPUT);
          digitalWrite(GPIO0_PIN, !dtr);
        }
      }
    }

  private:
    bool _lastDTR = false;
    bool _lastRTS = false;
    static const uint8_t EN_PIN = 28;
    static const uint8_t GPIO0_PIN = 26;
    static const uint8_t RX_PIN = 25;
    static const uint8_t TX_PIN = 24;
    static const uint32_t BAUD = 115200;
};

ESPToolCommand theESPToolCommand;

