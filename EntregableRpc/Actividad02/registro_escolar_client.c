#include "registro_escolar.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void registroalumnos_1(char *host, char *action, alumno *alumno_arg, busqueda *busqueda_arg)
{
	alumno *result_2;
	bool_t *result_3;
	bool_t *result_4;

	CLIENT *clnt = clnt_create(host, REGISTROALUMNOS, REGISTROALUMNOS_V1, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1);
	}

	if (strcmp(action, "post") == 0)
	{
		const bool_t *result_1 = registrar_alumno_1(alumno_arg, clnt);
		if (result_1 == (bool_t *) NULL)
		{
			clnt_perror(clnt, "call failed");
		}

		if (result_1 == NULL)
		{
			fprintf(stderr, "Error [get] with parameters: %i", alumno_arg->id);
			return;
		}
	}
	else if (strcmp(action, "get") == 0)
	{
		result_2 = buscar_alumno_1(busqueda_arg, clnt);
		if (result_2 == (alumno *) NULL)
		{
			clnt_perror(clnt, "call failed");
		}
	}
	else if (strcmp(action, "put") == 0)
	{
		result_3 = actualizar_alumno_1(alumno_arg, clnt);
		if (result_3 == (bool_t *) NULL)
		{
			clnt_perror(clnt, "call failed");
		}
	}
	else if (strcmp(action, "delete") == 0)
	{

		result_4 = eliminar_alumno_1(&alumno_arg->id, clnt);
		if (result_4 == (bool_t *) NULL)
		{
			clnt_perror(clnt, "call failed");
		}
	}
	else
	{
		printf("Unknown action: %s\n", action);
	}

	clnt_destroy(clnt);
}

int main(int argc, char *argv[])
{
	int opt;
	char *host;
	char *action;
	alumno alumno_arg;
	busqueda busqueda_arg;
	char *course;

	if (argc < 3)
	{
		printf("Usage: %s server_host action [options]\n", argv[0]);
		exit(1);
	}

	host = argv[1];
	action = argv[2];

	while ((opt = getopt(argc, argv, "n:a:e:i:c:")) != -1)
	{
		int arglen = strlen(optarg);
		switch (opt)
		{
		case 'n':
			alumno_arg.nombre = optarg;
			busqueda_arg.nombre = optarg;
			break;
		case 'a':
			alumno_arg.apellido = optarg;
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
		}
	}

	registroalumnos_1(host, action, &alumno_arg, &busqueda_arg);

	exit(0);
}
