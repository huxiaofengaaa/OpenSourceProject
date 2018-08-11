#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "coap.h"

static int order_opts(void *a, void *b);
static coap_list_t *new_option_node(unsigned short key, unsigned int length, unsigned char *data);

int resolveURLResource(char* p_URL, str** p_host, str** p_path, str** p_query, int* p_port)
{
	coap_uri_t l_coapUri;
	int l_ret = coap_split_uri((unsigned char *)p_URL, strlen(p_URL), &l_coapUri );
	if(l_ret != 0)
	{
		return -1;
	}
	*p_host = coap_new_string(l_coapUri.host.length);
	*p_path = coap_new_string(l_coapUri.path.length);
	*p_query = coap_new_string(l_coapUri.query.length);
	*p_port = l_coapUri.port;

	if(*p_host != NULL)
	{
		(*p_host)->length = l_coapUri.host.length;
		memcpy((*p_host)->s, l_coapUri.host.s, l_coapUri.host.length);
	}
	if(*p_path != NULL)
	{
		(*p_path)->length = l_coapUri.path.length;
		memcpy((*p_path)->s, l_coapUri.path.s, l_coapUri.path.length);
	}
	if(*p_query != NULL)
	{
		(*p_query)->length = l_coapUri.query.length;
		memcpy((*p_query)->s, l_coapUri.query.s, l_coapUri.query.length);
	}
	return 0;
}

void releaseURLResource(str* p_host, str* p_path, str* p_query)
{
	if(p_host != NULL)
	{
		coap_delete_string(p_host);
	}
	if(p_path != NULL)
	{
		coap_delete_string(p_path);
	}
	if(p_query != NULL)
	{
		coap_delete_string(p_query);
	}
}

void showURLResource(char* p_dstUrl, str* p_dstHost, str* p_dstPath, str* p_dstQuery,  int p_dstPort)
{
	if(p_dstUrl){
		printf("p_dstUrl = %s\n", p_dstUrl);
	}
	if(p_dstHost){
		printf("dstHost = %s\n", p_dstHost->s);
	}
	printf("dstPort = %d\n", p_dstPort);
	if(p_dstPath){
		printf("dstPath = %s\n", p_dstPath->s);
	}
	if(p_dstQuery){
		printf("dstQuery = %s\n", p_dstQuery->s);
	}
}

void showOptList(coap_list_t * p_currentOptList)
{
	int l_optlistCounter = 0;
	coap_list_t *l_tmp = p_currentOptList;
	for(l_tmp = p_currentOptList; l_tmp != NULL; l_tmp = l_tmp->next)
	{
		coap_option *l_option = l_tmp->data;
		printf("optlist[%d] = key:%d, length:%d, data:%s\n", l_optlistCounter++,
				l_option->key, l_option->length, ((unsigned char*)l_option + sizeof(coap_option)));
	}
}

void createOptList(int p_port, str* p_host, str* p_path, str* p_query, coap_list_t ** p_saveOptList)
{
	if (p_port != COAP_DEFAULT_PORT)
	{
		unsigned char l_portbuf[2];
		coap_insert(p_saveOptList, new_option_node(COAP_OPTION_URI_PORT,
				coap_encode_var_bytes(l_portbuf, p_port), l_portbuf), order_opts);
	}

	if (p_path != NULL && p_path->length)
	{
		const int BUFSIZE = 40;
		unsigned char _buf[BUFSIZE];
		unsigned char *l_buf = _buf;
		size_t l_buflen = BUFSIZE;
		int res = coap_split_path(p_path->s, p_path->length, l_buf, &l_buflen);
		while (res--)
		{
			coap_insert(p_saveOptList, new_option_node(COAP_OPTION_URI_PATH,
					COAP_OPT_LENGTH(l_buf), COAP_OPT_VALUE(l_buf)), order_opts);
			l_buf += COAP_OPT_SIZE(l_buf);
		}
	}

	if (p_query != NULL && p_query->length)
	{
		const int BUFSIZE = 40;
		unsigned char _buf[BUFSIZE];
		unsigned char *l_buf = _buf;
		size_t l_buflen = BUFSIZE;
		int res = coap_split_query(p_query->s, p_query->length, l_buf, &l_buflen);
		while (res--)
		{
			coap_insert(p_saveOptList, new_option_node(COAP_OPTION_URI_QUERY,
					COAP_OPT_LENGTH(l_buf), COAP_OPT_VALUE(l_buf)), order_opts);
			l_buf += COAP_OPT_SIZE(l_buf);
		}
	}
	if(p_host != NULL && p_host->length)
	{
		coap_insert(p_saveOptList, new_option_node(COAP_OPTION_URI_HOST,
				p_host->length, p_host->s), order_opts);
	}
}

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
		case AF_INET:
		{
			memcpy((void*)&(p_dstAddr->addr.sa), l_ainfo->ai_addr, l_ainfo->ai_addrlen);
			p_dstAddr->size = l_ainfo->ai_addrlen;
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
			break;
		case AF_INET6:
			l_addrptr = &(p_dstAddr->addr.sin6.sin6_addr);
			printf("p_dstAddr type = %s \n", "AF_INET6");
			break;
		}
		inet_ntop(p_dstAddr->addr.sa.sa_family, l_addrptr, l_tmpAddr, sizeof(l_tmpAddr));
		printf("p_dstAddr addr = %s \n", l_tmpAddr);
	}
}

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

