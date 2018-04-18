// server code for UDP socket programming
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/md5.h>

#define IP_PROTOCOL 0
#define PORT_NO 15050
#define PACKET_SIZE 1024
#define PACKET_DATA_SIZE 1014
#define BUFF_SIZE PACKET_SIZE

#define ACK_CODE 1
#define NEG_ACK_CODE 2
#define FILE_REQUEST_CODE 4
#define FILE_CHUNK_CODE 8
#define FILE_EOF_CODE 16 //Applied on last file chunk
#define SYN_CODE 32

#define cipherKey 'S'
#define sendrecvflag 0
#define nofile "File Not Found!"


typedef struct{
    u_int32_t seq; //4 bytes
	u_int32_t filesize; //4 bytes
    u_int16_t flags; //2 bytes
    char data[PACKET_DATA_SIZE];
}packet_t;

void generate_md5(FILE *file, char* result){
    MD5_CTX mdContext;
    int i, bytes;
    char data[1024];
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, file)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (result,&mdContext);
    printf("Generated md5:\n");
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", result[i]);
    printf("\n");
    rewind(file);
}

int sendallTo(int fd, struct sockaddr_in addr_con, char *buf, int *len)
{
	int addrlen = sizeof(addr_con);
	int total = 0;
	// how many bytes we've sent
	int bytesleft = *len; // how many we have left to send
	int n;
	while(total < *len) {
		n = sendto(fd,buf+total,bytesleft,0,
		(struct sockaddr*)&addr_con, addrlen);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	*len = total; // return number actually sent here
	return n==-1?-1:0; // return -1 on failure, 0 on success
}

int sendPacketStruct(packet_t packet, int fd, struct sockaddr_in addr_con){
    char send_buffer[BUFF_SIZE];
    // memset(&send_buffer,'\0',BUFF_SIZE);
	
	memcpy(send_buffer,&packet.seq,4);
	memcpy(send_buffer+4,&packet.filesize,4);
	memcpy(send_buffer+8,&packet.flags,2);
	memcpy(send_buffer+10,&packet.data,PACKET_DATA_SIZE);
	
    // send_buffer[BUFF_SIZE] = '\0';
    // printf("Preparing to send packet..\n");
    
	int len = BUFF_SIZE;
	int result = -1;
	if((result = sendallTo(fd, addr_con, send_buffer,&len)) != 0){
		perror("sendallTo");
		printf("Failed to send packet\n");
	}
	return result;

    
}
void recievePacketStruct(char receive_buffer[BUFF_SIZE], packet_t *packet){
	memcpy(&packet->seq,receive_buffer+0,4);
	memcpy(&packet->filesize,receive_buffer+4,4);
	memcpy(&packet->flags,receive_buffer+8,2);
	memset(packet+10,'\0',PACKET_DATA_SIZE);
    memcpy(&packet->data,receive_buffer+10,PACKET_DATA_SIZE);
}


int containsFlag(int packet_flags_field, int flag_code){
	return (packet_flags_field & flag_code) == flag_code;
}
int addFlag(int packet_flags_field, int flag_code){
	return (packet_flags_field | flag_code);
}

// funtion sending file
int sendFile(FILE* fp, int file_size, int fd, struct sockaddr_in addr_con)
{	
	
	int seq_number = 1;
	char file_read_buffer[PACKET_DATA_SIZE];
	
	int bytes_read = 0;
	int total_bytes_read = 0;
	packet_t send_packet = {};
	// memset(&send_packet,'\0',sizeof send_packet);
	
	while(total_bytes_read < file_size){
		bytes_read = fread(file_read_buffer,sizeof(char),PACKET_DATA_SIZE,fp);
		if(bytes_read < 0){
			printf("Error or EOF of file reached\n");
			break;
		}
		//ADD DATA
		memcpy(&send_packet.data,file_read_buffer,sizeof send_packet.data);
		//ADD FLAGS
		if(bytes_read < PACKET_DATA_SIZE){
			//sending last file chunk, set flags
		send_packet.flags = addFlag(send_packet.flags,FILE_EOF_CODE);	
		}
		send_packet.flags = addFlag(send_packet.flags,FILE_CHUNK_CODE);
		
		send_packet.seq = seq_number;
		sendPacketStruct(send_packet,fd,addr_con);
		total_bytes_read += bytes_read;
		seq_number += 1;
	}
	return 0;
}

void exitOnError(char* error){
		printf("%s\n",error);
		exit(0);
	}
	
// driver code
int main()
{
	int sockfd, nBytes;
	struct sockaddr_in addr_con;
	int addrlen = sizeof(addr_con);
	addr_con.sin_family = AF_INET;
	addr_con.sin_port = htons(PORT_NO);
	addr_con.sin_addr.s_addr = INADDR_ANY;
	char net_buf[BUFF_SIZE];
	FILE* fp;
	int last_seq_server = 0;
	int last_seq_client= 0;
	packet_t recv_packet = {};
	packet_t send_packet = {};
	char md5_result[MD5_DIGEST_LENGTH];
	// socket()
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);

	if (sockfd < 0)
		printf("\nfile descriptor not received!!\n");
	else
		printf("\nfile descriptor %d received\n", sockfd);

	// bind()
	if (bind(sockfd, (struct sockaddr*)&addr_con, sizeof(addr_con)) == 0)
		printf("\nSuccessfully binded!\n");
	else
		printf("\nBinding Failed!\n");

	//main loop
	
	while (1) {

		int received_filename = 0;
		
		while(!received_filename){
			// receive file name
			printf("\nWaiting for client to send filename...\n");
			memset(&net_buf,'\0',sizeof net_buf);
			memset(&recv_packet,'\0',sizeof recv_packet);
			nBytes = recvfrom(sockfd, net_buf,
							BUFF_SIZE, sendrecvflag,
							(struct sockaddr*)&addr_con, &addrlen);
	
			if(strcmp(net_buf,"exit") == 0 || strcmp(net_buf,"quit") == 0){
				break;
			}
			
			recievePacketStruct(net_buf,&recv_packet);
			printf("test\n");
			
			if(containsFlag(recv_packet.flags,FILE_REQUEST_CODE)){
				fp = fopen(recv_packet.data, "rb");
				printf("\nFile Name Received: %s\n", recv_packet.data);
				if (fp == NULL){
					//send negative ack
					send_packet.seq = 1;
					send_packet.flags = NEG_ACK_CODE;
					printf("\nFile open failed!\n");
					sendPacketStruct(send_packet,sockfd,addr_con); //TODO handle return code -1 for fail
					
				}else{
					printf("\nFile Successfully opened!\n");
					//send ack with filesize and checksum
					send_packet.seq = 1;
					send_packet.flags = ACK_CODE;
					//file size
					fseek(fp, 0, SEEK_END);
					int size = ftell(fp);
					rewind(fp);
					send_packet.filesize = size;
					generate_md5(fp, md5_result);
					memcpy(&send_packet.data, md5_result,PACKET_DATA_SIZE);
					sendPacketStruct(send_packet,sockfd,addr_con);

					sendFile(fp,size,sockfd,addr_con);
					printf("\nWaiting for client to check if the file is corrupted or not...\n");
					memset(&recv_packet,'\0',sizeof recv_packet);
					nBytes = recvfrom(sockfd, net_buf,
									BUFF_SIZE, sendrecvflag,
									(struct sockaddr*)&addr_con, &addrlen);
						}
					recievePacketStruct(net_buf,&recv_packet);
					if(containsFlag(recv_packet.flags, ACK_CODE)){
						printf("Checksums match, file sent successfully\n");
					}else if(containsFlag(recv_packet.flags, NEG_ACK_CODE)){
						printf("Checksums do not match, file corrupted on send\n");
					}else{
						printf("Received message other than ACK or NEG_ACK\n");
					}

			}else{
				printf("Received message but was not for file request\n");
				printf("Recieve code: %d", recv_packet.flags);
			}
			
		}
		
		if (fp != NULL)
			fclose(fp);
	}
	return 0;
}
