#include <Arduino.h>
#include <functions.h>

#include <HTTPClient.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Zanshin_BME680.h>
#include <DNSServer.h>
#include <painlessMesh.h>
#include <secret.h>

# define token token_secr

AsyncWebServer server(80);
DNSServer dnsServer; // used for the captive window
BME680_Class BME680; // make a sensor object

const char* PARAM_SLEEP = "name";
const char* ssid = "New Node";
String serverName = "http://192.168.3.3:8123/api/states/sensor.";

//***********************************************************************
//**************************** Post function ****************************

void post(String sensorVal, String name, String unit) {
  WiFiClient client;
  HTTPClient http;

  serverName = "http://192.168.3.3:8123/api/states/sensor." + name;

  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", token);

  String httpRequestData = "{\"state\":\""+ String(sensorVal);
  String httpAtributes = "\", \"attributes\": {\"unit_of_measurement\": \"" + unit;

//   Serial.println(httpRequestData + httpAtributes + "\"}}");

  int httpResponseCode = http.POST(httpRequestData + httpAtributes + "\"}}");
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode == -1){
    ESP.restart();
  }
  http.end();
}

//***********************************************************************
//**************************** Network scan *****************************
int networkScan(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidScan, passScan);
    Serial.print("Connecting to WiFi ..");
    for (int i = 0 ; i < 10 and WiFi.status() != WL_CONNECTED ; i++){
        Serial.print('.');
        delay(1000);
    }

    int RSSI = WiFi.RSSI();
    if (WiFi.localIP()[0] == 0){
        Serial.println("no IP");
        RSSI = -100;
    }

    Serial.println(WiFi.localIP());
    Serial.printf("RSSI: %i", RSSI);

    WiFi.disconnect();
    return(RSSI);
}

//***********************************************************************
//*************************** Network connect ***************************
void setupNetwork(){
    WiFi.begin(ssidScan, passScan);
    mesh.update();
    Serial.println("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        mesh.update();
        delay(100);
        mesh.update();
        delay(100);
        mesh.update();
        delay(100);
        mesh.update();
        delay(100);
        mesh.update();
        Serial.print(".");
    }
    Serial.println("Connected to the WiFi network");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}

//***********************************************************************
//************************ Captive config window ************************
class CaptiveRequestHandler : public AsyncWebHandler {
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", String(), false);
    }
};

//***********************************************************************
//************************** Next DNS request ***************************
void nextDnsRequest(){
  dnsServer.processNextRequest();
}

//***********************************************************************
//********************** Make configuration screen **********************
void setupWebserver(){
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

    // Bij ontvangst van een get request
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputParam;
    // GET sleep waarde
    if (request->hasParam(PARAM_SLEEP)) {
      inputName = request->getParam(PARAM_SLEEP)->value();
      inputParam = PARAM_SLEEP;
    }
    Serial.println(inputName);
    // trigger de "go to sleep" functie en gebruik de variabelen
  });
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  server.begin();
}

//***********************************************************************
//********************** End configuration screen ***********************
void endWebserver(){
    server.end();
}

//***********************************************************************
//**************************** Setup sensor *****************************
void setupSensor(){
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

//***********************************************************************
//************************* Make sensor message *************************
String makeSensorMessage(){
    static int32_t  temp, humidity, pressure, gas;

    BME680.getSensorData(temp, humidity, pressure, gas);
    float tempBME680 = (temp / 100) + float(temp % 100) / 100;
    float gasBME680 = (gas / 100) + float(gas % 100) /100;
    float humBME680 = (humidity / 1000)+ float(humidity % 1000) /1000;
    float presBME680 = (pressure / 100)+ float(pressure % 100) /100;

    String header = "[{\"bn\": \"" + inputName;
    String meas1 = "\", \"n\": \"temperature\", \"u\": \"°C\", \"v\":" + String(tempBME680);
    String meas2 = "}, {\"n\": \"gas\", \"u\": \"ohm\", \"v\":" + String(gasBME680);
    String meas3 = "}, {\"n\": \"humidity\", \"u\": \"%\", \"v\":" + String(humBME680);
    String meas4 = "}, {\"n\": \"pressure\", \"u\": \"Hpa\", \"v\":" + String(presBME680);

    return(header + meas1 + meas2 + meas3 + meas4 + "\"}]");
}