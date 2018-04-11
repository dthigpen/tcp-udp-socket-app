// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8080
  
int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Connection Requested from client";
    char buffer[1024] = {0};
	const char *filename = argv[1];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , hello , strlen(hello) , 0 );
    printf("Connection request sent\n");
	
	send(sock , filename , strlen(filename) , 0 );
	printf("Sent request for filename\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer,1024 );
  	memset(&buffer,'\0',strlen(buffer));
  	int size = read(sock, buffer, 1024);
  	int bytes_received = 0;
  	FILE *jpeg;
  	const char* added = ".jpg";
  	char* out_file;
  	out_file = malloc(strlen(filename) + 4 + 1);
  	jpeg = fopen(out_file,"wb");
    
  	while(bytes_received < size){
        memset(&buffer,'\0',strlen(buffer));	
        valread = read(sock,buffer,1024);
      	if(valread < 0){
          printf("Error recieving packet\n");
        }
      	fwrite(buffer,1,strlen(buffer)+1, jpeg);
    	printf("hello\n");
        bytes_received += valread;	
    }
  	close(jpeg);
    return 0;
}