import grpc
import ejemplo_pb2
import ejemplo_pb2_grpc

def ejecutar():
    #Establece el canal de comunicación
    canal = grpc.insecure_channel('localhost:50051')

    #Crea un objeto stub para el cliente
    cliente = ejemplo_pb2_grpc.CalculadoraStub(canal)

    #Par de nùmeros que se van a enviar al cliente
    numeros = ejemplo_pb2.Numeros(num1=3, num2=5)

    #Llamada a procedimiento remoto
    respuesta = cliente.Sumar(numeros)

    print(f"Resultado de la suma: {respuesta.resultado}")

if __name__ == '__main__':
    ejecutar()