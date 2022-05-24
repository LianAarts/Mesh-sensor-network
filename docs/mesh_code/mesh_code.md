# Mech code explanation

The code is divided into two parts. The main file is where all the decisions are made. The other file contains all the functions that are called from inside the main code.

## general explanation

There are some crucial steps that have to be taken:
- Setup an access point to enter configuration
- Check the wifi strength
- Start the mesh network
- Choose the root node
- Read sensor
- Start sending measurements to the root node
- Post measurements to Home Assistant
- Check if the root is still alive

All these steps have to be taken in a specific order. It is not possible to have a wifi connection and make a mesh network at the same time, because the same wifi chip is used. 

## configuration access point

Every sensor needs it's own identifier so it can be found in Home Assistant. This configuration has to be on the spot with no specific tools. We choose to make use the ESPAsyncWebServer.h library to create a small webserver. When you connect to the node via wifi a configuration window will pop up.

We only need to setup this webserver when we have no configuration stored on the eeprom.
``` cpp
  inputName = readSpiffs(); // read txt file in eeprom
  if (inputName == ""){     //
    setupWebserver();
    serverNeeded = true;
  }
```
We setup our webserver and we use our html and css file we stored in SPIFFS(**SPI** **F**lash **F**ile **S**torage) file system. 

``` cpp
void setupWebserver(){

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
```


