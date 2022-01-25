#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

#define BUTTON_NONE 0
#define BUTTON_SHORT 1
#define BUTTON_LONG 2

typedef enum {
  BTN_NONE,
  BTN_SHORT,
  BTN_LONG
} btn_status;

#define LONG_PRESS_DURATION 2000
#define SHORT_PRESS_DURATION 1000

/**
 * @brief Simple class that wraps button and is used to detect long and short presses.
 * 
 * Inspired by https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press
 * 
 */
class Button {
private:
    int _pin;
    int _lastState = LOW;  // the previous state from the input pin
    int _currentState;     // the current reading from the input pin
    unsigned long _pressedTime = 0;
    unsigned long _releasedTime = 0;
    bool _isPressing = false;
    bool _isLongDetected = false;


 public:
    Button(int pin);
    btn_status update();
   
};


#endif
