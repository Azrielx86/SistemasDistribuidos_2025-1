import grpc
import calculadora_pb2
import calculadora_pb2_grpc

def ejecutar():
    #Establece el canal de comunicación
    canal = grpc.insecure_channel('localhost:50051')

    #Crea un objeto stub para el cliente
    cliente = calculadora_pb2_grpc.CalculadoraStub(canal)

    option = 1

    while True:

        print("0 - Salir; 1 - Sumar; 2 - Restar; 3 - Multiplicar; 4 - Dividir")
        opcion = int(input("Seleccione una operacion: "))

        if opcion == 0:
            break

        numero1 = int(input("Ingrese el primer numero: "))
        numero2 = int(input("Ingrese el segundo numero: "))

        numeros = calculadora_pb2.Numeros(num1=numero1, num2=numero2)
        respuesta = cliente.Sumar(numeros)

        if opcion == 1:
            respuesta = cliente.Sumar(numeros)
        elif opcion == 2:
            respuesta = cliente.Restar(numeros)
        elif opcion == 3:
            respuesta = cliente.Multiplicar(numeros)
        elif opcion == 4:
            if numero2 != 0:
                respuesta = cliente.Dividir(numeros)
            else:
                print("No se puede dividir entre 0")
        
        print(f"Resultado de la operacion: {respuesta.resultado}")

if __name__ == '__main__':
    ejecutar()