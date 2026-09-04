#include "PwmFan.h"

void PwmFan::begin(const uint8_t pwmPin) {
  _pwmPin = pwmPin;
  pinMode(_pwmPin, OUTPUT);
  setSpeed(0);
}

void PwmFan::setSpeed(const uint8_t speed) {
  _speed = speed;
  analogWrite(_pwmPin, _speed);
}
