// ReSharper disable CppParameterNeverUsed
#include "registro_escolar.h"
#include "rpc/types.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#define DB_FILE "alumnos.db"

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

	sqlite3_open(DB_FILE, &db);
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
	memset(&result, 0, sizeof(alumno));
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;

	result.id = -1;

	sqlite3_open(DB_FILE, &db);

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
			return &result;

		result.id = sqlite3_column_int(stmt, 0);
		result.nombre = (char *) sqlite3_column_text(stmt, 1);
		result.apellido = (char *) sqlite3_column_text(stmt, 2);
		result.edad = sqlite3_column_int(stmt, 3);
		result.curso = (char *) sqlite3_column_text(stmt, 4);

		printf("Alumno encontrado con id=%d", result.id);
		fflush(stdout);
		return &result;
	}

	fflush(stdout);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return &result;
}

/**
 * Función para actualizar el alumno. Para actualizar el alumno, es necesario dar el ID
 * en el parámtro de entrada, y todos los valores que no sean cadenas vacías o -1, se
 * actualizarán en la base de datos.
 * @param argp Datos del alumno
 * @param rqstp Request del server
 * @return TRUE si hubo cambios, FALSE en caso contrario
 */
bool_t *actualizar_alumno_1_svc(alumno *argp, struct svc_req *rqstp)
{
	static bool_t result;
	memset(&result, 0, sizeof(bool_t));
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;
	char *columns = calloc(1024, sizeof(char) * 1024);
	char *values = calloc(1024, sizeof(char) * 1024);
	char temp[256];

	sqlite3_open(DB_FILE, &db);
	buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE id = %d", argp->id);
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);

	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		printf("No students found with id %d\n", argp->id);
		fflush(stdout);
		result = FALSE;
		return &result;
	}

	if (strlen(argp->nombre) > 0)
	{
		strcat(columns, "nombre,");
		snprintf(temp, 256, "'%s',", argp->nombre);
		strcat(values, temp);
	}

	if (strlen(argp->apellido) > 0)
	{
		strcat(columns, "apellido,");
		snprintf(temp, 256, "'%s',", argp->apellido);
		strcat(values, temp);
	}

	if (strlen(argp->curso) > 0)
	{
		strcat(columns, "curso,");
		snprintf(temp, 256, "'%s',", argp->curso);
		strcat(values, temp);
	}

	if (argp->edad > 0)
	{
		strcat(columns, "nombre,");
		snprintf(temp, 256, "%d,", argp->edad);
		strcat(values, temp);
	}

	// Eliminar la última coma
	columns[strlen(columns) - 1] = '\0';
	values[strlen(values) - 1] = '\0';

	// construir el query con los campos necesarios
	buffer = sqlite3_mprintf("UPDATE alumnos SET (%s) = (%s) WHERE id=%d", columns, values, argp->id);
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
	sqlite3_step(stmt);

	result = TRUE;
	printf("Student with id = %d updated. Columns changed: %s\n", argp->id, columns);
	fflush(stdout);

	free(columns);
	free(values);
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

	sqlite3_open(DB_FILE, &db);
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
