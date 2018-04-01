#include <stdio.h>
#include <strings.h>

int port_number;

enum Protocol{
    TCP,
    UDP
}protocol;

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("usage: %s [tcp/udp] [port]\n",argv[0]);
        exit(1);
    }else{
        if(strcasecmp(argv[1],"tcp")){
            protocol = TCP;
        }else if(strcasecmp(argv[1],"udp")){
            protocol = TCP;
        }else{
            printf("usage: %s [tcp/udp] [port]\n",argv[0]);
            exit(1);
        }

        port_number = atoi(argv[2]);

        printf("Initializing %s server on port %s\n",argv[1],argv[2]);
    }
}