#include "coap.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

coap_context_t *get_context(const char *node, const char *port)
{
	coap_context_t *ctx = NULL;
	int s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV | AI_ALL;

	s = getaddrinfo(node, port, &hints, &result);
	if ( s != 0 )
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return NULL;
	}

	/* iterate through results until success */
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		coap_address_t addr;

		if (rp->ai_addrlen <= sizeof(addr.addr))
		{
			coap_address_init(&addr);
			addr.size = rp->ai_addrlen;
			memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

			ctx = coap_new_context(&addr);
			if (ctx)
			{
				/* TODO: output address:port for successful binding */
				goto finish;
			}
		}
	}

	fprintf(stderr, "no context available for interface '%s'\n", node);

	finish:
	freeaddrinfo(result);
	return ctx;
}

coap_context_t  *getClientCoapContext(int p_netType, coap_response_handler_t p_handler)
{
	char port_str[NI_MAXSERV] = "0";
	coap_context_t* l_context = NULL;
	switch(p_netType)
	{
	case AF_INET:
		l_context = get_context("0.0.0.0", port_str);
		break;
	case AF_INET6:
		l_context = get_context("::", port_str);
		break;
	}
	if (l_context != NULL)
	{
		coap_register_option(l_context, COAP_OPTION_BLOCK2);
		coap_register_response_handler(l_context, p_handler);
	}
	return l_context;
}
