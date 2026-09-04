/*
  Thermalright TL-C12C-S X3 (4pin PWM Fan) テストコード (C++ / Arduino)

  【配線】
  Fan Pin1 (GND)  -> Arduino GND  ＆ 12V電源のGND (共通GND)
  Fan Pin2 (+12V) -> 外部12V電源の+側 (Arduinoの5V/Vinには繋がない)
  Fan Pin3 (Tach) -> Arduino D2 (回転数を読みたい場合。不要なら未接続でOK)
  Fan Pin4 (PWM)  -> Arduino D9

  ※ ファンモーター自体はArduinoの電源からは動きません。
     必ず12V電源を別途用意し、GNDだけArduinoと共通にしてください。

  必要ライブラリ: なし（Arduino標準関数のみ使用）
*/

#include <Arduino.h>

namespace
{
  class PwmFan {
  public:
    PwmFan(const uint8_t pwmPin, const uint8_t tachPin)
      : _pwmPin(pwmPin), _tachPin(tachPin) {}

    void begin() {
      pinMode(_pwmPin, OUTPUT);

      // 【推奨】Tach-5V間に外部10kΩプルアップ抵抗を追加した場合は
      // 下記を INPUT_PULLUP -> INPUT に変更してください（ノイズに強くなります）
      pinMode(_tachPin, INPUT_PULLUP);

      instance = this; // ISRから参照するため
      attachInterrupt(digitalPinToInterrupt(_tachPin), PwmFan::isrTrampoline, FALLING);
    }

    // duty: 0-255
    void setDuty(const uint8_t duty) const {
      analogWrite(_pwmPin, duty);
    }

    // 3秒間隔などで呼び出して概算RPMを取得する
    float readRpmAndReset(unsigned long intervalMs) {
      noInterrupts();
      unsigned long count = _pulseCount;
      _pulseCount = 0;
      interrupts();

      // PCファンのTachは通常1回転あたり2パルス
      float rpm = (static_cast<float>(count) / 2.0f) * (60000.0f / static_cast<float>(intervalMs));
      return rpm;
    }

  private:
    uint8_t _pwmPin;
    uint8_t _tachPin;
    volatile unsigned long _pulseCount = 0;
    volatile unsigned long _lastPulseMicros = 0;

    // ファン定格上限(2500RPM程度まで見込む)から、1パルスあたりの
    // 最短間隔を計算し、これより短い間隔のパルスはノイズとして無視する。
    // 2500RPM,2パルス/回転 -> 約12ms/パルス。
    // ノイズが密集している場合を考慮し、余裕をみて8ms未満は無視。
    // ノイズが酷い場合はこの値をさらに大きくして調整してください。
    static constexpr unsigned long MIN_PULSE_INTERVAL_US = 8000;

    static PwmFan* instance;

    static void isrTrampoline() {
      if (instance) {
        const unsigned long now = micros();
        // 前回パルスからの経過時間が短すぎる場合はチャタリングとして無視
        if (now - instance->_lastPulseMicros >= MIN_PULSE_INTERVAL_US) {
          instance->_pulseCount++;
          instance->_lastPulseMicros = now;
        }
      }
    }
  };
}

PwmFan* PwmFan::instance = nullptr;

// ---- 設定 ----
constexpr uint8_t PWM_PIN = 9;
constexpr uint8_t TACH_PIN = 2;
constexpr unsigned long STEP_INTERVAL_MS = 3000;

static PwmFan fan(PWM_PIN, TACH_PIN);

void setup() {
  Serial.begin(9600);
  fan.begin();

  Serial.println("TL-C12C-S X3 fan test start");

  // まずは全力回転でテスト
  fan.setDuty(255);
  delay(2000);
}

void loop() {
  // 0,51,102,153,204,255 (約0,20,40,60,80,100%) を明示的な配列で回す。
  // uint8_t型でduty+=51のループにすると255の次に306->オーバーフローして
  // 50になり、ループが終了せず意図しない値で回り続けるバグがあったため修正。
  static const uint8_t dutySteps[] = {0, 51, 102, 153, 204, 255};

  for (const unsigned char duty : dutySteps) {
    fan.setDuty(duty);

    Serial.print("Duty: ");
    Serial.print(map(duty, 0, 255, 0, 100));
    Serial.println("%");

    delay(STEP_INTERVAL_MS);

    float rpm = fan.readRpmAndReset(STEP_INTERVAL_MS);
    Serial.print("  -> RPM(概算): ");
    Serial.println(rpm);
  }
}