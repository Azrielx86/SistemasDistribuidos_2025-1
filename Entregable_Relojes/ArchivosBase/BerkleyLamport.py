import socket
import datetime
from multiprocessing import Process
import time
import random
import struct

def berkeley_server(port=12345, num_clients=3):
    ss = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ss.bind(("localhost", port))
    ss.listen(num_clients)    
    print("Servidor maestro esperando a los clientes...")
    connections = []
    times = []
    # Aceptar conexiones de los clientes
    for _ in range(num_clients):   
        conn, addr = ss.accept()
        print(f"Conexión establecida con {addr}")
        connections.append(conn)
    for conn in connections:    # Enviar solicitudes de tiempo a los clientes
        conn.send(b'TIME_REQUEST')
    # Recibir el tiempo de los clientes
    for conn in connections:
        client_time = struct.unpack('f', conn.recv(4))[0]
        print(f"Tiempo recibido del cliente: {client_time}")
        times.append(client_time)
    # Calcular la diferencia de tiempo
    server_time = time.time()
    print(f"Tiempo del servidor (maestro): {server_time}") 
    # Calcular la diferencia con respecto al tiempo del servidor
    adjustments = [server_time - client_time for client_time in times]
    # Enviar los ajustes a los clientes
    for conn, adjustment in zip(connections, adjustments):
        print(f"Enviando ajuste de {adjustment} al cliente")
        conn.send(struct.pack('f', adjustment))
    # Cerrar conexiones
    for conn in connections:
        conn.close()

def berkeley_client(port=12345):
    cs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    cs.connect(("localhost", port))

    # Simular tiempo local del cliente con un desfase aleatorio
    local_time = time.time() + random.uniform(-5, 5)  # Desfase simulado
    print(f"Tiempo local del cliente antes del ajuste: {local_time}")
    
    request = cs.recv(1024)    # Esperar solicitud de tiempo del servidor
    if request == b'TIME_REQUEST':
        # Enviar el tiempo actual al servidor
        cs.send(struct.pack('f', local_time))
    
    # Recibir el ajuste desde el servidor
    adjustment = struct.unpack('f', cs.recv(4))[0]
    print(f"Ajuste recibido del servidor: {adjustment}")
    
    # Aplicar el ajuste al tiempo local
    new_time = local_time + adjustment
    print(f"Nuevo tiempo ajustado: {new_time}")
    
    cs.close()

def run_clients(num_clients=3):
    processes = []
    
    for _ in range(num_clients):
        p = Process(target=berkeley_client)
        processes.append(p)
        p.start()
        time.sleep(1)  # Dar tiempo entre lanzamientos de clientes
    
    for p in processes:
        p.join()


if __name__ == "__main__":
    # Iniciar el servidor en un proceso separado
    server_process = Process(target=berkeley_server)
    server_process.start()

    # Darle un poco de tiempo para que el servidor inicie
    time.sleep(2)

    # Ejecutar los clientes
    run_clients(num_clients=3)

    # Esperar a que el servidor termine
    server_process.join()
