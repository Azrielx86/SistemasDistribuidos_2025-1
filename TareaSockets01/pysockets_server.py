import socket
from datetime import datetime
import re

HOST = "127.0.0.1"
PORT = 55600

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.bind((HOST, PORT))
    server_socket.listen()
    print(f"Servidor escuchando en {HOST}:{PORT}")

    stop_server = False
    while not stop_server:
        try:
            conn, addr = server_socket.accept()
            with conn:
                print(f"Conectado por {addr}")
                while True:
                    if data := conn.recv(1024):
                        content = data.decode()
                        print(f"Recibido: {content}")
                        if re.search("adios", content, flags=re.IGNORECASE):
                            conn.sendall("adios".encode())
                            print(f"Cerrando la conexión con {addr}")
                            break
                        conn.sendall(f"Mensaje recibido a las {datetime.now().strftime('(%H:%M:%S)')}".encode())
        except KeyboardInterrupt:
            stop_server = True
        except Exception as e:
            print(f"Ocurrió un error: {e}")
            stop_server = True
