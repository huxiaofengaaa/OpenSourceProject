#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
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
		printf("dstHost  = %s\n", p_dstHost->s);
	}
	printf("dstPort  = %d\n", p_dstPort);
	if(p_dstPath){
		printf("dstPath  = %s\n", p_dstPath->s);
	}
	if(p_dstQuery){
		printf("dstQuery = %s\n", p_dstQuery->s);
	}
	printf("\n");
}

char* getOptKeyName(int p_optKey)
{
	switch(p_optKey)
	{
	case  1: return "COAP_OPTION_IF_MATCH";
	case  3: return "COAP_OPTION_URI_HOST";
	case  4: return "COAP_OPTION_ETAG";
	case  5: return "COAP_OPTION_IF_NONE_MATCH";
	case  7: return "COAP_OPTION_URI_PORT";
	case  8: return "COAP_OPTION_LOCATION_PATH";
	case 11: return "COAP_OPTION_URI_PATH";
	case 12: return "COAP_OPTION_CONTENT_FORMAT";
	case 14: return "COAP_OPTION_MAXAGE";
	case 15: return "COAP_OPTION_URI_QUERY";
	case 17: return "COAP_OPTION_ACCEPT";
	case 20: return "COAP_OPTION_LOCATION_QUERY";
	case 35: return "COAP_OPTION_PROXY_URI";
	case 39: return "COAP_OPTION_PROXY_SCHEME";
	case 60: return "COAP_OPTION_SIZE1";
	default: return "unknown";
	}
}

void showOptList(coap_list_t * p_currentOptList)
{
	int l_optlistCounter = 0;
	coap_list_t *l_tmp = p_currentOptList;
	for(l_tmp = p_currentOptList; l_tmp != NULL; l_tmp = l_tmp->next)
	{
		coap_option *l_option = l_tmp->data;
		printf("optlist[%d] = key:%26s, length:%2d, data:%s\n", l_optlistCounter++,
				getOptKeyName(l_option->key), l_option->length,
				((unsigned char*)l_option + sizeof(coap_option)));
	}
	printf("\n");
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
	printf("\n");
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

#if 0
	switch(p_netType)
	{
	case AF_INET:
	{
		struct sockaddr_in l_localAddr;
		socklen_t l_localAddrLength = sizeof(struct sockaddr_in);
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
		break;
	}
	case AF_INET6:
	{
		struct sockaddr_in6 l_localAddr;
		socklen_t l_localAddrLength = sizeof(struct sockaddr_in6);
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
		break;
	}
	default:
	{
		printf("unsupported type %d\n", p_netType);
		break;
	}
	}
#endif
}
/*
 * p_msgType: COAP_MESSAGE_CON or COAP_MESSAGE_NON
 */
coap_pdu_t *createCoapNewRequest(coap_context_t *p_coapContext, char* p_method, coap_list_t *p_optlist, unsigned char p_msgType,
		str* p_Token, str* p_payload)
{
	if(p_coapContext == NULL || p_method == NULL)
	{
		return NULL;
	}

	int i = 0;
	int l_methodIndex = -1;
	char *methods[] = { 0, "get", "post", "put", "delete", 0};
	for(i = 1; methods[i] != 0; i++)
	{
		if(strcasecmp(p_method, methods[i]) == 0)
		{
			l_methodIndex = i;
			break;
		}
	}
	if(l_methodIndex < 1)
	{
		return NULL;
	}

	coap_pdu_t *l_newPdu = coap_new_pdu();
	if(l_newPdu == NULL)
	{
		return NULL;
	}

	l_newPdu->hdr->type = p_msgType; //
	l_newPdu->hdr->id = coap_new_message_id(p_coapContext);
	l_newPdu->hdr->code = l_methodIndex;

	if(p_Token && p_Token->length > 0)
	{
		l_newPdu->hdr->token_length = p_Token->length;
		if (!coap_add_token(l_newPdu, p_Token->length, p_Token->s))
		{
			printf("cannot add token to request\n");
		}
	}
	else
	{
		l_newPdu->hdr->token_length = 0;
	}

	coap_show_pdu(l_newPdu);


	coap_list_t *l_tmpOpt = NULL;
	for (l_tmpOpt = p_optlist; l_tmpOpt != NULL; l_tmpOpt = l_tmpOpt->next)
	{
		coap_add_option(l_newPdu, COAP_OPTION_KEY(*(coap_option *)l_tmpOpt->data),
				COAP_OPTION_LENGTH(*(coap_option *)l_tmpOpt->data),
				COAP_OPTION_DATA(*(coap_option *)l_tmpOpt->data));
	}

	if(p_Token != NULL && p_Token->length > 0)
	{
		l_newPdu->hdr->token_length = p_Token->length;
		if ( !coap_add_token(l_newPdu, p_Token->length, p_Token->s))
		{
			printf("cannot add token to request\n");
		}
	}

	if(p_payload != NULL && p_payload->length)
	{
		coap_add_data(l_newPdu, p_payload->length, p_payload->s);
	}
	return l_newPdu;
}

