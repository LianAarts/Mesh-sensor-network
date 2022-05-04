# Tutorial and Code Explanation of File Generator
When setting up the system, it needs a file with all the credentials and configured names to use in the mesh.
To easy this process we made a python script that can configure this file on its own with a few parameters from the command line (Linux/Windows/MacOS)


## Tutorial 
### Python Interpreter
For this programm, you will need to have python installed. Please make sure you have installed python 3.x.x and above! If not, please follow the instructions listed on <a href="https://www.tutorialspoint.com/how-to-install-python-in-windows">this page</a>!

### Required imports
For the programm to work you will need to add some imports to python.
```python
import sys
import argparse
```
 
With this pip command you can install the required packages:
```
sudo pip install sys argparse
```

### Run the program.
You can find the file in the folder __'file_generator\secretfile\secret_file_generator.py'__
Please go to this folder on your pc and execute as following:
```bash
sudo python3 gen.py -m <meshSSID> -m <meshPassword> - b <bearerToken> -a <appSSID> -A <appPassword>
```
Please fill in the information that corresponds with your setup.
- **-m** Your mesh SSID (can be anything)
- **-m** Your mesh Password (can be anything)
- **-b** Your retreived bearerToken from Home Assistant as explained <a href="https://www.home-assistant.io/docs/authentication/"> here </a>
- **-a** The acces points SSID
    - ex. Home Assistant AP SSID to perform the post requests to.
- **-A** The access points password
  - ex. Home Assistant AP password to perform the post requests to.

When succeeded on execution the python file, the programm will generate a file that can be used directly in the program without any further steps.

The file can be found under: __'rootselection/scr/secret.h'__


## Code explanation
Here are some of the most imported snippets from the code.

### Imports
- sys is the abbreviation of system
- argparse will take care of the arguments that will be provided in the command line.
```python
import sys
import argparse
```

### Parser
The following code will say to the program that this is a program which needs arguments.
```python
parser = argparse.ArgumentParser(description='A program to generate the secret file')
```
#### add an argument
This can be done by the statement parser.add_argument(). In our program we are using  4 values. 
-  The parameter abbreviation (-m)
-  The full parameter name (--meshSSID)
-  The explanation for this parameter. (help)
-  The deault value if this parameter is empty. (default)
-  If the value is required. (required True/False)
```python
parser.add_argument("-m", "--meshSSID", help="The Mesh SSID for the mesh system", default="meshSSID", required=True)
```

### File handeling
This code needs to read the arguments and write this into a file. In the code we do this as following:

```python
with open('lab/rootselection/secret.h', 'w') as f:
  # do stuff with f.

#f is closed.
```
This will open the file and make it accessible with the f variable.
You may use f as a variable in the **indent** ass you wish. After the indent it will close the file automatically.

#### file writing
```python
f.write('#define MESH_PASSWORD_secr      "' + meshPassword + '"  \n')
```
With the statement 'f.write' you can write a line to the file. 