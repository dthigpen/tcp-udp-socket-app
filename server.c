#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

const int MAX_CONNECTIONS = 5;
const int READ_BUFFER_SIZE = 256;
enum Protocol{
    UDP,
    TCP
}protocol;

int main(int argc, char *argv[]){
    int socketfd, new_socketfd, port_number,read_result;
    socklen_t client_len;
    char buffer[READ_BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    


    if(argc != 3){
        printf("usage: %s [port 1024-65535] [tcp/udp]\n",argv[0]);
        exit(0);
    }else{
        //set protocol type and create socket
        if(strcasecmp(argv[1],"tcp")){
            protocol = TCP;
            socketfd = socket(AF_INET, SOCK_STREAM, 0);
        }else if(strcasecmp(argv[1],"udp")){
            protocol = UDP;
            socketfd = socket(AF_INET, SOCK_DGRAM, 0);
        }else{
            printf("usage: %s [port 1024-65535] [tcp/udp]\n",argv[0]);
            exit(0);
        }
        if(socketfd < 0){
		printf("Error opening socket");
	    }

        memset((char *) &server_addr,sizeof(server_addr),0);
        
        //set port number
        port_number = atoi(argv[1]);
        if (port_number < 1024 || port_number > 65535){
            printf("Invalid port number. Port range: 1024-65535\n");
            exit(0);
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        //swap the endianness of the port number for network order
        server_addr.sin_port = htons(port_number);

        if(bind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
            printf("Error binding socket to IP address\n");
            exit(0);
        }

        listen(socketfd,MAX_CONNECTIONS);
        client_len = sizeof(client_addr);
        
        new_socketfd = accept(socketfd, (struct sockaddr *) &client_addr, client_len);
        if(new_socketfd < 0){
            printf("Error accepting connection\n");
            exit(0);
        }
        
        printf("[%s]: connection to %s on port %d",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        send(new_socketfd,"Successful connection\n",13,0);
    
        memset((char *) &buffer, 0,sizeof(buffer));
        read_result = read(new_socketfd, buffer, READ_BUFFER_SIZE - 1);
        if(read_result < 0){
            printf("Error reading message from client\n");
        }
        
        close(new_socketfd);
        close(socketfd);
        
        return 0;
    }
}