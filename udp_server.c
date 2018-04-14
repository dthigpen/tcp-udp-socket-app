// Server side C/C++ program to demonstrate UDP Socket programming 
/*
** listener.c -- a datagram sockets "server" demo
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
#include "encap.h"

// #define MYPORT "4950"    // the port users will be connecting to

// #define MAXBUFLEN 100


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



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[BUFF_SIZE+1];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char* PORT;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if (argc != 2){
        fprintf(stderr,"Error wrong args. Arg1: Port#\n");
        exit(1);
    }else{
        PORT = argv[1];
    }
    

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
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

    //WAITING TO RECEIVE FILENAME PACKET
    printf("listener: waiting to recvfrom...\n");

    //TODO BUFF_SIZE or BUFF_SIZE-1
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, BUFF_SIZE , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';
    //FILENAME PACKET RECEIVED

    //SPLIT UP PACKET
    struct header h;
    struct data d;
    ntoHeaderData(buf,&h,&d);
    printf("seq: %d flags: %d data: %s\n",h.seq,h.flags, d.packet_data);
    //END SPLIT UP PACKET

    //OPEN FILE TO SEND
    FILE *file;
    file = fopen(d.packet_data,"rb");
  	if(file != NULL){
      	int size = 0;
      	fseek(file, 0, SEEK_END);
      	size = ftell(file);
      	rewind(file);
        printf("FILE FOUND\n");

        //SEND FILE SIZE HERE
        //fill packet
        h.seq = 0;
        h.flags = 1;
        char data_buffer[DATA_BUFF_SIZE];
        memset(&data_buffer,'\0',DATA_BUFF_SIZE);
        sprintf(data_buffer,"%d",size);
        d.packet_data = data_buffer;
        memset(&buf,'\0',BUFF_SIZE+1);
        htoHeaderData(h,d,buf);
        sleep(1);
        //send
        addr_len = sizeof their_addr;
        if ((numbytes = sendto(sockfd, buf, BUFF_SIZE, 0,
            (struct sockaddr *)&their_addr, addr_len)) == -1) {
        perror("server: send size");
        exit(1);
        }
      }else{
          printf("FILE NOT FOUND\n");
          //TODO send neg ack
      }



    // printf("listener: got packet from %s\n",
    //     inet_ntop(their_addr.ss_family,
    //         get_in_addr((struct sockaddr *)&their_addr),
    //         s, sizeof s));
    // printf("listener: packet is %d bytes long\n", numbytes);
   
    fclose(file);
    close(sockfd);

    return 0;
}