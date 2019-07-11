#include "packets.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]) {
    int sd;

    //just datagram
    char buffer[PCKT_LEN];

    //custom headers
    struct ipheader *ip = (struct ipheader * ) buffer;
    struct udpheader *udp = (struct udpheader *)(buffer+sizeof(struct ipheader));
    
    //src/dest
    struct  sockaddr_in sin, din;
    int one = 1;
    const int *val = &one;
    
    memset(buffer, 0, PCKT_LEN);
    for(int i=0; i<strlen(argv[5]); i++) {
        buffer[i] = argv[5][i];
    }
    printf("The buffer is %s\n", buffer);



    if(argc < 5) {
        printf("- Invalid parameters!\n");
        printf("- Usage %s <source hostname/IP> <source port> <target hostname/IP> <target port> <data>\n", argv[0]);
        exit(-1);
    }
    
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if(sd < 0) {
        perror("socket() error");
        exit(-1);
    } else {
        printf("socket() Using SOCK_RAW socket & UDP protocol is OK.\n");
    }

    sin.sin_family = AF_INET;
    din.sin_family = AF_INET;

    sin.sin_port = htons(atoi(argv[2]));
    din.sin_port = htons(atoi(argv[4]));

    sin.sin_addr.s_addr = inet_addr(argv[1]);
    din.sin_addr.s_addr = inet_addr(argv[3]);

    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip-> iph_len = sizeof(struct ipheader) + sizeof(struct udpheader);
    ip->iph_ident = htons(54321);
    ip->iph_ttl = 64; //hops
    ip->iph_protocol = 17; //UDP
    //Set source ip to spoof (for later)
    ip->iph_src = inet_addr(argv[1]);
    ip->iph_dest = inet_addr(argv[3]);

    udp->udph_srcport = htons(atoi(argv[2]));
    udp->udph_destport = htons(atoi(argv[4]));
    udp->udph_len = htons(sizeof(struct udpheader));
    
    ip->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));

    //Don't fill up packet structure, build by us

    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt() error");
        exit(-1);
    } else {
        printf("setsockopt() is OK.\n");
    }

    //Send loop

    printf("Using Source IP: %s port: %u, Target IP: %s port: %u.\n", argv[1], atoi(argv[2]), argv[3], atoi(argv[4]));

    int cnt;
    for(cnt = 1; cnt <= 20; cnt++) {
        if(sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            perror("sendto() error");
            exit(-1);
        } else {
            printf("Count #%u - sendto() is OK.\n", cnt);
            sleep(2);
        }
        close(sd);
        return 0;
    }
}
