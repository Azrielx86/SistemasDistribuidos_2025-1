#include <errno.h>
#include <error.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct mpi_worker_info
{
	int np;
	int id;
	char proc_name[MPI_MAX_PROCESSOR_NAME];
	int proc_name_size;
} mpi_worker_info;

void print_array(float *array, size_t size)
{
	printf("[ ");
	for (size_t i = 0; i < size - 1; i++)
		printf(" %.2f ", (double) array[i]);
	printf(" %.2f ]\n", (double) array[size - 1]);
}

int main(int argc, char *argv[])
{
	mpi_worker_info process_info;
	MPI_Status status;
	float *array = NULL;
	unsigned int laps;

	// TODO : Get array size from terminal
	size_t arr_size = 25;
	laps = 2;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &process_info.id);
	MPI_Comm_size(MPI_COMM_WORLD, &process_info.np);
	MPI_Get_processor_name(process_info.proc_name, &process_info.proc_name_size);

	// printf("[GLOBAL]> Worker %d in processor %s is up!\n", process_info.id, process_info.processor_name);

	array = malloc(arr_size * sizeof(float));
	if (array == NULL)
	{
		perror("Cannot create array!");
		return errno;
	}

	if (process_info.id == 0)
	{
		printf("==> Master processor %d is %s\n", process_info.id, process_info.proc_name);
		for (int i = 1; i < process_info.np; i++)
		{
			char *worker_proc_name = malloc(MPI_MAX_PROCESSOR_NAME);
			MPI_Recv(worker_proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, i, MPI_COMM_WORLD, &status);

			printf("==> Worker processor %d is %s\n", i, worker_proc_name);

			free(worker_proc_name);
		}

		printf("==> Sending to %d workers in %d laps\n==> ", process_info.np, laps);
		for (size_t i = 0; i < arr_size; i++)
			array[i] = (float) i;
		print_array(array, arr_size);

		for (unsigned int lap = 0; lap < laps; lap++)
		{
			printf("==> Starting lap %d\n", lap + 1);
			for (size_t i = 0; i < arr_size; i++)
				array[i] += 1.0f;
			MPI_Send(array, (int) arr_size, MPI_FLOAT, 1, 1, MPI_COMM_WORLD);
			MPI_Recv(array, (int) arr_size, MPI_FLOAT, process_info.np - 1, 0, MPI_COMM_WORLD, &status);
		}

		MPI_Barrier(MPI_COMM_WORLD);
		printf("==> Execution ended. Final array: \n==> ");
		print_array(array, arr_size);
	}
	else
	{
		MPI_Send(process_info.proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, process_info.id, MPI_COMM_WORLD);
		for (unsigned int lap = 0; lap < laps; lap++)
		{
			MPI_Recv(array, (int) arr_size, MPI_FLOAT, process_info.id - 1, process_info.id, MPI_COMM_WORLD, &status);

			for (size_t i = 0; i < arr_size; i++)
				array[i] += 1.0f;
#ifdef DEBUG
			printf("[%s][%2d][lap (%d/%d)] Array modified: ", process_info.proc_name, process_info.id, lap + 1, laps);
			print_array(array, arr_size);
#endif /* ifdef DEBUG */
			MPI_Send(array, (int) arr_size, MPI_FLOAT, process_info.id == process_info.np - 1 ? 0 : process_info.id + 1, process_info.id == process_info.np - 1 ? 0 : process_info.id + 1, MPI_COMM_WORLD);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	free(array);
	MPI_Finalize();
	return 0;
}
