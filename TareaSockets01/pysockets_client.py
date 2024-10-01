import socket
import re

HOST = "127.0.0.1"
PORT = 55600

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
    client_socket.connect((HOST, PORT))
    print(f"Conectando con el servidor: {HOST}:{PORT}")

    while True:
        try:
            response = input("> ")
            client_socket.sendall(response.encode())

            if data := client_socket.recv(1024):
                content = data.decode()
                print(f"Respuesta del servidor: {content}")
                if re.search("adios", content, flags=re.IGNORECASE):
                    break
        except KeyboardInterrupt:
            print("Terminando cliente")
            client_socket.sendall("adios".encode())
            break
