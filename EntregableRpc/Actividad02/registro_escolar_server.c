// ReSharper disable CppParameterNeverUsed
// ReSharper disable CppParameterMayBeConstPtrOrRef
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
	static bool_t result = {0}; // Siempre hay que reiniciar el resultado global.

	if (strlen(argp->nombre) < 0 || strlen(argp->apellido) < 0 || strlen(argp->curso) < 0 || argp->edad < 0)
	{
		printf("Cannot add student, missing information.\n");
		fflush(stdout);
		result = -1;
		return &result;
	}

	sqlite3 *db;               // Archivo de la base de datos.
	sqlite3_stmt *stmt;        // Para almacenar los resultados de las query a la base de datos.
	const char *buffer = NULL; // Buffer para construir las querys.

	sqlite3_open(DB_FILE, &db); // Apertura del archivvo de base de datos
	// Para construir la query se utiliza sqlite3_mprintf, similar a snprintf.
	buffer = sqlite3_mprintf("INSERT INTO alumnos(nombre, apellido, edad, curso) VALUES('%s', '%s', %d, '%s');", argp->nombre, argp->apellido, argp->edad, argp->curso);
	sqlite3_exec(db, buffer, NULL, NULL, NULL);

	// Para mostrar qué id se insertó, se ejecuta esta query que recupera dicho ID.
	sqlite3_prepare_v2(db, "SELECT last_insert_rowid()", -1, &stmt, NULL);
	sqlite3_step(stmt);

	argp->id = sqlite3_column_int(stmt, 0);

	printf("Alumno registrado con id = %d | %s %s (%d) a %s\n", argp->id, argp->nombre, argp->apellido, argp->edad, argp->curso);
	// Por alguna razón si no se realiza un fflush, no se muestran resultados en la terminal...
	fflush(stdout);
	sqlite3_finalize(stmt); // Terminado de los resultados de la query
	sqlite3_close(db);      // Finalizado de la base de datos
	result = argp->id;
	return &result;
}

/**
 * Busca a un alumno en la base de datos.
 * @param argp Parámetros de búsqueda (Nombre, Apellido o Curso).
 * @param rqstp Request al server.
 * @return Alumno encontrado o NULL.
 */
alumno *buscar_alumno_1_svc(busqueda *argp, struct svc_req *rqstp)
{
	static alumno result = {0};
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;

	/*
	 * Es necesario inicializar los valores, incluso si se obtiene un resultado, ya que el no
	 * inicializarlos puede provocar memory leaks (detectables por ASAN o valgrind).
	 */
	result.id = -1;
	result.nombre = "";
	result.apellido = "";
	result.curso = "";
	result.edad = -1;

	sqlite3_open(DB_FILE, &db);

	// Dependiendo de por qué se quiera buscar, se construye una query distinta.
	if (strlen(argp->nombre) > 0)
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE nombre LIKE '%q'", argp->nombre);
	else if (strlen(argp->apellido) > 0)
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE apellido LIKE '%q'", argp->apellido);
	else if (strlen(argp->curso) > 0)
		buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE curso='%q'", argp->curso);

	// Solo buscará por el primer resultado que haga match.
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		const int num_cols = sqlite3_column_count(stmt);

		if (num_cols != 5) // Si no coincide con la cantidad de columnas de la tabla
		{
			printf("No se encontró ningún alumno\n");
			fflush(stdout);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			return &result;
		}

		// Almacenado de los valores que se pueden copiar.
		result.id = sqlite3_column_int(stmt, 0);
		result.edad = sqlite3_column_int(stmt, 3);

		/*
		 * Para los casos de las cadenas, es necesario reservar memoria en los
		 * apuntadores de la estructura alumno, en donde posteriormente se copian
		 * las cadenas desde stmt, ya que estas se eliminarán al finalizar el stmt.
		 */
		const char *first_name = (char *) sqlite3_column_text(stmt, 1);
		const char *last_name = (char *) sqlite3_column_text(stmt, 2);
		const char *course = (char *) sqlite3_column_text(stmt, 4);

		result.nombre = malloc(sizeof(char) * strlen(first_name) + 1);
		result.apellido = malloc(sizeof(char) * strlen(last_name) + 1);
		result.curso = malloc(sizeof(char) * strlen(course) + 1);

		strcpy(result.nombre, first_name);
		strcpy(result.apellido, last_name);
		strcpy(result.curso, course);

		printf("Alumno encontrado con id=%d\n", result.id);
		break;
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
	static bool_t result = {0};
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;
	/*
	 * Para poder actualizar los valores en una sola llamada, se construyen
	 * las querys de acuerdo con cuantas columnas se van a cambiar, con
	 * ayuda de algunas cadenas auxiliares.
	 */
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
		// Se van anexando los valores a actualizar en las cadenas auxiliares.
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

	// Elimina la última coma para formar la query
	columns[strlen(columns) - 1] = '\0';
	values[strlen(values) - 1] = '\0';

	// Construye el query con los campos necesarios
	buffer = sqlite3_mprintf("UPDATE alumnos SET (%s) = (%s) WHERE id=%d", columns, values, argp->id);
	sqlite3_exec(db, buffer, NULL, NULL, NULL); // Ejecución de la query

	// Si no hubo errores, se completó la actualización con éxito.
	result = TRUE;
	printf("Student with id = %d updated. Columns changed: %s\n", argp->id, columns);
	fflush(stdout);

	// Libera las cadenas.
	free(columns);
	free(values);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return &result;
}

/**
 * Elimina un alumno de la base de datos.
 * @param argp Id del alumno a eliminar.
 * @param rqstp Request al servidor.
 * @return TRUE si se eliminó, FALSE en caso contrario.
 */
bool_t *eliminar_alumno_1_svc(int *argp, struct svc_req *rqstp)
{
	static bool_t result = FALSE;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *buffer = NULL;

	sqlite3_open(DB_FILE, &db);

	// Comprobación para verificar que el alumno exista.
	buffer = sqlite3_mprintf("SELECT * FROM alumnos WHERE id = %d", *argp);
	sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);

	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		printf("No students found with id %d\n", *argp);
		fflush(stdout);
		result = FALSE;
		return &result;
	}

	// Eliminado del registro.
	buffer = sqlite3_mprintf("DELETE FROM alumnos WHERE id = %d", *argp);
	sqlite3_exec(db, buffer, NULL, NULL, NULL);

	printf("Student with id %d removed.\n", *argp);

	fflush(stdout);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	result = TRUE;
	return &result;
}
