#include <Arduino.h>
#include <functions.h> //! make name better and use "", ifndef
#include <secret.h>

#include <ArduinoJson.h>
#include <painlessMesh.h>
#include <WebServer.h>
// all the libraries we need

#define   MESH_PREFIX         MESH_PREFIX_secr
#define   MESH_PASSWORD       MESH_PASSWORD_secr
#define   MESH_PORT           5555
// configuration for the mesh  network

#define LED 2
// #define buttonPin 16
// two pins we need for the reset of the eeprom

Scheduler userScheduler; // to control your personal task

bool iAmRoot = false;
bool rootFound = false;
bool hasRun = false;
bool serverNeeded = false;
//variables used for decision making

//! put in class or header
String inputName  = "";
// variable that will store the identifier of the node
painlessMesh mesh;
// make a mesh object
WiFiClient client;

WebServer APIserver(80);

DynamicJsonDocument doc(512);
JsonArray arr;

StaticJsonDocument<250> jsonDocument;
char buffer[250];

int lastTime = 0;
int timerDelay = 10000;
// timing variable for send interval

int lastTimeButton = 0;
int timerDelayButton = 10000;
// timing variable for reset button (press 10 seconds for)

int lastTimeRSSI = 0;
int timerDelayRSSI = 30000;
// When there is no message with RSSI information for 30 seconds

int lastTimeRoot = 0;
int timerDelayRoot = 40000;
// When there is no root for 40 seconds
//! 



int RSSI = 0;
int rootAddress = 0;
// Variables that store RSSI measurement and address of the root node

void sendBroadcast(String message){
  mesh.sendBroadcast(message);
  Serial.println("Broadcast message has been send");
}

void sendSingleMessage(String message, int address){
  mesh.sendSingle(address, message);
  Serial.print("Single message has been send to "); //! debug levels and define in parameter 
  Serial.println(address);
}

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

void setup_API() {   
  // APIserver.on("/temperature", getTemperature);
  APIserver.on("/usedNameID", HTTP_POST, handlePost);
  APIserver.begin();
  Serial.println("API is ready");
}

//***********************************************************************
//**************************** Mesh functions ***************************


void receivedCallback( uint32_t from, String &msg ) {
//function that gets called when we receive a message from the mesh network
  Serial.print("Message received from ");
  Serial.println(from);

  deserializeJson(doc, msg);
  arr = doc.as<JsonArray>();
  // messages are send in JSON format so we make a JSON object an deserialize it
  const char* baseName = arr[0]["bn"]; //! check the json before reading
  // Read the first value

  // if the first value is connectiontest we stop sending messages with RSSI information
  if (strcmp(baseName, "connectionTest") == 0){
    Serial.printf("Message received from %u msg=%s\n", from, msg.c_str());
    //! always check json
    if (int(arr[0]["RSSI"]) > RSSI){
      rootFound = true;
      // if the received RSSI is better we are not the root
    }
    else if (int(arr[0]["RSSI"]) == RSSI){
      ESP.restart();
      // if we have the same RSSI we reset the node 
    }
    lastTimeRSSI = millis();
    lastTimeRoot = millis();
    // reset the timers
  }
  
  // if the first value is Root
  else if (strcmp(baseName, "Root") == 0){
    Serial.print(from);
    Serial.println(" is root");
    rootAddress = from;
    // we save the root address so we only send messages to the root
    rootFound = true;
    // I am not root
    lastTimeRoot = millis();
    // reset the timer
  }

  else if (strcmp(baseName, "nameNotAllowed") == 0){
    Serial.print(from);
    Serial.println(" Name is not allowed");
    writeSpiffs("");
    ESP.restart();
  }

  if ((millis() - lastTimeRSSI) > timerDelayRSSI && (!rootFound)){
  // if we have not found a root and we did not receive a RSSI message in the last 30 seconds we are the root
    Serial.println("This is root");
    setupNetwork();
    Serial.println("Connected to the WiFi network");
    setup_API();
    // connect to the Home Asstant acces point
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    // set me as root
    rootFound = true;
    iAmRoot = true;
  }

  // if I am the root and the messages are measurements we post them to Home Assistant
  else if (iAmRoot){
    postJson(msg);
  }
}

// when a new connection is made we print the layout of the network
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  Serial.printf("New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

// when a connection between nodes changes we print the layout of the network
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  Serial.printf("Changed connections, %s\n", mesh.subConnectionJson(true).c_str());
}

