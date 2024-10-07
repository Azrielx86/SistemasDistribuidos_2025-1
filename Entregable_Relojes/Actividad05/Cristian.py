import time
import random
from enum import Enum
from multiprocessing import Process, Pipe
# noinspection PyProtectedMember
from multiprocessing.connection import Connection


class Messages(Enum):
    REQUEST_TIME = 0


def format_time(t: float) -> str:
    return time.strftime('%Y-%m-%d %H:%M:%S%z', time.localtime(t))


def coordinator(pipes: list[tuple[Connection, Connection]]) -> None:
    """
    Función para el coordinador
    :param pipes: Array con los canales de comunicación de los procesos.
    El primer elemento es el canal hacia el nodo, el segundo es el canal
    hacia el coordinador.
    """
    nodes_coordinated = 0

    # Esperará a que todos los nodos soliciten actualizar su reloj
    while nodes_coordinated < len(pipes):
        try:
            for idx, (node, _) in enumerate(pipes):
                # Para comprobar si el nodo ya solicitó actualizar.
                if node.poll(1):
                    node_time = node.recv()  # Recibe la hora del nodo
                    server_time = time.time()
                    print(f"Received {format_time(node_time)} from node {idx}. Sending server time {format_time(server_time)}...")
                    node.send(server_time)  # Envía su propio reloj
                    nodes_coordinated += 1  # Incrementa el contador de nodos revisados.
        except KeyboardInterrupt or Exception:
            break
    print(f"Server time: {format_time(time.time())}")


def node(pid: int, pipe: tuple[Connection, Connection]) -> None:
    """
    Función para los nodos.
    :param pid: Id del proceso.
    :param pipe: Canales de comunicación entre el proceso y el coordinador.
    """
    print(f"[{pid}] Process started.")
    _, coord = pipe
    time.sleep(random.uniform(0.1, 2))  # Simula un retraso en el reloj
    t_0 = time.time()
    clock = t_0

    print(f"[{pid}] Requesting server time...")
    coord.send(t_0)  # Envía su hora
    server_time = coord.recv()  # Recibe la hora del servidor
    print(f"[{pid}] Server time is {format_time(server_time)}")

    t_1 = time.time()
    c = server_time + (t_1 - t_0) / 2  # Ajusta su reloj.

    # Acciones para ajustar su reloj.
    if c > t_1:
        clock = c
    elif c < t_1:
        time.sleep(t_1 - c)
        clock = time.time()

    print(f"[{pid}] New time is {format_time(clock)}")


if __name__ == '__main__':
    print("[ Algoritmo de Cristian ]".center(80, "="))
    nump = 4

    pipes: list[tuple[Connection, Connection]] = [Pipe() for _ in range(nump)]
    procesos = [Process(target=node, args=(i, pipe)) for i, pipe in enumerate(pipes)]

    coord = Process(target=coordinator, args=(pipes,))
    coord.start()

    for process in procesos:
        process.start()

    for process in procesos:
        process.join()
    coord.join()
