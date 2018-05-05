// client code for UDP socket programming
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


#define PACKET_SIZE 1024
#define PACKET_DATA_SIZE 1014
#define BUFF_SIZE PACKET_SIZE

#define ACK_CODE 1
#define NEG_ACK_CODE 2
#define FILE_REQUEST_CODE 4
#define FILE_CHUNK_CODE 8
#define FILE_EOF_CODE 16
#define SYN_CODE 32 //unused right now


// #define cipherKey 'S'
#define sendrecvflag 0



typedef struct{
    u_int32_t seq; //4 bytes
	u_int32_t filesize; //4 bytes
    u_int16_t flags; //2bytes
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
    // printf("Generated md5:\n");
    // for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", result[i]);
    // printf("\n");
    rewind(file);
}

void print_md5(char* result){
	int i = 0;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", result[i]);
    printf("\n");
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
	
    memset(&send_buffer,0,BUFF_SIZE);
	memcpy(send_buffer,&packet.seq,4);
	memcpy(send_buffer+4,&packet.filesize,4);
	memcpy(send_buffer+8,&packet.flags,2);
	memcpy(send_buffer+10,&packet.data,PACKET_DATA_SIZE);

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
	memset(&packet->data,'\0',PACKET_DATA_SIZE);
    memcpy(&packet->data,receive_buffer+10,PACKET_DATA_SIZE);
    printf("Packet struct from received buffer:\nseq: %d flags: %d\n",packet->seq,packet->flags);	
}


int containsFlag(int packet_flags_field, int flag_code){
	return (packet_flags_field & flag_code) == flag_code;
}
int addFlag(int packet_flags_field, int flag_code){
	return (packet_flags_field | flag_code);
}

void printPacketFlags(int packet_flags_field){

}

void clearBuf(char* b)
{
	memset(&b,'\0',BUFF_SIZE);
}

int writePacketData(FILE* file, int file_size, packet_t packet){
	if(file == NULL){
		printf("NULL FILE\n");
		return -1;
	}
	int write_position = (packet.seq - 1)*PACKET_DATA_SIZE;
	
	int num_bytes = (file_size > (packet.seq*PACKET_DATA_SIZE)) ? PACKET_DATA_SIZE : (PACKET_DATA_SIZE - (packet.seq*PACKET_DATA_SIZE - file_size));
	printf("Write start: %d, bytes: %d, file size: %d\n", write_position,num_bytes,file_size);
	if(write_position > 0)
		fseek(file,write_position,SEEK_SET);
	
	
	if(fwrite(packet.data,sizeof(char),num_bytes, file) < 0){
		perror("writePacketData");
		return -1;
	}
	return 0;

}


