#include "SerialHandler.h"

#include "CommandParser.h"

void SerialHandler::begin(const unsigned long baudRate) const {
  Serial.begin(baudRate);
}

void SerialHandler::update() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());

    switch (_reader.feed(c)) {
      case FrameReader::Status::Complete:
        process(_reader.payload(), _reader.length());
        break;
      case FrameReader::Status::Overflow:
        send(protocol::makeError(protocol::Error::FrameTooLong));
        break;
      case FrameReader::Status::Incomplete:
        break;
    }
  }
}

void SerialHandler::process(const char* payload, const uint8_t length) {
  protocol::Command command = {};

  const protocol::Error error = CommandParser::parse(payload, length, command);
  if (error != protocol::Error::None) {
    send(protocol::makeError(error));
    return;
  }

  send(_fanHandler.handle(command));
}

void SerialHandler::send(const protocol::Response& response) {
  Serial.print(response.code);
  Serial.print(protocol::SEPARATOR);
  Serial.print(response.dataCount);

  for (uint8_t i = 0; i < response.dataCount; ++i) {
    Serial.print(protocol::SEPARATOR);
    Serial.print(response.data[i]);
  }

  Serial.print(protocol::TERMINATOR);
  Serial.print(protocol::LINE_ENDING);
}
