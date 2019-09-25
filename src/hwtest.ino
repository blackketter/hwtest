/*
  hwtest - Hardware Test
 */

#include "Console.h"
#include "Clock.h"

Console console;
Clock clock;

#if defined(ARDUINO_AVR_LEONARDO)
  // leonardo pro micro has Rx LED on pin 17
  #define LED (17)
  #define LEDON (LOW)
#elif defined(ARDUINO_ESP32_DEV)
  #define LED (5)
  #define LEDON (HIGH)
#elif defined(ARDUINO_LOLIN32)
  #define LED (22)
  #define LEDON (HIGH)
#elif defined(ESP8266_WEMOS_D1MINI)
  #define LED (2)
  #define LEDON (LOW)
#else
  // Pin 13 has an LED connected on most Arduino boards:
  #define LED (13)
  #define LEDON (LOW)
#endif

#define LEDOFF (!LEDON)

void setup() {
  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT);
  console.begin();
  console.debugln("hello world!");
}

void loop() {
  console.idle();

  static time_t lastSecond = 0;
  if (clock.getSeconds() != lastSecond) {
    lastSecond = clock.getSeconds();
    if (lastSecond % 2) {
      digitalWrite(LED, LEDON);   // set the LED on
      console.debugln("blink on");
    } else {
      console.debugln("blink off");
      digitalWrite(LED, LEDOFF);  // set the LED off
    }
  }
}

uint32_t FreeMem() { // for Teensy 3.0 (wrong for teensy 4.0)
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is (approximately) the free, available ram.
    return stackTop - heapTop;
}

void printInfo(Print* p) {
  char string[100];
  clock.longTime(string);

  p->printf("Time: %s\n", string);
  p->printf("Compiled: " __DATE__ " " __TIME__ "\n");
  p->printf("Free ram: %10d\n", FreeMem());
  p->printf("Uptime: %f\n", Uptime::micros()/1000000.0);

  uint32_t rtcMillis = (uint32_t)((clock.getRTCMicros()%1000000)/1000);
  uint32_t clockMillis = clock.fracMillis();

  p->printf("RTC millis:%03d, clock: %03d, diff: %d\n", (uint32_t)rtcMillis, (uint32_t)clockMillis, (int)rtcMillis - (int)clockMillis);

}


class InfoCommand : public Command {
  public:
    const char* getName() { return "info"; }
    const char* getHelp() { return "Print System Info"; }
    void execute(Stream* c, uint8_t paramCount, char** params) {
      printInfo(c);
    }
};

InfoCommand theInfoCommand;
