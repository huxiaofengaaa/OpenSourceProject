#include "coap.h"
#include <stdio.h>

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
		perror("new_option_node coap malloc failed");
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

	printf("new_option_node: malloc failed\n");
	coap_free( l_option );
	return NULL;
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
	if(LOG_DEBUG <= coap_get_log_level())
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

