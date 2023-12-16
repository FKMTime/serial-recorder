#include <Arduino.h>
#include <LittleFS.h>

#include "utils.hpp"
#include "structs.h"
#include "player/player.h"
#include "recorder/recorder.h"

#define SERIAL_PIN 17

void playCommand(String input);
void deleteCommand(String input);
void recordCommand(String input);
void stopCommand();
void listCommand();
void statusCommand();
void interprateCommand(String &input);

Player player(SERIAL_PIN);
Recorder recorder(SERIAL_PIN);

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
  recorder.loop(); // BLOCKING
  player.loop();   // BLOCKING
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
    Serial.println("- status");
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
  } else if(input == "status") {
    statusCommand();
  } else {
    Serial.println("Invalid command!");
    showHelp = true;
  }

  Serial.println();
}

void statusCommand() {
  if (!player.state()) {
    Serial.println("Nothing is currently playing!");
    return;
  }

  player.printStatus();
}

void playCommand(String input) {
  String name = readNextArg(input);
  String loopString = readNextArg(input);

  if (name == "") {
    Serial.println("play takes the name!\n");
    showHelp = true;
    return;
  }

  player.play(name, loopString.toInt() == 1);
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

  recorder.record(rInfo, name);
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
  if (recorder.state()) {
    recorder.stop();
  } else if (player.state()) {
    player.stop();
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
    File f = dir.openFile("r");

    byte buff[dir.fileSize()];
    f.readBytes((char*)buff, dir.fileSize());
    f.close();

    Serial.print(" - ");
    Serial.print(dir.fileName());
    Serial.print(" | Size: ");
    Serial.println(dir.fileSize());
    // Serial.print(" | Time: ");
    // Serial.println(getRecordingDuration(buff, dir.fileSize()));
  }
}