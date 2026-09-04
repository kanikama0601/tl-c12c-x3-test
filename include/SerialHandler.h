#pragma once

#include "FanHandler.h"
#include "FrameReader.h"
#include "Protocol.h"

// シリアルの入出力を担当する。
// 受信 → フレーム組み立て → パース → コマンド実行 → 応答送信 をつなぐ。
class SerialHandler {
public:
  explicit SerialHandler(FanHandler& fanHandler) : _fanHandler(fanHandler) {}

  void begin(unsigned long baudRate) const;
  void update();

private:
  void process(const char* payload, uint8_t length);
  static void send(const protocol::Response& response);

  FanHandler& _fanHandler;
  FrameReader _reader;
};
