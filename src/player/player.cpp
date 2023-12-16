#include <LittleFS.h>
#include "player.h"
#include "structs.h"

bool Player::state() {
    return playback;
}

void Player::stop() {
    playback = false;
    Serial.println("Stopped the playback!");
}

void Player::play(String _name, bool _loop) {
    name = _name;
    playLoop = _loop;

    File f = LittleFS.open("/records/" + name, "r");
    if (!f) {
        Serial.println("Recording with that name doesn't exists!");
        return;
    }

    size = f.size();  
    offset = sizeof(RecordingInfo);
    buff = (byte*)malloc(size);
    memset(buff, 0, size);

    f.read(buff, size);
    f.close();
        
    RecordingInfo recordInfo;
    memcpy(&recordInfo, buff, sizeof(RecordingInfo));

    Serial.print("Playback baud: ");
    Serial.print(recordInfo.baud);
    Serial.print(" | Inverted: ");
    Serial.print(recordInfo.inverted ? "true" : "false");
    Serial.print(" | Loop: ");
    Serial.println(playLoop ? "true" : "false");

    if (recordInfo.inverted) serial.setInverted();
    serial.begin(recordInfo.baud);

    playback = true;
}

void Player::loop() {
    if (!playback) return;

    RecordingFrame frame;
    while (offset < size && playback) {
        memcpy(&frame, buff + offset, sizeof(RecordingFrame));
        offset += sizeof(RecordingFrame);

        // STOP FRAME
        if (frame.delta == -1 && frame.cheksum == 0) break;

        uint8_t cheksum = frame.delta ^ frame.data & 0xFF;
        if (frame.cheksum != cheksum) continue;

        delay(frame.delta);
        serial.write(frame.data);
    }

    if(playLoop && playback) {
        offset = sizeof(RecordingInfo);
        loop();

        return;
    }

    playback = false;
    serial.write(255);
    serial.end();

    Serial.println("Playback has stopped!");
    Serial.println();
    Serial.print("> ");
}

void Player::printStatus() {
    Serial.print("Currently playing: ");
    Serial.println(name);

    int totalTime = getRecordingDuration(buff, size);
    int currentTime = getRecordingDuration(buff, offset);
    Serial.print("Playback time: ");
    Serial.print(currentTime);
    Serial.print("ms / ");
    Serial.print(totalTime);
    Serial.print("ms (");
    Serial.print((currentTime / (double)totalTime) * 100);
    Serial.println("%)");
}

int Player::getRecordingDuration(byte* buffer, int length) {
  RecordingFrame frame;
  
  int durOffset = sizeof(RecordingInfo);
  int tmpSum = 0;
  while (durOffset < length) {
      memcpy(&frame, buffer + durOffset, sizeof(RecordingFrame));
      durOffset += sizeof(RecordingFrame);

      // STOP FRAME
      if (frame.delta == 65535 && frame.cheksum == 0) break;

      uint8_t cheksum = frame.delta ^ frame.data & 0xFF;
      if (frame.cheksum != cheksum) continue;

      tmpSum += frame.delta;
  }

  return tmpSum;
}