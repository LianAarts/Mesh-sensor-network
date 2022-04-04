#include <Arduino.h>
#include <functions.h>

#include <ArduinoJson.h>
#include <painlessMesh.h>
#include <secret.h>

#define   MESH_PREFIX         MESH_PREFIX_secr
#define   MESH_PASSWORD       MESH_PASSWORD_secr
#define   MESH_PORT           5555

#define LED 2

Scheduler userScheduler; // to control your personal task

bool iAmRoot = false;
bool rootFound = false;
bool hasRun = false;

String inputName  = "sensor4";
painlessMesh mesh;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

unsigned long lastTimeRSSI = 0;
unsigned long timerDelayRSSI = 30000;

unsigned long lastTimeRoot = 0;
unsigned long timerDelayRoot = 60000;

int RSSI = 0;

//***********************************************************************
//**************************** Mesh functions ***************************
void sendMessage(String message) {
  mesh.sendBroadcast(message);
  Serial.println("message has been send");
  // Serial.print("message has been sent: ");
  // Serial.println(message);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.print("Message received from ");
  Serial.println(from);
  DynamicJsonDocument doc(7000);
  deserializeJson(doc, msg);
  JsonArray arr = doc.as<JsonArray>();

  const char* baseName = arr[0]["bn"];

  if (strcmp(baseName, "Root") == 0){
    Serial.print(from);
    Serial.println(" is root");
    rootFound = true;
    lastTimeRoot = millis();
  }

  if ((millis() - lastTimeRoot) > timerDelayRoot && (!iAmRoot)){
    mesh.setRoot(false);
    mesh.setContainsRoot(false);
    rootFound = false;
    iAmRoot = false;
    lastTimeRoot = millis();
    lastTimeRSSI = millis();
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
    for (int i=0; i<arr.size(); i++) { 
      JsonObject repo = arr[i];
      String name = repo["n"];
      String unit = repo["u"];
      String value = repo["v"];
      String ID = String(baseName) + "_";
      post(value, ID + name, unit);
    }
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
  digitalWrite(LED, LOW);

  setupWebserver();
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
      endWebserver();
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
      hasRun = true;
    }

    mesh.update();

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
      }
      else{
        digitalWrite(LED, LOW);
        Serial.println("Not root");
        sendMessage(makeSensorMessage());
      }
      lastTime = millis();
    }
  }
}