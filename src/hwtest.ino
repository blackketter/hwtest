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
  if (0 && clock.getSeconds() != lastSecond) {
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

class pinCommand : public Command {
  public:
    pinCommand() { for (uint8_t i = 0; i < maxPins; i++) { blinking[i] = 0; } }
    const char* getName() { return "pin"; }
    const char* getHelp() { return "<on|off|blink|read|input|pullup|awrite val|aread> <pin numbers...> - Set or read a pin"; }
    void execute(Stream* c, uint8_t paramCount, char** params) {
      if (paramCount < 2) {
        printError(c);
      } else {
        char* command = params[1];
        for (uint8_t i = 2; i <= paramCount; i++) {
          uint8_t pin = atoi(params[i]);
          if (strcasecmp(command, "on") == 0) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, HIGH);
            c->printf("  Pin: %d HIGH\n", pin);
          } else if (strcasecmp(command, "off") == 0) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            blink(pin, 0);
            c->printf("  Pin: %d LOW\n", pin);
          } else if (strcasecmp(command, "input") == 0) {
            pinMode(pin, INPUT);
            c->printf("  Pin: %d INPUT\n", pin);
            blink(pin, 0);
          } else if (strcasecmp(command, "pullup") == 0) {
            pinMode(pin, INPUT_PULLUP);
            c->printf("  Pin: %d INPUT_PULLUP\n", pin);
            blink(pin, 0);
          } else if (strcasecmp(command, "read") == 0) {
            uint8_t val = digitalRead(pin);
            c->printf("  Pin: %d %s\n", pin, val ? "HIGH" : "LOW");
          } else if (strcasecmp(command, "aread") == 0) {
            uint8_t val = analogRead(pin);
            c->printf("  Pin: %d Analog: %d\n", pin, val);
          } else if (strcasecmp(command, "awrite") == 0) {
            uint8_t val = pin;
            i++;
            pin = atoi(params[i]);
            analogWrite(pin, val);
            c->printf("  Pin: %d  Analog: %d\n", pin, val);
            blink(pin, 0);
          } else if (strcasecmp(command, "blink") == 0) {
            blink(pin, 1);
            c->printf("  Pin: %d BLINKING\n", pin);
          } else {
            printError(c);
          }
        }
      }
    }

    void idle() override {
      if (Uptime::millis() > lastBlink + blinkInterval) {
        lastBlink = Uptime::millis();
        blinkState = !blinkState;
        for (uint8_t pin = 0; pin < maxPins; pin++) {
          if (isBlinking(pin)) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, blinkState ? HIGH : LOW);
          }
        }
     }
    }
  private:
    millis_t lastBlink = 0;
    bool blinkState = false;
    static const millis_t blinkInterval = 500;
    static const uint8_t maxPins = 100;
    uint8_t blinking[maxPins];
    void blink(uint8_t pin, uint8_t b) { if (pin < maxPins) blinking[pin] = b; };
    uint8_t isBlinking(uint8_t pin) {  return (pin < maxPins) ? blinking[pin] : 0;}
};

pinCommand thePinCommand;