void showPDU(coap_pdu_t *p_pdu)
{
	char* c_messageType[4] = {"COAP_MESSAGE_CON", "COAP_MESSAGE_NON", "COAP_MESSAGE_ACK", "COAP_MESSAGE_RST"};
	char *methods[] = { 0, "get", "post", "put", "delete", 0};
	int i = 0;
	if(p_pdu != NULL)
	{
		printf("[PUD] max size  %ld\n", p_pdu->max_size);
		if(p_pdu->hdr != NULL)
		{
			printf("[PUD header] token_length  %d\n", p_pdu->hdr->token_length);
			printf("[PUD header] type          %s\n", c_messageType[p_pdu->hdr->type]);
			printf("[PUD header] version       %d\n", p_pdu->hdr->version);
			printf("[PUD header] method        %s\n", methods[p_pdu->hdr->code]);
			printf("[PUD header] id            %d\n", ntohs(p_pdu->hdr->id));
			printf("[PUD header] token         ");
			for(i = 0 ; i < p_pdu->hdr->token_length ; i++)
			{
				printf("%c", p_pdu->hdr->token[i]);
			}
			printf("\n");
		}
		printf("[PUD] max_delta %d\n", p_pdu->max_delta);
		printf("[PUD] length    %d\n", p_pdu->length);
		if(p_pdu->data != NULL)
		{
			printf("[PUD] data    %s\n", p_pdu->data);
		}
	}
	printf("\n");
}

int sendCoapPdu(coap_context_t  *p_coapContext, coap_pdu_t * p_pdu, coap_address_t* p_clientDstAddr)
{
	coap_tid_t l_tid = COAP_INVALID_TID;
	int l_actionResult = -1;
	if(p_pdu == NULL || p_coapContext == NULL || p_clientDstAddr == NULL)
	{
		return l_actionResult;
	}
	if (p_pdu->hdr->type == COAP_MESSAGE_CON)
	{
		l_tid = coap_send_confirmed(p_coapContext, p_clientDstAddr, p_pdu);
	}
	else
	{
		l_tid = coap_send(p_coapContext, p_clientDstAddr, p_pdu);
	}
	if(l_tid == COAP_INVALID_TID)
	{
		printf("sendCoapPdu failed, %s\n", strerror(errno));
		l_actionResult = -1;
		coap_delete_pdu(p_pdu);
	}
	else
	{
		if(p_pdu->hdr->type != COAP_MESSAGE_CON)
		{
			coap_delete_pdu(p_pdu);
		}
		l_actionResult = 0;
	}
	return l_actionResult;
}

int sendCoapRequestMsg(char* p_dstUrl, char* p_method, char* p_token, char* p_payload)
{
	if(p_dstUrl == NULL || strlen(p_dstUrl) <= 0)
	{
		return -1;
	}
	coap_set_log_level(LOG_DEBUG);

	/*
	 * resolve input cmd line URL
	 */
	str *l_dstHost, *l_dstPath, *l_dstPuery;
	int l_dstPort;
	if(0 != resolveURLResource(p_dstUrl, &l_dstHost, &l_dstPath, &l_dstPuery, &l_dstPort))
	{
		return -1;
	}
	showURLResource(p_dstUrl, l_dstHost, l_dstPath, l_dstPuery, l_dstPort);

	/*
	 * resolve destination address
	 */
	coap_address_t l_dstAddr;
	void *addrptr = NULL;
	if(0 != resolveDstAddress(l_dstHost, l_dstPort, &l_dstAddr))
	{
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		return -1;
	}
	showDstAddr(&l_dstAddr);
	char l_host[32] = { 0 };
	int l_port = 0;
	getAddressString((struct sockaddr*)&(l_dstAddr.addr.sa), l_host, sizeof(l_host), &l_port);
	printf("host %s, port %d\n", l_host, l_port);

	/*
	 * create an coap client context used for send msg
	 */
	coap_context_t  *l_coapContext = getClientCoapContext(l_dstAddr.addr.sa.sa_family);
	if(l_coapContext == NULL)
	{
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		return -1;
	}
	showLocalAddress(l_coapContext->sockfd, l_dstAddr.addr.sa.sa_family);

	/*
	 * create opt list for current coap request message
	 */
	coap_list_t *l_currentOptlist = NULL;
	createOptList(l_dstPort, l_dstHost, l_dstPath, l_dstPuery, &l_currentOptlist);
	showOptList(l_currentOptlist);

	/*
	 * create message PDU
	 */
	str l_token;
	COAP_SET_STR(&l_token, strlen(p_token), p_token);
	str l_payload;
	COAP_SET_STR(&l_payload, strlen(p_payload), p_payload);
	const unsigned char c_msgType = COAP_MESSAGE_CON;
	coap_pdu_t * l_newPDU = createCoapNewRequest(l_coapContext, p_method, l_currentOptlist,
			c_msgType, &l_token, &l_payload);
	if(l_newPDU == NULL)
	{
		printf("create new coap request failed\n");
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		coap_free_context(l_coapContext);
		return -1;
	}
	showPDU(l_newPDU);

	/*
	 * send pdu message
	 */
	if(0 != sendCoapPdu(l_coapContext, l_newPDU, &l_dstAddr))
	{
		printf("send coap pdu failed\n");
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		coap_free_context(l_coapContext);
		return -1;
	}
	else
	{
		printf("send coap pdu successfully\n");
	}

	/*
	 * release resource and memory
	 */
	releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
	coap_free_context(l_coapContext);
	return 0;
}

int main(int argc, char** argv)
{
	char* l_inputUrl2 = "coap://[10.96.17.50]/sensors/temperature?value";
	char* l_token = "cafe";
	char* l_payload = "abcd";
	sendCoapRequestMsg(l_inputUrl2, "get", l_token, l_payload);
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