coap_context_t  *getClientCoapContext(int p_netType)
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
		//coap_register_response_handler(l_context, message_handler);
	}
	return l_context;
}

int sendCoapRequestMsg(char* p_dstUrl)
{
	if(p_dstUrl == NULL || strlen(p_dstUrl) <= 0)
	{
		return -1;
	}
	coap_set_log_level(LOG_DEBUG);

	str *l_dstHost, *l_dstPath, *l_dstPuery;
	int l_dstPort;
	if(0 != resolveURLResource(p_dstUrl, &l_dstHost, &l_dstPath, &l_dstPuery, &l_dstPort))
	{
		return -1;
	}
	showURLResource(p_dstUrl, l_dstHost, l_dstPath, l_dstPuery, l_dstPort);

	coap_address_t l_dstAddr;
	void *addrptr = NULL;
	if(0 != resolveDstAddress(l_dstHost, l_dstPort, &l_dstAddr))
	{
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		return -1;
	}
	showDstAddr(&l_dstAddr);

	coap_context_t  *l_coapContext = getClientCoapContext(l_dstAddr.addr.sa.sa_family);
	if(l_coapContext == NULL)
	{
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		return -1;
	}

	coap_list_t *l_currentOptlist = NULL;
	createOptList(l_dstPort, l_dstHost, l_dstPath, l_dstPuery, &l_currentOptlist);
	showOptList(l_currentOptlist);

	releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);

	coap_free_context(l_coapContext);
	return 0;
}

int main(int argc, char** argv)
{
	char* l_inputUrl2 = "coap://[10.96.17.51]/sensors/temperature?value";
	//char* l_inputUrl2 = "coap://[::1]/sensors/temperature?value";
	sendCoapRequestMsg(l_inputUrl2);
	return 0;
}

int order_opts(void *a, void *b)
{
	if (!a || !b)
	{
		return a < b ? -1 : 1;
	}
	if (COAP_OPTION_KEY(*(coap_option *)a) < COAP_OPTION_KEY(*(coap_option *)b))
	{
		return -1;
	}
	return COAP_OPTION_KEY(*(coap_option *)a) == COAP_OPTION_KEY(*(coap_option *)b);
}

coap_list_t *new_option_node(unsigned short p_key, unsigned int p_length, unsigned char *p_data)
{
	coap_option *l_option;
	coap_list_t *l_node;

	l_option = coap_malloc(sizeof(coap_option) + p_length);
	if (l_option == NULL)
	{
		perror("coap malloc failed");
		return NULL;
	}

	COAP_OPTION_KEY(*l_option) = p_key;
	COAP_OPTION_LENGTH(*l_option) = p_length;
	memcpy(COAP_OPTION_DATA(*l_option), p_data, p_length);

	/* we can pass NULL here as delete function since option is released automatically  */
	l_node = coap_new_listnode(l_option, NULL);
	if(l_node != NULL)
	{
		return l_node;
	}

	perror("new_option_node: malloc");
	coap_free( l_option );
	return NULL;
}
