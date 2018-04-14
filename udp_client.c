// Client side C/C++ program to demonstrate UDP Socket programming 
/*
** client.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include "encap.h"

// //DATA ENCAPSULATION METHODS
// //should probably be in own header file
// #define HEADER_BUFF_SIZE 4
// #define DATA_BUFF_SIZE 1020

// const int BUFF_SIZE = HEADER_BUFF_SIZE + DATA_BUFF_SIZE;

// struct header {
//     uint16_t seq;
//     uint16_t flags;
// };

// struct data {
//     unsigned char* packet_data;
// };

// void htoHeader(struct header h, unsigned char buffer[HEADER_BUFF_SIZE]){

//     unsigned char high = (unsigned char)(h.seq>>8);
//     unsigned char low  = h.seq & 0xff;
//     memcpy(buffer+0,&high, 1);
//     memcpy(buffer+1,&low, 1);

//     high = (unsigned char)(h.flags>>8);
//     low  = h.flags & 0xff;    
//     memcpy(buffer+2,&high, 1);
//     memcpy(buffer+3,&low, 1);
// }
// void htonData(struct data d, unsigned char buffer[DATA_BUFF_SIZE]) {
//     memcpy(buffer, d.packet_data, DATA_BUFF_SIZE);
//     // printf("data:\n%s\n",buffer);
// }

// void htoHeaderData(struct header h, struct data d, unsigned char buffer[BUFF_SIZE]){
//     htoHeader(h,buffer+0);
//     htonData(d,buffer+HEADER_BUFF_SIZE);
// }

// void ntoHeaderData(unsigned char buffer[BUFF_SIZE], int *seq, int *flags, char data_buffer[DATA_BUFF_SIZE]){
//     *seq = buffer[1] | (buffer[0]<<8);
//     *flags = buffer[3]| (buffer[2]<<8);
//     memcpy(data_buffer,buffer+HEADER_BUFF_SIZE,DATA_BUFF_SIZE);

// }
// //DATA ENCAPSULATION METHODS END

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char* my_addr;
    struct sockaddr_storage their_addr;
    char* server_addr;
    socklen_t addr_len;
    // struct sockaddr_storage their_addr;
    char* PORT;
    char* filename;
    if (argc != 4){
        fprintf(stderr,"usage: client hostname message\n");
        exit(1);
    }else{
        PORT = argv[1];
        server_addr = argv[2];
        filename = argv[3];
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 

    //arg1 my address arg2 server address
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return 2;
    }

    

    //SETUP FINISHED ------------------------------

    //REQUEST FILE BY FILENAME
    char buffer[BUFF_SIZE];
    struct header h;
    struct data d;
    h.seq = 1;
    h.flags = 1;
    d.packet_data = filename;
    
    htoHeaderData(h,d,buffer);

    //testing my own send data
    // int seq = 0;
    // int flags = 0;
    // char data_buffer[DATA_BUFF_SIZE];
    // ntoHeaderData(buffer,&seq,&flags,data_buffer);
    // printf("test: seq: %d flags: %d data: %s",seq,flags, data_buffer);


    //send back to client
    printf("%s\n",buffer);
    if ((numbytes = sendto(sockfd, buffer, BUFF_SIZE, 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto - request filename");
        exit(1);
    }

    //WAIT FOR RESPONSE EITHER SIZE OR NEGATIVE ACK
    memset(&buffer,'\0',BUFF_SIZE);
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buffer, BUFF_SIZE , 0,
        (struct sockaddr *)&their_addr ,&addr_len )) == -1) {
        perror("recvfrom");
        exit(1);
    }
    memset(&h,0,sizeof h);
    memset(&d,0,sizeof d);
    ntoHeaderData(buffer,&h,&d);
     printf("recieved back: seq: %d flags: %d data: %s",h.seq,h.flags, d.packet_data);
    // buffer[numbytes] = '\0';

    printf("client: sent %d bytes to %s\n", numbytes, server_addr);
    close(sockfd);

    freeaddrinfo(servinfo);
    return 0;

}