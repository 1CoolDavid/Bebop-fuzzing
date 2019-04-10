#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <unistd.h>
 
#define bool _Bool

struct header {
	u_int16_t f1;
	u_int16_t f2;
	uint32_t f3;
};

struct data {
	uint16_t pf1;
	uint64_t pf2;
};

void htonHeader(struct header h, char buffer[8]) {
	uint16_t u16;
	uint32_t u32;
	u16 = htons(h.f1);
	memcpy(buffer+0, &u16, 2);
	u16 = htons(h.f2);
	memcpy(buffer+2, &u16, 2);
	u32 = htonl(h.f3);
	memcpy(buffer+4, &u32, 4);
}

void htonData(struct data d, char buffer[10]) {
	uint16_t u16;
	uint32_t u32;
	u16 = htons(d.pf1);
	memcpy(buffer+0, &u16, 2);
	u32 = htons(d.pf2>>32);
	memcpy(buffer+2, &u32, 2);
	u32 = htonl(d.pf2);
	memcpy(buffer+6, u32, 4);
}

void htonHeaderData(struct header h, struct data d, char buffer[18]) {
	htonHeader(h, buffer+0);
	htonData(d, buffer+8);
}

void sendPacket(struct header h, struct data d, int sockfd) {
	char buffer[18];
	
	htonHeaderData(h, d, buffer);
	send(sockfd, buffer, 18, 0);
}

int createSocket(int port, bool udp, bool ipv4) {
	int packet_type = udp ? SOCK_DGRAM : SOCK_STREAM;
	int ip_version = ipv4 ? AF_INET : AF_INET6;
	int sockfd = socket(ip_version, packet_type, 0);
	struct sockaddr_in address;  
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0};

	if(sockfd == 0) {
		printf("\nsockfd error\n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(&port);  
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "192.168.42.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed\n"); 
        return -1; 
    } 

	return sockfd;
}

int main() {
	struct header h;
	struct data d;
	int sock = createSocket(1551, true, true);
	if(sock == -1) {
		perror("\nSOCKET ERROR\n");
		exit(1);
	}
	float delay	= 0.5; 
	while(true) {
		sendPacket(h, d, sock);
		sleep(delay); //delays in seconds -> 0.5 = 500 milliseconds	
	}
	return 0;
}