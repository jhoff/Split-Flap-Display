#include "SplitFlapWebServer.h"
#include "ArduinoJson.h"
#include "AsyncJson.h"

#define AP_SSID "Split Flap Display"

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

// Constructor
SplitFlapWebServer::SplitFlapWebServer(JsonSettings &settings)
    : settings(settings), server(80), multiWordDelay(1000),
      rebootRequired(false), attemptReconnect(false), multiWordCurrentIndex(0),
      numMultiWords(0), wifiCheckInterval(1000), connectionMode(0),
      checkDateInterval(250), centering(1) {
  lastSwitchMultiTime = millis();
}

void SplitFlapWebServer::init() {
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  setTimezone();
}

void SplitFlapWebServer::setTimezone() {
  const char *timezoneSetting = settings.getString("timezone").c_str();
  const char *sntpServer = "pool.ntp.org";
  const char *posixTimezone = "UTC0";

  File file = LittleFS.open("/timezones.json", "r");
  if (!file) {
    Serial.println("Failed to open timezones.json; defaulting to UTC");
    return configTzTime(posixTimezone, sntpServer);
  }

  size_t size = file.size();
  std::unique_ptr<char[]> buffer(new char[size]);
  file.readBytes(buffer.get(), size);
  file.close();

  JsonDocument timezones;
  deserializeJson(timezones, buffer.get());

  for (JsonPair kv : timezones.as<JsonObject>()) {
    if (kv.key().c_str() == timezoneSetting) {
      posixTimezone = kv.value().as<String>().c_str();
      break;
    }
  }

  configTzTime(posixTimezone, sntpServer);
}

// Totally didn't use AI to make these functions
//  Function to get current minute as a string
String SplitFlapWebServer::getCurrentMinute() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "";
  }
  char minuteStr[3];                           // Max "59" + null terminator
  sprintf(minuteStr, "%02d", timeinfo.tm_min); // Format as two-digit string
  return String(minuteStr);
}

// Function to get current hour as a string
String SplitFlapWebServer::getCurrentHour() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "";
  }
  char hourStr[3];                            // Max "59" + null terminator
  sprintf(hourStr, "%02d", timeinfo.tm_hour); // Format as two-digit string
  return String(hourStr);
}

// Function to get the first n characters of the day
String SplitFlapWebServer::getDayPrefix(int n) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Err"; // Return error if time not available
  }

  // Get full weekday name
  char fullDay[10]; // Buffer for full day name
  strftime(fullDay, sizeof(fullDay), "%A", &timeinfo);

  // Extract first n characters
  char dayPrefix[n + 1];
  strncpy(dayPrefix, fullDay, n);
  dayPrefix[n] = '\0'; // Null-terminate the string

  return String(dayPrefix);
}

// Function to get the first n characters of the month
String SplitFlapWebServer::getMonthPrefix(int n) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Err"; // Return error if time not available
  }

  // Get full month name
  char fullMonth[10]; // Buffer for full month name
  strftime(fullMonth, sizeof(fullMonth), "%B", &timeinfo);

  // Extract first n characters
  char monthPrefix[n + 1];
  strncpy(monthPrefix, fullMonth, n);
  monthPrefix[n] = '\0'; // Null-terminate the string

  return String(monthPrefix);
}

String SplitFlapWebServer::getCurrentDay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Err"; // Return error if time is not available
  }

  char dayStr[3]; // Buffer for the day number (max "31" + null terminator)
  sprintf(dayStr, "%02d", timeinfo.tm_mday); // Format as two-digit string

  return String(dayStr);
}

void SplitFlapWebServer::setMode(int targetMode) {
  settings.putInt("mode", targetMode);
}

int SplitFlapWebServer::getMode() { return settings.getInt("mode"); }

void SplitFlapWebServer::checkWiFi() {
  if (connectionMode == 1) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi lost! Forcing reconnect...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}

bool SplitFlapWebServer::loadWiFiCredentials() {
  // Allow WIFI_SSID and WIFI_PASS to be overridden by compile-time definitions
  String ssid = String(WIFI_SSID).isEmpty() ? settings.getString("ssid")
                                            : String(WIFI_SSID);
  String password = String(WIFI_PASS).isEmpty() ? settings.getString("password")
                                                : String(WIFI_PASS);

  if (ssid != "" && password != "") {
    Serial.println("Wi-Fi credentials loaded successfully.");
    Serial.print("Connecting to Network: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
#ifdef WIFI_TX_POWER
    delay(100);
    WiFi.setTxPower((wifi_power_t)WIFI_TX_POWER);
#endif
    WiFi.begin(ssid.c_str(), password.c_str());
    return true; // Return true if credentials exist
  }
  return false; // Return false if no credentials were found
}

void SplitFlapWebServer::checkRebootRequired() {
  if (rebootRequired) {
    Serial.println("Reboot required. Restarting...");
    delay(1000);
    ESP.restart();
  }
}

void SplitFlapWebServer::handleOta() { ArduinoOTA.handle(); }
void SplitFlapWebServer::enableOta() {
  // Skip OTA initialisation if no password is set
  if (settings.getString("otaPass") == "") {
    return;
  }
  ArduinoOTA.setHostname(
      settings.getString("mdns")
          .c_str()); // otherwise mdns name gets overwritten with default
  ArduinoOTA.setPassword(settings.getString("otaPass").c_str());

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "sketch";
        } else { // U_LITTLEFS
          type = "filesystem";
          LittleFS.end(); // Unmount the filesystem before update
        }
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
        LittleFS.begin(); // Remount filesystem
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        LittleFS.begin(); // Remount filesystem
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
        }
      });

  ArduinoOTA.begin();
  Serial.println("OTA Initialized");
}

