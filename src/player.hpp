#include <LittleFS.h>
#include <SoftwareSerial.h>
#include "structs.h"
#include "utils.hpp"

SoftwareSerial outputSerial(-1, 17);

bool playback = false;

bool playbackLoop = false;
String playbackName;

byte* playbackBuff;
int playbackOffset = 0;
int playbackLen = 0;

void playerInit(String name, bool loop) {
    playbackName = name;
    playbackLoop = loop;

    File f = LittleFS.open("/records/" + name, "r");
    if (!f) {
        Serial.println("Recording with that name doesn't exists!");
        return;
    }

    playbackLen = f.size();  
    playbackOffset = sizeof(RecordingInfo);
    playbackBuff = (byte*)malloc(playbackLen);
    memset(playbackBuff, 0, playbackLen);

    // TODO: read file on playback (don't copy file to ram)
    f.read(playbackBuff, playbackLen);
    f.close();
        
    RecordingInfo recordInfo;
    memcpy(&recordInfo, playbackBuff, sizeof(RecordingInfo));

    Serial.print("Playback baud: ");
    Serial.print(recordInfo.baud);
    Serial.print(" | Inverted: ");
    Serial.print(recordInfo.inverted ? "true" : "false");
    Serial.print(" | Loop: ");
    Serial.println(playbackLoop ? "true" : "false");

    if (recordInfo.inverted) outputSerial.setInverted();
    outputSerial.begin(recordInfo.baud);

    playback = true;
}

void play() {
    RecordingFrame frame;
    while (playbackLen > playbackOffset && playback) {
        memcpy(&frame, playbackBuff + playbackOffset, sizeof(RecordingFrame));
        playbackOffset += sizeof(RecordingFrame);

        // STOP FRAME
        if (frame.delta == -1 && frame.cheksum == 0) {
            break;
        }

        uint8_t cheksum = frame.delta ^ frame.data & 0xFF;
        if (frame.cheksum != cheksum) continue;

        delay(frame.delta);
        outputSerial.write(frame.data);
    }

    if(playbackLoop && playback) {
        playbackOffset = sizeof(RecordingInfo);
        play();

        return;
    }

    playback = false;
    outputSerial.end();

    Serial.println("Playback has stopped!");
    Serial.println();
    Serial.print("> ");
}

void printPlaybackStatus() {
    Serial.println("Playback status:");

    Serial.print("Currently playing: ");
    Serial.println(playbackName);

    int totalTime = getRecordingDuration(playbackBuff, playbackLen);
    int currentTime = getRecordingDuration(playbackBuff, playbackOffset);
    Serial.print("Playback time: ");
    Serial.print(currentTime);
    Serial.print("ms / ");
    Serial.print(totalTime);
    Serial.print("ms (");
    Serial.print((currentTime / (double)totalTime) * 100);
    Serial.println("%)");
}