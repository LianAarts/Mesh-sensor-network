from __future__ import annotations
import sys
import argparse

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    
def checkIfHasDigits(s):
    return any(i.isdigit() for i in s)

# args
parser = argparse.ArgumentParser(description='A program to generate the secret file')
parser.add_argument("-m", "--meshSSID", help="The Mesh SSID for the mesh system", default="meshDefault")
parser.add_argument("-M", "--meshPassword", help="The Mesh Password for the mesh system", default="meshPassDefault")
parser.add_argument("-b", "--bearerToken", help="The Bearer token of yout Home Assistant system", default="eyxxxxxxxxxxxxxx", required=True)
parser.add_argument("-a", "--appSSID", help="The AP SSID", default="APSSID")
parser.add_argument("-A", "--appPAssword", help="The Password for your AP SSID", default="APPassword",)

args = parser.parse_args()
filename = sys.argv[0]
meshSSID = sys.argv[2]
meshPassword = sys.argv[4]
bearerToken = sys.argv[6]
apSSID = sys.argv[8]
apPassword = sys.argv[10]


try:
        with open('lab/rootselection/secret.h', 'w') as f:
            f.write("// Secret.h configured with the python configurator. \n \n")
            f.write('#define MESH_PREFIX_secr      "' + meshSSID + '"  \n')
            # Password needs to bigger than 5 chars and needs to include a digit
            if(len(meshPassword) > 5 and checkIfHasDigits(meshPassword)):
                f.write('#define MESH_PASSWORD_secr      "' + meshPassword + '"  \n')
            else:
                print(bcolors.FAIL + "FAILED" + bcolors.ENDC + ", Please make a longer password with digits and letters. \n")

            #bearer needs to start with 'ey' for good 64encoding.
            if(bearerToken[0:2] == "ey"):
                f.write('#define token_secr      "' + bearerToken+ '"\n')
            else:
                print(bcolors.FAIL + "FAILED" + bcolors.ENDC + ", Please enter a valid Bearer token. \n")
                sys.exit()

            f.write('#define ssidScan_secr      "' + apSSID + '" \n')
            f.write('#define passScan_secr      "' + apPassword + '" \n')

            print(bcolors.OKGREEN + "Success" + bcolors.ENDC + ", The file is made in 'lab/rootselection/secret.h' ")
except:
    e = sys.exc_info()[0]
    print( "Error:", e )


