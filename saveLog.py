import serial

port = 'COM4'
baudrate = 38400
ser = serial.Serial(port, baudrate)

file_path = 'log.txt'
with open(file_path, 'a') as file:
    while True:
        try:
            data = ser.readline().decode('utf-8', errors='ignore').strip()
            if data:
                print("Received:", data)
                file.write(data + '\n')
                file.flush()  # Ensure data is written immediately to the file
        except UnicodeDecodeError as e:
            print(f"Error decoding data: {e}")
            continue  # Skip this iteration and continue reading
