#include "operaciones.h"
#include <math.h>
#include <stdio.h>

/**
 * Función para realizar una operación.
 * @param argp Información de la operación a realizar.
 * @param rqstp Request del server
 * @return Resultado de la operación.
 */
float *calcular_1_svc(operacion *argp, struct svc_req *rqstp)
{
	static float result;

	// Elección de la operación a realizar.
	switch (argp->accion)
	{
	case '+': // Caso suma.
		result = argp->a + argp->b;
		break;
	case '-': // Caso resta.
		result = argp->a - argp->b;
		break;
	case '*': // Caso multiplicación.
		result = argp->a * argp->b;
		break;
	case '/': // Caso división.
		result = argp->a / argp->b;
		break;
	default: // El resultado por default si ninguna operación es seleccionada, se toma INFINITY.
	         // No se toma NAN ya que puede o no estar definido en algunas versiones / compiladores de C.
		result = INFINITY;
	}

	printf("==> %.3f %c %.3f = %.3f\n", argp->a, argp->accion, argp->b, result);

	// Retorno de la operación.
	return &result;
}
