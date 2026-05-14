// --- StorageDriver.h ---
#ifndef STORAGE_DRIVER_H
#define STORAGE_DRIVER_H

#include <Arduino.h>

class StorageDriver {
public:
    static void init();
    static void writeLog(const char* message);
};
#endif