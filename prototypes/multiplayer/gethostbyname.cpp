/* Code written by Hugh Smith	April 2017	*/

/* replacement code for gethostbyname - works for IPv4 and IPV6  */
/* Warning - this is NOT thread safe. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
  
#include "gethostbyname.h"

static char * getIPAddressString46(unsigned char * ipAddress, int addressFamily);
static unsigned char * getIPAddress46(const char * hostName, struct sockaddr_storage * aSockaddr, int addressFamily);
 
 void printIPInfo(struct sockaddr_in6 * ipAddressStruct)
{
	// Prints out IP address and Port number
	
	char * ipString = ipAddressToString(ipAddressStruct);
	
	printf("IP: %s Port: %d\n", ipString, ntohs(ipAddressStruct->sin6_port));
	
}

char * ipAddressToString(struct sockaddr_in6 * ipAddressStruct)
{
	// puts IP address into a printable format
	
	static char ipString[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &ipAddressStruct->sin6_addr, ipString, sizeof(ipString));
	
	return ipString;
}
 
 
unsigned char * gethostbyname4(const char * hostName, struct sockaddr_in * aSockaddr)
{
	// returns ipv address and fills in the aSockaddr with address (unless its NULL)
	struct sockaddr_in * aSockaddrPtr = aSockaddr;
	struct sockaddr_in aSockaddrTemp;
	
	// if user does not care about the struct make a temp one
	if (aSockaddr == NULL)
	{
		aSockaddrPtr = &aSockaddrTemp;
	}
		
	return(getIPAddress46(hostName, (struct sockaddr_storage *) aSockaddrPtr, AF_INET));
}
 
unsigned char * gethostbyname6(const char * hostName, struct sockaddr_in6 * aSockaddr6)
{
	// returns ipv6 address and fills in the aSockaddr6 with address (unless its NULL)
	struct sockaddr_in6 * aSockaddr6Ptr = aSockaddr6;
	struct sockaddr_in6 aSockaddr6Temp;
	
	// if user does not care about the struct make a temp one
	if (aSockaddr6 == NULL)
	{
		aSockaddr6Ptr = &aSockaddr6Temp;
	}
		
	return(getIPAddress46(hostName, (struct sockaddr_storage *) aSockaddr6Ptr, AF_INET6));
}


char * getIPAddressString4(unsigned char * ipAddress)
{
	return getIPAddressString46(ipAddress, AF_INET);
}

char * getIPAddressString6(unsigned char * ipAddress)
{
	return getIPAddressString46(ipAddress, AF_INET6);
}


static char * getIPAddressString46(unsigned char * ipAddress, int addressFamily)
{
	// makes it easy to print the IP address (v4 or v6)
	static char ipString[INET6_ADDRSTRLEN];

	if (ipAddress != NULL)
	{
		inet_ntop(addressFamily, ipAddress, ipString, sizeof(ipString));			
	}
	else
	{
		strcpy(ipString, "(IP not found)");
	}
	
	return ipString;
}

static unsigned char * getIPAddress46(const char * hostName, struct sockaddr_storage * aSockaddr, int addressFamily) 
{
	// Puts host IPv6 (or mapped IPV) into the aSockaddr6 struct and return pointer to 16 byte address (NULL on error)
	// Only pulls the first IP address from the list of possible addresses
	
	static unsigned char ipAddress[INET6_ADDRSTRLEN];
	
	uint8_t * returnValue = NULL;
	int addrError = 0;
	struct addrinfo hints;	
	struct addrinfo *hostInfo = NULL;

	memset(&hints,0,sizeof(hints));
	if (addressFamily == AF_INET)
	{
		hints.ai_family = AF_INET;
	}
	else
	{
		hints.ai_flags = AI_V4MAPPED | AI_ALL;
		hints.ai_family = AF_INET6;
	}
	
	if ((addrError = getaddrinfo(hostName, NULL, &hints, &hostInfo)) != 0)
	{
		fprintf(stderr, "Error getaddrinfo (host: %s): %s\n", hostName, gai_strerror(addrError));
		returnValue = NULL;
	}
	else 
	{
		if (addressFamily == AF_INET)
		{
			memcpy(&((struct sockaddr_in *)aSockaddr)->sin_addr.s_addr, &((struct sockaddr_in*)hostInfo->ai_addr)->sin_addr.s_addr, 4);
			memcpy(ipAddress, &((struct sockaddr_in *)aSockaddr)->sin_addr.s_addr, 4); 
		}
		else
		{
			memcpy(((struct sockaddr_in6 *)aSockaddr)->sin6_addr.s6_addr, &(((struct sockaddr_in6 *)hostInfo->ai_addr)->sin6_addr.s6_addr), 16);
			memcpy(ipAddress, &((struct sockaddr_in6 *)aSockaddr)->sin6_addr.s6_addr, 16); 
		}
		
		returnValue = ipAddress;
		freeaddrinfo(hostInfo);
	}

  return returnValue;    // Either Null or IP address
}

