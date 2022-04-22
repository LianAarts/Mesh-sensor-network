#ifndef rd_22_mesh
#define rd_22_mesh

void sendSingleMessage(String message, int address);
void sendBroadcast(String message);
void setupMesh();
void updateMesh();

String getNodeIdMesh();

int getRootAddress();

bool getIAmRoot();
bool getRootFound();
bool rootTimer();

void setIAmRoot(bool state);
void setRootFound(bool state);
void setRSSI(int rssi);

#endif