/*
  hwtest - Hardware Test
 */

#include "Console.h"
#include "Clock.h"

Console console;
Clock clock;

void setup() {
  console.begin();
  console.debugln("hello world!");
}

void loop() {
  console.idle();
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
  clock.longDate(string);
  p->printf("Date: %s\n", string);

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
    static const millis_t blinkInterval = 250;
    static const uint8_t maxPins = NUM_DIGITAL_PINS;
    uint8_t blinking[maxPins];
    void blink(uint8_t pin, uint8_t b) { if (pin < maxPins) blinking[pin] = b; };
    uint8_t isBlinking(uint8_t pin) {  return (pin < maxPins) ? blinking[pin] : 0;}
};

pinCommand thePinCommand;

class DateCommand : public Command {
  public:
    const char* getName() { return "date"; }
    const char* getHelp() { return "<year> <month> <day> <hour> <minute> <second> - Print or Set Date"; }
    void execute(Stream* c, uint8_t paramCount, char** params) {
      int p = paramCount;
      uint16_t year = 0;
      uint8_t month = 1;
      uint8_t day = 1;
      uint8_t hour = 0;
      uint8_t minute = 0;
      uint8_t second = 0;
      if (p>0) { year = atoi(params[1]); }
      if (p>1) { month = atoi(params[2]); }
      if (p>2) { day = atoi(params[3]); }
      if (p>3) { hour = atoi(params[4]); }
      if (p>4) { minute = atoi(params[5]); }
      if (p>5) { second = atoi(params[6]); }
      if (year && month && day) {
        clock.setDateTime(year,month,day,hour,minute,second);
      }
      clock.longDate(*c);
      c->print(" ");
      clock.longTime(*c);
      c->println();
    }
};

DateCommand theDateCommand;
