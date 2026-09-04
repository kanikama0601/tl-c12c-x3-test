#pragma once

#include "Protocol.h"

// フレームのペイロードを Command に変換する。
// 検出するエラーは README.md の判定順序に従い -5 → -4 → -3 → -8 の順。
namespace CommandParser {
  protocol::Error parse(const char* payload, uint8_t length, protocol::Command& out);
}
