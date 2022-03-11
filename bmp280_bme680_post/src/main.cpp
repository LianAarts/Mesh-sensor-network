#include <Arduino.h>

#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <Adafruit_BMP280.h>
#include <Zanshin_BME680.h>

#include <secret.h>

const char* ssid = ssid_secr;
const char* password = password_secr;

String serverName = "http://192.168.1.4:8123/api/states/sensor.";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

BME680_Class BME680;

Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

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
 
  // status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  bmp.begin();

  while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start BME680 using I2C, use first device found
    Serial.print(F("-  Unable to find BME680. Trying again in 5 seconds.\n"));
    delay(5000);
  }  // of loop until device is located

  // Serial.print(F("- Setting 16x oversampling for all sensors\n"));
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  // Serial.print(F("- Setting IIR filter to a value of 4 samples\n"));
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  // Serial.print(F("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n"));  // "�C" symbols
  BME680.setGas(320, 150);  // 320�c for 150 milliseconds

}

void loop() {

  static int32_t  temp, humidity, pressure, gas;  // BME readings

  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  // BME680.getSensorData(temp, humidity, pressure, gas);  // Get readings
  //                      // Ignore first reading, might be incorrect
  // sprintf(buf, "%4d %3d.%02d", (loopCounter - 1) % 9999,  // Clamp to 9999,
  //         (int8_t)(temp / 100), (uint8_t)(temp % 100));   // Temp in decidegrees
  // Serial.print(buf);
  // sprintf(buf, "%3d.%03d", (int8_t)(humidity / 1000),
  //         (uint16_t)(humidity % 1000));  // Humidity milli-pct
  // Serial.print(buf);
  // sprintf(buf, "%7d.%02d", (int16_t)(pressure / 100),
  //         (uint8_t)(pressure % 100));  // Pressure Pascals
  // Serial.print(buf);
  // sprintf(buf, "%4d.%02d\n", (int16_t)(gas / 100), (uint8_t)(gas % 100));  // Resistance milliohms
  // Serial.print(buf);
  // delay(10000);  // Wait 10s

  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){

      BME680.getSensorData(temp, humidity, pressure, gas);
      float tempBME680 = (temp / 100) + float(temp % 100) / 100;
      post(tempBME680, "BME680_temp", "°C");
      float gasBME680 = (gas / 100) + float(gas % 100) /100;
      post(gasBME680, "BME680_gas", "ohm");

      float tempBMP280 = temp_event.temperature;
      post(tempBMP280, "BMP280_temp", "°C");

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}