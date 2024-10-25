from concurrent import futures
import grpc
import ejemplo_pb2
import ejemplo_pb2_grpc

#Define los metodos de la calculadora
class CalculadoraServicer(ejemplo_pb2_grpc.CalculadoraServicer):
    def Sumar(self, request, context):
        #Toma los valores del request, envia el resultado
        resultado = request.num1 + request.num2
        return ejemplo_pb2.Resultado(resultado=resultado)

def servir():
    servidor = grpc.server(futures.ThreadPoolExecutor(max_workers=10))

    #Agrega la calculadora al server
    ejemplo_pb2_grpc.add_CalculadoraServicer_to_server(CalculadoraServicer(), servidor)

    #Establece el puerto de comunicación e inicia el servidor
    servidor.add_insecure_port('[::]:50051')
    servidor.start()
    print("Servidor gRPC en funcionamiento en el puerto 50051")
    servidor.wait_for_termination()

if __name__== '__main__':
        servir()

#python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. ejemplo.proto
