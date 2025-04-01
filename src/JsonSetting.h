#pragma once

#include <Preferences.h>
#include <vector>

typedef enum {
    JST_STR,
    JST_INT,
    JST_FLOAT,
    JST_INT_VECTOR
} JsonSettingType;

class JsonSetting {
  public:
    JsonSetting(String strDefault) : type(JsonSettingType::JST_STR), strDefault(strDefault) {}
    JsonSetting(int intDefault) : type(JsonSettingType::JST_INT), intDefault(intDefault) {}
    JsonSetting(float floatDefault) : type(JsonSettingType::JST_FLOAT), floatDefault(floatDefault) {
        strDefault = String(floatDefault);
    }
    JsonSetting(std::vector<int> intVectorDefault)
        : type(JsonSettingType::JST_INT_VECTOR), intVectorDefault(intVectorDefault) {
        strDefault = intVectorToString(intVectorDefault);
    }

    bool validate(String str);
    String getLastValidationError() { return lastValidationError; }

  private:
    JsonSettingType type;

    String strDefault;
    int intDefault;
    float floatDefault;
    std::vector<int> intVectorDefault;

    String intVectorToString(const std::vector<int> &vec);

    String lastValidationError;
    bool validateIntVector(String str);

    friend class JsonSettings;
};
