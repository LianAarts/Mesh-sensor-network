#define MESH_PREFIX_secr "LiansNetwork2"
#define MESH_PASSWORD_secr "LiansNetwork2"
#define MESH_PORT_conf 5555
#define token_secr                                                             \
  "Bearer "                                                                    \
  "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9."                                      \
  "eyJpc3MiOiI4YTMyYWI2ZDMzNGE0MjIzODM5MzIzYzQ4MTE5YzE2OCIsImlhdCI6MTY0OTE2Mj" \
  "c1NiwiZXhwIjoxOTY0NTIyNzU2fQ.AXA7uUs4cRBxNI0JDZGX1U9fqrxuLeWr_XiN5_jRndk"
#define ssidScan_secr "home-assistant-AP"
#define passScan_secr "networkubdx"

#define AP_NAME "New Node"

#define DEBUG_LEVEL 3
// define debug level
// 1: only critical information (errors)
// 2: state of code
// 3: verbose, print everything that is happening

#define SERVERNAME "http://192.168.99.1:8123/api/states/sensor."
// API POST address

#define timerDelay 10000
// interval between mesh messages

#define timerDelayButton 10000
// how long does the button have to be pressed before name reset

#define timerDelayRoot 40000
// when no root for given time we reset

#define timerDelayRSSI 30000
// assign root after this interval
// 30000 is 30 seconds

// #define bme280
#define bme680
// choose the right sensor (uncomment for BME280)