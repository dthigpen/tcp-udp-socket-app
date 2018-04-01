#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

int port_number;

enum Protocol{
    TCP,
    UDP
}protocol;

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("usage: %s [tcp/udp] [port 1024-65535]\n",argv[0]);
        exit(1);
    }else{
        //set protocol type
        if(strcasecmp(argv[1],"tcp")){
            protocol = TCP;
        }else if(strcasecmp(argv[1],"udp")){
            protocol = UDP;
        }else{
            printf("usage: %s [tcp/udp] [port]\n",argv[0]);
            exit(1);
        }
        //set port number
        port_number = atoi(argv[2]);
        if (port_number < 1024 || port_number > 65535){
            printf("Invalid port number. Port range: 1024-65535\n");
            exit(1);
        }
    

        

        printf("Initializing %s server on port %s\n",argv[1],argv[2]);
    }
}