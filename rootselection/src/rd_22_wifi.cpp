#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "secret.h"

#define token token_secr
#define ssidScan ssidScan_secr
#define passScan passScan_secr

// get the token from the secret.h

// make a sensor object

HTTPClient http;
String serverName = "";

WiFiClient client;

IPAddress ip;

String getIp(){
  return(ip.toString());
}

//***********************************************************************
//**************************** Post function ****************************

void post(String sensorVal, String name, String unit) {
  serverName = "http://192.168.99.1:8123/api/states/sensor." + name; //! in configuration file
  // the name in the JSON is used as the entity ID
  
  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", token);
  // headers we need to make the post request to 

  String httpRequestData = "{\"state\":\""+ String(sensorVal);
  String httpAtributes = "\", \"attributes\": {\"unit_of_measurement\": \"" + unit;
  // make the json we have to send

  int httpResponseCode = http.POST(httpRequestData + httpAtributes + "\"}}");
  // POST the json
  Serial.print(sensorVal);
  Serial.print("\t");
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode == -1){
    // if the response code is -1 we don't have a connection to Home Assistant
    // we reset the ESP
    ESP.restart();
  }
  http.end();
}

//***********************************************************************
//**************************** Network scan *****************************
int networkScan(){
  WiFi.mode(WIFI_STA);
  // we use STA mode because we have to connect to the home assistant AP
  WiFi.begin(ssidScan, passScan);
  // connect to the wifi
  Serial.print("Connecting to WiFi ..");
  for (int i = 0 ; i < 10 and WiFi.status() != WL_CONNECTED ; i++){
    // we wait for the connection for 10 seconds
    // if we don't have a connection after 10 seconds we are probably not in range
    Serial.print('.');
    delay(1000); //! why delay (do some research)
  }

  int RSSI = WiFi.RSSI();
  if (WiFi.localIP()[0] == 0){
    // if the IP is 0.0.0.0 we don't have a connection
    Serial.println("no IP");
    RSSI = -100;
    // make the RSSI -100
  }

  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %i", RSSI);

  WiFi.disconnect();
  // disconnect after we have our RSSI
  return(RSSI);
  // return the RSSI
}

//***********************************************************************
//*************************** Network connect ***************************
void setupNetwork(){
  // connect to the AP if we are the host
  // it is difficult to connect to the AP while the mesh is running
  // we cannot block the mesh.update but the wifi library needs it's delays
  WiFi.begin(ssidScan, passScan);
  // updateMesh();
  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    for (int i=0; i<=5; i++){
      delay(100); //! for loop here
      // updateMesh();
      Serial.print(".");
    }
  }
  Serial.println("Connected to the WiFi network");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}


//***********************************************************************
//************************ Split up json and post ***********************
void postJson(String json){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonArray arr = doc.as<JsonArray>();
  // deserialize the string as a JSON object

  const char* baseName = arr[0]["bn"];
  for (int i=0; i<arr.size(); i++) {
    // make a post for every measurement in the JSON
    JsonObject repo = arr[i];
    String unit = repo["u"];
    // units of the measurements
    String value = repo["v"];
    // value we want to post
    String name = repo["n"];
    String ID = String(baseName) + "_";
    // create a name from the sensorname and the name of the measurement
    // like: sensor5_temperature
    post(value, ID + name, unit);
    // post the values
  }
}