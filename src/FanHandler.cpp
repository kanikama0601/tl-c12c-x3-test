#include "FanHandler.h"

namespace {
  // ---- 設定 ----
  // マイコン番号。コントローラーごとにこの値を変更する (S コマンドで返す)
  constexpr uint8_t CONTROLLER_INDEX = 0;

  // デバイス識別番号。ファンの制御用マイコンは 0
  constexpr uint8_t DEVICE_ID = 0;

  // 各ファンの PWM 出力ピン
  constexpr uint8_t FAN_PWM_PINS[FanHandler::FAN_COUNT] = {9};
}

void FanHandler::begin() {
  for (uint8_t i = 0; i < FAN_COUNT; ++i) {
    _fans[i].begin(FAN_PWM_PINS[i]);
  }
}

protocol::Response FanHandler::handle(const protocol::Command& command) {
  switch (command.code) {
    case 'S': return handleStatus();
    case 'I': return handleInitialize();
    case 'D': return handleDefine(command);
    case 'R': return handleRotate(command);
    case 'Z': return handleStop();
    case 'C': return handleCheck(command);
    default:  return protocol::makeError(protocol::Error::UnknownCommand);
  }
}

// S: 接続確認。初期化前でも応答する
protocol::Response FanHandler::handleStatus() const {
  return protocol::makeResponse('S', DEVICE_ID, CONTROLLER_INDEX, FAN_COUNT);
}

// I: 全ファンを初期回転速度に戻し、初期化済みとする
protocol::Response FanHandler::handleInitialize() {
  for (uint8_t i = 0; i < FAN_COUNT; ++i) {
    _fans[i].setSpeed(_initialSpeeds[i]);
  }
  _initialized = true;
  return protocol::makeResponse('I');
}

// D: 初期化設定。回転には反映せず、初期回転速度のみ更新する
protocol::Response FanHandler::handleDefine(const protocol::Command& command) {
  const uint16_t index = command.data[0];
  const uint16_t speed = command.data[1];

  if (!isValidFanIndex(index)) {
    return protocol::makeError(protocol::Error::InvalidFanIndex);
  }
  if (!isValidSpeed(speed)) {
    return protocol::makeError(protocol::Error::InvalidSpeed);
  }

  _initialSpeeds[index] = static_cast<uint8_t>(speed);
  return protocol::makeResponse('D', index, speed);
}

// R: 回転速度の変更
protocol::Response FanHandler::handleRotate(const protocol::Command& command) {
  const uint16_t index = command.data[0];
  const uint16_t speed = command.data[1];

  if (!_initialized) {
    return protocol::makeError(protocol::Error::NotInitialized);
  }
  if (!isValidFanIndex(index)) {
    return protocol::makeError(protocol::Error::InvalidFanIndex);
  }
  if (!isValidSpeed(speed)) {
    return protocol::makeError(protocol::Error::InvalidSpeed);
  }

  _fans[index].setSpeed(static_cast<uint8_t>(speed));
  return protocol::makeResponse('R', index, speed);
}

// Z: 全ファンを停止する。初期化状態は変えない
protocol::Response FanHandler::handleStop() {
  for (uint8_t i = 0; i < FAN_COUNT; ++i) {
    _fans[i].setSpeed(0);
  }
  return protocol::makeResponse('Z');
}

// C: 現在の設定値 (直近に指示した duty) を返す
protocol::Response FanHandler::handleCheck(const protocol::Command& command) const {
  const uint16_t index = command.data[0];

  if (!_initialized) {
    return protocol::makeError(protocol::Error::NotInitialized);
  }
  if (!isValidFanIndex(index)) {
    return protocol::makeError(protocol::Error::InvalidFanIndex);
  }

  return protocol::makeResponse('C', index, _fans[index].speed());
}
