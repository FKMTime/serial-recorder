#include <Arduino.h>
#include <LittleFS.h>
// #include <EEPROM.h>

String readSerialCommand();

bool recordStarted = false;
bool playbackStarted = false;
bool showHelp = true;

void setup() {
  LittleFS.begin();

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

void loop1() {
  while (!Serial) {
    showHelp = true;
    delay(1);
  }

  if (showHelp) {
    Serial.println("Execute function:");
    Serial.println("- record [name] [baud]");
    Serial.println("- play [name]");
    Serial.println("- list");
    Serial.println("- delete [name]");
    Serial.println("- restart");
    Serial.println();

    showHelp = false;
  }
  String input = readSerialCommand();

  if (input == "restart") {
    rp2040.reboot();
  } else if(input.startsWith("record ")) {
    input.remove(0, 7);
    Serial.print("\"");
    Serial.print(input);
    Serial.println("\"");
  } else {
    Serial.println("Invalid command!");
    Serial.println();

    showHelp = true;
  }
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