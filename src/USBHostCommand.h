#ifndef _USBHostCommand_

#if defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)

#if defined(__IMXRT1052__) || defined(__IMXRT1062__)
#if !defined(USB_ENABLE_PIN)
#define USB_ENABLE_PIN 27
#endif
#endif

#include "USBHost_t36.h"
USBHost usbHost;
USBHub hub1(usbHost);
USBHub hub2(usbHost);
USBHub hub3(usbHost);

USBHIDParser hid1(usbHost);
KeyboardController keyboard1(usbHost);

MIDIDevice midi1(usbHost);

USBHIDParser hid2(usbHost);
MouseController mouse1(usbHost);

Stream* _output = nullptr;

void OnPress(int key)
{
  if (!_output) return;
	_output->print("key '");
	_output->print((char)key);
	_output->print("'  ");
	_output->println(key);
	//_output->print("key ");
	//_output->print((char)keyboard1.getKey());
	//_output->print("  ");
}

void OnNoteOn(byte channel, byte note, byte velocity)
{
  if (!_output) return;
	_output->print("Note On, ch=");
	_output->print(channel);
	_output->print(", note=");
	_output->print(note);
	_output->print(", velocity=");
	_output->print(velocity);
	_output->println();
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  if (!_output) return;
	_output->print("Note Off, ch=");
	_output->print(channel);
	_output->print(", note=");
	_output->print(note);
	//_output->print(", velocity=");
	//_output->print(velocity);
	_output->println();
}

void OnControlChange(byte channel, byte control, byte value)
{
  if (!_output) return;
	_output->print("Control Change, ch=");
	_output->print(channel);
	_output->print(", control=");
	_output->print(control);
	_output->print(", value=");
	_output->print(value);
	_output->println();
}

class USBHostCommand : public Command {
  public:
    const char* getName() { return "host"; }
    const char* getHelp() { return "USB Host Activity"; }

    void begin() override {
      usbHost.begin();
#if defined(USB_ENABLE_PIN)
      pinMode(USB_ENABLE_PIN, OUTPUT);
      digitalWrite(USB_ENABLE_PIN, 1);
#endif
      keyboard1.attachPress(OnPress);
      midi1.setHandleNoteOff(OnNoteOff);
      midi1.setHandleNoteOn(OnNoteOn);
      midi1.setHandleControlChange(OnControlChange);
    }

    void idle(Console* c) override {
      usbHost.Task();
      midi1.read();

      if (_output) {
        if (mouse1.available()) {
          _output->print("Mouse: buttons = ");
          _output->print(mouse1.getButtons());
          _output->print(",  mouseX = ");
          _output->print(mouse1.getMouseX());
          _output->print(",  mouseY = ");
          _output->print(mouse1.getMouseY());
          _output->print(",  wheel = ");
          _output->print(mouse1.getWheel());
          _output->print(",  wheelH = ");
          _output->print(mouse1.getWheelH());
          _output->println();
          mouse1.mouseDataClear();
        }
      }
    }

    void execute(Console* c, uint8_t paramCount, char** params) {

      if (!_output) {
        _output = c;
        c->println("USBHost Debug On");
      } else {
        _output = nullptr;
        c->println("USBHost Debug Off");
      }
    }
};

USBHostCommand theUSBHostCommand;

#endif
#endif
