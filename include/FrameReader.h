#pragma once

#include "Protocol.h"

// 受信バイト列を終端文字 '#' 単位のフレームに組み立てる。
// 改行は読み飛ばし、長すぎるフレームは次の終端文字まで読み捨てる。
class FrameReader {
public:
  enum class Status : uint8_t {
    Incomplete, // フレーム未確定
    Complete,   // フレーム確定 (payload() / length() で取得する)
    Overflow,   // 受信長超過 (-7)
  };

  Status feed(char c);

  const char* payload() const { return _buffer; }
  uint8_t length() const { return _length; }

private:
  char _buffer[protocol::MAX_PAYLOAD_LENGTH + 1] = {};
  uint8_t _length = 0;
  bool _completed = false;  // 確定済みフレームを保持中
  bool _discarding = false; // 超過フレームを読み捨て中
};
