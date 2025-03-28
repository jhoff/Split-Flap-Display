#include "SplitFlapWebServer.h"

#if __has_include("config.h")
  #include "config.h"
#else
  #include "config.dist.h"
#endif

#ifndef WIFI_SSID
  #define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
  #define WIFI_PASS ""
#endif

//Constructor
SplitFlapWebServer::SplitFlapWebServer()
  : server(80),
    multiWordDelay(1000),
    attemptReconnect(false),
    multiWordCurrentIndex(0),
    numMultiWords(0),
    wifiCheckInterval(1000),
    connectionMode(0),
    mode(0),
    checkDateInterval(250),
    centering(1),
    use24HourFormat(false)
{
  lastSwitchMultiTime = millis();
}

void SplitFlapWebServer::init() {
    preferences.begin("display", true);
    use24HourFormat = preferences.getBool("format24", false); // default = false
    String tz = preferences.getString("tzinfo", String(TZ_INFO)); // fallback to TZ_INFO macro
    preferences.end();

    configTzTime(tz.c_str(), NTP_SERVER);

    mode = readMode();
}

//Totally didn't use AI to make these functions
// Function to get current minute as a string
String SplitFlapWebServer::getCurrentMinute() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "";
    }
    char minuteStr[3];  // Max "59" + null terminator
    sprintf(minuteStr, "%02d", timeinfo.tm_min);  // Format as two-digit string
    return String(minuteStr);
}

// Function to get current hour as a string
String SplitFlapWebServer::getCurrentHour() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "";
    }

    int currentHour = timeinfo.tm_hour;

    if (!use24HourFormat) {
        currentHour = currentHour % 12;
        if (currentHour == 0) currentHour = 12;
    }

    char hourStr[3];
    sprintf(hourStr, "%02d", currentHour);

    return String(hourStr);
}

// Function to get the first n characters of the day
String SplitFlapWebServer::getDayPrefix(int n) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Err";  // Return error if time not available
    }

    // Get full weekday name
    char fullDay[10];  // Buffer for full day name
    strftime(fullDay, sizeof(fullDay), "%A", &timeinfo);

    // Extract first n characters
    char dayPrefix[n + 1];
    strncpy(dayPrefix, fullDay, n);
    dayPrefix[n] = '\0';  // Null-terminate the string

    return String(dayPrefix);
}

// Function to get the first n characters of the month
String SplitFlapWebServer::getMonthPrefix(int n) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Err";  // Return error if time not available
    }

    // Get full month name
    char fullMonth[10];  // Buffer for full month name
    strftime(fullMonth, sizeof(fullMonth), "%B", &timeinfo);

    // Extract first n characters
    char monthPrefix[n + 1];
    strncpy(monthPrefix, fullMonth, n);
    monthPrefix[n] = '\0';  // Null-terminate the string

    return String(monthPrefix);
}

String SplitFlapWebServer::getCurrentDay() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Err";  // Return error if time is not available
    }

    char dayStr[3];  // Buffer for the day number (max "31" + null terminator)
    sprintf(dayStr, "%02d", timeinfo.tm_mday);  // Format as two-digit string

    return String(dayStr);
}

int SplitFlapWebServer::readMode(){
  preferences.begin("display", true);  // Open the preferences for Wi-Fi with write access
  int lastMode = preferences.getInt("mode", 0);  // Retrieve SSID, default empty if not found
  preferences.end();  // Close preferences
  return lastMode;
}

void SplitFlapWebServer::setMode(int targetMode){
  mode = targetMode;
  preferences.begin("display", false);  // Open the preferences for Wi-Fi with write access
  preferences.putInt("mode", targetMode); // Save Mode
  preferences.end();  // Close preferences
}

int SplitFlapWebServer::getMode(){
  return mode;
}


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
    preferences.begin("wifi", true);  // Open the preferences for Wi-Fi with read-only access
    String ssid = preferences.getString("ssid", String(WIFI_SSID));  // Retrieve SSID, default empty if not found
    String password = preferences.getString("password", String(WIFI_PASS));  // Retrieve password, default empty if not found
    preferences.end();

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
        return true;  // Return true if credentials exist
    }
    return false;  // Return false if no credentials were found
}


