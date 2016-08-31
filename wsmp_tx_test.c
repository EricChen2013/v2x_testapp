#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/if_ether.h>
#define ETH_P_WSMP 0x88DC /* WAVE Short Message Protocol */

void showerr()
{
    char buf[4096];
    int code = errno;
    strerror_r(errno, buf, sizeof(buf));
    printf("result: %s(%d) \n", buf, code);
}

int main()
{
	int ret;
	char buf[2048];
	int sockfd;
	struct ether_header *eh;
	char payload[] = { 
		0x00, 64, //frame length
		0xaa, 0xaa, 0x03, //LLC
		0x00, 0x00, 0x00, 0x88, 0xdc, //snap
		0x02, //version
		0x0a, //psid
		0x0f, 0x01, 0xac, //channel: 172
		0x10, 0x01, 0x0c, //data rate
		0x04, 0x01, 0x0f, //tx power
		0x80, //wave element  id: wsmp
		0x00, 0x03, //wsm length
		0x01,0x02, 0x03
	};
	char src_mac[ETH_ALEN] = {0x01,0x01,0x01,0x01,0x01,0x01};
	char dst_mac[ETH_ALEN] = {0x02,0x02,0x02,0x02,0x02,0x02};
	struct sockaddr_ll socket_address;
	int ifindex = 0; //any
	int tx_len = 0;
	struct ifreq ifidx;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_WSMP));
	showerr();
	
	memset(&ifidx, 0, sizeof(ifidx) );
	strcpy(ifidx.ifr_name, "wlan0" );
	ret = ioctl(sockfd, SIOCGIFINDEX, &ifidx);
	showerr();

	/* Construct Ethernet Header */
	eh = (struct ether_header *)buf; 
	eh->ether_type = htons(ETH_P_WSMP); 
	memcpy(eh->ether_shost, src_mac, ETH_ALEN); 
	memcpy(eh->ether_dhost, dst_mac, ETH_ALEN); 
	tx_len = 12;
	//set payload
	memcpy( &eh->ether_type, payload, sizeof(payload));
	tx_len += sizeof(payload);
	
	/* Construct sockaddr_ll */ 
	socket_address.sll_ifindex = ifidx.ifr_ifindex; 
	socket_address.sll_halen = ETH_ALEN; 
	memset(socket_address.sll_addr, 0xFF, ETH_ALEN); 

	/* File WSMP packet as payload */
	ret = sendto(sockfd, buf, tx_len, 0,  (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll) );
	showerr();
}
