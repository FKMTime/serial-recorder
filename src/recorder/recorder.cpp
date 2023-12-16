#include "recorder.h"

bool Recorder::state() {
    return recording;
}

void Recorder::stop() {
    recording = false;
    Serial.println("Stopped the recording and saved the file!");
}

void Recorder::record(RecordingInfo info, String name) {
    file = LittleFS.open("/records/" + name, "w");
    file.write((byte*) &info, sizeof(info));
    
    if (info.inverted) serial.setInverted();
    serial.begin(info.baud);

    recording = true;
    lastTime = millis();

    Serial.print("Starting recording to file with name: ");
    Serial.println(name);
}

void Recorder::loop() {
    if (!recording) return;

    RecordingFrame frame;
    while (recording) {
        if(serial.available()) {
            frame.delta = millis() - lastTime;
            frame.data = serial.read();
            frame.cheksum = frame.delta ^ frame.data & 0xFF;
            file.write((byte*)&frame, sizeof(frame));

            lastTime = millis();
        }
    }

    serial.end();

    // STOP FRAME
    frame.delta = 65535;
    frame.data = 0;
    frame.cheksum = 0;
    file.write((byte*)&frame, sizeof(frame));
    file.close();

    Serial.println("Recording has stopped!");
    Serial.println();
    Serial.print("> ");
}