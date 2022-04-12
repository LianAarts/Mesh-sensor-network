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
// get the token from the secret.h

AsyncWebServer server(80);
// server on port 80
DNSServer dnsServer; 
// used for the captive window
BME680_Class BME680; 
// make a sensor object

const char* PARAM_NAME = "name";
// for AP input
const char* ssid = "New Node";
// SSID when we need the setupwindow
String serverName = "";

//***********************************************************************
//**************************** Post function ****************************

void post(String sensorVal, String name, String unit) {
  WiFiClient client;
  HTTPClient http;

  serverName = "http://192.168.99.1:8123/api/states/sensor." + name;
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
    delay(1000);
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
  // dns for the captive window
}

//***********************************************************************
//********************** Make configuration screen **********************
void setupWebserver(){
  // make the webserver where we can set our name
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  // get the ip address
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });
  // get the index page from spiffs

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  // get the css file from spiffs

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
  // when we receive a get request
    String inputParam;
    // het the input value
    if (request->hasParam(PARAM_NAME)) {
      inputName = request->getParam(PARAM_NAME)->value();
      inputParam = PARAM_NAME;
      // this input will be the inputname we use
    }
    Serial.println(inputName);
    
  });
  dnsServer.start(53, "*", WiFi.softAPIP());
  // start the DNS server
  
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  //only when requested from AP
  server.begin();
  // start the server
}

//***********************************************************************
//********************** End configuration screen ***********************
void endWebserver(){
  server.end();
  // end the webserver
}

//***********************************************************************
//**************************** Setup sensor *****************************
void setupSensor(){
  while (!BME680.begin(I2C_STANDARD_MODE)) {  
    // Start BME680 using I2C, use first device found
    Serial.print(F("-  Unable to find BME680. Trying again in 5 seconds.\n"));
    delay(5000);
    // loop until the device is found
  }

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
  // variables for the measurements

  BME680.getSensorData(temp, humidity, pressure, gas);
  // get the sensordata
  float tempBME680 = (temp / 100) + float(temp % 100) / 100;
  float gasBME680 = (gas / 100) + float(gas % 100) /100;
  float humBME680 = (humidity / 1000)+ float(humidity % 1000) /1000;
  float presBME680 = (pressure / 100)+ float(pressure % 100) /100;
  // calculate all the measurements

  String header = "[{\"bn\": \"" + inputName;
  String meas1 = "\", \"n\": \"temperature\", \"u\": \"°C\", \"v\":" + String(tempBME680);
  String meas2 = "}, {\"n\": \"gas\", \"u\": \"ohm\", \"v\":" + String(gasBME680);
  String meas3 = "}, {\"n\": \"humidity\", \"u\": \"%\", \"v\":" + String(humBME680);
  String meas4 = "}, {\"n\": \"pressure\", \"u\": \"Hpa\", \"v\":" + String(presBME680);
  // make the JSON message

  return(header + meas1 + meas2 + meas3 + meas4 + "\"}]");
  // return the JSON string to the main loop
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

//***********************************************************************
//*************************** Read saved data ***************************
String readSpiffs(){
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  // start the spiffs file system

  File file = SPIFFS.open("/assets.txt");
  // open the text file as "file"
  if(!file){
    Serial.println("Failed to open file for reading");
  }
  String data = "";
  // variable that will store the string
  while(file.available()){
    data = data +char(file.read());
    // if we can open the file we read the content
  }
  Serial.print("saved data: ");
  Serial.println(data);
  file.close();
  // close the file
  return(data);
  // return the string
}

//***********************************************************************
//************************** Write to save data *************************
void writeSpiffs(String data){
  File file = SPIFFS.open("/assets.txt", FILE_WRITE);
  // open the file to write (append will add, write will overwrite this file)
  if(!file){
    Serial.println("There was an error opening the file for writing");
    return;
  }
  
  if(file.print(data)){
    // write the data
    Serial.println("File was written");;
  } 
  else{
    Serial.println("File write failed");
  }
  file.close(); 
  // close the file
}