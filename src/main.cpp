#include <Arduino.h>
#include <LittleFS.h>
// #include <EEPROM.h>

byte* intToBytes(int n);
int readIntFromBytes(byte* buff, int offset);
void interprateCommand(String &input);
String readSerialCommand();
String readNextArg(String &input);

bool recording = false;
unsigned long recordingStartedTime = 0;
File recordFile;

bool playback = false;
bool showHelp = true;

struct recordingInfo{
  int baud;
};

struct recordingFrame {
  int delta;
  int data;
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
  delay(100);

  if (recording) {
    recordingFrame frame;
    frame.delta = millis() - recordingStartedTime;
    frame.data = 69420;
    recordFile.write((byte*)&frame, sizeof(frame));
  }
}

void loop1() {
  while (!Serial) {
    showHelp = true;
    delay(1);
  }

  if (showHelp) {
    Serial.println("Execute function:");
    Serial.println("- record [name] <baud> // starts the recording");
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
      recordFile.close();

      Serial.println("Stopped the recording and saved the file!");
    } else {
      Serial.println("Recording isn't started!");
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

    Serial.print("Starting recording to file with name: ");
    Serial.println(name);

    recordFile = LittleFS.open("/records/" + name, "w");
    recordingInfo rInfo;
    rInfo.baud = baudRate;

    recordFile.write((byte*) &rInfo, sizeof(rInfo));
    recording = true;
    recordingStartedTime = millis();
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
      recordingInfo dsInfo;
      memcpy(&dsInfo, buff + offset, sizeof(dsInfo));
      offset += sizeof(dsInfo);

      Serial.println(dsInfo.baud);

      recordingFrame frame;
      while (len > offset) {
        memcpy(&frame, buff + offset, sizeof(frame));
        offset += sizeof(frame);

        Serial.println("Frame: ");
        Serial.println(frame.delta);
        Serial.println(frame.data);
      }
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