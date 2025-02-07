#include "SplitFlapWebServer.h"

//Constructor
SplitFlapWebServer::SplitFlapWebServer() : server(80) {
}

void SplitFlapWebServer::init(){
  // Replace with your network credentials
  const char* ssid = "Hot Signals In Your Area";
  const char* password = "dogfishcat";
  // Connect to Wi-Fi

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  // Initialize mDNS
  if (!MDNS.begin("splitflap")) {   // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  // Set up server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", WEBPAGE_MAIN);
  });

  // Set up server routes
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", WEBPAGE_SETTINGS);
  });
  // Start the server
  server.begin();

}