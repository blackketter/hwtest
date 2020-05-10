/*
  hwtest - Hardware Test
 */

#include <SD.h>
#include <SPI.h>

#include "Clock.h"
#include "Console.h"

#include "USBHostCommand.h"
#include "Commands/SerialCommand.h"
#include "Commands/DateCommand.h"

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

class ESPToolCommand : public SerialCommand {
  public:
    const char* getName() { return "esp"; }
    const char* getHelp() { return ("Turn on esp32 passthrough"); }

    void execute(Console* c, uint8_t paramCount, char** params) {
      pinMode(EN_PIN, INPUT);
      pinMode(GPIO0_PIN, INPUT);
      openSerial(c,RX_PIN,TX_PIN,BAUD);
    }

    void idle(Console* c) override {
      SerialCommand::idle(c);
      if (isRunning()) {
        bool dtr = Serial.dtr();
        bool rts = Serial.rts();
        if (_lastDTR != dtr) {
          _lastDTR = dtr;
          if (dtr) {
            c->debugln("dtr on");
            pinMode(EN_PIN, OUTPUT);
            digitalWrite(EN_PIN, LOW);
          } else {
            c->debugln("dtr off");
            pinMode(EN_PIN, INPUT);
          }
        }
        if (_lastRTS != rts) {
          _lastRTS = rts;
          if (rts) {
            c->debugln("rts on");
            pinMode(GPIO0_PIN, OUTPUT);
            digitalWrite(GPIO0_PIN, LOW);
          } else {
            c->debugln("rts off");
            pinMode(GPIO0_PIN, INPUT);
          }
        }
      }
    }
    void kill() override {
      SerialCommand::kill();
      pinMode(EN_PIN, INPUT);
      pinMode(GPIO0_PIN, INPUT);
    }

  private:
    bool _lastDTR = false;
    bool _lastRTS = false;
    static const uint8_t EN_PIN = 26;
    static const uint8_t GPIO0_PIN = 28;
    static const uint8_t RX_PIN = 25;
    static const uint8_t TX_PIN = 24;
    static const uint32_t BAUD = 115200;
};

ESPToolCommand theESPToolCommand;

