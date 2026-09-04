/*
  Thermalright TL-C12C-S X3 (4pin PWM Fan) 制御コード (C++ / Arduino)

  【配線】
  Fan Pin1 (GND)  -> Arduino GND  ＆ 12V電源のGND (共通GND)
  Fan Pin2 (+12V) -> 外部12V電源の+側 (Arduinoの5V/Vinには繋がない)
  Fan Pin3 (Tach) -> Arduino D2 (現在のプロトコルでは未使用)
  Fan Pin4 (PWM)  -> Arduino D9

  ※ ファンモーター自体はArduinoの電源からは動きません。
     必ず12V電源を別途用意し、GNDだけArduinoと共通にしてください。

  通信仕様は README.md を参照。
  必要ライブラリ: なし（Arduino標準関数のみ使用）
*/

#include <Arduino.h>

#include "FanHandler.h"
#include "SerialHandler.h"

namespace {
  constexpr unsigned long SERIAL_BAUD_RATE = 9600;

  FanHandler fanHandler;
  SerialHandler serialHandler(fanHandler);
}

void setup() {
  fanHandler.begin();
  serialHandler.begin(SERIAL_BAUD_RATE);
}

void loop() {
  serialHandler.update();
}
