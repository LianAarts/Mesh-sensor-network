#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "rd_22_mesh.h"

StaticJsonDocument<250> jsonDocument;
char buffer[250];

WebServer APIserver(80);

void handlePost() {
  if (APIserver.hasArg("plain") == false) {
  }
  String body = APIserver.arg("plain");
  deserializeJson(jsonDocument, body);
  Serial.print("Message has been send to: ");
  Serial.println(int(jsonDocument["chipID"]));
  sendSingleMessage("[{\"bn\": \"nameNotAllowed\"}]", int(jsonDocument["chipID"]));

  APIserver.send(200, "application/json", "{}");
}

void setup_API(){
  // APIserver.on("/temperature", getTemperature);
  APIserver.on("/usedNameID", HTTP_POST, handlePost);
  APIserver.begin();
  Serial.println("API is ready");
}

void apiCheck(){
  APIserver.handleClient();
}

// void getTemperature() {
//   Serial.println("Get temperature");
//   APIserver.send(200, "application/json", buffer);
// }