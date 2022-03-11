#include <Arduino.h>

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HTTPClient.h>

#include <secret.h>

Adafruit_BME280 bme;

const char* ssid = ssid_secr;
const char* password = password_secr;

String serverName = "http://192.168.1.4:8123/api/states/sensor.";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void post(float sensorVal, String name, String unit) {
  WiFiClient client;
  HTTPClient http;

  serverName = "http://192.168.1.4:8123/api/states/sensor." + name;

  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", token_secr);

  String httpRequestData = "{\"state\":"+ String(sensorVal);
  String httpAtributes = ", \"attributes\": {\"unit_of_measurement\": \"" + unit;

  Serial.println(httpRequestData + ", \"attributes\": {\"unit_of_measurement\": \"°C\"}}");

  int httpResponseCode = http.POST(httpRequestData + httpAtributes + "\"}}");
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  bme.begin(); 
}

void loop() { 
  if ((millis() - lastTime) > timerDelay) {
  //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      Serial.println("ok");

      float tempBME280 = bme.readTemperature();
      Serial.println(tempBME280);
      post(tempBME280, "BME280_temp", "°C");

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
