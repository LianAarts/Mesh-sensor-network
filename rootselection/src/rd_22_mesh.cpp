#include <ArduinoJson.h>
#include <painlessMesh.h>

#include "rd_22_setup_portal.h"
#include "rd_22_wifi.h"
#include "secret.h"

#define   MESH_PREFIX         MESH_PREFIX_secr
#define   MESH_PASSWORD       MESH_PASSWORD_secr
#define   MESH_PORT           5555
// configuration for the mesh  network

Scheduler userScheduler; // to control your personal task

DynamicJsonDocument doc(512);
JsonArray arr;

painlessMesh mesh;
// make a mesh object

int lastTimeRSSI = 0;
int timerDelayRSSI = 30000;
// When there is no message with RSSI information for 30 seconds

int lastTimeRoot = 0;
int timerDelayRoot = 40000;
// When there is no root for 40 seconds

int meshRSSI = 0;
int rootAddress = 0;
// Variables that store RSSI measurement and address of the root node

bool iAmRoot = false;
bool rootFound = false;

String getNodeIdMesh(){
  return(String(mesh.getNodeId()));
}

void setIAmRoot(bool state){
  iAmRoot = state;
}

void setRootFound(bool state){
  rootFound = state;
}

void setRSSI(int rssi){
  meshRSSI = rssi;
}

// return function of the root, if root true
bool getIAmRoot(){
  return(iAmRoot);
}

// return the state of the root (is there a root?)
bool getRootFound(){
  return(rootFound);
}

// timer that resets when keep alive message from root has been received
// return a True if we have not received a message from the root for 40 seconds
bool rootTimer(){
  if ((millis() - lastTimeRoot) > timerDelayRoot){
    return(true);
  }
  else{
    return(false);
  }
}

int getRootAddress(){
    return(rootAddress);
}

// send a broadcast message
void sendBroadcast(String message){
  mesh.sendBroadcast(message);
  Serial.println("Broadcast message has been send");
}

// send a single message to a specific node
void sendSingleMessage(String message, int address){
  mesh.sendSingle(address, message);
  Serial.print("Single message has been send to "); //! debug levels and define in parameter 
  Serial.println(address);
}

// update our mesh
// this should be done as often as possible
void updateMesh(){
  mesh.update();
}

//***********************************************************************
//**************************** Mesh callback ****************************
void receivedCallback( uint32_t from, String &msg ) {
//function that gets called when we receive a message from the mesh network
  Serial.print("Message received from ");
  Serial.println(from);

  deserializeJson(doc, msg);
  arr = doc.as<JsonArray>();
  // messages are send in JSON format so we make a JSON object an deserialize it
  if (arr[0].containsKey("bn")){
    // we check the JSON if it has the right key
    const char* baseName = arr[0]["bn"];
    // Read the first value

    // if the first value is connectiontest we stop sending messages with RSSI information
    if (strcmp(baseName, "connectionTest") == 0){
      Serial.printf("Message received from %u msg=%s\n", from, msg.c_str());
      //! always check json
      if (arr[0].containsKey("RSSI")){
        // only if we have the right key
        if (int(arr[0]["RSSI"]) > meshRSSI){
          rootFound = true;
          // if the received RSSI is better we are not the root
        }
        else if (int(arr[0]["RSSI"]) == meshRSSI){
          ESP.restart();
          // if we have the same RSSI we reset the node 
        }
        lastTimeRSSI = millis();
      }; // true

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

//***********************************************************************
//**************************** Mesh callbacks ***************************
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

//***********************************************************************
//*********************** Setup mesh when needed ************************
void setupMesh(){
  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes( ERROR | STARTUP );  

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}
