#include "JsonSetting.h"

String JsonSetting::intVectorToString(const std::vector<int>& vec) {
    String result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += String(vec[i]);
        if (i < vec.size() - 1) {
            result += ",";
        }
    }
    return result;
}

bool JsonSetting::validate(String str) {
  switch (type) {
    case JsonSettingType::JST_INT_VECTOR:
      return validateIntVector(str);
    default:
      return true;
  }
}

bool JsonSetting::validateIntVector(String str) {
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == ',' || str[i] == '-') {
      continue;
    }
    if (str[i] < '0' || str[i] > '9') {
      lastValidationError = "Non-integer value found";
      return false;
    }
  }
  return true;
}
