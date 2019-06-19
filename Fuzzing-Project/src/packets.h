#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define PCKT_LEN 8192

struct udpheader {
  unsigned short int udph_srcport;

  unsigned short int udph_destport;

  unsigned short int udph_len;
  
  unsigned short int udph_chksum;
};/* total udp header length: 8 bytes (= 64 bits) */

struct tcpheader {

  unsigned short int tcph_srcport;

  unsigned short int tcph_destport;

  unsigned int tcph_seqnum;

  unsigned int tcph_acknum;

  unsigned char tcph_reserved:4, tcph_offset:4;

  unsigned char tcph_flags;

  unsigned short int tcph_win;

  unsigned short int tcph_chksum;

  unsigned short int tcph_urgptr;
}; /* total tcp header length: 20 bytes (= 160 bits) */

struct ipheader {
  unsigned char iph_ihl:5, iph_ver:4;

  unsigned char iph_tos;
  
  unsigned short int iph_len;

  unsigned short int iph_ident;

  unsigned char iph_flag;
  
  unsigned short int iph_offset;

  unsigned char iph_ttl;

  unsigned char iph_protocol;

  unsigned short int iph_chksum;

  unsigned int iph_src;

  unsigned int iph_dest;
  
};

unsigned short csum(unsigned short *buffer, int nwords) {
  unsigned long sum;
  for(sum = 0; nwords>0; nwords--)
    sum += *buffer++;
  sum = (sum >> 16) + (sum &0xffff);
  sum += (sum >> 16);
  return (unsigned short)(~sum);  
} 

  