// needed for the mesh to work
void nodeTimeAdjustedCallback(int32_t offset) {
}

//! use namespace
//***********************************************************************
//********************************* API *********************************


void apiCheck(){
  APIserver.handleClient();
}

// void getTemperature() {
//   Serial.println("Get temperature");
//   APIserver.send(200, "application/json", buffer);
// }


//***********************************************************************
//*********************** Setup mesh when needed ************************
void setupMesh(){
  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes( ERROR | STARTUP );  

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

//***********************************************************************
//******************************** setup ********************************
void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  // pinMode(buttonPin, INPUT);
  digitalWrite(LED, LOW);
  // setup button and led, turn off led

  inputName = readSpiffs();
  // we read the eeprom
  if (inputName == ""){
    // if the eeprom is empty we setup the ap where we can enter our name of the sensor
    setupWebserver();
    serverNeeded = true;
    // when the server was needed we also have to shut this server down so we can setup our mesh
  }
  delay(1000); //! remove maybe?
  setupSensor();
  // start the BME680 sensor
}

//***********************************************************************
//******************************** loop *********************************
void loop() {
  nextDnsRequest();
  // while we don't have a sensorname we have to wait
  // the dns is used for the captive window
  
  while (inputName != ""){
    // when we have a sensorname we enter the main loop
    //? Only run this one time (setup of the mesh network)
    if(hasRun == false){
      // we have to run this part once after we got our sensorname
      if (serverNeeded == true){
        endWebserver();
        // kill the webserver when it was needed
        writeSpiffs(inputName);
        // we got a name if the server was needed
        // write this name to the memory
      }
      Serial.print("Setup is finnished!! The sensor ID is: ");
      Serial.println(inputName);
      delay(100);
      Serial.println("Let's check the wifi strength!!!");
      RSSI = networkScan();
      // check the wifi strength of the Home Assistant AP
      if (RSSI == -100){
        rootFound = true;
        iAmRoot = false;
        // if we are not in range of the AP we cannot be the root
      }
      delay(100);
      setupMesh();
      // we setup the mesh after we checked the wifi strength
      hasRun = true;
      // don't run this part again
    }

    mesh.update();
    apiCheck();
    // this has to be done as often as possible

    // if (digitalRead(buttonPin) == LOW and ((millis() - lastTimeButton) > timerDelayButton)){
    //   // when the button is pressed for 10 seconds we reset the eeprom and restart the esp
    //   Serial.print(digitalRead(buttonPin));
    //   Serial.println(" Button has been pressed for 10 seconds");
    //   writeSpiffs("");
    //   // write a empty string
    //   ESP.restart();
    // }
    // if (digitalRead(buttonPin) == HIGH){
    //   lastTimeButton = millis();
    //   // if the buttonpin is not pressed we reset the timer
    // }

    if(!rootFound and ((millis() - lastTime) > timerDelay)){
      // as long we have not selected a root send a message every 10 seconds
      Serial.println("Selecting root...");
      String connectionStrength = "[{\"bn\": \"connectionTest\", \"RSSI\": " + String(RSSI);
      // make the message in JSON format
      sendBroadcast(connectionStrength + "}]");
      // send a message with RSSI information
      lastTime = millis();
      // reset the timer
    }
    else if ((millis() - lastTime) > timerDelay){
      // if a root has been found do this every 10 seconds
      if (iAmRoot){
        // if this is the root
        digitalWrite(LED, HIGH);
        // turn the blue led on
        Serial.println("I am the root");
        String rootStatus = "[{\"bn\": \"Root\"";
        sendBroadcast(rootStatus + "}]");
        // make and broadcast the root is alive message
        postJson(makeSensorMessage(true));
        // we read the sensor and post everything to Home Assistant
      }
      else{
        digitalWrite(LED, LOW);
        // if we are not the root we turn off the blue light
        Serial.println("Not root");
        if (rootAddress == 0){
          sendBroadcast(makeSensorMessage(false));
        }
        else{
          sendSingleMessage(makeSensorMessage(false), rootAddress);
        }
        // the sensor is read and we send it to the root
        if ((millis() - lastTimeRoot) > timerDelayRoot && (!iAmRoot)){
          // if we are not the root and we have not received a message from the root for 40 seconds
          // reset the esp (start the rootselection again)
          ESP.restart();
        }
      }
      lastTime = millis();
      // reset the timer
    }
  }
}