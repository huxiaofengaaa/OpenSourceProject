#include "coap.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

int resolveDstAddress(str* p_host, int p_port, coap_address_t* p_dstAddr)
{
	if(p_dstAddr == NULL)
	{
		return -1;
	}

	char l_addrstr[256] = { 0 };
	if (p_host != NULL && p_host->length > 0)
	{
		memcpy(l_addrstr, p_host->s, p_host->length);
	}
	else
	{
		memcpy(l_addrstr, "localhost", 9);
	}

	struct addrinfo *l_res = NULL;
	struct addrinfo l_hints;
	memset(&l_hints, 0, sizeof(struct addrinfo));
	l_hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	l_hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */

	int l_error = getaddrinfo(l_addrstr, "", &l_hints, &l_res);
	if(l_error != 0)
	{
		fprintf(stderr, "getaddrinfo: %d, %s, %s\n", l_error, gai_strerror(l_error), l_addrstr);
		return -1;
	}
	p_dstAddr->addr.sin.sin_port = htons(p_port);

	struct addrinfo *l_ainfo = NULL;
	for (l_ainfo = l_res; l_ainfo != NULL; l_ainfo = l_ainfo->ai_next)
	{
		switch (l_ainfo->ai_family)
		{
		case AF_INET6:
		{
			memcpy((void*)&(p_dstAddr->addr.sa), l_ainfo->ai_addr, l_ainfo->ai_addrlen);
			p_dstAddr->size = l_ainfo->ai_addrlen;
			p_dstAddr->addr.sin.sin_port = htons(p_port);
			break;
		}
		case AF_INET:
		{
			memcpy((void*)&(p_dstAddr->addr.sa), l_ainfo->ai_addr, l_ainfo->ai_addrlen);
			p_dstAddr->size = l_ainfo->ai_addrlen;
			p_dstAddr->addr.sin6.sin6_port = htons(p_port);
			break;
		}
		default:
			printf("unknow type %d \n", l_ainfo->ai_family);
		}
	}
	freeaddrinfo(l_res);
	return 0;
}

void showDstAddr(coap_address_t* p_dstAddr)
{
	if(LOG_DEBUG <= coap_get_log_level())
	{
		if(p_dstAddr != NULL)
		{
			char l_tmpAddr[INET6_ADDRSTRLEN] = { 0 };
			void* l_addrptr = NULL;
			printf("p_dstAddr size = %d \n", p_dstAddr->size);
			switch(p_dstAddr->addr.sa.sa_family)
			{
			case AF_INET:
				l_addrptr = &(p_dstAddr->addr.sin.sin_addr);
				printf("p_dstAddr type = %s \n", "AF_INET");
				printf("p_dstAddr port = %d \n", ntohs(p_dstAddr->addr.sin.sin_port));
				break;
			case AF_INET6:
				l_addrptr = &(p_dstAddr->addr.sin6.sin6_addr);
				printf("p_dstAddr port = %d \n", ntohs(p_dstAddr->addr.sin6.sin6_port));
				printf("p_dstAddr type = %s \n", "AF_INET6");
				break;
			}
			inet_ntop(p_dstAddr->addr.sa.sa_family, l_addrptr, l_tmpAddr, sizeof(l_tmpAddr));
			printf("p_dstAddr addr = %s \n", l_tmpAddr);
		}
		printf("\n");
	}
}

int getAddressString(struct sockaddr* p_sa, char* p_buffer, int p_bufferSize, int* p_port)
{
	if(p_sa == NULL || p_buffer == NULL || p_port == NULL)
	{
		return -1;
	}

	switch(p_sa->sa_family)
	{
	case AF_INET:
	{
		if(NULL == inet_ntop(AF_INET, &(((struct sockaddr_in*)p_sa)->sin_addr), p_buffer, p_bufferSize))
		{
			return -1;
		}
		*p_port = ntohs(((struct sockaddr_in*)p_sa)->sin_port);
		break;
	}
	case AF_INET6:
	{
		if(NULL == inet_ntop(AF_INET6, &(((struct sockaddr_in6*)p_sa)->sin6_addr), p_buffer, p_bufferSize))
		{
			return -1;
		}
		*p_port = ntohs(((struct sockaddr_in6*)p_sa)->sin6_port);
		break;
	}
	default:
		return -1;
	}
	return 0;
}

void showLocalAddress(int p_socket, int p_netType)
{
	if(LOG_DEBUG <= coap_get_log_level())
	{
		char l_host[32] = { 0 };
		int l_port = 0;

		struct sockaddr l_localAddr;
		socklen_t l_localAddrLength = sizeof(struct sockaddr);
		if(0 != getsockname(p_socket, (struct sockaddr*)&l_localAddr, &l_localAddrLength))
		{
			printf("showLocalAddress - getsockname failed\n");
		}
		else
		{
			if(0 == getAddressString((struct sockaddr*)&l_localAddr, l_host, sizeof(l_host), &l_port))
			{
				printf("local address %s:%d\n", l_host, l_port);
			}
		}
		printf("\n");
	}
}
