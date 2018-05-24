import machine
import time
from math import log

from network import LoRa
import socket
import time
import binascii
import pycom



## Defining some functions for later use

# Logarithm in base 10 not defined in math
# lib in micropython, only natural logarithm
def log10(x):
    # Logarithm change base formula
    return log(x)/log(10)


# Convert the value from the quantizer to voltage
def adc_to_voltage(adc_value, volt_ref, adc_quantizer_bits):
    volt_per_bit = volt_ref / (2**adc_quantizer_bits)
    voltage = adc_value * volt_per_bit
    return voltage



## LoRa setup

# Colors for showing the LoRa status
colors = {
    "blue"  : 0x2139D4,
    "green" : 0x007f00,
    "yellow": 0x7f7f00,
    "red"   : 0x7f0000,
    "off"   : 0x000000         # no color, led off
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
#   2) Copy in the code the app_key (password)
# Change the '******' with your credentials
app_eui = binascii.unhexlify('****************')
app_key = binascii.unhexlify('********************************')


# Join a network using OTAA (Over the Air Activation)
lora.join(activation=LoRa.OTAA, auth=(app_eui, app_key), timeout=0)
print("Connecting to LoRa network... ", end='')

# Wait until the module has joined the network
while not lora.has_joined():
    pycom.rgbled(colors["blue"])            # Blue meaning connecting to LoRa
    time.sleep(5)
    print('...Not joined yet ', end="")

pycom.rgbled(colors["green"])               # green meaning connected to LoRa
print("...Connected to LoRa!")
time.sleep(2)
pycom.rgbled(colors["off"])                 # led off after some time

# Create a LoRa socket
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)

# Set the LoRaWAN data rate
s.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)

# Make the socket blocking
# (waits for the data to be sent and for the 2 receive windows to expire)
s.setblocking(True)

# Preparing the first message to send
# the data it should be encoded as strings
my_greeting = "Starting communication"
data_bytes = my_greeting.encode('utf_8')
print("Data to send: " + my_greeting)

# Sending the data already converted to bytes
s.send(data_bytes)

# Make the socket non-blocking
# (because if there's no data received it will block forever...)
s.setblocking(False)



## ADC setup

# Create an Analog Digital Converter object
adc = machine.ADC()

# Number of quantizer bits for the ADC
adc_quantizer_bits = 12

adc.init(bits=adc_quantizer_bits)

# Attenuation level according to documentation:
#   "ADC pin input range is 0-1.1V. This maximuma
#   value can be increased up to 3.3V using the
#   highest attenuation of 11dB. Do not exceed
#   the maximum of 3.3V, to avoid damaging the
#   device"
# attenuation = machine.ADC.ATTN_6DB
# attenuation = machine.ADC.ATTN_6DB
# attenuation = machine.ADC.ATTN_2_5DB
attenuation = machine.ADC.ATTN_0DB
volt_ref = 1

# Pin only valid values are P13 and P20
# This is not true, tested empirically
# ADC available pins ar pins from P13 to P20
# To get transalation between expansion board
# and P_numbering just issue in the REPL:
#   >>> machine.Pin.exp_board.G6
#   Pin('P19', mode=Pin.IN, pull=Pin.PULL_DOWN, alt=-1)
# Where G6 is the pin name as in expansion board
analog_pin = 'P13'

# Setting all this parameters and getting a
# ADC pin
adc_pin = adc.channel(pin=analog_pin, attn=attenuation)



## Sampling setup

# Taking sending samples each 30s
report_time = 30
loop_times = int(report_time / 0.1)



## Looping

while True:
    # Reading a value from the ADC pin
    adc_value = adc_pin()

    for i in range(loop_times):
        # Storing only the highest values from all
        adc_value = max(adc_pin(), adc_value)
        time.sleep(0.1)

    # Getting the sound presure in dBA
    if adc_value < 3:
        # For values < 3 the log10 does not exit
        sound = 32
    else:
        # Formula and constant taken from empiric calibration
        sound = 20 * log10(adc_value * 91.3670324 - 220.0092)

    print("Sound Level : " + str(sound) + "  dBA")

    # Transforming data to char and then to strings
    # To send the data it should be encoded as strings
    data_string = str(sound)
    data_bytes = data_string.encode('utf_8')

    try:
        print("Sending data... ", end="")
        pycom.rgbled(colors["yellow"])      # Yellow = sending data

        # Sending data and setting socket to not blocking
        bytes_sent = s.send(data_bytes)
        s.setblocking(False)

        print("Sent " + str(bytes_sent) + "B")
        print("")
        time.sleep(0.5)
        pycom.rgbled(colors["off"])      # Yellow = sending data

    except:
        pass
