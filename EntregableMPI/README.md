# Entregable 1 MPI

Comandos para compilar

```bash
mpicc -O2 -Wall -Wextra -pedantic Actividad_0n.c -o Actividad0n
```

Comandos para ejecutar el programa

En un solo nodo

```bash
mpirun -np 8 ./Actividad_0n
```

En el cluster


```bash
mpirun -hostfile mpi_hosts /opt/fimpi/Actividad0n
```

Para mostrar más información acerca de la ejecución

```bash
mpirun --mca mpi_preconnect_all true -verbose -display-map -hostfile mpi_hosts /opt/fimpi/Actividad01
```
