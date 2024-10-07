import socket
import time
import datetime

def convertirCadenaAHora(cadena):
	formato='%Y%m%d %H:%M:%S%f'
	horaCadena=datetime.datetime.strptime(cadena,formato)
	return horaCadena

IPServidor="192.168.100.120"
puertoServidor=9099
#apertura del socket
socketCliente=socket.socket(socket.AF_INET,socket.SOCK_STREAM)

inicio=time.time()
#solicitar conexion con el servidor
socketCliente.connect((IPServidor,puertoServidor))
#Recibir del servidor
horaCadena=socketCliente.recv(4096).decode()

horaCliente = datetime.datetime.now()
print("Hora del Cliente: ",horaCliente)

final=time.time()
tiempo=final-inicio

hora=convertirCadenaAHora(horaCadena)
print("El tiempo Total de la ida y vuelta fue:",tiempo)

mitadtiempo=tiempo/2

horaexacta= hora + datetime.timedelta(seconds=mitadtiempo)

print("Tiempo total de ida",mitadtiempo)
print("Hora servidor", hora)
print("La hora exacta es", horaexacta)

socketCliente.close()







