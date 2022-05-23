#ifndef rd_22_node_class
#define rd_22_node_class

/**
 * @brief Class of a node.
 * 
 * This class will contain all the information about the node.
 *
 */
class Node_Information {
 public:
   /**
   * @brief The chip ID of this node
   * 
   */
  String nodeID;

  /**
   * @brief Chip ID of the root node, we have to send messages to this node
   * 
   */
  int rootID;

  /**
  * @brief RSSI value of the node and the Home Assistant AP
  * 
  */
  int RSSI;

  /**
   * @brief This value is true when this node is the root
   * 
   */
  bool iAmRoot;
  
  /**
   * @brief This value is true when there is a root node in the network
   * 
   */
  bool rootFound;


  void setNodeID(String Id);
  void setRSSI(int rssi);
  void setRootID(int address);
  void setIAmRoot(bool state);
  void setRootFound(bool state);

  String getNodeID();

  int getRSSI();
  int getRootID();

  bool getIAmRoot();
  bool getRootFound();
};

#endif