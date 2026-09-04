#pragma once

#include <Arduino.h>

// 1台の 4pin PWM ファンの出力を担当する。
// 回転速度は PWM duty (0-255) で保持する。
class PwmFan {
public:
  void begin(uint8_t pwmPin);
  void setSpeed(uint8_t speed);
  uint8_t speed() const { return _speed; }

private:
  uint8_t _pwmPin = 0;
  uint8_t _speed = 0;
};
