#pragma once

#include "Protocol.h"
#include "PwmFan.h"

// コマンドの意味づけ (初期化状態・ファン番号・回転速度の管理) を担当する。
// 検出するエラーは README.md の判定順序に従い -1 → -6 → -2 の順。
class FanHandler {
public:
  // ファン接続数。FanHandler.cpp の FAN_PWM_PINS と個数を一致させること
  static constexpr uint8_t FAN_COUNT = 1;

  void begin();
  protocol::Response handle(const protocol::Command& command);

private:
  protocol::Response handleStatus() const;
  protocol::Response handleInitialize();
  protocol::Response handleDefine(const protocol::Command& command);
  protocol::Response handleRotate(const protocol::Command& command);
  protocol::Response handleStop();
  protocol::Response handleCheck(const protocol::Command& command) const;

  static bool isValidFanIndex(uint16_t index) { return index < FAN_COUNT; }
  static bool isValidSpeed(uint16_t speed) { return speed <= protocol::MAX_SPEED; }

  PwmFan _fans[FAN_COUNT];
  uint8_t _initialSpeeds[FAN_COUNT] = {};
  bool _initialized = false;
};
