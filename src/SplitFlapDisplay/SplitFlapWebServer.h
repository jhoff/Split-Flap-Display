// SplitFlapWebServer.h (header file)

#ifndef SplitFlapWebServer_h
#define SplitFlapWebServer_h
#include "Arduino.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>  
#include <ESPmDNS.h>
#include "WebPages.h" //HTML/CSS for Webpages

class SplitFlapWebServer {
  public:
    SplitFlapWebServer();
    void init();
  private:
    AsyncWebServer server; // Declare server as a class member

};
#endif
//          __
// (QUACK)>(o )___
//          ( ._> /
//           `---'
// Morgan Manly
// 28/01/2025