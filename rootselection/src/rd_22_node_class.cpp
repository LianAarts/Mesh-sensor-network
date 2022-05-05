#include <Arduino.h>
#include "rd_22_node_class.h"

// These functions are not really necessary, I might expand on this later.
// Why use classes with no functions :O

/**
 * @brief Set the address of the root node.
 *
 * @param ID chipID of the root node.
 */
void Node_Information::setRootID(int ID) { 
    rootID = ID; 
}

/**
 * @brief Set the node ID value
 * 
 * @param ID The Node ID
 */
void Node_Information::setNodeID(String ID) { 
    nodeID = ID;
}

/**
 * @brief Set root status.
 *
 * @param state true: this is the root, false: this is not root.
 */
void Node_Information::setIAmRoot(bool state) { 
    iAmRoot = state; 
}

/**
 * @brief Set root found status.
 *
 * @param state true: root is found, false: root is not found.
 */
void Node_Information::setRootFound(bool state) { 
    rootFound = state; 
}

/**
 * @brief Set the RSSI value.
 * Received Signal Strength Indicator (RSSI) is the measurement of how strong
 * the signal of a router or access point is to a device. We use this
 * measurement to determine which node has the better connection to the Home
 * Assistant access point.
 *
 * @param rssi RSSI value (between -100 and 0).
 */
void Node_Information::setRSSI(int rssi) { 
    RSSI = rssi; 
}

/**
 * @brief Get the address of the root node.
 * We send our measurements to this address.
 *
 * @return int Root chip ID.
 */
int Node_Information::getRootID() { 
    return (rootID); 
}

/**
 * @brief Get the Node Id of this node.
 *
 * @return String Node ID.
 */
String Node_Information::getNodeID() { 
    return (nodeID); 
}

/**
 * @brief Get the RSSI of this node
 * 
 * @return int RSSI value
 */
int Node_Information::getRSSI() { 
    return (RSSI); 
}

/**
 * @brief Returns the root status of this node.
 *
 * @return true This node is the root.
 * @return false This node is not root.
 */
bool Node_Information::getIAmRoot() { 
    return (iAmRoot); 
}

/**
 * @brief Return the state of the root.
 *
 * @return true If there is a root in the network.
 * @return false If there is no root in the network.
 */
bool Node_Information::getRootFound() { 
    return (rootFound); 
}

