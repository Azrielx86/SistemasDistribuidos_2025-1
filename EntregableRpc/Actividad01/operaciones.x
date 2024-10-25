struct operacion {
  float a;
  float b;
  char accion;
};

program CALCULADORA {
  version CALCULADORA_V1 {
    float calcular(operacion) = 1;
  } = 1;
} = 0x20000001;

