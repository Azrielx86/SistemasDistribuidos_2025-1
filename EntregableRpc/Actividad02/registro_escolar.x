struct alumno {
  int id;
  string nombre<>;
  string apellido<>;
  int edad;
  string curso<>;
};

struct busqueda {
  int id;
  string nombre<>;
  string apellido<>;
  string curso<>;
};

program REGISTROALUMNOS {
  version REGISTROALUMNOS_V1 {
    int registrar_alumno(alumno) = 1;
    alumno buscar_alumno(busqueda) = 2;
    bool actualizar_alumno(alumno) = 3;
    bool eliminar_alumno(int) = 4;
  } = 1;
} = 0x20000001;
