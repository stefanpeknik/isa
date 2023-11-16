import socket
from typing import Tuple


class UDPClient:
    def __init__(self, server_ip: str, server_port: int):
        self.server_ip = server_ip
        self.server_port = server_port
        self.client_socket = socket.socket(
            family=socket.AF_INET, type=socket.SOCK_DGRAM
        )

    def send_message(self, message: str) -> None:
        self.client_socket.sendto(message.encode(), (self.server_ip, self.server_port))

    def receive_message(self) -> Tuple[bytes, Tuple[str, int]]:
        data, addr = self.client_socket.recvfrom(70000)
        return data, addr

    def close_connection(self) -> None:
        self.client_socket.close()
