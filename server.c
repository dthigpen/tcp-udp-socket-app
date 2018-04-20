// Server side C/C++ program to demonstrate TCP Socket programming 
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/md5.h>

#define BUFFSIZE 1024
#define ADDR_STR_LEN 50

int sendall(int s,char *buffer, int*len);
void generate_md5(FILE *file, char *result);
void print_md5(char* result);

int main(int argc, char const *argv[])
{
    struct addrinfo hints, *servinfo, *p;
    int server_fd, new_socket, valread, rv;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFSIZE];
    char *connection = "Connection granted to client\n";
    FILE* jpeg;
  	int PORT;
    char* file_md5_buffer;
    char md5_result[MD5_DIGEST_LENGTH];
  	char ack[]="ACK!";
	char nack[] = "NACK";
  	
  	if(argc < 2){
      printf("Error need exactly 1 argument. Arg1: Port Number\n");
      exit(0);
    }
  	
  	PORT = atoi(argv[1]);
  

    // Creating socket file descriptor
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE ; // use my IP, host name

    

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((server_fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_fd);
            perror("listener: bind");
            continue;
        }
        
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
    //SETUP FINISHED
    // Forcefully attaching socket to the port 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    printf("Waiting for a client to connect\n");
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
  	// send connection message
    memset(buffer, '\0',BUFFSIZE);
    send(new_socket , connection , strlen(connection) , 0 );
  	memset(buffer, '\0',BUFFSIZE);
  
  	// read in the name of the file to send
  	valread = read(new_socket,buffer,BUFFSIZE);
  	jpeg = fopen(buffer,"rb+");
  	
  	if(jpeg == NULL){
		// if file does not exist send NACK
		send(new_socket,nack,strlen(nack),0);
	}
  	if(jpeg != NULL){
      	// if file exists send ACK 
		send(new_socket,ack,strlen(ack),0);
      	
      	// if file exists do md5 checksum and send it
		generate_md5(jpeg, md5_result);
        memset(buffer, '\0',BUFFSIZE);
        memcpy(buffer,md5_result,BUFFSIZE);
        send(new_socket,buffer,BUFFSIZE,0);
 
      	int size = 0;
      	int num_bytes_sent = 0;
      	int result = 0;
      	int bytes_sent = 0;
      	int max_bytes_to_send = BUFFSIZE;
      	fseek(jpeg, 0, SEEK_END);
      	size = ftell(jpeg);
      	rewind(jpeg);
        
        //send the size of the file to the client first
        char size_str[16];
        sprintf(size_str,"%d",size);
      	send(new_socket,size_str,sizeof(size_str),0);
      	printf("Sending file size: %s\n",size_str);
        
        printf("Calculated File size: %d \n", size);
        
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
                break;
            }
            //the last section of the file may be smaller than the buffer size
            int send_size = bytes_left < BUFFSIZE ? bytes_read : BUFFSIZE;
            //send all of the data in the buffer so far

            result = sendall(new_socket,buffer,&send_size);
            if(result == -1){
                printf("Failed to send buffered data\n");
                break;
            }else{
                num_bytes_sent += send_size;
                printf("bytes sent: %d / %d\n",num_bytes_sent,size);
            }
        }
        printf("bytes sent: %d / %d\n",num_bytes_sent,size);
        printf("File sent\n");
        //wait for checksum ack or nack
        memset(buffer, '\0',BUFFSIZE);
        valread = recv(new_socket,buffer,5,0);
        if(buffer[0] == 'N'){
            printf("Client reports a checksum mismatch. Send corrupted\n");
        }else{
            printf("Client reports checksums match. Send successful\n");
        }

    }
	
    shutdown(new_socket, 2);
  	fclose(jpeg);  
  	return 0;
}

int sendall(int s, char* buffer, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
	
    while(total < *len) {
        n = send(s, buffer + total , bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
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
