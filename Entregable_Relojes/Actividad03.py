from typing import Callable
from multiprocessing import Process, Pipe
# noinspection PyProtectedMember
from multiprocessing.connection import Connection
from datetime import datetime


def local_time(counter):
    return ' (LAMPORT_TIME={}, LOCAL_TIME={})'.format(counter, datetime.now())


def calc_recv_timestamp(recv_time_stamp, counter):
    return max(recv_time_stamp, counter) + 1


def event(pid: int, counter: int) -> int:
    counter += 1
    print(f'Algo sucedio en {pid}! {local_time(counter)}')
    return counter


def send_message(pipe: Connection, pid: int, counter: int) -> int:
    counter += 1
    pipe.send(('Empty shell', counter))
    print(f'Message sent from {str(pid)}{local_time(counter)}')
    return counter


def recv_message(pipe: Connection, pid: int, counter: int) -> int:
    message, timestamp = pipe.recv()
    counter = calc_recv_timestamp(timestamp, counter)
    print(f'Mensaje enviado a  {str(pid)}{local_time(counter)}')
    return counter


def process_one(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = 0
    counter = send_message(pipes[1][0], pid, counter)
    counter = recv_message(pipes[3][1], pid, counter)
    print(f"[{pid}] Final logical clock: {counter}")


def process_two(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = 0
    counter = recv_message(pipes[0][1], pid, counter)
    counter = event(pid, counter)
    print(f"[{pid}] Final logical clock: {counter}")


def process_three(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = 0
    counter = send_message(pipes[3][0], pid, counter)
    print(f"[{pid}] Final logical clock: {counter}")


def process_four(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = 0
    counter = recv_message(pipes[2][1], pid, counter)
    counter = send_message(pipes[0][0], pid, counter)
    print(f"[{pid}] Final logical clock: {counter}")


def process_five(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = 0
    counter = send_message(pipes[5][0], pid, counter)
    print(f"[{pid}] Final logical clock: {counter}")


def process_six(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = 0
    counter = recv_message(pipes[4][1], pid, counter)
    counter = event(pid, counter)
    print(f"[{pid}] Final logical clock: {counter}")


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
