# RD

This project aims to study, implement and deploy a network of environmental sensors (e.g. temperature, humidity) covering all the rooms of the Department Informatique building. The final purpose of the project is to be able to perform statistics on these environmental data but also to send some alerts in case of an anomaly (e.g. rapid drop in temperature). 
Because of the size of the building, it is impossible to set up a centralized solution as is the case for most home automation systems. Moreover, the Wi-Fi infrastructure provided by the university cannot be used due to security reasons. Therefore, we are interested in the implementation of an autonomous mesh network. 
The proposed approach will be to deploy each sensor with an ESP32 that can manage such a mesh network. For the data collection, visualization, and alert systems, we propose to set up a small Linux server running on top of an intelligent home software like Home Assistant. 
The project consists of the following stages:
•	A global study of both hardware & software.
•	Demonstration of a mesh network of ESP32s.
•	Study and choice of an environmental sensor. 
•	Study, installation, and configuration of a smart home server.
•	Integration of the smart home server into the mesh network. 
