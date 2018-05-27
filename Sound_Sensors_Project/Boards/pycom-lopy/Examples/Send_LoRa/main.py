from network import LoRa
import socket
import time
import binascii

# Initialize LoRa in LORAWAN mode.
# The region frequencies are set when loading the firmware of the LoPy
lora = LoRa(mode=LoRa.LORAWAN)

# The autehntication works like this:
# On the server side
#   1) Create a new app in the network console, this will give you an app_eui
#   2) Copy the device_eui in the application, this will generate an app_key
# On the node sidew 
#   1) Copy in the code the app_eui (identifier)
#   2) Copy in the code the app_key (password

app_eui = binascii.unhexlify('****************')
app_key = binascii.unhexlify('********************************')

# join a network using OTAA (Over the Air Activation)
lora.join(activation=LoRa.OTAA, auth=(app_eui, app_key), timeout=0)
print("Preparing the connection to LoRa")

# wait until the module has joined the network
while not lora.has_joined():
    time.sleep(5)
    print('Not yet joined...')

# create a LoRa socket
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)

# set the LoRaWAN data rate
s.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)

# make the socket blocking
# (waits for the data to be sent and for the 2 receive windows to expire)
s.setblocking(True)

# send some data
s.send(bytes([0x01, 0x02, 0x03]))

# make the socket non-blocking
# (because if there's no data received it will block forever...)
s.setblocking(False)

# get any data received (if any...)
data = s.recv(64)
print(data)

# make the socket non-blocking
s.setblocking(False)

# send some data
while True:
    try:
        print("Sending data... ", end="")
        bytes_sent = s.send(bytes([0x01, 0x02, 0x03]))
        print("Sent " + str(bytes_sent) + "B")


        # get any data received...
        data = s.recv(64)
        if len(data) > 0:
            print(data)
    except:
        pass
    time.sleep(10)