bool SplitFlapWebServer::connectToWifi() {
  if (loadWiFiCredentials()) {

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 20000; // 20 seconds
    unsigned long lastPrintTime = startAttemptTime;

    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startAttemptTime >= timeout) {
        Serial.println("_");
        Serial.println("Wi-Fi connection failed! Timeout reached.");
        return false; // Return false if unable to connect in 30 seconds
      }
      if ((millis() - lastPrintTime) > 1000) {
        Serial.print(".");
        lastPrintTime = millis();
      }
      yield();
    }

    // connected succesfully
    connectionMode = 1;
    WiFi.softAPdisconnect(); // Turns off SoftAP mode only after connected to
                             // actual network
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true); // Saves Wi-Fi settings to flash memory
    WiFi.setSleep(false);
    Serial.println("Connected to Wi-Fi!");
    Serial.println("IP Address: http://" + WiFi.localIP().toString());
    return true;
  }
  return false;
}

void SplitFlapWebServer::startAccessPoint() {
  connectionMode = 0;
  const char *apSSID = AP_SSID;
  WiFi.softAP(apSSID);
#ifdef WIFI_TX_POWER
  delay(100);
  WiFi.setTxPower((wifi_power_t)WIFI_TX_POWER);
#endif
  Serial.println("AP Mode Started!");
  Serial.println("Connect to: " + String(apSSID));
  Serial.println("AP IP Address: http://" + WiFi.softAPIP().toString());
}

void fourOhFour(AsyncWebServerRequest *request) {
  Serial.println("Request: " + request->url());
  Serial.println("Method: " + String(request->methodToString()));
  request->send(404);
}

void SplitFlapWebServer::endMDNS() {
  MDNS.end();
  Serial.println("mDNS responder stopped");
}

void SplitFlapWebServer::startMDNS() {
  if (!MDNS.begin(settings.getString("mdns").c_str())) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("mDNS: http://" + settings.getString("mdns") + ".local");
}

