#include <Arduino.h>
#include <WebServer.h>

// #include "functions.h" //! make name better and use "", ifndef
#include "secret.h" //sensitive passwords

#include "rd_22_mesh.h" //mesh part of the code
#include "rd_22_setup_portal.h" //SPIFFS and captive setup portal 
#include "rd_22_wifi.h" //wifi connection and api POST to Home Assistant
#include "rd_22_json.h" //Json message decoding
#include "rd_22_api.h" // REST server for 
// all the libraries we need

#define LED 2
// this led will show the root status
// #define buttonPin 16
// button pin we need for the reset of the eeprom

bool hasRun = false;
bool serverNeeded = false;
//variables used for decision making

//! put in class or header
// variable that will store the identifier of the node

int lastTimeButton = 0;
int timerDelayButton = 10000;
// timing variable for reset button (press 10 seconds for)

int RSSI = 0;

String nodeName = "";

//! 

//! use namespace

void meshRoutine( void * parameter ) {
  // threading routing that will run side by side the main loop
  for(;;) { 
    if(!getRootFound()){
      // as long we have not selected a root send a message every 10 seconds
      Serial.println("Selecting root...");
      String connectionStrength = "[{\"bn\": \"connectionTest\", \"RSSI\": " + String(RSSI);
      // make the message in JSON format
      sendBroadcast(connectionStrength + "}]");
      // send a message with RSSI information
    }
    else{
      // if a root has been found do this every 10 seconds
      if (getIAmRoot()){
        // if this is the root
        digitalWrite(LED, HIGH);
        // turn the blue led on
        Serial.println("I am the root");
        String rootStatus = "[{\"bn\": \"Root\"";
        sendBroadcast(rootStatus + "}]");
        // make and broadcast the root is alive message
        postJson(makeSensorMessage(true, nodeName, getIp(), getNodeIdMesh()));
        // we read the sensor and post everything to Home Assistant
      }
      else{
        digitalWrite(LED, LOW);
        // if we are not the root we turn off the blue light
        Serial.println("Not root");
        if (getRootAddress() == 0){
          sendBroadcast(makeSensorMessage(false, nodeName, getIp(), getNodeIdMesh()));
        }
        else{
          sendSingleMessage(makeSensorMessage(false, nodeName, getIp(), getNodeIdMesh()), getRootAddress());
        }
        // the sensor is read and we send it to the root
        if (rootTimer() && !getIAmRoot()){
          // if we are not the root and we have not received a message from the root for 40 seconds
          // reset the esp (start the rootselection again)
          ESP.restart();
        }
      }
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS); 
    // execute this every 10 seconds
  }
  /* delete a task when finish, this will never
  happen because this is infinity loop */ 
  vTaskDelete( NULL );
}

//***********************************************************************
//******************************** setup ********************************
void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  // pinMode(buttonPin, INPUT);
  digitalWrite(LED, LOW);
  // setup button and led, turn off led

  nodeName = readSpiffs();
  // we read the eeprom
  if (nodeName == ""){
    // if the eeprom is empty we setup the ap where we can enter our name of the sensor
    setupWebserver();
    serverNeeded = true;
    // when the server was needed we also have to shut this server down so we can setup our mesh
  }
  delay(1000); //! remove maybe?
  setupSensor();
  // start the BME680 sensor
  
  xTaskCreate(
  // task that will run every 10 seconds
  // this will use multithreading so we don't interupt the mesh network
    meshRoutine,
    "mesh routine",
    10000,
    NULL,
    1,
    NULL
  );
}

//***********************************************************************
//******************************** loop *********************************
void loop() {
  nextDnsRequest();
  nodeName = readSpiffs();

  // while we don't have a sensorname we have to wait
  // the dns is used for the captive window
  
  while (nodeName != ""){
    // when we have a sensorname we enter the main loop
    //? Only run this one time (setup of the mesh network)
    if(hasRun == false){
      // we have to run this part once after we got our sensorname
      if (serverNeeded == true){
        endWebserver();
        // kill the webserver when it was needed
        writeSpiffs(nodeName);
        // we got a name if the server was needed
        // write this name to the memory
      }
      Serial.print("Setup is finnished!! The sensor ID is: ");
      Serial.println(nodeName);
      delay(100);
      Serial.println("Let's check the wifi strength!!!");
      RSSI = networkScan();
      setRSSI(RSSI);
      // if we have our RSSI we set this value in the mesh library
      // check the wifi strength of the Home Assistant AP
      if (RSSI == -100){
        setRootFound(true);
        setIAmRoot(false);
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
  }
}