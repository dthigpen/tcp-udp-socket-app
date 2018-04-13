// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8088
#define BUFFSIZE 1024




int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Connection Requested from client";
    char buffer[BUFFSIZE] = {0};
    char work_buffer[BUFFSIZE*2];
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
    valread = recv( sock , buffer, BUFFSIZE,0);
    printf("%s\n",buffer);
  	memset(&buffer,'\0',BUFFSIZE);
    //RECEIVE SIZE INFO
    //next message should be file size
    int read_code = recv(sock, buffer, BUFFSIZE,0);
    printf("%s\n",buffer);
    int size = atoi(buffer);
    //RECEIVE SIZE INFO
    
    printf("Received size of file: %d bytes\n", size);
  	int bytes_received = 0;
  	FILE *jpeg;
  	char* out_file;
    int fail_count = 0;
    int write_fail_count = 0;
    int packets_received = 0;
    int wb_offset = 0;
  	jpeg = fopen("out_file.jpg","wb");
    memset(&work_buffer,'\0',BUFFSIZE*2);	
  	while(bytes_received < size){
        
        memset(&buffer,'\0',BUFFSIZE);
        // valread = recv(sock,work_buffer+wb_offset,BUFFSIZE,0);
        valread = recv(sock,buffer,BUFFSIZE,0);
      	if(valread < 0){ //an error has occured
            printf("Error recieving packet\n");
            fail_count++;
            continue;
        }else if(valread == 0){ //the connection has been closed
            printf("Server has closed the connection\n");
            break;
        }else{

            wb_offset += valread;
            if(wb_offset > BUFFSIZE){
                //then a full packet has been received
                //TODO write first BUFFSIZE of work_buffer to file
                //TODO shift over work_buffer by BUFF_SIZE
            }else{
                //still a partial packet OR last packet
            }
            packets_received++;
            int write_error = fwrite(buffer,sizeof(char),valread, jpeg);
            if(write_error <= 0){
                write_fail_count++;
            }
            bytes_received += valread;	
            printf("bytes received: %d / %d (%d received %d dropped)\n",bytes_received,size,packets_received, fail_count);
            // printf("%s\n",buffer);
        }
    }
    
    memset(&buffer,'\0',BUFFSIZE);
    printf("Write fails: %d\n", write_fail_count);
    printf("File received\n");
  	fclose(jpeg);
    shutdown(sock,2);
    return 0;
}