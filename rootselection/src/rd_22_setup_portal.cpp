#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DNSServer.h>

AsyncWebServer server(80);
// server on port 80
DNSServer dnsServer; 
// used for the captive window

String PARAM_NAME = "name";
// for AP input
String inputName  = "";

const char* ssid = "New Node"; //! use string maybe
// SSID when we need the setupwindow

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

//***********************************************************************
//************************ Captive config window ************************
class CaptiveRequestHandler : public AsyncWebHandler { //!declaration on the header
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    //! if request
    //if()
      // request->send(SPIFFS, "/index.html", String(), false);
    request->send(SPIFFS, "/index.html", String(), false);
  }
}; //! research this part

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

  // get the ip address
  Serial.print("AP IP address: ");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //! same here
    request->send(SPIFFS, "/index.html", String(), false);
  });
  // get the index page from spiffs

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    //! same here
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
    writeSpiffs(inputName);
    
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