bool SplitFlapWebServer::connectToWifi() {
  if (loadWiFiCredentials()) {

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 10000; // 10 seconds
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

    //connected succesfully
    connectionMode = 1;
    WiFi.softAPdisconnect();  // Turns off SoftAP mode only after connected to actual network
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);  //Saves Wi-Fi settings to flash memory
    WiFi.setSleep(false);
    Serial.println("Connected to Wi-Fi!");
    Serial.println("IP Address: http://" + WiFi.localIP().toString()); // Print IP address
    return true;
  }
  return false;
}

void SplitFlapWebServer::startAccessPoint() {
  connectionMode = 0;
  const char* apSSID = "Split Flap Display";
  WiFi.softAP(apSSID);
  #ifdef WIFI_TX_POWER
    delay(100);
    WiFi.setTxPower((wifi_power_t)WIFI_TX_POWER);
  #endif
  Serial.println("AP Mode Started!");
  Serial.println("Connect to: " + String(apSSID));
  Serial.println("AP IP Address: http://" + WiFi.softAPIP().toString());
}

void SplitFlapWebServer::startWebServer(){

  // Initialize mDNS
  if (!MDNS.begin("splitflap")) {   // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/settings-script.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/settings-script.js", "application/javascript");
  });

  server.on("/custom-text-script.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/custom-text-script.js", "application/javascript");
  });

  server.on("/mode-script.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/mode-script.js", "application/javascript");
  });

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html");
  });

  server.on("/custom-text", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/custom-text.html");
  });

  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/mode.html");
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/settings.html");
  });

server.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request){
    preferences.begin("display", true);
    String tz = preferences.getString("tzinfo", String(TZ_INFO));
    bool format24 = preferences.getBool("format24", false);
    int savedMode = preferences.getInt("mode", 0);
    preferences.end();

    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", String(WIFI_SSID));
    preferences.end();

    String json = "{";
    json += "\"format24\":" + String(format24 ? "true" : "false") + ",";
    json += "\"timezone\":\"" + tz + "\",";
    json += "\"ssid\":\"" + ssid + "\",";
    json += "\"mode\":" + String(savedMode);
    json += "}";

    request->send(200, "application/json", json);
});

  // Handle the form POST request
  server.on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request){

    if (request->hasParam("inputType", true)) {
      String inputType = decodeURIComponent(request->getParam("inputType", true)->value());

      centering = (request->getParam("centering", true)->value().toInt());
      // Serial.println(centering);

      if (inputType == "multiple" && request->hasParam("words", true)) {
        String words = decodeURIComponent(request->getParam("words", true)->value());
        Serial.println("Multi Words: " + words);

        this->setMultiInputString(words);
        //count how many words
        int wordCount = 1;
        for (int i = 0; i < words.length(); i++) {
            if (words.charAt(i) == ',') {
                wordCount++;
            }
        }
        this->numMultiWords = wordCount;

        if (request->hasParam("delay", true)) {
          float delay = (request->getParam("delay", true)->value().toFloat());
          // Serial.println("Delay: " + String(delay));
          this->setMultiDelay(int(delay*1000));
        }
        this->setLastSwitchMultiTime(0); //force first word to appear instantly, rather than delay
        this->setMode(1);//change mode last once all variables updated
      }

      else if (inputType == "single" && request->hasParam("word", true)) {
          String word = decodeURIComponent(request->getParam("word", true)->value());
          Serial.println("Single word: " + word);
          this->setInputString(word);
          this->setMode(0);
      }
    }

    if (request->hasParam("mode", true)){ //mode selection page
      String selectedMode = decodeURIComponent(request->getParam("mode", true)->value());
      // Serial.println(selectedMode);
      if (selectedMode == "Date"){
        Serial.println("Date Mode");
        this->setMode(2);
      }
      else if (selectedMode == "Time"){
        Serial.println("Time Mode");
        this->setMode(3);
      }
      else if (selectedMode == "Count"){
        Serial.println("Count Mode");
        this->setMode(4);
      }
      else if (selectedMode == "Random"){
        Serial.println("Random Mode");
        this->setMode(5);
      }
    }

    if (request->hasParam("password", true)) {
        String password = decodeURIComponent(request->getParam("password", true)->value());

        if (password != "") {
            String ssid = decodeURIComponent(request->getParam("ssid", true)->value());

            preferences.begin("wifi", false);
            if (ssid != "") {
                preferences.putString("ssid", ssid);
                Serial.println("Saved SSID: " + ssid);
            }
            preferences.putString("password", password);
            preferences.end();

            Serial.println("Received Password: [hidden]");
            this->attemptReconnect = true;
        } else {
            Serial.println("Password not filled — skipping WiFi update.");
        }
    }

    if (request->hasParam("format24", true)) {
        String format = request->getParam("format24", true)->value();
        use24HourFormat = (format == "1");

        preferences.begin("display", false);
        preferences.putBool("format24", use24HourFormat);
        preferences.end();

        Serial.println("24-Hour Format: " + String(use24HourFormat ? "Enabled" : "Disabled"));
    }

    if (request->hasParam("timezone", true)) {
        String tz = decodeURIComponent(request->getParam("timezone", true)->value());
        Serial.println("Received Timezone: " + tz);

        preferences.begin("display", false);
        preferences.putString("tzinfo", tz);
        preferences.end();

        configTzTime(tz.c_str(), NTP_SERVER);
    }

    request->send(200, "application/json", "{\"message\":\"Text updated successfully\"}"); // Send JSON response
  });

  server.begin();
}

