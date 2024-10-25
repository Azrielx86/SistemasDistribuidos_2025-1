#include "operaciones.h"
#include <math.h>
#include <stdio.h>

/**
 * Función del cliente para realizar una operación.
 * @param host Dirección del servidor.
 * @param op Información de la operación a realizar.
 */
void calculadora_1(char *host, operacion *op)
{
	CLIENT *clnt;
	float *result_1;
	operacion calcular_1_arg;

	calcular_1_arg.a = op->a;
	calcular_1_arg.b = op->b;
	calcular_1_arg.accion = op->accion;

	printf("Operación a realizar: %f %c %f\n", op->a, op->accion, op->b);

	clnt = clnt_create(host, CALCULADORA, CALCULADORA_V1, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1);
	}

	result_1 = calcular_1(&calcular_1_arg, clnt);
	if (result_1 == (float *) NULL)
	{
		clnt_perror(clnt, "call failed");
	}

	if (*result_1 == INFINITY)
		printf("Operación inválida\n");
	else
		printf("El resultado es: %.3f\n", *result_1);

	clnt_destroy(clnt);
}

int main(int argc, char *argv[])
{
	operacion op;
	char *host;

	// Comprobación de que se hayan pasado todos los argumentos.
	if (argc < 5)
	{
		printf("usage: %s server_host a [+ - * /] b\n", argv[0]);
		exit(1);
	}

	// El primer argumento es la dirección del servidor.
	host = argv[1];

	// La operación se acomoda como a [op] b en ese orden.
	op.a = (float) atof(argv[2]);
	op.accion = argv[3][0];
	op.b = (float) atof(argv[4]);

	calculadora_1(host, &op);
	exit(0);
}
