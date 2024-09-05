#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int np, id;
	char *proc_name;
	int proc_name_size;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	proc_name = malloc(MPI_MAX_PROCESSOR_NAME);
	MPI_Get_processor_name(proc_name, &proc_name_size);

	if (id == 0)
	{
		printf("Master processor %d is %s\n", id, proc_name);
		for (int i = 1; i < np; i++)
		{
			char *worker_proc_name = malloc(MPI_MAX_PROCESSOR_NAME);
			MPI_Recv(worker_proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, i, MPI_COMM_WORLD, &status);

			printf("Worker processor %d is %s\n", i, worker_proc_name);

			free(worker_proc_name);
		}
	}
	else
	{
		MPI_Send(proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, id, MPI_COMM_WORLD);
	}

	free(proc_name);
	MPI_Finalize();
	return 0;
}
