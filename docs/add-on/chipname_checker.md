# ADD-ON: chipname checker
Add-ons allow the user to extend the functionality around Home Assistant by installing additional applications.

This can be running an application that Home Assistant can integrate with (like an MQTT broker) or to share the configuration via Samba for easy editing from other computers.

## installation
For installation I kindly refer to the 'home_assistant_install.md' in the 'docs' folder and the https://developers.home-assistant.io/docs/add-ons/tutorial/ page.

## Custom add on: chipname Checker
For this add-on we will need 3 files.
- Dockerfile
- requirements.txt
- config.yaml
- chipId_checker.py

### Dockerfile
This is the image that will be used to build your add-on. 
In my Dockerfile we willl get our image to be build on python3.

It is important to have writh and access rights on the python file.

```Dockerfile
FROM python:3

ADD listenToMesh.py /
RUN chmod a+x /checkid.py

RUN pip freeze > requirements.txt && \
    pip install requests

CMD [ "python3", "./checkid.py" ]
```

### Requirements.txt
This file includes the python imports. In my case I only needed to use 3.
```txt
requests
time
os
```


### config.yaml
This is the structure of Home Assistant and how it will be named and displayed inside the add-on store. The following paremeters are configured:
- **name**: The name of the add-on.
- **description**: description of the add on.
- **version**: Version of the add-on. If you are using a docker image with the image option, this needs to match the tag of the image that will be used.
- **slug**:  This needs to be unique in the scope of the repository that the add-on is published in and URI friendly.
- **arch**: A list of supported architectures: armhf, armv7, aarch64, amd64, i386.

```yaml
name: "nodeChecker"
description: "This add-on will check if the configured node name already exist and will an callback to the corresponding node."
version: "2.0"
slug: "chipid"
arch:
  - amd64
```

### File to be run
In my case this will be chipId_checker.py that runs on the Docker image. 
```python
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

```