String SplitFlapWebServer::decodeURIComponent(String encodedString) {
  String decodedString = encodedString;
  // Replace common URL-encoded characters with their actual symbols
  decodedString.replace("%20", " ");    // space
  decodedString.replace("%21", "!");    // exclamation mark
  decodedString.replace("%22", "\"");   // double quote
  decodedString.replace("%23", "#");    // hash
  decodedString.replace("%24", "$");    // dollar sign
  decodedString.replace("%25", "%");    // percent
  decodedString.replace("%26", "&");    // ampersand
  decodedString.replace("%27", "'");    // single quote
  decodedString.replace("%28", "(");    // left parenthesis
  decodedString.replace("%29", ")");    // right parenthesis
  decodedString.replace("%2A", "*");    // asterisk
  decodedString.replace("%2B", "+");    // plus
  decodedString.replace("%2C", ",");    // comma
  decodedString.replace("%2D", "-");    // hyphen
  decodedString.replace("%2E", ".");    // period
  decodedString.replace("%2F", "/");    // forward slash
  decodedString.replace("%3A", ":");    // colon
  decodedString.replace("%3B", ";");    // semicolon
  decodedString.replace("%3C", "<");    // less than
  decodedString.replace("%3D", "=");    // equal sign
  decodedString.replace("%3E", ">");    // greater than
  decodedString.replace("%3F", "?");    // question mark
  decodedString.replace("%40", "@");    // at symbol
  decodedString.replace("%5B", "[");    // left bracket
  decodedString.replace("%5C", "\\");   // backslash
  decodedString.replace("%5D", "]");    // right bracket
  decodedString.replace("%5E", "^");    // caret
  decodedString.replace("%5F", "_");    // underscore
  decodedString.replace("%60", "`");    // grave accent
  decodedString.replace("%7B", "{");    // left brace
  decodedString.replace("%7C", "|");    // vertical bar
  decodedString.replace("%7D", "}");    // right brace
  decodedString.replace("%7E", "~");    // tilde

  // Handle percent-encoded values for characters beyond basic ASCII (e.g., extended Unicode)
  decodedString.replace("%", "");

  return decodedString;
}
