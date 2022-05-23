/**
 * @file rd_22_configuration.h
 * @author Lian Aarts
 * @brief Configuration file for the mesh nodes
 * @date 2022-05-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/**
 * @brief SSID of the mesh network.
 * 
 */
#define MESH_PREFIX_secr "LiansNetwork2"

/**
 * @brief Password of the mesh network.
 * 
 */
#define MESH_PASSWORD_secr "LiansNetwork2"

/**
 * @brief Port of the mesh network.
 * 
 */
#define MESH_PORT_conf 5555

/**
 * @brief Bearer token for Home Assistant
 * 
 * Long lived access token in Home Assistant (https://developers.home-assistant.io/docs/auth_api/)
 * 
 */
#define token_secr "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiI4YTMyYWI2ZDMzNGE0MjIzODM5MzIzYzQ4MTE5YzE2OCIsImlhdCI6MTY0OTE2Mjc1NiwiZXhwIjoxOTY0NTIyNzU2fQ.AXA7uUs4cRBxNI0JDZGX1U9fqrxuLeWr_XiN5_jRndk"

/**
 * @brief SSID of the network the root needs to be connected to.
 * 
 */
#define ssidScan_secr "home-assistant-AP"

/**
 * @brief Password of the network the root needs to connect to.
 * 
 */
#define passScan_secr "networkubdx"

/**
 * @brief SSID for the configuration access point of the node.
 * 
 */
#define AP_NAME "New Node"

/**@brief The debug level will decide how much information is printed in the serial monitor
 * 
 * 1: only critical information (errors)
 * 
 * 2: state of code
 * 
 * 3: verbose, print everything that is happening 
 * 
*/
#define DEBUG_LEVEL 3

/**
 * @brief Address for the API POST request.
 */
#define SERVERNAME "http://192.168.99.1:8123/api/states/sensor."


/**
 * @brief Interval between messages to the mesh network.
 * 
 * if sensornode: single message with sensordata will be send
 * 
 * if rootnode: broadcast essage with root information will be send
 */
#define timerDelay 10000

/**
 * @brief The amount of time the button has to be pressed for a name reset.
 */
#define timerDelayButton 10000


/**
 * @brief Reset the node after this interval.
 * 
 * When a node doesn't receieve a message from the root
 * for this period of time it will reset itsself.
 * 
 * The root node will send messages at set intervals defined in #timerDelay.
 * This should always be atleast two times the #timerDelay.
 * 
 * Time in miliseconds.
 */
#define timerDelayRoot 40000

/**
 * @brief The nodes gets assigned as root after this interval
 * 
 * When a node doesn't receieve RSSI information from other 
 * nodes for this period of time it will assign itself as root.
 * 
 * Time in miliseconds.
 */
#define timerDelayRSSI 30000

/**
 * @brief Define which sensor you want to use.
 * 
 * bme680 or bme280
 */
// #define bme280
#define bme680