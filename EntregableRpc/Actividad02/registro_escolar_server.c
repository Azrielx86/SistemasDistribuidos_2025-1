// ReSharper disable CppParameterNeverUsed
#include "registro_escolar.h"
#include <sqlite3.h>

/**
 * Registra un alumno en el sistema.
 * @param argp Datos del alumno.
 * @param rqstp Request al server.
 * @return TRUE si se registró, FALSE en caso contrario.
 * @note Requiere definir todos los campos de argp.
 */
int *registrar_alumno_1_svc(alumno *argp, struct svc_req *rqstp)
{
	static bool_t result;

	if (strlen(argp->nombre) < 0 || strlen(argp->apellido) < 0 || strlen(argp->curso) < 0 || argp->edad < 0)
	{
		printf("Cannot add student, missing information.\n");
		fflush(stdout);
		result = -1;
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
	result = argp->id;
	return &result;
}

/**
 * Busca a un alumno en la base de datos.
 * @param argp Parámetros de búsqueda (Nombre, Apellido o Curso).
 * @param rqstp Request al server.
 * @return Alumno encontrado o NULL.
 */
// ReSharper disable once CppParameterMayBeConstPtrOrRef
alumno *buscar_alumno_1_svc(busqueda *argp, struct svc_req *rqstp)
{
	static alumno result;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;

	sqlite3_open("alumnos.db", &db);

	if (strlen(argp->nombre) > 0)
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE nombre='%q'", argp->nombre);
	else if (strlen(argp->apellido) > 0)
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE apellido='%q'", argp->apellido);
	else if (strlen(argp->curso) > 0)
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE curso='%q'", argp->curso);
	
	// Solo buscará por el primer resultado que haga match.
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		const int num_cols = sqlite3_column_count(stmt);

		if (num_cols != 5)
		{
			result.id = -1;
			result.nombre = "";
			result.apellido = "";
			result.curso = "";
			result.edad = -1;
			return &result;
		}

		result.id = sqlite3_column_int(stmt, 0);
		result.nombre = (char *) sqlite3_column_text(stmt, 1);
		result.apellido = (char *) sqlite3_column_text(stmt, 2);
		result.edad = sqlite3_column_int(stmt, 3);
		result.curso = (char *) sqlite3_column_text(stmt, 4);

		printf("Alumno encontrado con id=%d", result.id);
		return &result;
	}

	fflush(stdout);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
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

/**
 * Elimina un alumno de la base de datos.
 * @param argp Id del alumno a eliminar.
 * @param rqstp Request al servidor.
 * @return TRUE si se eliminó, FALSE en caso contrario.
 */
// ReSharper disable once CppParameterMayBeConstPtrOrRef
bool_t *eliminar_alumno_1_svc(int *argp, struct svc_req *rqstp)
{
	static bool_t result;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;

	sqlite3_open("alumnos.db", &db);
	buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE id = %d", *argp);
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);

	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		printf("No students found with id %d\n", *argp);
		fflush(stdout);
		result = FALSE;
		return &result;
	}

	buffer = sqlite3_mprintf("DELETE FROM alumnos WHERE id = %d", *argp);
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
	sqlite3_step(stmt);

	printf("Student with id %d removed.\n", *argp);

	fflush(stdout);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	result = TRUE;
	return &result;
}
