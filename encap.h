#ifndef ENCAP_H
#define ENCAP_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <limits.h>

#define HEADER_BUFF_SIZE 4
#define DATA_BUFF_SIZE 1020
#define BUFF_SIZE HEADER_BUFF_SIZE+DATA_BUFF_SIZE

struct header {
    uint16_t seq;
    uint16_t flags;
};

struct data {
    unsigned char* packet_data;
};

void htoHeader(struct header h, unsigned char buffer[HEADER_BUFF_SIZE]);
void htonData(struct data d, unsigned char buffer[DATA_BUFF_SIZE]);

void htoHeaderData(struct header h, struct data d, unsigned char buffer[BUFF_SIZE]);
void ntoHeaderData(unsigned char buffer[BUFF_SIZE], struct header *h, struct data *d);

#endif