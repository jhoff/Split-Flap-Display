#pragma once

// General Settings
#define DEFAULT_NAME "My Display"
#define DEFAULT_MDNS "splitflap"
#define DEFAULT_OTA_PASS ""
#define DEFAULT_TIMEZONE "UTC0"
#define DEFAULT_DATE_FORMAT "{dd}-{MM}-{yy}"
#define DEFAULT_TIME_FORMAT "{HH}:{mm}"

// WiFi Settings
#define DEFAULT_SSID ""
#define DEFAULT_PASSWORD ""

// MQTT Settings
#define DEFAULT_MQTT_SERVER ""
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_USER ""
#define DEFAULT_MQTT_PASS ""

// Hardware Settings
#define DEFAULT_MODULE_COUNT 8
#define DEFAULT_MAGNET_POSITION 730
#define DEFAULT_DISPLAY_OFFSET 0
#define DEFAULT_SDA_PIN 8
#define DEFAULT_SCL_PIN 9
#define DEFAULT_STEPS_PER_ROT 2048
#define DEFAULT_MAX_VEL 15.0f
#define DEFAULT_CHARSET 37
#define DEFAULT_TRANSITION_TYPE 0 // 0 = Normal, 1 = Synchronized Landing

#ifndef DEFAULT_MODULE_ADDRESSES
#define DEFAULT_MODULE_ADDRESSES {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27}
#endif

#ifndef DEFAULT_MODULE_OFFSETS
#define DEFAULT_MODULE_OFFSETS {0, 0, 0, 0, 0, 0, 0, 0}
#endif

// Operational States
#define DEFAULT_MODE 0
