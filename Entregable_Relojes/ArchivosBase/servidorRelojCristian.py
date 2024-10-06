import socket
import datetime

direcServidor="localhost"
puertoServidor=9099
#abrimos el Socket
socketServidor=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
#avisar al SO de cr
socketServidor.bind ((direcServidor,puertoServidor))
#escuchar
socketServidor.listen()

while True:
	#Aceptar conexiones de cliente
	socketConexcion,adde=socketServidor.accept()
	print("Conectado con el cliente", addr)
	#se calcula la hora despues de la peticion
	hora=datetime.datetime.now()
	#convietiendo a cadena
	horaCadena=hora.strftime('%Y%m%d %H:%M:%f')
	#notificacion de la hora a la fecha
	print("Envio la hora al cliente",addr)
	print(hora)
	socketConexion.sent(horaCadena.encode())
	socketConexion.close()
