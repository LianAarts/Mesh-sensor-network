#include <Arduino.h>
#include <functions.h>

#include <ArduinoJson.h>
#include <painlessMesh.h>
#include <secret.h>

#define   MESH_PREFIX         MESH_PREFIX_secr
#define   MESH_PASSWORD       MESH_PASSWORD_secr
#define   MESH_PORT           5555

#define LED 2
#define buttonPin 16

Scheduler userScheduler; // to control your personal task

bool iAmRoot = false;
bool rootFound = false;
bool hasRun = false;
bool serverNeeded = false;

String inputName  = "";
painlessMesh mesh;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

unsigned long lastTimeButton = 0;
unsigned long timerDelayButton = 10000;

unsigned long lastTimeRSSI = 0;
unsigned long timerDelayRSSI = 30000;

unsigned long lastTimeRoot = 0;
unsigned long timerDelayRoot = 60000;

int RSSI = 0;
int rootAddress = 0;

//***********************************************************************
//**************************** Mesh functions ***************************
void sendMessage(String message) {
  // mesh.sendBroadcast(message);
  if (rootAddress != 0){
    mesh.sendSingle(rootAddress, message);
    Serial.println("Single message has been send");
  }
  else{
    mesh.sendBroadcast(message);
    Serial.println("Broadcast message has been send");
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.print("Message received from ");
  Serial.println(from);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);
  JsonArray arr = doc.as<JsonArray>();

  const char* baseName = arr[0]["bn"];

  if (strcmp(baseName, "Root") == 0){
    Serial.print(from);
    Serial.println(" is root");
    rootAddress = from;
    rootFound = true;
    lastTimeRoot = millis();
  }

  if (strcmp(baseName, "connectionTest") == 0){
    Serial.printf("Message received from %u msg=%s\n", from, msg.c_str());
    
    if (int(arr[0]["RSSI"]) > RSSI){
      rootFound = true;
    }
    else if (int(arr[0]["RSSI"]) == RSSI){
      ESP.restart();
    }
    lastTimeRSSI = millis();
    lastTimeRoot = millis();  
  }

  if ((millis() - lastTimeRSSI) > timerDelayRSSI && (!rootFound)){
    Serial.println("This is root");
    setupNetwork();
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    rootFound = true;
    iAmRoot = true;
  }

  else if (iAmRoot){
    postJson(msg);
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  Serial.printf("New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  Serial.printf("Changed connections, %s\n", mesh.subConnectionJson(true).c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

//***********************************************************************
//*********************** Setup mesh when needed ************************
void setupMesh(){
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

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
  pinMode(buttonPin, INPUT);
  digitalWrite(LED, LOW);
  inputName = readSpiffs();
  if (inputName == ""){
    setupWebserver();
    serverNeeded = true;
  }
  delay(1000);
  setupSensor();
}

//***********************************************************************
//******************************** loop *********************************
void loop() {
  nextDnsRequest();
  
  while (inputName != ""){
    //? Only run this one time (setup of the mesh network)
    if(hasRun == false){
      if (serverNeeded == true){
        endWebserver();
        writeSpiffs(inputName);
      }
      Serial.print("Setup is finnished!! The sensor ID is: ");
      Serial.println(inputName);
      delay(100);
      Serial.println("Let's check the wifi strength!!!");
      RSSI = networkScan();
      if (RSSI == -100){
        rootFound = true;
        iAmRoot = false;
      }
      delay(100);
      setupMesh();
      lastTimeButton = millis();
      hasRun = true;
    }

    
    mesh.update();

    if (digitalRead(buttonPin) == LOW and ((millis() - lastTimeButton) > timerDelayButton)){
      Serial.print(digitalRead(buttonPin));
      Serial.println(" Button has been pressed for 10 seconds");
      writeSpiffs("");
      ESP.restart();
    }
    if (digitalRead(buttonPin) == HIGH){
      lastTimeButton = millis();
    }

    if(!rootFound and ((millis() - lastTime) > timerDelay)){
      Serial.println("Selecting root...");
      String connectionStrength = "[{\"bn\": \"connectionTest\", \"RSSI\": " + String(RSSI);
      sendMessage(connectionStrength + "}]");
      lastTime = millis();
    }
    else if ((millis() - lastTime) > timerDelay){
      if (iAmRoot){
        digitalWrite(LED, HIGH);
        Serial.println("I am the root");
        String rootStatus = "[{\"bn\": \"Root\"";
        sendMessage(rootStatus + "}]");
        postJson(makeSensorMessage());
      }
      else{
        digitalWrite(LED, LOW);
        Serial.println("Not root");
        sendMessage(makeSensorMessage());
        if ((millis() - lastTimeRoot) > timerDelayRoot && (!iAmRoot)){
          ESP.restart();
        }
      }
      lastTime = millis();
    }
  }
}