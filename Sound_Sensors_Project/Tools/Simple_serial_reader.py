import serial

# Open serial with standar params "9600,8,N,1”
ser = serial.Serial('/dev/ttyUSB0')
f = open('data_sensor.txt', 'w')

while True:
   line = ser.readline()
   print(line)
   f.write(line.decode("utf-8"))
