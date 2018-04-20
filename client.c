// Client side C/C++ program to demonstrate TCP Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <openssl/md5.h>

#define BUFFSIZE 1024
#define MSGSIZE 16

void generate_md5(FILE *file, char *result);
void print_md5(char* result);

int main(int argc, char const *argv[])
{
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in address;
    int sock = 0, valread, rv;
    struct sockaddr_in serv_addr;
    char buffer[BUFFSIZE] = {0};
	const char *filename;
    int PORT = 8088;
    const char* server_ip;
    char received_md5[MD5_DIGEST_LENGTH];
	char calculated_md5[MD5_DIGEST_LENGTH];
    char ack[]="ACK!";
	char nack[] = "NACK";
    
    if(argc < 4){
      printf("Error need exactly 3 arguments. Arg1: Server-IP Arg2: Port# Arg3: File Name\n");
        exit(0);
    }else{
        server_ip = argv[1];
        PORT = atoi(argv[2]);
        filename =  argv[3];
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    //argv[2] is the port# arg
    if ((rv = getaddrinfo(server_ip, argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //connect to the first result from our getaddrinfo lookup on the specified ip
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        break;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Connection request sent\n");
	
  	// receive connection message
  	valread = recv( sock , buffer, BUFFSIZE,0);
    printf("%s\n",buffer);
  	memset(&buffer,'\0',BUFFSIZE);
  
  	//send the filename to the server
	send(sock , filename , strlen(filename) , 0 );
	printf("Filename request sent\n");
    
  	// Receive ACK or NACK message
	valread = recv(sock,buffer,5,0);
	printf("%s\n",buffer);
  	if(buffer[0] == 'N')
	{
		printf("Error file does not exist in server.\n");
		exit(1);
	}
	//Receive the checksum
    memset(&buffer,'\0',BUFFSIZE);
    valread = recv(sock,buffer,BUFFSIZE,0);
    memcpy(received_md5,buffer,MD5_DIGEST_LENGTH);

  
  	//RECEIVE SIZE INFO
    //next message should be file size
    int read_code = recv(sock, buffer, MSGSIZE,0);
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
    
  	jpeg = fopen("out_file.jpg","rb+");
  	
  	while(bytes_received < size){
        
        memset(&buffer,'\0',BUFFSIZE);
        // valread = recv(sock,work_buffer+wb_offset,BUFFSIZE,0);
        valread = recv(sock,buffer,BUFFSIZE,0);
      	if(valread < 0){ //an error has occured
            printf("Error recieving packet\n");
            fail_count++;
            break;
        }else if(valread == 0){ //the connection has been closed
            printf("Server has closed the connection\n");
            break;
        }else{
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
    fclose(jpeg);
    //check the received md5 with the calculated one
    jpeg = fopen("out_file.jpg","rb");
    generate_md5(jpeg, calculated_md5);
    printf("Calculated MD5:\n");
    print_md5(calculated_md5);
    printf("Received MD5:\n");
    print_md5(received_md5);

    if(strncmp(calculated_md5, received_md5, MD5_DIGEST_LENGTH) == 0){
        //send match
        send(sock,ack,strlen(ack),0);
        printf("Checksums match\n");
    }else{
        //send mismatch
        send(sock,nack,strlen(nack),0);
        printf("Checksum mismatch\n");
    }
    printf("Write fails: %d\n", write_fail_count);
    printf("File received\n");
  	fclose(jpeg);
    shutdown(sock,2);
    return 0;
}

void generate_md5(FILE *file, char* result){
    MD5_CTX mdContext;
    int i, bytes;
    unsigned char data[1024];
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, file)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (result,&mdContext);
    rewind(file);
}

void print_md5(char* result){
	int i = 0;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", result[i]);
    printf("\n");
}