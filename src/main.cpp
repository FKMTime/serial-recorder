#include <Arduino.h>
#include <LittleFS.h>
#include <SoftwareSerial.h>

#include "recorder.hpp"
#include "player.hpp"
#include "utils.hpp"
#include "structs.h"

void playCommand(String input);
void deleteCommand(String input);
void recordCommand(String input);
void stopCommand();
void listCommand();
void interprateCommand(String &input);

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
    record(); // IT LOOPS - SO ITS BLOCKING
  }

  if(playback) {
    play();   // IT LOOPS - SO ITS BLOCKING
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

  playerInit(name, loopString.toInt() == 1);
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

  RecordingInfo rInfo;
  rInfo.baud = baudRate;
  rInfo.inverted = inverted;

  recordInit(rInfo, name);
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