void SplitFlapWebServer::startWebServer() {
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->redirect("/index.html");
  });

  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory or not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (String(file.name()).endsWith(".gz")) {
      const char *filename = file.name();
      String tempFilename = (String("/") + String(filename));
      tempFilename.replace(".gz", "");
      filename = tempFilename.c_str();

      server.serveStatic(filename, LittleFS, filename, "max-age=600");
    }
    file = root.openNextFile();
  }

  server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, "application/json", settings.toJson().as<String>());
  });

  server.on(
      "/settings/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        settings.reset();

        JsonDocument response;
        response["message"] = "Settings reset successfully! Reconnect to the " +
                              String(AP_SSID) + " network";
        response["persistent"] = true;

        request->send(200, "application/json", response.as<String>());

        this->attemptReconnect = true;
      });

  server.addHandler(new AsyncCallbackJsonWebHandler(
      "/settings", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (request->method() != HTTP_POST) {
          return request->send(405, "application/json",
                               "{\"error\":\"Method Not Allowed\"}");
        }

        Serial.println("Received settings update request");
        Serial.println(json.as<String>());

        bool rebootRequired = false;
        bool reconnect = false;
        JsonDocument response;
        response["message"] = "Settings saved successfully!";

        // TODO Refactor this it's gross
        if ((json["ssid"].is<String>() &&
             json["ssid"].as<String>() != settings.getString("ssid")) ||
            (json["password"].is<String>() &&
             json["password"].as<String>() != settings.getString("password"))) {
          reconnect = true;
          response["message"] = "Settings updated successfully, Network "
                                "settings have changed, reconnect to the " +
                                json["ssid"].as<String>() + " network";
        }

        if (json["otaPass"].is<String>() &&
            json["otaPass"].as<String>() != settings.getString("otaPass")) {
          rebootRequired =
              true; // OTA password change can only be applied by rebooting
          response["message"] = "Settings updated successfully, OTA Password "
                                "has changed. Rebooting...";
        }

        if (json["mdns"].is<String>() &&
            json["mdns"].as<String>() != settings.getString("mdns")) {
          reconnect = true;
          response["message"] =
              "Settings updated successfully, mDNS name has changed, "
              "automatically redirecting to http://" +
              json["mdns"].as<String>() + ".local...";
          response["redirect"] =
              "http://" + json["mdns"].as<String>() + ".local/settings.html";
        }

        if (!settings.fromJson(json)) {
          response["message"] = "Failed to save settings";
          response["type"] = "error";
          response["errors"]["key"] = settings.getLastValidationKey();
          response["errors"]["message"] = settings.getLastValidationError();
          return request->send(400, "application/json", response.as<String>());
        }

        response["type"] = "success";
        response["persistent"] = reconnect;

        request->send(200, "application/json", response.as<String>());

        this->rebootRequired = rebootRequired;
        this->attemptReconnect = reconnect;
      }));

  server.addHandler(new AsyncCallbackJsonWebHandler(
      "/text", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (request->method() != HTTP_POST) {
          return request->send(405, "application/json",
                               "{\"error\":\"Method Not Allowed\"}");
        }

        Serial.println("Received text update request");
        Serial.println(json.as<String>());

        // {"mode":"single","words":["adfasdf"],"delay":1,"center":false}
        // {"mode":"multiple","words":["asdf","asdfasdf","fffff"],"delay":"14","center":true}
        JsonDocument response;

        if (!json["mode"].is<String>()) {
          response["message"] = "Invalid mode type";
        }

        if (!json["words"].is<JsonArray>()) {
          response["message"] = "Invalid words array";
        }

        float delay = json["delay"].as<float>();
        if (delay < 1) {
          response["message"] = "Invalid delay type / value";
        }

        if (!json["center"].is<bool>()) {
          response["message"] = "Invalid center type";
        }

        if (response["message"].is<String>()) {
          response["type"] = "error";
          return request->send(400, "application/json", response.as<String>());
        }

        this->setMultiDelay(delay * 1000);
        Serial.println("Delay: " + String(this->getMultiWordDelay()));

        centering = json["center"].as<bool>() ? 1 : 0;
        Serial.println("centering: " + String(centering ? "true" : "false"));

        if (json["mode"] == "single") {
          String word = decodeURIComponent(json["words"][0].as<String>());
          Serial.println("Single Word: " + word);
          this->setInputString(word);
          this->setMode(0); // change mode last once all variables updated
        }

        if (json["mode"] == "multiple") {
          JsonArray wordsArray = json["words"].as<JsonArray>();
          String words = "";
          for (JsonVariant v : wordsArray) {
            words += decodeURIComponent(v.as<String>()) + ",";
          }
          if (words.length() > 0) {
            words.remove(words.length() - 1);
          }

          this->setMultiInputString(words);
          this->numMultiWords = wordsArray.size();
          Serial.println("Multiple Words: " + words);
          Serial.println("Number of Words: " + String(this->numMultiWords));

          this->setMode(1);
        }

        response["message"] = "Text updated successfully!";
        response["type"] = "success";

        request->send(200, "application/json", response.as<String>());
      }));

  server.onNotFound(fourOhFour);

  server.begin();
}

String SplitFlapWebServer::decodeURIComponent(String encodedString) {
  String decodedString = encodedString;
  // Replace common URL-encoded characters with their actual symbols
  decodedString.replace("%20", " ");  // space
  decodedString.replace("%21", "!");  // exclamation mark
  decodedString.replace("%22", "\""); // double quote
  decodedString.replace("%23", "#");  // hash
  decodedString.replace("%24", "$");  // dollar sign
  decodedString.replace("%25", "%");  // percent
  decodedString.replace("%26", "&");  // ampersand
  decodedString.replace("%27", "'");  // single quote
  decodedString.replace("%28", "(");  // left parenthesis
  decodedString.replace("%29", ")");  // right parenthesis
  decodedString.replace("%2A", "*");  // asterisk
  decodedString.replace("%2B", "+");  // plus
  decodedString.replace("%2C", ",");  // comma
  decodedString.replace("%2D", "-");  // hyphen
  decodedString.replace("%2E", ".");  // period
  decodedString.replace("%2F", "/");  // forward slash
  decodedString.replace("%3A", ":");  // colon
  decodedString.replace("%3B", ";");  // semicolon
  decodedString.replace("%3C", "<");  // less than
  decodedString.replace("%3D", "=");  // equal sign
  decodedString.replace("%3E", ">");  // greater than
  decodedString.replace("%3F", "?");  // question mark
  decodedString.replace("%40", "@");  // at symbol
  decodedString.replace("%5B", "[");  // left bracket
  decodedString.replace("%5C", "\\"); // backslash
  decodedString.replace("%5D", "]");  // right bracket
  decodedString.replace("%5E", "^");  // caret
  decodedString.replace("%5F", "_");  // underscore
  decodedString.replace("%60", "`");  // grave accent
  decodedString.replace("%7B", "{");  // left brace
  decodedString.replace("%7C", "|");  // vertical bar
  decodedString.replace("%7D", "}");  // right brace
  decodedString.replace("%7E", "~");  // tilde

  // Handle percent-encoded values for characters beyond basic ASCII (e.g.,
  // extended Unicode)
  decodedString.replace("%", "");

  return decodedString;
}
