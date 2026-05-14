// --- AudioDriver.h ---
#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <Arduino.h>

class AudioDriver {
public:
    static void init();
    static void playBeep();
};
#endif