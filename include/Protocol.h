#pragma once

#include <Arduino.h>

// シリアルプロトコルの定数・エラーコード・フレーム表現をまとめる。
// 詳細は README.md の「通信仕様」「フレーム形式」「エラー出力一覧」を参照。
namespace protocol {
  constexpr char TERMINATOR = '#';
  constexpr char SEPARATOR = ',';

  // 終端文字を含めた受信フレームの上限。これを超えたフレームは -7 で弾く
  constexpr uint8_t MAX_FRAME_LENGTH = 64;
  constexpr uint8_t MAX_PAYLOAD_LENGTH = MAX_FRAME_LENGTH - 1;

  // 1フレームで扱うデータの最大個数 (S の応答が 3 個で最大)
  constexpr uint8_t MAX_DATA_COUNT = 3;

  // 数値の上限。これを超える入力は飽和させ、範囲エラー (-2 / -6) で弾く
  constexpr uint16_t VALUE_LIMIT = 65535;

  // 回転速度 (PWM duty) の範囲
  constexpr uint16_t MIN_SPEED = 0;
  constexpr uint16_t MAX_SPEED = 255;

  enum class Error : int8_t {
    None = 0,
    NotInitialized = -1,       // 初期化未完了
    InvalidSpeed = -2,         // 無効な速度
    InvalidDataLength = -3,    // 無効なデータ長
    UnknownCommand = -4,       // 不明なコマンド
    ParseFailure = -5,         // パース不正
    InvalidFanIndex = -6,      // 無効なモーター番号
    FrameTooLong = -7,         // 受信長超過
    InvalidArgumentCount = -8, // 引数個数不正
  };

  // 受信コマンド。data は引数個数の検証後に確定するため、常に dataCount 個が有効
  struct Command {
    char code;
    uint8_t dataCount;
    uint16_t data[MAX_DATA_COUNT];
  };

  // 送信レスポンス。エラー時は code = 'E'、data[0] にエラーコードが入る
  struct Response {
    char code;
    uint8_t dataCount;
    int16_t data[MAX_DATA_COUNT];
  };

  inline Response makeResponse(const char code) {
    return Response{code, 0, {0, 0, 0}};
  }

  inline Response makeResponse(const char code, const int16_t first, const int16_t second) {
    return Response{code, 2, {first, second, 0}};
  }

  inline Response makeResponse(const char code, const int16_t first, const int16_t second, const int16_t third) {
    return Response{code, 3, {first, second, third}};
  }

  inline Response makeError(const Error error) {
    return Response{'E', 1, {static_cast<int16_t>(error), 0, 0}};
  }
}
