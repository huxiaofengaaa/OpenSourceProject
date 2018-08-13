#include "coap.h"
#include <stdio.h>

int resolveURLResource(char* p_URL, str** p_host, str** p_path, str** p_query, int* p_port)
{
	if(p_URL == NULL || strlen(p_URL) <= 0)
	{
		printf("resolveURLResource failed, input is NULL\n");
		return -1;
	}
	coap_uri_t l_coapUri;
	int l_ret = coap_split_uri((unsigned char *)p_URL, strlen(p_URL), &l_coapUri );
	if(l_ret != 0)
	{
		printf("resolveURLResource failed, coap_split_uri error\n");
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
	if(LOG_DEBUG <= coap_get_log_level())
	{
		if(p_dstUrl)
		{
			printf("p_dstUrl = %s\n", p_dstUrl);
		}
		if(p_dstHost)
		{
			printf("dstHost  = %s\n", p_dstHost->s);
		}
		printf("dstPort  = %d\n", p_dstPort);
		if(p_dstPath)
		{
			printf("dstPath  = %s\n", p_dstPath->s);
		}
		if(p_dstQuery)
		{
			printf("dstQuery = %s\n", p_dstQuery->s);
		}
		printf("\n");
	}
}


