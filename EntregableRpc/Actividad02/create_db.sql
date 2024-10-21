CREATE TABLE IF NOT EXISTS alumnos
(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nombre VARCHAR(30),
    apellido VARCHAR(30),
    edad INT(2),
    curso VARCHAR(30)
);