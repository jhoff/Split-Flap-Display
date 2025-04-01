#pragma once

#include "JsonSetting.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <map>

class JsonSettings {
  public:
    JsonSettings(const char *name, std::map<String, JsonSetting> map) : name(name), map(map) {}

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

    String getLastValidationError() {
        return lastValidationError;
    }
    String getLastValidationKey() {
        return lastValidationKey;
    }

  private:
    const char *name;
    std::map<String, JsonSetting> map;

    String lastValidationError;
    String lastValidationKey;

    JsonSetting find(const char *key);

    Preferences preferences;
};
