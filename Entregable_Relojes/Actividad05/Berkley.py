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
    local_times: list[float] = []
    for node, _ in pipes:
        node.send(Messages.REQUEST_TIME)

    for node, _ in pipes:
        local_time = node.recv()
        local_times.append(local_time)

    avg_local_time = sum(local_times) / len(local_times)
    diffs = [avg_local_time - ltime for ltime in local_times]

    print(f"Average time: {avg_local_time} | Diffs: {diffs}")

    for idx, (node, _) in enumerate(pipes):
        node.send(diffs[idx])

    print("Corrections sended")
    for node, _ in pipes:
        node.close()


def node(pid: int, pipe: tuple[Connection, Connection]):
    print(f"Process {pid}")
    _, coord = pipe

    if coord.recv() == Messages.REQUEST_TIME:
        print("Requested...")
        time.sleep(random.uniform(0.1, 2))
        local_time = time.time()
        coord.send(local_time)
        diff = coord.recv()
        new_time = local_time + diff
        print(f"Process {pid}: Local Time: {local_time:.9f} | New: {new_time:.9f} | Diff: {diff:.9f}")


if __name__ == '__main__':
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
