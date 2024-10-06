import time
from multiprocessing import Process, Pipe
# noinspection PyProtectedMember
from multiprocessing.connection import Connection
from os import getpid
from datetime import datetime
import random


def local_time(counter: int) -> str:
    return f'(LAMPORT_TIME={counter}, LOCAL_TIME={datetime.now()})'


def calc_recv_timestamp(recv_time_stamp: int, counter: int) -> int:
    return max(recv_time_stamp, counter) + 1


def event(pid: int, counter: int) -> int:
    counter += 1
    time.sleep(random.uniform(0.1, 1))
    print(f"Something happened on {pid}! {local_time(counter)}")
    return counter


def send_message(pipe: Connection, pid: int, counter: int) -> int:
    counter += 1
    time.sleep(random.uniform(0.1, 1))
    pipe.send(('Empty shell', counter))
    print(f'Message sent from {str(pid)}{local_time(counter)}')
    return counter


def recv_message(pipe: Connection, pid: int, counter: int) -> int:
    message, timestamp = pipe.recv()
    time.sleep(random.uniform(0.1, 1))
    counter = calc_recv_timestamp(timestamp, counter)
    print(f"Message sent to {str(pid)}{local_time(counter)}")
    return counter


def process_one(pipe12: Connection) -> None:
    pid = getpid()
    counter = 0
    counter = event(pid, counter)
    counter = send_message(pipe12, pid, counter)
    counter = event(pid, counter)
    counter = recv_message(pipe12, pid, counter)
    counter = event(pid, counter)


def process_two(pipe21: Connection, pipe23: Connection) -> None:
    pid = getpid()
    counter = 0
    counter = recv_message(pipe21, pid, counter)
    counter = send_message(pipe21, pid, counter)
    counter = send_message(pipe23, pid, counter)
    counter = recv_message(pipe23, pid, counter)


def process_three(pipe32: Connection) -> None:
    pid = getpid()
    counter = 0
    counter = recv_message(pipe32, pid, counter)
    counter = send_message(pipe32, pid, counter)


if __name__ == '__main__':
    oneandtwo, twoandone = Pipe()
    twoandthree, threeandtwo = Pipe()

    process1 = Process(target=process_one,
                       args=(oneandtwo,))

    process2 = Process(target=process_two,
                       args=(twoandone, twoandthree))

    process3 = Process(target=process_three,
                       args=(threeandtwo,))

    process1.start()
    process2.start()
    process3.start()

    process1.join()
    process2.join()
    process3.join()
