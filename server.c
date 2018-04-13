// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8088
#define BUFFSIZE 1024


int sendall(int s,char* buffer, int*len);

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
    memset(buffer, '\0',BUFFSIZE);
    valread = read( new_socket , buffer, BUFFSIZE);
    printf("%s\n",buffer );
    send(new_socket , connection , strlen(connection) , 0 );
    
  	memset(buffer, '\0',BUFFSIZE);
  	// read in the name of the file to send
  	valread = read(new_socket,buffer,BUFFSIZE);
  	jpeg = fopen(buffer,"rb");
  	if(jpeg != NULL){
        //send file
      	int size = 0;
      	int num_bytes_sent = 0;
      	int result = 0;
      	int bytes_sent = 0;
      	int max_bytes_to_send = BUFFSIZE;
      	int count = 0;
        int sent_count = 0;
      	fseek(jpeg, 0, SEEK_END);
      	size = ftell(jpeg);
      	rewind(jpeg);
        
        //SEND SIZE INFO
        //send the size of the file to the client first
        char* size_str[16];
        sprintf(size_str,"%d",size);
      	send(new_socket,size_str,sizeof(size_str),0);
      	printf("Sending file size: %s\n",size_str);
        //SEND SIZE INFO
        
        printf("Calculated File size: %d", size);
        
      	while(num_bytes_sent < size){
            memset(buffer, '\0',BUFFSIZE);
            int bytes_left = size - ftell(jpeg);
            int bytes_read = fread(&buffer,sizeof(char),BUFFSIZE,jpeg);
            if(bytes_read == 0){
                printf("Finished reading\n");
                break;
            }
            if(bytes_read < 0){
                printf("Error reading file\n");
                //TODO file read error
                break;
            }
            //the last section of the file may be smaller than the buffer size
            int send_size = bytes_left < BUFFSIZE ? bytes_read : BUFFSIZE;
            //send all of the data in the buffer so far
            if(send_size != BUFFSIZE){
                printf("new send size: %d\n",send_size);
            }
            result = sendall(new_socket,buffer,&send_size);
            if(result == -1){
                printf("Failed to send buffered data\n");
                //TODO failed to send error
                break;
            }else{
                num_bytes_sent += BUFFSIZE;
                printf("bytes sent: %d / %d\n",num_bytes_sent,size);
            }
        }
        printf("after loop bytes sent: %d / %d\n",num_bytes_sent,size);
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

int sendall(int s, char* buffer, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buffer, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 