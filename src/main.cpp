#include <Arduino.h>
#include <LittleFS.h>
// #include <EEPROM.h>

void setup() {
  Serial.begin(115200);

  LittleFS.begin();
  // EEPROM.begin(64);

  // File file = LittleFS.open("/test.txt", "a+");
  // if (file) {
  //   file.print('.');
  //   file.close();
  // }
}

void loop() {
  // delay(1000);

  // FSInfo64 fsinfo;
  // LittleFS.info64(fsinfo);
  // Serial.print(fsinfo.usedBytes);
  // Serial.print(" / ");
  // Serial.println(fsinfo.totalBytes);

  // fs::Dir dir = LittleFS.openDir("/");
  // Serial.println("Files: ");
  // while(dir.next()) {
  //   Serial.print(" - ");
  //   Serial.print(dir.fileName());
  //   Serial.print(" / ");
  //   Serial.println(dir.fileSize());
  // }
}