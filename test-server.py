import socket

# Define TFTP opcodes
RRQ_OPCODE = 1
DATA_OPCODE = 3
ACK_OPCODE = 4

# TFTP Block Number
block_number = 1

# Fake data to send in DATA packets
fake_data = b"Hello, this is fake data."

# Create a UDP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to a specific host and port (e.g., localhost and port 69)
server_socket.bind(("127.0.0.1", 69))

print("TFTP server is running...")

while True:
    # Wait for incoming RRQ request
    data, client_address = server_socket.recvfrom(516)  # TFTP packets are usually 516 bytes

    # Extract opcode and file name from the RRQ request
    opcode, file_name, mode = data[0:2], data[2:data.find(b"\0", 2)], data[data.find(b"\0", 2) + 1:data.find(b"\0", data.find(b"\0", 2) + 1)]

    if opcode == RRQ_OPCODE:
        print(f"Received RRQ request for file: {file_name.decode('utf-8')}")

        # Prepare a fake DATA packet
        data_packet = bytearray()
        data_packet.extend(DATA_OPCODE.to_bytes(2, "big"))
        data_packet.extend(block_number.to_bytes(2, "big"))
        data_packet.extend(fake_data)

        # Send the fake DATA packet to the client
        server_socket.sendto(data_packet, client_address)

        print(f"Sent fake DATA packet (Block {block_number})")

        # Increment the block number for the next DATA packet
        block_number += 1

        # Wait for ACK packet and validate it
        ack_packet, _ = server_socket.recvfrom(4)
        ack_opcode, ack_block = ack_packet[0:2], ack_packet[2:4]

        if ack_opcode == ACK_OPCODE and int.from_bytes(ack_block, "big") == block_number - 1:
            print(f"Received valid ACK for block {block_number - 1}")
        else:
            print("Received an invalid ACK. Terminating connection.")

        if len(fake_data) < 512:
            print("Last DATA packet sent. Terminating connection.")
            break  # Last DATA packet, terminate the connection

server_socket.close()
