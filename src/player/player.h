#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <SoftwareSerial.h>

class Player {
  public:
    Player(int pin) : serial(-1, pin) {}

    void play(String _name, bool _loop);
    void loop();
    void stop();
    void printStatus();

    bool state();

  private:
    SoftwareSerial serial;

    bool playback = false;
    bool playLoop = false;

    String name;

    uint8_t* buff;
    int offset = 0;
    int size = 0;

    int getRecordingDuration(byte* buffer, int length);
};

#endif