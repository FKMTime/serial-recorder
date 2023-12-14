#include <Arduino.h>

int getRecordingDuration(byte* buff, int size) {
  RecordingFrame frame;
  
  int offset = sizeof(RecordingInfo);
  int tmpSum = 0;
  while (size > offset) {
      memcpy(&frame, buff + offset, sizeof(RecordingFrame));
      offset += sizeof(RecordingFrame);

      // STOP FRAME
      if (frame.delta == -1 && frame.cheksum == 0) break;

      uint8_t cheksum = frame.delta ^ frame.data & 0xFF;
      if (frame.cheksum != cheksum) continue;

      tmpSum += frame.delta;
  }

  return tmpSum;
}

String readSerialCommand() {
  Serial.print("> ");
  String input = "";
  while (Serial) {
    if (!Serial.available()) {
      delay(10);
      continue;
    }

    char c = Serial.read();
    if (c == '\r') {
      continue;
    } else if (c == '\n') {
      Serial.print("\n");
      break;
    } else if (c == '\b') {
      input.remove(input.length() - 1);
      Serial.println();
      Serial.print("> ");
      Serial.print(input);

      continue;
    }

    input += c;
    Serial.print(c);
  }

  return input;
}

String readNextArg(String &input) {
  String tmp = "";
  while(input.length() > 0 && input[0] != ' ') {
    tmp += input[0];
    input.remove(0, 1);
  }
  if (input.length() > 0) input.remove(0, 1);

  return tmp;
}