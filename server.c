// Server side C/C++ program to demonstrate Socket programming
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
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *connection = "Connection granted to client";
    FILE* jpeg;
  
    // Creating socket file descriptor
    //TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
  
    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    send(new_socket , connection , strlen(connection) , 0 );
    
  	memset(buffer, '\0',sizeof(buffer));
  	// read in the name of the file to send
  	valread = read(new_socket,buffer,1024);
  	jpeg = fopen(buffer,"rb");
  	if(jpeg != NULL){
        //send file
      	int size = 0;
      	int num_bytes_sent = 0;
      	char buf2;
      	int result;
      	int bytes_sent = 0;
      	int max_bytes_to_send = 1024;
      	int count = 0;
        printf("Sending file\n");
      	fseek(jpeg, 0, SEEK_END);
      	size = ftell(jpeg);
      	rewind(jpeg);
      	send(new_socket,size,sizeof(size),0);
      	
      	buf2 = (char*) malloc (sizeof(char)*size);
        // printf("here\n");
        // if(max_bytes_to_send > strlen(buf2)){
        //   max_bytes_to_send = strlen(buf2);
        // }
        
      	if (buf2 == NULL) {
          fputs ("Memory error\n",stderr); 
          exit (2);
        }
        
		result = fread(buf2,1,size,jpeg);
        // result = fscanf(jpeg,"%s",buf2);
        
        printf("result: %d size: %d",result,size);
  		
        if (result != size) {
          fputs ("Reading error\n",stderr); 
          exit (3);
        }
      	
      	while(num_bytes_sent != size){
          bytes_sent = send(new_socket,buf2,1024,0);
          if(bytes_sent < 0)
          {
            count++;
            printf("Fail count: %d\n",count);
            continue;
          }
          num_bytes_sent += bytes_sent;
        }
      	printf("File sent\n");
      
      
    }else{
      	//send neg ack
      	printf("File does not exist\n");
    }
  
  
  	printf("Any more requests?\n");
    return 0;
}