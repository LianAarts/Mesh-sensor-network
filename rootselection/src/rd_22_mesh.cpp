#include <ArduinoJson.h>
#include <painlessMesh.h>

#include "rd_22_api.h"
#include "rd_22_wifi.h"
#include "rd_22_setup_portal.h"

#include "rd_22_configuration.h"
#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

#define MESH_PREFIX     MESH_PREFIX_secr
#define MESH_PASSWORD   MESH_PASSWORD_secr
#define MESH_PORT       5555
// configuration for the mesh  network

// Scheduler userScheduler; // to control your personal task

DynamicJsonDocument doc(512);
JsonArray arr;

painlessMesh mesh;
// make a mesh object

int lastTimeRSSI = 0;
// When there is no message with RSSI information for 30 seconds

int lastTimeRoot = 0;
// When there is no root for 40 seconds

int meshRSSI = 0;
int rootAddress = 0;
// Variables that store RSSI measurement and address of the root node

bool iAmRoot = false;
bool rootFound = false;

class RootStatus{
  public:
};

/**
 * @brief Get the Node Id of the node.
 * 
 * @return String node ID.
 */
String getNodeIdMesh(){
  return(String(mesh.getNodeId()));
}

/**
 * @brief Set root status.
 * 
 * @param state true: this is root, false: this is not root.
 */
void setIAmRoot(bool state){
  iAmRoot = state;
}

/**
 * @brief Set root found status.
 * 
 * @param state true: root is found, false: root is not found.
 */
void setRootFound(bool state){
  rootFound = state;
}

/**
 * @brief Set the RSSI value.
 * Received Signal Strength Indicator (RSSI) is the measurement of how strong the signal of a router of access point is to a device.
 * We use this measurement to determen which node has the better connection to the Home Assistant access point.
 * 
 * @param rssi RSSI value (between -100 and 0).
 */
void setRSSI(int rssi){
  meshRSSI = rssi;
}

/**
 * @brief Returns the root status of this node.
 * 
 * @return true This node is the root.
 * @return false This node is not root.
 */
bool getIAmRoot(){
  return(iAmRoot);
}

/**
 * @brief Return the state of the root.
 * 
 * @return true If there is a root in the network.
 * @return false If there is no root in the network.
 */
bool getRootFound(){
  return(rootFound);
}

/**
 * @brief Timer that resets when keep alive message from root has been received.
 * 
 * We can use this to check if there is a root node in the network.
 * The root node will send a broadcast at set intervals.
 * We have to selest a new node when there is no more root.
 * 
 * @return true No message of the root has been received in the timeframe that has been set.
 * @return false A message has been received in the timeframe.
 */
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

/**
 * @brief Get the address of the root node.
 * 
 * @return int Root address.
 */
int getRootAddress(){
    return(rootAddress);
}

/**
 * @brief Send a broadcast message.
 * 
 * @param message Content of the message.
 */
// send a broadcast message
void sendBroadcast(String message){
  mesh.sendBroadcast(message);
  debugln3("Broadcast message has been send");
}

/**
 * @brief Send a message to a specific node in the network
 * 
 * @param message message content
 * @param address Address of the recipient node 
 */
// send a single message to a specific node
void sendSingleMessage(String message, int address){
  mesh.sendSingle(address, message);
  debug3("Single message has been send to "); //! debug levels and define in parameter 
  debugln3(address);
}

/**
 * @brief Update the mesh network.
 * @warning This should be done as often as possible.
 * 
 */
// update our mesh
// this should be done as often as possible
void updateMesh(){
  mesh.update();
}

//***********************************************************************
//**************************** Mesh callback ****************************
/**
 * @brief When a mesh meassage is received this function is called.
 * 
 * @param from Node ID of the sender of the message.
 * @param msg Message content in JSON.
 */
void receivedCallback( uint32_t from, String &msg ) {
//function that gets called when we receive a message from the mesh network
  debug3("Message received from ");
  debugln3(from);

  deserializeJson(doc, msg);
  arr = doc.as<JsonArray>();
  //Messages are send in JSON format so we make a JSON object an deserialize it
  if (arr[0].containsKey("bn")){
    // we check the JSON if it has the right key
    const char* baseName = arr[0]["bn"];
    // Read the first value

    // if the first value is connectiontest we stop sending messages with RSSI information
    if (strcmp(baseName, "connectionTest") == 0){
      debug3("Message received from ");
      debug3(from);
      debug3("msg= ");
      debugln3(msg.c_str());
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
      debug3(from);
      debugln3(" is root");
      rootAddress = from;
      // we save the root address so we only send messages to the root
      rootFound = true;
      // I am not root
      lastTimeRoot = millis();
      // reset the timer
    }

    else if (strcmp(baseName, "nameNotAllowed") == 0){
      debug3(from);
      debugln3(" Name is not allowed");
      writeSpiffs("", "/assets.txt");
      ESP.restart();
    }
  }

  if ((millis() - lastTimeRSSI) > timerDelayRSSI && (!rootFound)){
  // if we have not found a root and we did not receive a RSSI message in the last 30 seconds we are the root
    debugln2("This is root");
    setupNetwork();
    debugln2("Connected to the WiFi network");
    setupAPI();
    // connect to the Home Asstant acces point
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    // set me as root
    rootFound = true;
    iAmRoot = true;
  }

  // if I am the root and the messages are measurements we post them to Home Assistant
  else if (iAmRoot){
    splitJson(msg);
  }
}

//***********************************************************************
//**************************** Mesh callbacks ***************************
/**
 * @brief When we have a new connection in the mesh network this function is called.
 * 
 * The layout is printed when this happens
 * 
 * @param nodeId The nodeId of the new node
 */
// when a new connection is made we print the layout of the network
void newConnectionCallback(uint32_t nodeId) {
  debug3("New Connection, nodeId = ");
  debugln3(nodeId);
  debug3("New Connection, ");
  debugln3(mesh.subConnectionJson(true).c_str());
}

/**
 * @brief When we have a connection changes in the mesh network this function is called.
 * 
 * The layout is printed when this happens
 */
// when a connection between nodes changes we print the layout of the network
void changedConnectionCallback() {
  debug3("Changed Connection, ");
  debugln3(mesh.subConnectionJson(true).c_str());
}

/**
 * @brief This function is called when a node syncs its own time with that of the mesh.
 */
// needed for the mesh to work
void nodeTimeAdjustedCallback(int32_t offset) {
  debug3("Adjusted time ");
  debug3(mesh.getNodeTime());
  debug3(". Offset = ");
  debugln3(offset);
}

//***********************************************************************
//*********************** Setup mesh when needed ************************
/**
 * @brief Setup the mesh network when we need it.
 * 
 */
void setupMesh(){
  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes( ERROR | STARTUP );  

  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}
