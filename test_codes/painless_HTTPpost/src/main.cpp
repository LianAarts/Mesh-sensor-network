
//-------------------------------//
//    MESH NETWORK IUT           //
//    INTERNSHIP BELGIUM        //
//        2021-2022             //
// -----------------------------//


#include <painlessMesh.h>
#include <ArduinoHttpClient.h>
#include "WiFi.h"
#include "secret.h"

painlessMesh  mesh;
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, 8123);
Scheduler userScheduler;

void MessageReceived( const uint32_t &from, const String &msg );
void sendMessage() ;
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage() {
  String msg = "NodeData1";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

void post(String sensorVal, const uint32_t name, String unit) { 
  client.beginRequest(); 
  String httpRequestData = "{\"state\":\""+ String(sensorVal);
  String httpAtributes = "\", \"attributes\": {\"unit_of_measurement\": \"" + unit;
  String toPost = httpRequestData + httpAtributes + "\"}}";

  client.post("/api/states/sensor." + name);
  client.sendHeader("Authorization", TOKEN);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", toPost.  length());
  client.endRequest();
  client.print(toPost);
  Serial.println(client.responseBody());
}

void MessageReceived( const uint32_t &from, const String &msg ) {
  Serial.printf("Message from %u with msg=%s\n", from, msg.c_str()); 
  if(mesh.isRoot()){
      // mesh.setRoot(true);
      Serial.printf("Node %u is root. Send data to HA...\n", from);
      post(msg, from, ""); 
  }else{
    mesh.setContainsRoot(true);
    Serial.print("Node is not root. Sending data to root..");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  Serial.println("Connected to the WiFi network");
  Serial.print("SSID: ");Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();Serial.print("IP Address: ");Serial.println(ip);

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | GENERAL | MSG_TYPES | REMOTE );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&MessageReceived);
  mesh.stationManual(ssid, pass);
  mesh.setHostname(HOSTNAME);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}
