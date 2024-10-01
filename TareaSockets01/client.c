#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
	if (argc < 2)
	{
		printf("Argumentos necesarios: <ip> <puerto>");
		return -1;
	}

	char *ip;
	int fd, numbytes, puerto;
	char buff[BUFFER_SIZE];

	puerto = atoi(argv[2]);
	ip = *(argv + 1);

	struct hostent *he;
	struct sockaddr_in server;

	char cadena[BUFFER_SIZE];
	FILE *myf = fopen("chatCliente.txt", "a");
	time_t t;
	struct tm *tm;
	char hora[BUFFER_SIZE];
	char *tmp;
	char sendline[BUFFER_SIZE] = "usando el puerto";

	if ((he = gethostbyname(ip)) == NULL)
		THROW_ERROR("Cannot get hostname")

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		THROW_ERROR("Cannot create socket")

	server.sin_family = AF_INET;
	server.sin_addr = *(struct in_addr *) he->h_addr;
	server.sin_port = htons(puerto);
	bzero(&server.sin_zero, 8);

	if (connect(fd, (struct sockaddr *) &server, sizeof(struct sockaddr)) < 0)
		THROW_ERROR("Cannot connect to server")

	while (!strstr(cadena, "adios") && !strstr(sendline, "adios"))
	{
		bzero(cadena, BUFFER_SIZE);
		t = time(NULL);
		tm = localtime(&t);
		strftime(hora, BUFFER_SIZE, "\n yo (%H:%M) -> ", tm);
		printf("%s", hora);

		fgets(sendline, BUFFER_SIZE, stdin);
		tmp = strcat(hora, sendline);

		fputs(tmp, myf);

		write(fd, sendline, strlen(sendline) + 1);

		if (!strstr(cadena, "adios"))
		{
			strftime(hora, BUFFER_SIZE, "\n Otro usuario (%H:%M) -> ", tm);
			read(fd, cadena, BUFFER_SIZE);
			tmp = strcat(hora, cadena);
			printf("%s", tmp);
			fputs(tmp, myf);
		}
		fflush(myf);
	}

	printf("\n\nFin conversación\n");
	fputs("\n\nFin conversación\n", myf);
	fclose(myf);
	close(fd);

	return 0;
}
