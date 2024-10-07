import time
from typing import Callable
from multiprocessing import Process, Pipe
# noinspection PyProtectedMember
from multiprocessing.connection import Connection
from os import getpid
from datetime import datetime
import random

num_processes = 0


def local_time(counter: int) -> str:
    return f'(LAMPORT_TIME={counter}, LOCAL_TIME={datetime.now()})'


def calc_recv_timestamp(pid: int, recv_time_stamp: int, counter: list[int]) -> list[int]:
    counter[pid] = max(recv_time_stamp, counter[pid]) + 1
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


def recv_message(pipe: Connection, pid: int, counter: list[int]) -> list[int]:
    message, timestamp = tuple[str, list[int]](pipe.recv())
    counter = calc_recv_timestamp(pid, timestamp[pid], counter)
    print(f"Message sent to {str(pid)}{local_time(counter[pid])}")
    return counter


def process_one(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = event(pid, counter)
    counter = send_message(pipes[1][0], pid, counter)
    counter = event(pid, counter)
    counter = recv_message(pipes[1][0], pid, counter)
    counter = event(pid, counter)

    print(f"Final vector: {counter}")


def process_two(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = recv_message(pipes[0][1], pid, counter)
    counter = send_message(pipes[0][1], pid, counter)
    counter = send_message(pipes[2][0], pid, counter)
    counter = recv_message(pipes[2][0], pid, counter)

    print(f"Final vector: {counter}")


def process_three(pid: int, pipes: list[tuple[Connection, Connection] | None]) -> None:
    counter = [0 for _ in range(num_processes)]
    counter = recv_message(pipes[1][1], pid, counter)
    counter = send_message(pipes[1][1], pid, counter)

    print(f"Final vector: {counter}")


if __name__ == '__main__':
    print("Relojes vectoriales".center(80, "="))

    process_functions: list[Callable[[int, list[tuple[Connection, Connection]] | None], None]] = [
        process_one,
        process_two,
        process_three
    ]

    num_processes = len(process_functions)
    processes: list[Process] = []
    pipes = [[None for _ in range(num_processes)] for _ in range(num_processes)]
    for i in range(num_processes):
        proc_pip = pipes[i]
        for j in range(num_processes):
            if i == j:
                continue
            if (existing_pipe := pipes[j][i]) is not None:
                proc_pip[j] = existing_pipe
            else:
                proc_pip[j] = Pipe()

    for i in range(num_processes):
        proc = Process(target=process_functions[i], args=(i, pipes[i]))
        processes.append(proc)

    for proc in processes:
        proc.start()

    for proc in processes:
        proc.join()

    print("end")
