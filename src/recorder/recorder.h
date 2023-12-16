#ifndef __RECORDER_H__
#define __RECORDER_H__

#include <SoftwareSerial.h>
#include <LittleFS.h>
#include "structs.h"

class Recorder {
  public:
    Recorder(int pin) : serial(pin, -1) {}

    void record(RecordingInfo info, String name);
    void loop();
    void stop();

    bool state();

  private:
    SoftwareSerial serial;

    bool recording = false;
    unsigned long lastTime = 0;
    File file;
};

#endif