#include "registro_escolar.h"
#include <sqlite3.h>

bool_t *
registrar_alumno_1_svc(alumno *argp, struct svc_req *rqstp)
{
	static bool_t result;

	if (argp->nombre == NULL || argp->apellido == NULL || argp->curso == NULL)
	{
		printf("Cannot add student, missing information.\n");
		result = FALSE;
		return &result;
	}

	sqlite3 *db;
	sqlite3_stmt *stmt;

	const char *buffer = NULL;

	sqlite3_open("alumnos.db", &db);
	buffer = sqlite3_mprintf("INSERT INTO alumnos(nombre, apellido, edad, curso) VALUES('%s', '%s', %d, '%s');", argp->nombre, argp->apellido, argp->edad, argp->curso);
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
	sqlite3_step(stmt);
	
	sqlite3_prepare_v2(db, "SELECT last_insert_rowid()", -1, &stmt, NULL);
	sqlite3_step(stmt);
	
	argp->id = sqlite3_column_int(stmt, 0);

	printf("Alumno registrado: [%d] %s %s %d", argp->id, argp->nombre, argp->apellido, argp->edad);
	fflush(stdout);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	result = TRUE;
	return &result;
}

alumno *
buscar_alumno_1_svc(busqueda *argp, struct svc_req *rqstp)
{
	static alumno result;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char* buffer = NULL;
	
	sqlite3_open("alumnos.db", &db);

	// busaqueda por curso
	if (argp->curso != NULL)
	{
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE curso=%q", argp->curso);
	}

	return &result;
}

bool_t *
actualizar_alumno_1_svc(alumno *argp, struct svc_req *rqstp)
{
	static bool_t result;

	/*
	 * insert server code here
	 */

	return &result;
}

bool_t *
eliminar_alumno_1_svc(int *argp, struct svc_req *rqstp)
{
	static bool_t result;

	/*
	 * insert server code here
	 */

	return &result;
}
