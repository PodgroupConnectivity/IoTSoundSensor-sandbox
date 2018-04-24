from network import LoRa
import socket
import time
import binascii
import pycom

# Same as example 1 but with led colors for debugging

# Colors for showing the status
colors = {
    "blue": 0x2139D4,
    "green": 0x007f00,
    "yellow": 0x7f7f00,
    "red": 0x7f0000,
    "off": 0x000000         # no color, led off
}

# Disabling hearbeat for using led colors
pycom.heartbeat(False)

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
print("Connecting to LoRa... ", end='')

# wait until the module has joined the network
while not lora.has_joined():
    pycom.rgbled(colors["blue"])    # Blue meaning connecting to LoRa
    time.sleep(5)
    print('...Not joined yet ', end="")

pycom.rgbled(colors["green"])    # green meaning connected to LoRa
print("...Connected to LoRa!")
time.sleep(2)
pycom.rgbled(colors["off"])   # led off after some time

# create a LoRa socket
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)

# set the LoRaWAN data rate
s.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)

# make the socket blocking
# (waits for the data to be sent and for the 2 receive windows to expire)
s.setblocking(True)

# to send the data it should be encoded as strings
my_greeting = "Hello World!"
data_bytes = my_greeting.encode('utf_8')

# sending the data already converted to bytes
s.send(data_bytes)

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
        pycom.rgbled(colors["yellow"])      # Yellow = sending data
        bytes_sent = s.send(data_bytes)
        print("Sent " + str(bytes_sent) + "B")
        time.sleep(0.5)
        pycom.rgbled(colors["off"])      # Yellow = sending data

        # get any data received...
        data = s.recv(64)
        if len(data) > 0:
            print(data)
    except:
        pass
    time.sleep(10)
