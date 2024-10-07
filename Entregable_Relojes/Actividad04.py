from typing import Callable
from multiprocessing import Process, Pipe
# noinspection PyProtectedMember
from multiprocessing.connection import Connection
from datetime import datetime

num_processes = 0


def local_time(counter: int) -> str:
    return f'(LAMPORT_TIME={counter}, LOCAL_TIME={datetime.now()})'


def calc_recv_timestamp(pid: int, recv_time_stamp: list[int], counter: list[int]) -> list[int]:
    counter[pid] = max(recv_time_stamp[pid], counter[pid]) + 1
    for i in range(len(counter)):
        counter[i] = max(recv_time_stamp[i], counter[i])
    return counter


def event(pid: int, counter: list[int]) -> list[int]:
    counter[pid] += 1
    print(f"Something happened on {pid}! {local_time(counter[pid])}")
    return counter


def send_message(pipe: Connection, pid: int, counter: list[int]) -> list[int]:
    counter[pid] += 1
    pipe.send(('Empty shell', counter))
    print(f'Message sent from {str(pid)}{local_time(counter[pid])}')
    return counter


def recv_message(pipe: Connection, pid: int, sender_pid: int, counter: list[int]) -> list[int]:
    message, timestamp = tuple[str, list[int]](pipe.recv())
    counter = calc_recv_timestamp(pid, timestamp, counter)
    print(f"Message sent to {str(pid)}{local_time(counter[pid])}")
    return counter


"""
    Cada proceso se define con una función con la estructura:
        Callable[[int, list[tuple[Connection, Connection]] | None], None]
    de esta manera, cada proceso tendrá un array con los canales de comunicación
    con los otros procesos, el cual se construye al inicio del programa.
    Además, al tener todos los procesos con la misma estructura, se puede
    escalar el programa de una manera más sencilla, solo creando nuevas
    funciones con la misma estructura y agregándolas al array de funciones
    al inicio del programa.
    
    Cada mensaje de salida corresponde al canal de comunicación 0, es decir,
    para enviar un mensaje de p1 a p3, se usaría el canal:
        
                 Canal de salida = 0
                 v
        pipes[2][0]
              ^
              índice de p3 en el array pipes[3 - 1]
              
    Cada mensaje de entrada corresponde al canal de comunicación 1, es decir,
    para recibir un mensaje de p4 a p1, se usaría el canal:
        
                 Canal de entrada = 1
                 v
        pipes[3][1]
              ^
              índice de p4 en el array pipes[4 - 1]
"""


def process_one(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = send_message(pipes[1][0], pid, counter)
    counter = recv_message(pipes[3][1], pid, 3, counter)
    print(f"[{pid}] Final vector: {counter}")


def process_two(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = recv_message(pipes[0][1], pid, 0, counter)
    counter = event(pid, counter)
    print(f"[{pid}] Final vector: {counter}")


def process_three(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = send_message(pipes[3][0], pid, counter)
    print(f"[{pid}] Final vector: {counter}")


def process_four(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = recv_message(pipes[2][1], pid, 2, counter)
    counter = send_message(pipes[0][0], pid, counter)
    print(f"[{pid}] Final vector: {counter}")


def process_five(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = send_message(pipes[5][0], pid, counter)
    print(f"[{pid}] Final vector: {counter}")


def process_six(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = recv_message(pipes[4][1], pid, 4, counter)
    counter = event(pid, counter)
    print(f"[{pid}] Final vector: {counter}")


if __name__ == '__main__':
    print("Relojes vectoriales".center(80, "="))

    # Array de funciones, representan cada nodo. Todas deben de tener la misma
    # estructura. El tamaño de este array definirá el número de procesos.
    process_functions: list[Callable[[int, list[tuple[Connection, Connection]] | None], None]] = [
        process_one,
        process_two,
        process_three,
        process_four,
        process_five,
        process_six
    ]

    num_processes = len(process_functions)
    processes: list[Process] = []
    pipes = [[None for _ in range(num_processes)] for _ in range(num_processes)]

    # Creación de los canales de comunicación para cada nodo.
    for i in range(num_processes):
        proc_pip = pipes[i]
        for j in range(num_processes):
            # Se omite crear un canal de un nodo consigo mismo
            if i == j:
                continue

            # Si el canal ya existe, se referencia de nuevo.
            if (existing_pipe := pipes[j][i]) is not None:
                proc_pip[j] = existing_pipe
            else:
                # Si el canal no existe, se crea uno nuevo
                proc_pip[j] = Pipe()

    for i in range(num_processes):
        proc = Process(target=process_functions[i], args=(i, pipes[i]))
        processes.append(proc)

    for proc in processes:
        proc.start()

    for proc in processes:
        proc.join()
