#include <Arduino.h>
#include <LittleFS.h>
#include <SoftwareSerial.h>
// #include <EEPROM.h>

SoftwareSerial inputSerial(17, -1);
SoftwareSerial outputSerial(-1, 17);

byte* intToBytes(int n);
int readIntFromBytes(byte* buff, int offset);
void interprateCommand(String &input);
String readSerialCommand();
String readNextArg(String &input);

bool recording = false;
unsigned long lastRecordingTime = 0;
File recordFile;

bool playback = false;
byte* playbackBuff;
int playbackOffset = 0;
int playbackLen = 0;

bool showHelp = true;

struct recordingInfo{
  int baud;
  bool inverted;
};

struct recordingFrame {
  int16_t delta;
  uint8_t data;
  uint8_t cheksum;
};

void setup() {
  LittleFS.begin();
  if (!LittleFS.exists("/records")) {
    LittleFS.mkdir("/records");
  }

  // EEPROM.begin(64);

  // File file = LittleFS.open("/test.txt", "a+");
  // if (file) {
  //   file.print('.');
  //   file.close();
  // }
}

void setup1() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  recordingFrame frame;
  if (recording) {
    while (recording) {
      if(inputSerial.available()) {
        frame.delta = millis() - lastRecordingTime;
        frame.data = inputSerial.read();
        frame.cheksum = frame.delta ^ frame.data & 0xFF;
        recordFile.write((byte*)&frame, sizeof(frame));

        lastRecordingTime = millis();
      }
    }

    inputSerial.end();

    // STOP FRAME
    frame.delta = -1;
    frame.data = 0;
    frame.cheksum = 0;
    recordFile.write((byte*)&frame, sizeof(frame));
    recordFile.close();

    Serial.println("Recording has stopped!");
    Serial.println();
    Serial.print("> ");
  }

  if(playback) {
    recordingFrame frame;
    while (playbackLen > playbackOffset) {
      memcpy(&frame, playbackBuff + playbackOffset, sizeof(frame));
      playbackOffset += sizeof(recordingFrame);

      if (frame.delta == -1 && frame.cheksum == 0) {
        // STOP FRAME
        break;
      }

      uint8_t cheksum = frame.delta ^ frame.data & 0xFF;
      if (frame.cheksum != cheksum) {
        continue;
      }

      delay(frame.delta);
      outputSerial.write(frame.data);
      // Serial.print((char)frame.data);
    }

    playback = false;
    outputSerial.end();

    Serial.println("Playback has stopped!");
    Serial.println();
    Serial.print("> ");
  }
}

void loop1() {
  while (!Serial) {
    showHelp = true;
    delay(1);
  }

  if (showHelp) {
    Serial.println("Execute function:");
    Serial.println("- record [name] <baud> <inverted 0|1> // starts the recording");
    Serial.println("- stop // stops the recording");
    Serial.println("- play [name] // plays the recorded file");
    Serial.println("- list // lists all recorded files");
    Serial.println("- delete [name] // deletes recorded file");
    Serial.println("- restart // restarts pico");
    Serial.println();

    showHelp = false;
  }

  String input = readSerialCommand();
  interprateCommand(input);
}

void interprateCommand(String &input) {
  if (input == "restart") {
    rp2040.reboot();
  } else if (input == "list") {
    Serial.println("Disk usage (used / total): ");
    FSInfo64 fsinfo;
    LittleFS.info64(fsinfo);
    Serial.print(fsinfo.usedBytes);
    Serial.print(" / ");
    Serial.println(fsinfo.totalBytes);
    Serial.println();

    fs::Dir dir = LittleFS.openDir("/records");
    Serial.println("Records: ");
    while(dir.next()) {
      Serial.print(" - ");
      Serial.print(dir.fileName());
      Serial.print(" | Size: ");
      Serial.println(dir.fileSize());
    }
  } else if (input == "stop") {
    if (recording) {
      // recordFile.flush();
      recording = false;

      Serial.println("Stopped the recording and saved the file!");
    } else {
      Serial.println("Recording isn't started!");
    }
  } else if(input.startsWith("record")) {
    input.remove(0, 7);
    String name = readNextArg(input);
    String baud = readNextArg(input);
    String invertedString = readNextArg(input);

    if (name == "") {
      Serial.println("record takes at least name!\n");
      showHelp = true;

      return;
    }

    int baudRate = baud.toInt();
    if (baudRate == 0) baudRate = 115200;
    bool inverted = invertedString.toInt() == 1;

    if (inverted) inputSerial.setInverted();
    inputSerial.begin(baudRate);

    Serial.print("Starting recording to file with name: ");
    Serial.println(name);

    recordFile = LittleFS.open("/records/" + name, "w");
    recordingInfo rInfo;
    rInfo.baud = baudRate;
    rInfo.inverted = inverted;

    recordFile.write((byte*) &rInfo, sizeof(rInfo));
    recording = true;
    lastRecordingTime = millis();
  } else if(input.startsWith("delete")) {
    input.remove(0, 7);
    String name = readNextArg(input);

    if (name == "") {
      Serial.println("delete takes the name!\n");
      showHelp = true;

      return;
    }

    Serial.print("Deleting \"");
    Serial.print(name);
    Serial.println("\" recording!");

    if (!LittleFS.remove("/records/" + name)) {
      Serial.println("FAILED!!!");
    }
  } else if(input.startsWith("play")) {
    input.remove(0, 5);
    String name = readNextArg(input);

    if (name == "") {
      Serial.println("play takes the name!\n");
      showHelp = true;

      return;
    }

    File f = LittleFS.open("/records/" + name, "r");
    if (f) {
      int len = f.size();
      byte buff[len];
      f.readBytes((char*)buff, len);
      f.close();
      
      int offset = 0;
      recordingInfo recordInfo;
      memcpy(&recordInfo, buff + offset, sizeof(recordInfo));
      offset += sizeof(recordInfo);

      delay(500);

      int dbg = 0;
      Serial.print("Playback baud: ");
      Serial.println(recordInfo.baud);
      Serial.print("Inverted: ");
      Serial.println(recordInfo.inverted);

      if (recordInfo.inverted) outputSerial.setInverted();
      outputSerial.begin(recordInfo.baud);

      playbackLen = len;
      playbackOffset = offset;
      playbackBuff = buff;
      playback = true;
      // Serial.println(readIntFromBytes(buff, 0));
    } else {
      Serial.println("Recording with that name doesn't exists!");
    }
  } else {
    Serial.println("Invalid command!");
    showHelp = true;
  }

  Serial.println();
}

byte* intToBytes(int n) {
  byte bytes[4];
  bytes[0] = (byte)(n >> 24);
  bytes[1] = (byte)(n >> 16);
  bytes[2] = (byte)(n >> 8);
  bytes[3] = (byte)n;

  return bytes;
}
int readIntFromBytes(byte* buff, int offset) {
  return (buff[offset] << 24) + (buff[offset + 1] << 16) + (buff[offset + 2] << 8) + buff[offset + 3];
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