// driver code
int main(int argc, char const *argv[])
{
	int sockfd, nBytes;
	struct sockaddr_in addr_con;
	int addrlen = sizeof(addr_con);
	const char* ip_address;
	int port_number;
	char net_buf[BUFF_SIZE];
	char name_buf[PACKET_DATA_SIZE];
	FILE* fp;
	packet_t recv_packet, send_packet;
	int last_seq_client = 0;
	int last_seq_server = 0;
	int file_size = 0;
	int packets_needed = 0;
	int packet_count = 0;
	char received_md5[MD5_DIGEST_LENGTH];
	char calculated_md5[MD5_DIGEST_LENGTH];

	if(argc < 3){
      printf("Error need exactly 2 arguments. Arg1: Server-IP Arg2: Port#\n");
        exit(0);
    }else{
        ip_address = argv[1];
        port_number = atoi(argv[2]);
    }
	addr_con.sin_family = AF_INET;
	addr_con.sin_port = htons(port_number);
	addr_con.sin_addr.s_addr = inet_addr(ip_address);



	// socket()
	sockfd = socket(AF_INET, SOCK_DGRAM,
					IP_PROTOCOL);

	if (sockfd < 0)
		printf("\nfile descriptor not received!!\n");
	else
		printf("\nfile descriptor %d received\n", sockfd);

	

	//receive filename
	while (1) {
		memset(&name_buf,'\0',PACKET_DATA_SIZE);
		printf("\nPlease enter file name to receive:\n");
		scanf("%s", name_buf);
		if(strcmp(name_buf,"exit") == 0 || strcmp(name_buf,"quit") == 0){
			break;
		}
		//build send packet
		//clear packets
		memset(&recv_packet,0,sizeof recv_packet);
		memset(&send_packet,0,sizeof send_packet);
		send_packet.seq = 1;
		send_packet.filesize = 0;
		
		send_packet.flags = FILE_REQUEST_CODE;
		memcpy(&send_packet.data,name_buf,PACKET_DATA_SIZE);

		if(sendPacketStruct(send_packet,sockfd,addr_con) != 0){
			continue;
		}
		
		//receive packets, could be neg ack or ack with filesize/checksum or file chunk, or file chuck with EOF
		while (1) {
			printf("\n---------Waiting for data---------\n");
			
			memset(&net_buf,'\0',sizeof net_buf);
			memset(&recv_packet,'\0',sizeof recv_packet);
			memset(&send_packet,'\0',sizeof send_packet);
			
			nBytes = recvfrom(sockfd, net_buf, BUFF_SIZE,
							sendrecvflag, (struct sockaddr*)&addr_con,
							&addrlen);
			recievePacketStruct(net_buf,&recv_packet);
			
			printf("PACKET seq:%i flags:%i size:%zu\n",recv_packet.seq,recv_packet.flags, sizeof net_buf);
			if(containsFlag(recv_packet.flags, NEG_ACK_CODE)){
				printf("NEG ACK received, file not found\n");
				break;
			}else if(containsFlag(recv_packet.flags, ACK_CODE)){
				printf("FLAG: ACK\n");
				fp = fopen("out_file.jpg","wb");
				file_size = recv_packet.filesize;
				packets_needed = ((int) (file_size / PACKET_DATA_SIZE)) + 1;
				memcpy(&received_md5, recv_packet.data, MD5_DIGEST_LENGTH);
				packet_count++;
			}else if(containsFlag(recv_packet.flags, FILE_CHUNK_CODE)){
				//if the file size has not been recorded do so now. This is the case if this is the first packet to arrive
				if(file_size == 0){
					fp = fopen("out_file.jpg","wb");
					file_size = recv_packet.filesize;
					packets_needed = ((int) (file_size / PACKET_DATA_SIZE)) + 1 + 1; //extra plus one for checksum packet
					printf("file size: %d packets needed: %d\n", file_size, packets_needed);
				}
				printf("FLAG: FILE CHUNK\n");
				if(containsFlag(recv_packet.flags, FILE_EOF_CODE)){
				printf("FLAG: FILE EOF\n");
				}
				writePacketData(fp,file_size,recv_packet);
				//all packets have been received
				if(packet_count == packets_needed){
					fclose(fp);
					fp = fopen("out_file.jpg","rb");
					generate_md5(fp,calculated_md5);
					print_md5(received_md5);
					print_md5(calculated_md5);
					memset(&send_packet,'\0',sizeof send_packet);
					//send ACK code if checksums match send neg_ack if they do not, indicating if the file is corrupted or not
					if(strncmp(calculated_md5, received_md5, MD5_DIGEST_LENGTH) == 0){
						send_packet.flags = ACK_CODE;
						sendPacketStruct(send_packet,sockfd,addr_con);
						printf("Checksums match\n");
					}else{
						send_packet.flags = NEG_ACK_CODE;
						sendPacketStruct(send_packet,sockfd,addr_con);
						printf("Checksum mismatch\n");
					}
					fclose(fp);
					break;
				 }
				 packet_count++;
			}else{
				printf("FLAG: OTHER\n");
			}
			
		}
		printf("debug outer loop\n");
		
		memset(&net_buf,'\0',sizeof net_buf);
		printf("\n-------------------------------\n");
	}
	close(sockfd);
	
	return 0;
}
