from django.http import HttpResponse
from django.shortcuts import render
import serial

port = 'COM3'
baudRate = 38400
ser = serial.Serial(port, baudRate)

def index(request):
    return render(request, 'index.html')

def submit_data(request):
    if request.method == 'POST':
        data = request.POST.get('data')
        x = send_value(data)
        # x = str(x)
        return HttpResponse("Data Transmit!!!!!!")


def send_value(value):
    value = str(value) + '\n'
    value_str = value.encode('utf-8')
    ser.write(value_str)
    # return ser.

# def read_data():
#     file_path = 'log.txt'
#     with open(file_path, 'a') as file:
#         while True:
#             try:
#                 data = ser.readline().decode('utf-8', errors='ignore').strip()
#                 if data:
#                     print("Received:", data)
#                     file.write(data + '\n')
#                     file.flush()  # Ensure data is written immediately to the file
#             except UnicodeDecodeError as e:
#                 print(f"Error decoding data: {e}")
#                 continue  # Skip this iteration and continue reading



