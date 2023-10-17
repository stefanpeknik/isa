import socket
from datetime import datetime

# Define the server's IP address and port
server_host = "127.0.0.1"
server_port = 2025  # Change this to the desired port number

# Create a UDP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the server address and port
server_socket.bind((server_host, server_port))

print(f"UDP server is listening on {server_host}:{server_port}")

while True:
    try:
        # Receive data from the client
        data, client_address = server_socket.recvfrom(1024)  # Adjust the buffer size as needed

        # Process the received data (handle binary data)
        print(f"{datetime.now()} : Received data from {client_address}: {data}")

    except Exception as e:
        print(f"Error: {str(e)}")
        server_socket.close()


# Close the socket (this part may never be reached in practice)
server_socket.close()
