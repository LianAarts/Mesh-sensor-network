# -------------------------------------------------#
#                 NodeId check                     #             
# -------------------------------------------------#
#imports
import requests
import time
import json
import os


# Empty dict and lastUpdated
dataforDb = {}
dataforLiveJSON = {}
dataforBeforeLive = 0
lastChangedList = []
hasRun = False

print("Initializing.. please wait.")
# time.sleep(2)
head = {'Authorization': 'Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiI4YTMyYWI2ZDMzNGE0MjIzODM5MzIzYzQ4MTE5YzE2OCIsImlhdCI6MTY0OTE2Mjc1NiwiZXhwIjoxOTY0NTIyNzU2fQ.AXA7uUs4cRBxNI0JDZGX1U9fqrxuLeWr_XiN5_jRndk'}
r = requests.get('http://192.168.29.80:8123/api/states', headers=head)
dataHAforDb = r.json()
for item in dataHAforDb:
    if("chipid" in item['entity_id'].strip('"')):
        sensorName =  item['entity_id'].strip('"').split(".")[1].split("_")[0]
        chipId = item['state']
        lastChanged = item['last_changed'].split("+")[0]
        dataforDb[sensorName] = chipId
        with open('dataDB.json', 'w+') as outfile:
            json.dump(dataforDb, outfile, indent=4, sort_keys=True)
        
print("Configuration succeeded, starting the checker... ")
print("Checker is running! Waiting for feedback from mesh. \n \n")

while True:
    

    # CURRENT FROM HA
    hasrun = True
    dataforLive = {}
    head = {'Authorization': 'Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiI4YTMyYWI2ZDMzNGE0MjIzODM5MzIzYzQ4MTE5YzE2OCIsImlhdCI6MTY0OTE2Mjc1NiwiZXhwIjoxOTY0NTIyNzU2fQ.AXA7uUs4cRBxNI0JDZGX1U9fqrxuLeWr_XiN5_jRndk'}
    r = requests.get('http://192.168.29.80:8123/api/states', headers=head)
    dataHAforLiveCheck = r.json()
    for item in dataHAforLiveCheck:
        if("chipid" in item['entity_id'].strip('"')):
            sensorName =  item['entity_id'].strip('"').split(".")[1].split("_")[0]
            chipId = item['state']
            lastChanged = item['last_changed'].split("+")[0]
            dataforLiveJSON[sensorName] = chipId
    #-------------------#

    # FROM DB
    with open('dataDB.json', 'r') as openfile:
        if os.stat('dataDB.json').st_size == 0:
            print("Home Assistant does not have any running nodes, please check HA or the mesh.")
            #create empty dict because HA has no running nodes.
            dataFromDbFromFile = {}
            #for no serial overload
            time.sleep(1)
        else:
            # read the nodes in json.load
            dataFromDbFromFile = json.load(openfile)
    #-------------------

     # GET ROOTIP
    for item in dataHAforLiveCheck:
        if("rootip" in item['entity_id'].strip('"')):
            if item['state'] != "0.0.0.0":
                rootIP = item['state']
    #--------------------

    # CHANGE IN NODE NAME
    if dataforLiveJSON != dataFromDbFromFile:
        for nodeID in dataforLiveJSON.keys() & dataFromDbFromFile.keys():
            if dataFromDbFromFile[nodeID] != dataforLiveJSON[nodeID]:
                #ChipID from nodeID changed
                data = {'chipID' : dataforLiveJSON[nodeID]}
                sendIP = 'http://' + rootIP + '/usedNameID'
                headers = {'Content-type': 'application/json'}
                r = requests.post(sendIP, data=json.dumps(data), headers=headers)
                if r.status_code == 200:
                    print("Node change detected. Reboot request sended to node", nodeID, "!")
                    print("Please give node", nodeID, "another name. \n")
                    #wait for the next reboot request.
                    time.sleep(2.5)
                else:
                    print("Error while sending the reset request to node", nodeID, "! \n")
                    print(r)
        
    # ADDED NODE    
    if len(dataforDb) != len(dataforLiveJSON):
        for item in dataHAforLiveCheck:
            if("chipid" in item['entity_id'].strip('"')):
                last_changed =  item['last_changed'].split("+")[0]
                lastChangedList.append(lastChanged)
                latestChangedItem = max(lastChangedList)
                if latestChangedItem == last_changed:
                    sensorName =  item['entity_id'].strip('"').split(".")[1].split("_")[0]
                    chipId = item['state']
                    if sensorName not in dataforDb:
                        dataforDb[sensorName] = chipId
                        print("added node:", sensorName, dataforDb)
                        with open('dataDB.json', 'w+') as outfile:
                            json.dump(dataforDb, outfile, indent=4, sort_keys=True)
                        
    time.sleep(2)




