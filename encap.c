#include "encap.h"

void htoHeader(struct header h, unsigned char buffer[HEADER_BUFF_SIZE]){

    unsigned char high = (unsigned char)(h.seq>>8);
    unsigned char low  = h.seq & 0xff;
    memcpy(buffer+0,&high, 1);
    memcpy(buffer+1,&low, 1);

    high = (unsigned char)(h.flags>>8);
    low  = h.flags & 0xff;    
    memcpy(buffer+2,&high, 1);
    memcpy(buffer+3,&low, 1);
}
void htonData(struct data d, unsigned char buffer[DATA_BUFF_SIZE]) {
    memcpy(buffer, d.packet_data, DATA_BUFF_SIZE);
}

void htoHeaderData(struct header h, struct data d, unsigned char buffer[BUFF_SIZE]){
    htoHeader(h,buffer+0);
    htonData(d,buffer+HEADER_BUFF_SIZE);
}

void ntoHeaderData(unsigned char buffer[BUFF_SIZE], struct header *h, struct data *d){
    h->seq = buffer[1] | (buffer[0]<<8);
    h->flags = buffer[3]| (buffer[2]<<8);
    d->packet_data=(buffer+HEADER_BUFF_SIZE);
}
