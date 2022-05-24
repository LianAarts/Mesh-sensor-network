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
RUN chmod a+x /send_alerts.py

RUN pip freeze > requirements.txt && \
    pip install requests

CMD [ "python3", "./send_alerts.py" ]
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
#                      Alerts                      #             
# -------------------------------------------------#
#imports
import requests
import json
import time

list_temps = []
list_gas = []
list_humidity = []
treshold_temp =  8
treshold_gas = 15
treshold_humidity = 12

def avg_float(list_temps):
    sum = 0
    for ele in list_temps:
        sum += float(ele)
    return sum / len(list_temps)

while True:
    head = {'Authorization': 'Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiI4YTMyYWI2ZDMzNGE0MjIzODM5MzIzYzQ4MTE5YzE2OCIsImlhdCI6MTY0OTE2Mjc1NiwiZXhwIjoxOTY0NTIyNzU2fQ.AXA7uUs4cRBxNI0JDZGX1U9fqrxuLeWr_XiN5_jRndk'}
    r = requests.get('http://192.168.29.80:8123/api/states', headers=head)
    data = r.json()
    for item in data:

        #Temperature Alert
        if("temperature" in item['entity_id'].strip('"') and item['state'] !="unknown"):
            nodeId =  item['entity_id'].strip('"').split(".")[1].split("_")[0]
            temperature =  item['state']
            if temperature not in list_temps:
                list_temps.append(temperature)
            add_perc_to_avg = str(((treshold_temp * avg_float(list_temps)) / 100) +  avg_float(list_temps))
            if temperature > add_perc_to_avg:
                data = {"sort" : "temperature", "data" : {"nodeId" : nodeId, "state": temperature, "attr": "°C"}}
                sendIP = 'http://192.168.29.80:8123/api/webhook/jeroen-notify-9dFNWgQEI_lhWSpxII7TpH9O'
                headers = {'Content-type': 'application/json'}
                r = requests.post(sendIP, data=json.dumps(data), headers=headers)
                if r.status_code == 200:
                    print(json.dumps(data))
                    print("Temperature of", nodeId , "went up to", temperature ,"°C! Sending a notify!")
                    time.sleep(10)
                else:
                    print(r) 

        # gas alert
        if("gas" in item['entity_id'].strip('"')):
            nodeId =  item['entity_id'].strip('"').split(".")[1].split("_")[0]
            gas =  item['state']
            if gas not in list_gas:
                list_gas.append(gas)
            add_perc_to_avg = str(((treshold_gas * avg_float(list_gas)) / 100) +  avg_float(list_gas))
            if 1750 > float(gas) > 750:
                if gas > add_perc_to_avg:
                    data = {"sort" : "air quality", "data":{"nodeId" : nodeId, "state": gas}}
                    sendIP = 'http://192.168.29.80:8123/api/webhook/alert-monitoring-isZdBIppCIzpc_XNNt5mSm12'
                    headers = {'Content-type': 'application/json'}
                    r = requests.post(sendIP, data=json.dumps(data), headers=headers)
                    print(json.dumps(data))
                    if r.status_code == 200:
                        print("Air Quality of", nodeId , "is getting a weird value, sending a notify!")
                        time.sleep(5)
                    else:
                        print(r) 
        
        # humidity alert
        if("humidity" in item['entity_id'].strip('"') and item['state'] !="unknown"):
            nodeId =  item['entity_id'].strip('"').split(".")[1].split("_")[0]
            humidity =  item['state']
            if humidity not in list_humidity:
                list_humidity.append(humidity)
            add_perc_to_avg = str(((treshold_humidity * avg_float(list_humidity)) / 100) +  avg_float(list_humidity))
            if  80 > float(humidity) > 10:
                if humidity > add_perc_to_avg:
                    data = {"sort" : "humidity", "data":{"nodeId" : nodeId, "state": humidity, "attr": "%"}}
                    sendIP = 'http://192.168.29.80:8123/api/webhook/alert-monitoring-isZdBIppCIzpc_XNNt5mSm12'
                    headers = {'Content-type': 'application/json'}
                    r = requests.post(sendIP, data=json.dumps(data), headers=headers)
                    print(json.dumps(data))
                    if r.status_code == 200:
                        print("Humidity of", nodeId , ", sending alert!")
                    else:
                        print(r) 

            
            time.sleep(1)
         

```
