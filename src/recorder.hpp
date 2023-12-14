#include <LittleFS.h>
#include <SoftwareSerial.h>
#include "structs.h"

SoftwareSerial inputSerial(17, -1);
bool recording = false;
unsigned long lastRecordingTime = 0;
File recordFile;

void recordInit(RecordingInfo info, String name) {
    recordFile = LittleFS.open("/records/" + name, "w");
    recordFile.write((byte*) &info, sizeof(info));
    
    if (info.inverted) inputSerial.setInverted();
    inputSerial.begin(info.baud);

    recording = true;
    lastRecordingTime = millis();

    Serial.print("Starting recording to file with name: ");
    Serial.println(name);
}

void record() {
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