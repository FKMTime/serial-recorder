#include <Arduino.h>
#include <LittleFS.h>
#include <SoftwareSerial.h>

#include "utils.hpp"

void playCommand(String input);
void deleteCommand(String input);
void recordCommand(String input);
void stopCommand();
void listCommand();
void interprateCommand(String &input);

SoftwareSerial inputSerial(17, -1);
SoftwareSerial outputSerial(-1, 17);

bool recording = false;
unsigned long lastRecordingTime = 0;
File recordFile;

bool playback = false;
bool playbackLoop = false;
byte* playbackBuff;
int playbackOffset = 0;
int playbackLen = 0;

bool showHelp = true;

void setup() {
  LittleFS.begin();
  if (!LittleFS.exists("/records")) {
    LittleFS.mkdir("/records");
  }
}

void setup1() {
  Serial.begin(115200);
}

void loop() {
  if (recording) {
    RecordingFrame frame;
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
    RecordingFrame frame;
    while (playbackLen > playbackOffset) {
      memcpy(&frame, playbackBuff + playbackOffset, sizeof(frame));
      playbackOffset += sizeof(RecordingFrame);

      // STOP FRAME
      if (frame.delta == -1 && frame.cheksum == 0) {
        if(playbackLoop) {
          playbackOffset = sizeof(RecordingInfo);
          continue;
        }
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
    Serial.println("- record [name] <baud> <inverted 0|1>");
    Serial.println("- stop");
    Serial.println("- play [name] <loop 0|1>");
    Serial.println("- delete [name]");
    Serial.println("- list");
    Serial.println("- reboot");
    Serial.println();

    showHelp = false;
  }

  String input = readSerialCommand();
  interprateCommand(input);
}

void interprateCommand(String &input) {
  if (input == "reboot") {
    rp2040.reboot();
  } else if (input == "list") {
    listCommand();
  } else if (input == "stop") {
    stopCommand();
  } else if(input.startsWith("record")) {
    input.remove(0, 7);
    recordCommand(input);
  } else if(input.startsWith("delete")) {
    input.remove(0, 7);
    deleteCommand(input);
  } else if(input.startsWith("play")) {
    input.remove(0, 5);
    playCommand(input);
  } else {
    Serial.println("Invalid command!");
    showHelp = true;
  }

  Serial.println();
}

void playCommand(String input) {
  String name = readNextArg(input);
  String loopString = readNextArg(input);

  if (name == "") {
    Serial.println("play takes the name!\n");
    showHelp = true;
    return;
  }

  playbackLoop = loopString.toInt() == 1;

  File f = LittleFS.open("/records/" + name, "r");
  if (!f) {
    Serial.println("Recording with that name doesn't exists!");
    return;
  }

  int len = f.size();
  byte buff[len];
  f.readBytes((char*)buff, len);
  f.close();
     
  int offset = 0;
  RecordingInfo recordInfo;
  memcpy(&recordInfo, buff + offset, sizeof(recordInfo));
  offset += sizeof(recordInfo);

  delay(500);

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
}

void recordCommand(String input) {
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

  RecordingInfo rInfo;
  rInfo.baud = baudRate;
  rInfo.inverted = inverted;
  recordFile.write((byte*) &rInfo, sizeof(rInfo));

  recording = true;
  lastRecordingTime = millis();
}

void deleteCommand(String input) {
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
}

void stopCommand() {
  if (recording) {
    recording = false;
    Serial.println("Stopped the recording and saved the file!");
  } else if (playback) {
    playback = false;
    Serial.println("Stopped the playback!");
  } else {
    Serial.println("Recording/Playback isn't started!");
  }
}

void listCommand() {
  Serial.println("Disk usage (used / total): ");

  FSInfo64 fsinfo;
  LittleFS.info64(fsinfo);

  Serial.print(fsinfo.usedBytes);
  Serial.print(" / ");
  Serial.println(fsinfo.totalBytes);
  Serial.println();

  Serial.println("Records: ");

  fs::Dir dir = LittleFS.openDir("/records");
  while(dir.next()) {
    Serial.print(" - ");
    Serial.print(dir.fileName());
    Serial.print(" | Size: ");
    Serial.println(dir.fileSize());
  }
}