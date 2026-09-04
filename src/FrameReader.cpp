#include "FrameReader.h"

FrameReader::Status FrameReader::feed(const char c) {
  // 直前に確定したフレームは呼び出し側が取り出し済みとして破棄する
  if (_completed) {
    _completed = false;
    _length = 0;
  }

  // 改行はフレームの一部として扱わない
  if (c == '\r' || c == '\n') {
    return Status::Incomplete;
  }

  // 超過したフレームは終端文字までを捨て、後続フレームから復帰する
  if (_discarding) {
    if (c == protocol::TERMINATOR) {
      _discarding = false;
    }
    return Status::Incomplete;
  }

  if (c == protocol::TERMINATOR) {
    _buffer[_length] = '\0';
    _completed = true;
    return Status::Complete;
  }

  if (_length >= protocol::MAX_PAYLOAD_LENGTH) {
    _length = 0;
    _discarding = true;
    return Status::Overflow;
  }

  _buffer[_length++] = c;
  return Status::Incomplete;
}
