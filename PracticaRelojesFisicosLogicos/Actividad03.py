import time
from multiprocessing import Process
from typing import Literal
from argparse import ArgumentParser
from BaseProg import BerkleyLamport
from paramiko.client import SSHClient
import json
from concurrent.futures import ThreadPoolExecutor


class Machine:
    def __init__(self, host: str, user: str, mode: Literal["server", "client"]):
        self.host = host
        self.user = user
        self.mode = mode


def launch_process(machine: Machine, ip: str, port: int) -> None:
    with SSHClient() as client:
        client.load_system_host_keys()
        client.connect(machine.host, username=machine.user)
        stdin, stdout, stderr = client.exec_command(f"python /opt/mpi_sd/Actividad03.py --type client --ip {ip} --port {port}")
        output = stdout.readlines()
        print(output)


def exec_server(ip: str, port: int, clients: int):
    machines: list[Machine] = []
    with open("./machines.json", "r") as mfile:
        mobjects = json.load(mfile)
        for obj in mobjects:
            m = Machine(obj["host"], obj["user"], obj["mode"])
            machines.append(m)

    server_process = Process(target=BerkleyLamport.berkeley_server, args=[ip, port, clients])
    server_process.start()

    with ThreadPoolExecutor(max_workers=len(machines)) as executor:
        for m in machines:
            executor.submit(launch_process, m, ip, port)

        executor.shutdown(wait=True)

    server_process.join()


def exec_client(ip: str, port: int):
    BerkleyLamport.berkeley_client(ip, port)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("-t", "--type", choices=["server", "client"], required=True, help="Execution mode: server or client")
    parser.add_argument("-p", "--port", type=int, required=True, help="Port from the server")
    parser.add_argument("-i", "--ip", type=str, required=True, help="IP from the server")
    parser.add_argument("-n", "--nclients", type=int, required=False, help="(server only) number of clients")
    parser.add_argument("-a", "--auto", help="Auto mode, requieres SSH")

    args = parser.parse_args()

    ip = args.ip
    port = args.port

    if args.auto:
        if args.type == "server":
            clients = args.nclients

            if clients and port and ip:
                exec_server(ip, port, clients)
                time.sleep(2)
        else:
            exec_client(ip, port)
    else:
        if args.type == "server":
            clients = args.nclients
            BerkleyLamport.berkeley_server(ip, port, clients)
        else:
            BerkleyLamport.berkeley_client(ip, port)
