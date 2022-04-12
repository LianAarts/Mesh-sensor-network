import sys
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

print(bcolors.OKBLUE + "------------------------------------")
print("  File Generator - Mesh IoT Study  ")
print("------------------------------------" +bcolors.ENDC)

print("Welcome, let's generate the secret file!\nThis overwrites the current secret.h in this folder. \n" )

try:
    with open('lab/file_generator/secret.h', 'w') as f:
        f.write("// Secret.h configured with the python configurator. \n \n")

        print(bcolors.BOLD + "Enter a Mesh SSID:" + bcolors.ENDC, end=" ")
        meshSSID = input()
        f.write('#define MESH_PREFIX_secr      "' + meshSSID + '"  \n')

        print(bcolors.BOLD +  "Enter a Mesh Password: "+ bcolors.ENDC, end=" ")
        meshPassword = input()
        f.write('#define MESH_PASSWORD_secr      "' + meshPassword + '"  \n')
        
        print(bcolors.BOLD +  "Please enter your Bearer token from Home Assistant"+ bcolors.ENDC, end=" ")
        bearerToken = input()
        if(bearerToken[0:2] == "ey"):
            f.write('#define token_secr      "' + bearerToken+ '"\n')
        else:
            print(bcolors.FAIL + "FAILED" + bcolors.ENDC + ", Please enter a valid Bearer token. \n")
            sys.exit()
            
        print(bcolors.BOLD +  "Please enter a SSID for your AP" + bcolors.ENDC, end=" ")
        apSSID = input()
        f.write('#define ssidScan_secr      "' + apSSID + '" \n')

        print(bcolors.BOLD +   "Please enter a password for " + apSSID + bcolors.ENDC, end=" ")
        apPassword = input()
        f.write('#define passScan_secr      "' + apPassword + '" \n')

        print(bcolors.OKGREEN + "Success" + bcolors.ENDC + ", The file is made in the current folder! \n")

except:
    e = sys.exc_info()[0]
    print( "Error:", e )

finally:
    f.close()