#include <Arduino.h>
#include <LittleFS.h>
// #include <EEPROM.h>

void interprateCommand(String &input);
String readSerialCommand();
String readNextArg(String &input);

bool record = false;
bool playback = false;
bool showHelp = true;

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
  // delay(1000);
}

void loop1() {
  while (!Serial) {
    showHelp = true;
    delay(1);
  }

  if (showHelp) {
    Serial.println("Execute function:");
    Serial.println("- record [name] <baud>");
    Serial.println("- play [name]");
    Serial.println("- list");
    Serial.println("- delete [name]");
    Serial.println("- restart");
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
  } else if(input.startsWith("record")) {
    input.remove(0, 7);
    String name = readNextArg(input);
    String baud = readNextArg(input);

    if (name == "") {
      Serial.println("record takes at least name!\n");
      showHelp = true;

      return;
    }

    int baudRate = baud.toInt();
    if (baudRate == 0) baudRate = 115200;

    Serial.println(name);
    Serial.println(baudRate);

    File f = LittleFS.open("/records/" + name, "w");
    f.println(baudRate);
    f.close();
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
      String contents = f.readString();
      f.close();

      Serial.println("Contents: ");
      Serial.println(contents);
    } else {
      Serial.println("Recording with that name doesn't exists!");
    }
  } else {
    Serial.println("Invalid command!");
    showHelp = true;
  }

  Serial.println();
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