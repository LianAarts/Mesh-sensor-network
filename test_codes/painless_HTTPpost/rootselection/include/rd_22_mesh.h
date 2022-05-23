#ifndef rd_22_mesh
#define rd_22_mesh

#include "rd_22_node_class.h"

extern Node_Information node;

void sendSingleMessage(String message, int address);
void sendBroadcast(String message);
void setupMesh();
void updateMesh();

bool rootTimer();

#endif