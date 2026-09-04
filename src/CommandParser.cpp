#include "CommandParser.h"

namespace {
  struct CommandSpec {
    char code;
    uint8_t argCount; // cmd と data_len を除いた引数の個数
  };

  constexpr CommandSpec COMMAND_SPECS[] = {
    {'S', 0}, // 接続確認
    {'I', 0}, // 回転を初期化
    {'D', 2}, // 初期化設定 (ファン番号, 初期回転速度)
    {'R', 2}, // ファン回転 (ファン番号, 回転速度)
    {'Z', 0}, // 回転停止
    {'C', 1}, // ステータス確認 (ファン番号)
  };
  constexpr uint8_t COMMAND_SPEC_COUNT = sizeof(COMMAND_SPECS) / sizeof(COMMAND_SPECS[0]);

  const CommandSpec* findSpec(const char code) {
    for (uint8_t i = 0; i < COMMAND_SPEC_COUNT; ++i) {
      if (COMMAND_SPECS[i].code == code) {
        return &COMMAND_SPECS[i];
      }
    }
    return nullptr;
  }

  // 10進の符号なし整数を読む。空文字列・数字以外を含む場合は失敗。
  // VALUE_LIMIT を超える値は飽和させ、後段の範囲チェックでエラーにする
  bool parseUnsigned(const char* text, const uint8_t length, uint16_t& out) {
    if (length == 0) {
      return false;
    }

    uint32_t value = 0;
    for (uint8_t i = 0; i < length; ++i) {
      const char c = text[i];
      if (c < '0' || c > '9') {
        return false;
      }
      if (value <= protocol::VALUE_LIMIT) {
        value = value * 10 + static_cast<uint32_t>(c - '0');
      }
    }

    out = (value > protocol::VALUE_LIMIT) ? protocol::VALUE_LIMIT : static_cast<uint16_t>(value);
    return true;
  }
}

protocol::Error CommandParser::parse(const char* payload, const uint8_t length, protocol::Command& out) {
  out.code = '\0';
  out.dataCount = 0;

  uint8_t fieldIndex = 0;
  uint8_t fieldStart = 0;
  uint16_t declaredLength = 0;
  uint8_t actualCount = 0;

  // 区切り文字と終端 (i == length) でフィールドを切り出す
  for (uint8_t i = 0; i <= length; ++i) {
    if (i < length && payload[i] != protocol::SEPARATOR) {
      continue;
    }

    const char* field = payload + fieldStart;
    const uint8_t fieldLength = i - fieldStart;

    if (fieldIndex == 0) {
      // コマンド文字は1文字ちょうど
      if (fieldLength != 1) {
        return protocol::Error::ParseFailure;
      }
      out.code = field[0];
    } else if (fieldIndex == 1) {
      if (!parseUnsigned(field, fieldLength, declaredLength)) {
        return protocol::Error::ParseFailure;
      }
    } else {
      uint16_t value = 0;
      if (!parseUnsigned(field, fieldLength, value)) {
        return protocol::Error::ParseFailure;
      }
      // 個数超過分は保持しない。データ長・引数個数の検証で弾かれる
      if (actualCount < protocol::MAX_DATA_COUNT) {
        out.data[actualCount] = value;
      }
      ++actualCount;
    }

    ++fieldIndex;
    fieldStart = i + 1;
  }

  // cmd と data_len は必須
  if (fieldIndex < 2) {
    return protocol::Error::ParseFailure;
  }

  const CommandSpec* spec = findSpec(out.code);
  if (spec == nullptr) {
    return protocol::Error::UnknownCommand;
  }

  if (declaredLength != actualCount) {
    return protocol::Error::InvalidDataLength;
  }

  if (actualCount != spec->argCount) {
    return protocol::Error::InvalidArgumentCount;
  }

  out.dataCount = actualCount;
  return protocol::Error::None;
}
