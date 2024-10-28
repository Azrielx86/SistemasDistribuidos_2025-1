# Entregable RPC

```
Semestre 2025-1
Facultad de Ingeniería UNAM
```
## Integrantes del equipo

* Moreno Chalico Edgar Ulises
* Tapia Navarro Rodrigo

## Compilación y Ejecución

> [!NOTE]
> Para las actividades 1 y 2 se recomienda utilizar el archivo Makefile incluido.

> [!NOTE]
> Para las actividades 3 y 4 se requiere grpcio y grpcio-tools

### Actividad 1

```bash
$ rpcgen operaciones.x
$ clang -I/usr/include/tirpc -o client operaciones_client.c operaciones_clnt.c operaciones_xdr.c -lnsl -ltirpc
$ clang -I/usr/include/tirpc -o server operaciones_server.c operaciones_svc.c operaciones_xdr.c -lnsl -ltirpc
```

### Actividad 2

> [!NOTE]
> Para esta actividad se requiere sqlite3.

```bash
$ rpcgen registro_escolar.x
$ clang -I/usr/include/tirpc -o server registro_escolar_server.c registro_escolar_svc.c registro_escolar_xdr.c -lnsl -ltirpc -lsqlite3
$ clang -I/usr/include/tirpc -o client registro_escolar_client.c registro_escolar_clnt.c registro_escolar_xdr.c -lnsl -ltirpc -lsqlite3
```

### Actividad 3

```bash
$ python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. ejemplo.proto
```
### Actividad 4

```bash
$ python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. calculadora.proto
```
