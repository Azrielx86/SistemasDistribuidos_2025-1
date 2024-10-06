import socket
import time
import datetime

def convertirCadenaAHora(cadena):
	formato='%Y%m%d %H:%M:%f'
	horaCadena=datetime.datetime.strptime(cadena,formato)
	return horaCadena

IPServidor="localhost"
puertoServidor=9099
#apertura del socket
socketCliente=socket.socket(socket.AF_INET,socket.SOCK_STREAM)

inicio=time.time()
#solicitar conexion con el servidor
socketCliente.connect((IPServidor,puertoServidor))
#Recibir del servidor
horaCadena=socketCliente.recv(4096).decode()

final=time.time()
tiempo=final-inicio

hora=convertirCadenaAHora(horaCadena)
print("El tiempoTotal de la ida y vuelta fue:",tiempo)

mitadtiempo=tiempo/2

print("Tiempo total de ida",mitadtiempo)
print("Hora servidor", horaCadena)
print("La hora excacta es", hora+datetime.timedelta(secons=mitadtiempo))

socketCliente.close()







