/*  Autor(es):            Tapia Rodrigo, Chalico Edgar
    Fecha de creación:    08/09/2024
    Descripción:          Programa que calcula el producto punto de dos vectores
                          de n dimensiones utilizando una interfaz de paso de mensajes
                          y un clúster para que se procese de forma distribuida.
*/
#include <errno.h>
#include <error.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

//#define DEBUG

//Tipo de dato para guardar la información del proceso
typedef struct mpi_worker_info
{
	int np;
	int id;
	char proc_name[MPI_MAX_PROCESSOR_NAME];
	int proc_name_size;
} mpi_worker_info;


//Función para imprimir un arreglo de numeros reales
void print_array_float(float *array, size_t size)
{
	printf("[ ");
	for (size_t i = 0; i < size - 1; i++)
		printf(" %.2f ", (double) array[i]);
	printf(" %.2f ]\n", (double) array[size - 1]);
}

//Función para imprimir un arreglo de numeros reales
void print_array_int(int *array, size_t size)
{
	printf("[ ");
	for (size_t i = 0; i < size - 1; i++)
		printf(" %d ",  array[i]);
	printf(" %d ]\n",  array[size - 1]);
}

//Imprime los datos de los procesos en los que se está ejecutando el programa
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

//Calcula el producto punto de fos vectores.
float calculaProductoPunto(const float *v1, const float *v2, const int vec_size){
    float result = 0;
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

    //Inicialización del ambiente MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_info.id);
	MPI_Comm_size(MPI_COMM_WORLD, &process_info.np);
	MPI_Get_processor_name(process_info.proc_name, &process_info.proc_name_size);

    //Variables necesarias para el cálculo del producto punto
    const int vec_size = 3;
    float *vec1, *vec2;
    float resultado = 0.0f;

    //elementosPorNodo guarda el número de elementos del vector que va a procesar cada uno de los procesos.
    //Si el número de elementos no se divide de forma exacta, el residuo se guarda en elementosNoAsignados
    int elementosPorNodo = vec_size / (process_info.np - 1);
    int elementosNoAsignados = vec_size % (process_info.np - 1);
	
    //Arreglo en el que se guarda la cantidad de elementos que se van a enviar a cada proceso
    int *distribucionelementos = NULL;

    distribucionelementos = malloc((process_info.np - 1) * sizeof(int));

    for(int i = 0; i < process_info.np - 1; i++){
        distribucionelementos[i] = elementosPorNodo;
    }
    //Si hay un sobrante de elementos se reparten entre los nodos.
    if(elementosNoAsignados != 0){
        for(int i = 0; i< elementosNoAsignados; i++){
            distribucionelementos[i] += 1;
        }
    }

	if (process_info.id == 0)
	{
        //Creación de los vectore de tamaño vec_size
        vec1 = malloc(vec_size * sizeof(float));
        vec2 = malloc(vec_size * sizeof(float));

        if (vec1 == NULL || vec2 == NULL)
	    {
            perror("Cannot create vectors!");
            return errno;
	    }
        
		printWorkerInfo(&process_info, &status);

        //Llenado de los vectores con números aleatorios
		for (size_t i = 0; i < vec_size; i++){
            vec1[i] = (float) (rand() % 50);
            vec2[i] = (float) (rand() % 50);
        }
			
        
        printf("==> Vector 1: \n");
		print_array_float(vec1, vec_size);
        printf("==> Vector 2: \n");
		print_array_float(vec2, vec_size);
        printf("==> Number of elements being sent to each process:\n");
		print_array_int(distribucionelementos, process_info.np - 1);
        
        //Variable que indica el índice de los vectores a partir del cuál se van a enviar los elementos
        int startingIndex = 0;

        //Ciclo para el envio de datos
        for (int nProcess = 1; nProcess < process_info.np; nProcess++){
            MPI_Send(&vec1[startingIndex], distribucionelementos[nProcess-1], MPI_FLOAT, nProcess, nProcess, MPI_COMM_WORLD);
            MPI_Send(&vec2[startingIndex], distribucionelementos[nProcess-1], MPI_FLOAT, nProcess, nProcess, MPI_COMM_WORLD);
            startingIndex += distribucionelementos[nProcess-1];
        }

        //Ciclo para recepción de datos, suma los productos parciales.
        for (int nProcess = 1; nProcess < process_info.np; nProcess++){
            float tmp; 
            MPI_Recv(&tmp, 1, MPI_FLOAT, nProcess, nProcess, MPI_COMM_WORLD, &status);

            resultado += tmp;
        }

        printf("vector 1 * vector 2  =  %0.2f\n", resultado);

		MPI_Barrier(MPI_COMM_WORLD);
	}
	else
	{
        //Envia datos de los procesos workers
		MPI_Send(process_info.proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, process_info.id, MPI_COMM_WORLD);

        //Calcula el tamaño de cada vector para cada proceso, con base en el contenido del arreglo distribuciónelementos
        int vec_size_local = distribucionelementos[process_info.id - 1];

        //Creación de los vectore de los procesos workers
        vec1 = malloc(vec_size_local * sizeof(float));
        vec2 = malloc(vec_size_local * sizeof(float));

        if (vec1 == NULL || vec2 == NULL)
        {
            perror("Cannot create vectors!");
            return errno;
        }

        //Recepción de datos de los vectores
        MPI_Recv(vec1, vec_size_local, MPI_FLOAT, 0, process_info.id, MPI_COMM_WORLD, &status);
        MPI_Recv(vec2, vec_size_local, MPI_FLOAT, 0, process_info.id, MPI_COMM_WORLD, &status);

        resultado = calculaProductoPunto(vec1, vec2, vec_size_local);

        //Envia al proceso 0 el resultado parcial
        MPI_Send(&resultado, 1, MPI_FLOAT, 0, process_info.id, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
	}
    
	free(vec1);
    free(vec2);
    free(distribucionelementos);
	MPI_Finalize();
	return 0;
}
