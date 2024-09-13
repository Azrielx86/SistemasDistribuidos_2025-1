#include <errno.h>
#include <error.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

//#define DEBUG

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

void print_array(int *array, size_t size)
{
	printf("[ ");
	for (size_t i = 0; i < size - 1; i++)
		printf(" %d ",  array[i]);
	printf(" %d ]\n",  array[size - 1]);
}

void printWorkerInfo(const mpi_worker_info *process_info, MPI_Status *status ){
    printf("==> Master processor %d is %s\n", process_info->id, process_info->proc_name);
		for (int i = 1; i < process_info->np; i++)
		{
			char *worker_proc_name = malloc(MPI_MAX_PROCESSOR_NAME);
			MPI_Recv(worker_proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, i, MPI_COMM_WORLD, status);

			printf("==> Worker processor %d is %s\n", i, worker_proc_name);

			free(worker_proc_name);
		}
}

float calculaProductoPunto(const float *v1, const float *v2, const int vec_size){
    float result;
    for(int i = 0;  i< vec_size; i++){
        result += v1[i] * v2[i];
    }

    return result;
}

int main(int argc, char *argv[])
{
	mpi_worker_info process_info;
	MPI_Status status;

    srand((int) &status);

	// TODO : Get array size from terminal

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &process_info.id);
	MPI_Comm_size(MPI_COMM_WORLD, &process_info.np);
	MPI_Get_processor_name(process_info.proc_name, &process_info.proc_name_size);

	// printf("[GLOBAL]> Worker %d in processor %s is up!\n", process_info.id, process_info.processor_name);
    const int vec_size = 3;
    float *vec1, *vec2;
    float resultado = 0.0f;


    //IDEA: crear arreglo de tamaño np - 1 que indique la cantidad de elementos a enviar al nodo
    int elementosPorNodo = vec_size / (process_info.np - 1);
    int elementosNoAsignados = vec_size % (process_info.np - 1);
	
    int *distribucionelementos = NULL;

    distribucionelementos = malloc((process_info.np - 1) * sizeof(int));

    for(int i = 0; i < process_info.np - 1; i++){
        distribucionelementos[i] = elementosPorNodo;
    }

    if(elementosNoAsignados != 0){
        for(int i = 0; i< elementosNoAsignados; i++){
            distribucionelementos[i] += 1;
        }
    }

	if (process_info.id == 0)
	{
        vec1 = malloc(vec_size * sizeof(float));
        vec2 = malloc(vec_size * sizeof(float));

        if (vec1 == NULL || vec2 == NULL)
	    {
            perror("Cannot create vectors!");
            return errno;
	    }
		printWorkerInfo(&process_info, &status);

		for (size_t i = 0; i < vec_size; i++){
            vec1[i] = (float) (rand() % 50);
            vec2[i] = (float) (rand() % 50);
        }
			
        
        printf("\n\n=====\t\t VECTOR1 \t\t =====\n\n");
		print_array(vec1, vec_size);
        printf("\n\n=====\t\t VECTOR2 \t\t =====\n\n");
		print_array(vec2, vec_size);
        printf("\n\n=====\t\t Distribución elementos \t\t =====\n\n");
		print_array(distribucionelementos, process_info.np - 1);
        
        //dividir los vectores entre los otros procesos que no sean el master

        int startingIndex = 0;
        //Manda los pedazos del vector
        for (int nProcess = 1; nProcess < process_info.np; nProcess++){
            startingIndex = (nProcess - 1) * distribucionelementos[nProcess-1];
            MPI_Send(&vec1[startingIndex], distribucionelementos[nProcess-1], MPI_FLOAT, nProcess, nProcess, MPI_COMM_WORLD);
            MPI_Send(&vec2[startingIndex], distribucionelementos[nProcess-1], MPI_FLOAT, nProcess, nProcess, MPI_COMM_WORLD);
        }

        for (int nProcess = 1; nProcess < process_info.np; nProcess++){
            float tmp;
            MPI_Recv(&tmp, 1, MPI_FLOAT, nProcess, nProcess, MPI_COMM_WORLD, &status);

            resultado += tmp;
        }

        printf("El producto punto de los vectores es %0.2f\n", resultado);

		MPI_Barrier(MPI_COMM_WORLD);
	}
	else
	{
		MPI_Send(process_info.proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, process_info.id, MPI_COMM_WORLD);
        int vec_size_local = distribucionelementos[process_info.id - 1]
        vec1 = malloc(vec_size_local * sizeof(float));
        vec2 = malloc(vec_size_local * sizeof(float));

        if (vec1 == NULL || vec2 == NULL)
        {
            perror("Cannot create vectors!");
            return errno;
        }

        MPI_Recv(vec1, vec_size_local, MPI_FLOAT, 0, process_info.id, MPI_COMM_WORLD, &status);
        MPI_Recv(vec2, vec_size_local, MPI_FLOAT, 0, process_info.id, MPI_COMM_WORLD, &status);

        resultado = calculaProductoPunto(vec1, vec2, vec_size_local);

        MPI_Send(&resultado, 1, MPI_FLOAT, 0, process_info.id, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
	}

	free(vec1);
    free(vec2);
    free(distribucionelementos);
	MPI_Finalize();
	return 0;
}
