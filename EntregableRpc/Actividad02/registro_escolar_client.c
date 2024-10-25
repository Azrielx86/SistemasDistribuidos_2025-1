#include "registro_escolar.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void registroalumnos_1(char *host, char *action, alumno *alumno_arg, busqueda *busqueda_arg)
{

	CLIENT *clnt = clnt_create(host, REGISTROALUMNOS, REGISTROALUMNOS_V1, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1);
	}

	// Comparación de todas las opciones posibles.
	if (strcmp(action, "post") == 0)
	{
		const int *result_1 = registrar_alumno_1(alumno_arg, clnt);
		if (result_1 == (bool_t *) NULL)
		{
			clnt_perror(clnt, "Error [post] cannot register student\n");
			clnt_destroy(clnt);
			return;
		}

		printf("Student registered with id = %d\n", *result_1);
	}
	else if (strcmp(action, "get") == 0)
	{
		const alumno *result_2 = buscar_alumno_1(busqueda_arg, clnt);
		if (result_2 == (alumno *) NULL || result_2->id == -1)
		{
			printf("Cannot find any student.\n");
			clnt_destroy(clnt);
			return;
		}

		printf("==> Student found with id %d\n", result_2->id);
		printf("\tName: %s %s\n", result_2->nombre, result_2->apellido);
		printf("\tAge: %d\n", result_2->edad);
		printf("\tCourse: %s\n", result_2->curso);
	}
	else if (strcmp(action, "put") == 0)
	{
		const bool_t *result_3 = actualizar_alumno_1(alumno_arg, clnt);
		if (result_3 == (bool_t *) NULL)
		{
			clnt_perror(clnt, "call failed");
			clnt_destroy(clnt);
			return;
		}
	}
	else if (strcmp(action, "delete") == 0)
	{
		const bool_t *result_4 = eliminar_alumno_1(&alumno_arg->id, clnt);
		if (result_4 == (bool_t *) NULL)
		{
			clnt_perror(clnt, "call failed");
			clnt_destroy(clnt);
			return;
		}

		if (*result_4 == FALSE)
		{
			printf("Cannot delete student, not found.\n");
			clnt_destroy(clnt);
			return;
		}

		printf("Student deleted.\n");
	}
	else
	{
		printf("Unknown action: %s\n", action);
		printf("Possible actions: [get, post, put, delete]\n");
	}

	clnt_destroy(clnt);
}

// ReSharper disable once CppParameterMayBeConst
int main(int argc, char *argv[])
{
	int opt;
	alumno alumno_arg;
	busqueda busqueda_arg;

	alumno_arg.nombre = "";
	alumno_arg.apellido = "";
	alumno_arg.curso = "";
	alumno_arg.edad = -1;
	alumno_arg.id = -1;

	busqueda_arg.nombre = "";
	busqueda_arg.apellido = "";
	busqueda_arg.curso = "";
	busqueda_arg.id = -1;

	if (argc < 3)
	{
		printf("Usage: %s server_host action [options]\n", argv[0]);
		printf("Options:\n");
		printf("\t-i id -n first_name -a last_name -c course -e age\n");
		printf("Actions:\n");
		printf("\tget: [-n || -a || -c]\n");
		printf("\tpost: [-n && -a && -c && -e]\n");
		printf("\tput: -i && [-n &| -a &| -c &| -e]\n");
		printf("\tdelete: -i\n");
		exit(1);
	}

	char *host = argv[1];
	char *action = argv[2];

	// Se empareja cada opción dada con cada argumento que se puede enviar.
	while ((opt = getopt(argc, argv, "n:a:e:i:c:")) != -1)
	{
		switch (opt)
		{
		case 'n':
			alumno_arg.nombre = optarg;
			busqueda_arg.nombre = optarg;
			break;
		case 'a':
			alumno_arg.apellido = optarg;
			busqueda_arg.apellido = optarg;
			break;
		case 'e':
			alumno_arg.edad = atoi(optarg);
			break;
		case 'i':
			alumno_arg.id = atoi(optarg);
			busqueda_arg.id = atoi(optarg);
			break;
		case 'c':
			alumno_arg.curso = optarg;
			busqueda_arg.curso = optarg;
			break;
		default:;
		}
	}

	registroalumnos_1(host, action, &alumno_arg, &busqueda_arg);

	exit(0);
}
