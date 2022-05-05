#include <Arduino.h>

// #include "functions.h" //! make name better and use "", ifndef
#include "rd_22_api.h"            // REST server for
#include "rd_22_configuration.h"  //sensitive passwords
#include "rd_22_mesh.h"           //mesh part of the code
#include "rd_22_node_class.h"
#include "rd_22_setup_portal.h"  //SPIFFS and captive setup portal
#include "rd_22_wifi.h"  //wifi connection and api POST to Home Assistant
// all the libraries we need
// #define bme280

#ifdef bme280
#include "rd_22_280sensor.h"
#define setupSensor() setupSensor280()
#define makeSensorMessage(a, b, c, d) makeSensorMessage280(a, b, c, d)
#endif
#ifdef bme680
#include "rd_22_680sensor.h"
#define setupSensor() setupSensor680()
#define makeSensorMessage(a, b, c, d) makeSensorMessage680(a, b, c, d)
#endif

#define DEBUG DEBUG_LEVEL
#include "rd_22_debug.h"

#define LED 2
// this led will show the root status
// #define buttonPin 16
// button pin we need for the reset of the eeprom

bool hasRun = false;
bool serverNeeded = false;
// variables used for decision making

//! put in class or header
// variable that will store the identifier of the node

int lastTimeButton = 0;
int lastTime = 0;
int RSSI = 0;
// timing variable for reset button (press 10 seconds for)

String nodeName = "";

//!

//! use namespace

//***********************************************************************
//******************************** setup ********************************
void setup() {
  Serial.begin(115200);
  debugln2("test");
  pinMode(LED, OUTPUT);
  // pinMode(buttonPin, INPUT);
  digitalWrite(LED, LOW);
  // setup button and led, turn off led

  nodeName = readSpiffs("/assets.txt");
  // we read the eeprom
  if (nodeName == "") {
    // if the eeprom is empty we setup the ap where we can enter our name of the
    // sensor
    setupWebserver();
    serverNeeded = true;
    // when the server was needed we also have to shut this server down so we
    // can setup our mesh
    debugln2("server is needed");
  }
  delay(1000);  //! remove maybe?
  setupSensor();
  // start the BME680 sensor
}

//***********************************************************************
//******************************** loop *********************************
void loop() {
  nextDnsRequest();
  nodeName = readSpiffs("/assets.txt");

  // while we don't have a sensorname we have to wait
  // the dns is used for the captive window

  while (nodeName != "") {
    // when we have a sensorname we enter the main loop
    //? Only run this one time (setup of the mesh network)
    if (hasRun == false) {
      // we have to run this part once after we got our sensorname
      if (serverNeeded == true) {
        endWebserver();
        // kill the webserver when it was needed
        writeSpiffs(nodeName, "/assets.txt");
        // we got a name if the server was needed
        // write this name to the memory
      }
      debug2("Setup is finnished!! The sensor ID is: ");
      debugln2(nodeName);
      delay(100);
      debugln2("Let's check the wifi strength!!!");
      RSSI = networkScan();
      node.setRSSI(RSSI);
      // if we have our RSSI we set this value in the mesh library
      // check the wifi strength of the Home Assistant AP
      if (RSSI == -100) {
        node.setRootFound(true);
        node.setIAmRoot(false);
        // if we are not in range of the AP we cannot be the root
      }
      delay(100);
      setupMesh();
      // we setup the mesh after we checked the wifi strength
      hasRun = true;
      // don't run this part again
    }

    updateMesh();
    apiCheck();
    // this has to be done as often as possible
    // this will be the only things that are executed on this thread

    // if (digitalRead(buttonPin) == LOW and ((millis() - lastTimeButton) >
    // timerDelayButton)){
    //   // when the button is pressed for 10 seconds we reset the eeprom and
    //   restart the esp debug1(digitalRead(buttonPin)); debugln1(" Button has
    //   been pressed for 10 seconds, ESP will be reset"); writeSpiffs("",
    //   "/assets.txt");
    //   // write a empty string
    //   ESP.restart();
    // }
    // if (digitalRead(buttonPin) == HIGH){
    //   lastTimeButton = millis();
    //   // if the buttonpin is not pressed we reset the timer
    // }

    if (!node.rootFound and ((millis() - lastTime) > timerDelay)) {
      // as long we have not selected a root send a message every 10 seconds
      Serial.println("Selecting root...");
      String connectionStrength =
          "[{\"bn\": \"connectionTest\", \"RSSI\": " + String(RSSI);
      // make the message in JSON format
      sendBroadcast(connectionStrength + "}]");
      // send a message with RSSI information
      lastTime = millis();
      // reset the timer
    } else if ((millis() - lastTime) > timerDelay) {
      // if a root has been found do this every 10 seconds
      if (node.iAmRoot) {
        // if this is the root
        digitalWrite(LED, HIGH);
        // turn the blue led on
        Serial.println("I am the root");
        String rootStatus = "[{\"bn\": \"Root\"";
        sendBroadcast(rootStatus + "}]");
        // make and broadcast the root is alive message
        splitJson(makeSensorMessage(true, nodeName, getIp(), node.nodeID));
        // we read the sensor and post everything to Home Assistant
      } else {
        digitalWrite(LED, LOW);
        // if we are not the root we turn off the blue light
        Serial.println("Not root");
        if (node.rootID == 0) {
          sendBroadcast(
              makeSensorMessage(false, nodeName, getIp(), node.nodeID));
        } else {
          sendSingleMessage(
              makeSensorMessage(false, nodeName, getIp(), node.nodeID),
              node.rootID);
        }
        // the sensor is read and we send it to the root
        if (rootTimer() && !node.iAmRoot) {
          // if we are not the root and we have not received a message from the
          // root for 40 seconds reset the esp (start the rootselection again)
          ESP.restart();
        }
      }
      lastTime = millis();
      // reset the timer
    }
  }
}