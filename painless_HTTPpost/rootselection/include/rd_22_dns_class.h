#ifndef rd_22_dns_class
#define rd_22_dns_class

#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

//***********************************************************************
//************************ Captive config window ************************
/**
 * @brief Captive handler class
 * 
 */
class CaptiveRequestHandler
    : public AsyncWebHandler {
 public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    //! if request
    // if()
    //  request->send(SPIFFS, "/index.html", String(), false);
    request->send(SPIFFS, "/index.html", String(), false);
  }
};  //! research this part

#endif