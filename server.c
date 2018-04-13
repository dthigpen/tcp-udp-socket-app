// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8080
#define BUFFSIZE 1024

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFSIZE];
    char *connection = "Connection granted to client\n";
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
  
    valread = read( new_socket , buffer, BUFFSIZE);
    printf("%s\n",buffer );
    send(new_socket , connection , strlen(connection) , 0 );
    
  	memset(buffer, '\0',sizeof(buffer));
  	// read in the name of the file to send
  	valread = read(new_socket,buffer,BUFFSIZE);
  	jpeg = fopen(buffer,"rb");
  	if(jpeg != NULL){
        //send file
      	int size = 0;
      	int num_bytes_sent = 0;
      	int result;
      	int bytes_sent = 0;
      	int max_bytes_to_send = BUFFSIZE;
      	int count = 0;
        int sent_count = 0;
      	fseek(jpeg, 0, SEEK_END);
      	size = ftell(jpeg);
      	rewind(jpeg);
        //send the size of the file to the client first
        // char* size_str[16];
        // sprintf(size_str,"%d",size);
      	// send(new_socket,size_str,sizeof(size_str),0);
      	// printf("Sending file size: %s\n",size_str);
        printf("Calculated File size: %d", size);
        memset(buffer, '\0',BUFFSIZE);
      	while(num_bytes_sent < size){
            
            int bytes_left = size - ftell(jpeg);
            if(bytes_left >= max_bytes_to_send){
                bytes_sent = send(new_socket,jpeg,max_bytes_to_send,0);
            }else{
                bytes_sent = send(new_socket,jpeg,bytes_left,0);
            }
            
            if(bytes_sent < 0)
            {
            count++;
            printf("Fail count: %d\n",count);
            // printf("bytes sent: %d size %d\n",num_bytes_sent,size);
            continue;
            }
            sent_count++;
            num_bytes_sent += bytes_sent;
            printf("bytes sent: %d / %d (%d total packets)\n",num_bytes_sent,size, sent_count);
            fseek(jpeg,bytes_sent,SEEK_CUR);
        }
        printf("Fail count: %d\n",count);
        printf("File sent\n");
        

    }else{
      	//send neg ack
      	printf("File does not exist\n");
    }

    shutdown(new_socket, 1);
  	printf("Any more requests?\n");
  	fclose(jpeg);  
  	return 0;
}