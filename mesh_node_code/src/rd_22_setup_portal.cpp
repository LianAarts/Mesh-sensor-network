#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "rd_22_dns_class.h"
#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

#define ssid AP_NAME
// ssid for the setup window

/**
 * @brief Object of the webserver on port 80.
 * 
 */
AsyncWebServer server(80);
// server on port 80

/**
 * @brief DNS server for the captive window.
 * 
 */
DNSServer dnsServer;
// used for the captive window

/**
 * @brief Name of the field in the HTML file.
 * 
 */
String PARAM_NAME = "name";
// for AP input

/**
 * @brief Variable that will store the input of our webserver.
 * 
 */
String inputName = "";

//***********************************************************************
//*************************** Read saved data ***************************
/**
 * @brief Read a file in spiffs and return the content
 *
 * @param fileName File we want to read
 * @return String Content of the file
 */
String readSpiffs(String fileName) {
  if (!SPIFFS.begin(true)) {
    debugln1("An Error has occurred while mounting SPIFFS");
  }
  // start the spiffs file system

  File file = SPIFFS.open(fileName);
  // open the text file as "file"
  if (!file) {
    debugln1("Failed to open file for reading");
  }
  String data = "";
  // variable that will store the string
  while (file.available()) {
    data = data + char(file.read());
    // if we can open the file we read the content
  }
  debug2("saved data: ");
  debugln2(data);
  file.close();
  // close the file
  return (data);
  // return the string
}

//***********************************************************************
//************************** Write to save data *************************
/**
 * @brief Write to a file in Spiffs
 * @warning This will overwrite all the data!
 *
 * @param data String that will be written in the file
 * @param fileName File we want to write to
 */
void writeSpiffs(String data, String fileName) {
  File file = SPIFFS.open(fileName, FILE_WRITE);
  // open the file to write (append will add, write will overwrite this file)
  if (!file) {
    debugln1("There was an error opening the file for writing");
    return;
  }

  if (file.print(data)) {
    // write the data
    debugln2("File was written");
    ;
  } else {
    debugln1("File write failed");
  }
  file.close();
  // close the file
}

//***********************************************************************
//************************** Next DNS request ***************************
/**
 * @brief Handler for DNS requests
 *
 */
void nextDnsRequest() {
  dnsServer.processNextRequest();
  // dns for the captive window
}

//***********************************************************************
//********************** Make configuration screen **********************
/**
 * @brief Webserver that is needed when no name is saved in Spiffs
 * With this small webserver, we enable the user to connect to the node and
 * configure the name of the node. This name will be used in Home Assistant as
 * the entity ID. Example: name = node -> entity ID = node.temperature
 *
 * The DNS server is used to make a captive portal.
 */
void setupWebserver() {
  // make the webserver where we can set our name
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);

  // get the ip address
  debugln2("AP IP address: ");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    //! same here
    request->send(SPIFFS, "/index.html", String(), false);
  });
  // get the index page from spiffs

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    //! same here
    request->send(SPIFFS, "/style.css", "text/css");
  });
  // get the CSS file from spiffs

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    // when we receive a get request
    String inputParam;
    // het the input value
    if (request->hasParam(PARAM_NAME)) {
      inputName = request->getParam(PARAM_NAME)->value();
      inputParam = PARAM_NAME;
      // this input will be the inputname we use
    }
    debugln2(inputName);
    writeSpiffs(inputName, "/assets.txt");
  });
  dnsServer.start(53, "*", WiFi.softAPIP());
  // start the DNS server

  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  // only when requested from AP
  server.begin();
  // start the server
}

//***********************************************************************
//********************** End configuration screen ***********************
/**
 * @brief End the webserver
 *
 */
void endWebserver() {
  server.end();
  // end the webserver
}