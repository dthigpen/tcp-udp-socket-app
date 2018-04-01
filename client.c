#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


int main(int argc, char **argv){
	int socketfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	int TCP = 1;
	if (argc != 4)
	{
		printf("Error must have 3 arguments: server name, port number, and TCP/UDP} \n");
		exit(0);
	}
	portno = atoi(arv[2]);
	// initialize socket for TCP or UDP connection
	if(argv[3] == "UDP"  || arv[3] == "udp"){
		TCP = 0;
		socketfd = socket(AF_INET,SOCK_DGRAM,0);
	}
	else if (argv[3] == "TCP"  || arv[3] == "tcp"){
		TCP = 1;
		socketfd = socket(AF_INET,SOCK_STREAM,0);
	}
	else{
		printf("Error protocol must be either TCP or UDP \n");
		exit(0);
	}
	if(socketfd < 0)
	{
		printf("Error opening socket");
		exit(0);
	}
	
	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	memset((void *) &serv_addr,0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove( (char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the message: ");
    memset((void *)(buffer),0,256);
    fgets(buffer,255,stdin);
    n = write(socketfd, buffer, strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    memset((void *)(buffer),0,256);
    n = read(socketfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n", buffer);
    close(socketfd);
    return 0;
}