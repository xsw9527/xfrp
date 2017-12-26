#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#include "utils.h"
#include <sys/epoll.h>
// s_sleep using select instead of sleep
// s: second, u: usec 10^6usec = 1s
void s_sleep(unsigned int s, unsigned int u)
{
	struct timeval timeout;

	timeout.tv_sec = s;
	timeout.tv_usec = u;
	select(0, NULL, NULL, NULL, &timeout);
}

// is_valid_ip_address:
// return 0:ipaddress unlegal
int is_valid_ip_address(const char *ip_address) 
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_address, &(sa.sin_addr));
	return result;
}
#define IFNAMSIZ 16
#define NI_MAXHOST 
//	net_if_name: name of network interface, e.g. br-lan
//	return: 1: error 0:get succeed

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/if_ether.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <assert.h>

int get_net_mac( char *pEthName, char *pEthMac,  int iEthMacLen)
{
	unsigned char mac[7];
	int sockfd;
	struct ifreq req;
	if ( ( sockfd = socket ( PF_INET,SOCK_DGRAM,0 ) ) ==-1 )
	{
		fprintf ( stderr,"NetGetMAC:Sock Error:%s\n\a",strerror ( errno ) );
		return ( -1 );
	}

	memset ( &req,0,sizeof ( req ) );
	strcpy ( req.ifr_name, pEthName);
	if (ioctl ( sockfd,SIOCGIFHWADDR, ( char * ) &req ) ==-1 )
	{
		fprintf ( stderr,"NetGetMAC ioctl SIOCGIFHWADDR:%s\n\a",strerror ( errno ) );
		close ( sockfd );
		return ( -1 );
	}

	memcpy ( mac,req.ifr_hwaddr.sa_data,6 );
	close ( sockfd );

	snprintf(pEthMac, iEthMacLen, "%02x%02x%02x%02x%02x%02x", 
	mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	return 0;
}


//int get_net_mac(char *net_if_name, char *mac, int mac_len) {
//	strcpy(mac,"001122334455");
//	return 0;
//}


// return: 0: network interface get succeed
int get_net_ifname(char *if_buf, int blen)
{
	if (NULL == if_buf || blen < 8) return -1;

	struct ifaddrs *ifaddr, *ifa;
	int family, n;
	int ret = 1;
	if (getifaddrs(&ifaddr) == -1) {
	   perror("getifaddrs");
	   exit(EXIT_FAILURE);
	}

	int found = 0;
	char tmp_if_buf[16];
	memset(tmp_if_buf, 0, sizeof(tmp_if_buf));
	/* Walk through linked list, maintaining head pointer so we
	  can free list later */
	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
	    if (ifa->ifa_addr == NULL) continue;

	    family = ifa->ifa_addr->sa_family;

		if (family == AF_INET) {
			// for LEDE/OpenWRT embedded router os
			if (strcmp(ifa->ifa_name, "br-lan") == 0) {
				found = 1;
				break;
			}
		} else if (family == AF_PACKET && 
			ifa->ifa_data != NULL && 
			strcmp(ifa->ifa_name, "lo") != 0) { // skip local loop interface
			
			strncpy(tmp_if_buf, ifa->ifa_name, 16);
		}
	}

	if (found) {
		strncpy(if_buf, ifa->ifa_name, blen);
		ret = 0;
	} else if (tmp_if_buf[0] != 0) {
		strncpy(if_buf, tmp_if_buf, blen);
		ret = 0;
	}

	freeifaddrs(ifaddr);
	return ret;
}

// e.g. wWw.Baidu.com/China will be trans into www.baidu.com/China
// return: 0:check and trant succeed, 1:failed or domain name is invalid
int dns_unified(const char *dname, char *udname_buf, int udname_buf_len)
{
	if (! dname || ! udname_buf || udname_buf_len < strlen(dname)+1)
		return 1;
	
	int has_dot = 0;
	int dlen = strlen(dname);
	int i = 0;
	for(i=0; i<dlen; i++) {
		if(dname[i] == '/')
			break;

		if (dname[i] == '.' && i != dlen-1)
			has_dot = 1;

		udname_buf[i] = tolower(dname[i]);
	}

	if (! has_dot)	//domain name should have 1 dot leastly
		return 1;

	return 0;
}