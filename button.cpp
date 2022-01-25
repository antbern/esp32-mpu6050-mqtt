
#include "Arduino.h"
#include "button.h"


Button::Button(int pin) : _pin(pin) {
    pinMode(_pin, INPUT);
}

btn_status Button::update() {
  btn_status result = btn_status::BTN_NONE;

  // read the state of the switch/button:
  _currentState = digitalRead(_pin);

  if(_lastState == LOW && _currentState == HIGH) {        // button is pressed
    _pressedTime = millis();
    _isPressing = true;
    _isLongDetected = false;
  } else if(_lastState == HIGH && _currentState == LOW) { // button is released
    _isPressing = false;
    _releasedTime = millis();

    long pressDuration = _releasedTime - _pressedTime;

    if( pressDuration < SHORT_PRESS_DURATION ) {
      result = btn_status::BTN_SHORT;
    }
  }

  if(_isPressing == true && _isLongDetected == false) {
    long pressDuration = millis() - _pressedTime;

    if( pressDuration > LONG_PRESS_DURATION ) {
      _isLongDetected = true;
      result = btn_status::BTN_LONG;
    }
  }

  // save the the last state
  _lastState = _currentState;
  
  return result;
}