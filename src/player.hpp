#include <LittleFS.h>
#include <SoftwareSerial.h>
#include "structs.h"

SoftwareSerial outputSerial(-1, 17);

bool playback = false;
bool playbackLoop = false;
byte* playbackBuff;
int playbackOffset = 0;
int playbackLen = 0;

void playerInit(String name, bool loop) {
    playbackLoop = loop;

    File f = LittleFS.open("/records/" + name, "r");
    if (!f) {
        Serial.println("Recording with that name doesn't exists!");
        return;
    }

    int len = f.size();
    byte buff[len];
    f.readBytes((char*)buff, len);
    f.close();
        
    RecordingInfo recordInfo;
    memcpy(&recordInfo, buff, sizeof(RecordingInfo));

    Serial.print("Playback baud: ");
    Serial.print(recordInfo.baud);
    Serial.print(" | Inverted: ");
    Serial.println(recordInfo.inverted);

    if (recordInfo.inverted) outputSerial.setInverted();
    outputSerial.begin(recordInfo.baud);

    playbackOffset = sizeof(RecordingInfo);
    playbackLen = len;
    playbackBuff = buff;
    playback = true;
}

void play() {
    RecordingFrame frame;
    while (playbackLen > playbackOffset) {
        memcpy(&frame, playbackBuff + playbackOffset, sizeof(RecordingFrame));
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
        if (frame.cheksum != cheksum) continue;

        delay(frame.delta);
        outputSerial.write(frame.data);
    }

    playback = false;
    outputSerial.end();

    Serial.println("Playback has stopped!");
    Serial.println();
    Serial.print("> ");
}