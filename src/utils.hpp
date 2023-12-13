#include <Arduino.h>

struct RecordingInfo{
  int baud;
  bool inverted;
};

struct RecordingFrame {
  int16_t delta;
  uint8_t data;
  uint8_t cheksum;
};

// byte* intToBytes(int n) {
//   byte bytes[4];
//   bytes[0] = (byte)(n >> 24);
//   bytes[1] = (byte)(n >> 16);
//   bytes[2] = (byte)(n >> 8);
//   bytes[3] = (byte)n;

//   return bytes;
// }

// int readIntFromBytes(byte* buff, int offset) {
//   return (buff[offset] << 24) + (buff[offset + 1] << 16) + (buff[offset + 2] << 8) + buff[offset + 3];
// }

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