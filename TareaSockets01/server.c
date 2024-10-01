#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 100

#define THROW_ERROR(msg) \
	{                    \
		perror(msg);     \
		return errno;    \
	}

int main(int argc, char *argv[])
{

	if (argc < 1)
	{
		printf("No se ingresó el puerto como parámetro.\n");
		return 1;
	}

	int fd, fd2, puerto;
	socklen_t longitud_cliente;
	puerto = atoi(argv[1]);

	char cadena[BUFFER_SIZE];
	FILE *myf = fopen("chatServer.txt", "a");
	time_t t;
	struct tm *tm;
	char hora[BUFFER_SIZE];
	char *tmp;
	char sendline[BUFFER_SIZE];

	struct sockaddr_in server;
	struct sockaddr_in client;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(puerto);
	bzero(&server.sin_zero, 8);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		THROW_ERROR("Cannot create socket: ")

	if (bind(fd, (struct sockaddr *) &server, sizeof(struct sockaddr)) < 0)
		THROW_ERROR("Cannot bind socket: ")

	if (listen(fd, 5) < 0)
		THROW_ERROR("Cannot listen on socket: ")

	longitud_cliente = sizeof(struct sockaddr_in);

	if ((fd2 = accept(fd, (struct sockaddr *) &client, &longitud_cliente)) < 0)
		THROW_ERROR("Cannot accept a connection: ")

	printf("\n\n\n\nSe inicia el chat\n\n");
	fputs("\n\n\n\nSe inicia el chat\n\n", myf);

	while (!strstr(cadena, "adios") && !strstr(sendline, "adios"))
	{
		bzero(cadena, BUFFER_SIZE);
		t = time(NULL);
		tm = localtime(&t);
		strftime(hora, BUFFER_SIZE, "\n Otro usuario (%H:%M) ->", tm);

		read(fd2, cadena, BUFFER_SIZE);
		tmp = strcat(hora, cadena);
		printf("%s", tmp);
		fputs(tmp, myf);

		if (!strstr(cadena, "adios"))
		{
			strftime(hora, BUFFER_SIZE, "\n yo (%H:%M) -> ", tm);
			printf("%s", hora);
			fgets(sendline, BUFFER_SIZE, stdin);
			tmp = strcat(hora, sendline);
			write(fd2, sendline, strlen(sendline) + 1);
			fputs(tmp, myf);
			fflush(myf);
		}
	}

	close(fd2);
	printf("\n\nFin conversación\n");
	printf("\nSe generó el archivo\n");
	fclose(myf);

	return 0;
}
