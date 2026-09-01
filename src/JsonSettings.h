#pragma once

#include "JsonSetting.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <map>

class JsonSettings {
  public:
    JsonSettings(const char *name, std::map<String, JsonSetting> map)
        : name(name), map(map), mutex(xSemaphoreCreateRecursiveMutex()) {}

    String getString(const char *key);
    int getInt(const char *key);
    float getFloat(const char *key);
    std::vector<int> getIntVector(const char *key);

    void putString(const char *key, String value);
    void putInt(const char *key, int value);
    void putFloat(const char *key, float value);
    void putIntVector(const char *key, std::vector<int> value);

    JsonDocument toJson();
    bool fromJson(JsonDocument settings);
    bool reset();

    String getLastValidationError() { return lastValidationError; }
    String getLastValidationKey() { return lastValidationKey; }

  private:
    const char *name;
    std::map<String, JsonSetting> map;

    String lastValidationError;
    String lastValidationKey;

    JsonSetting find(const char *key);

    // Every accessor below opens and closes NVS on the single `preferences`
    // instance. The Arduino loop task reads settings (getMode() runs on every
    // pass through loop()) while the AsyncTCP task writes them from the HTTP
    // handlers, so without this lock one task's end() closes the handle the
    // other is mid-way through using: reads silently fall back to the compiled
    // default and writes are lost while the request still answers "success".
    //
    // Recursive because reset() calls fromJson(toJson()), and both take it.
    SemaphoreHandle_t mutex;

    class Guard {
      public:
        explicit Guard(SemaphoreHandle_t mutex) : mutex(mutex) {
            xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
        }
        ~Guard() {
            xSemaphoreGiveRecursive(mutex);
        }

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

      private:
        SemaphoreHandle_t mutex;
    };

    Preferences preferences;
};
