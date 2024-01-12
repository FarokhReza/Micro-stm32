import serial
import datetime

port = 'COM3'
baudrate = 38400
#
# Open the serial port
ser = serial.Serial(port, baudrate)

currentTime = str(datetime.datetime.now())
print(currentTime)
final_time = currentTime[11:19] + '\n'
print(final_time)

ser.write(final_time.encode('utf-8'))

ser.close()