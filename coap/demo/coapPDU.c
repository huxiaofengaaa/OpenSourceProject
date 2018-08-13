#include "coap.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
/*
 * p_msgType: COAP_MESSAGE_CON or COAP_MESSAGE_NON
 */
coap_pdu_t *createCoapNewRequest(coap_context_t *p_coapContext, char* p_method,
		coap_list_t *p_optlist, unsigned char p_msgType, str* p_Token, str* p_payload)
{
	if(p_coapContext == NULL || p_method == NULL)
	{
		printf("createCoapNewRequest failed, input is NULL\n");
		return NULL;
	}

	/*
	 * Parse the coap request method
	 */
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
		printf("createCoapNewRequest failed, method %s is error\n", p_method);
		return NULL;
	}

	coap_pdu_t *l_newPdu = coap_new_pdu();
	if(l_newPdu == NULL)
	{
		printf("createCoapNewRequest create new pdu failed\n");
		return NULL;
	}

	l_newPdu->hdr->type = p_msgType; //
	l_newPdu->hdr->id = coap_new_message_id(p_coapContext);
	l_newPdu->hdr->code = l_methodIndex;

	if(p_Token != NULL && p_Token->length > 0)
	{
		l_newPdu->hdr->token_length = p_Token->length;
		if (!coap_add_token(l_newPdu, p_Token->length, p_Token->s))
		{
			printf("createCoapNewRequest add token to request failed\n");
		}
	}
	else
	{
		l_newPdu->hdr->token_length = 0;
	}

	coap_list_t *l_tmpOpt = NULL;
	for (l_tmpOpt = p_optlist; l_tmpOpt != NULL; l_tmpOpt = l_tmpOpt->next)
	{
		coap_add_option(l_newPdu, COAP_OPTION_KEY(*(coap_option *)l_tmpOpt->data),
				COAP_OPTION_LENGTH(*(coap_option *)l_tmpOpt->data),
				COAP_OPTION_DATA(*(coap_option *)l_tmpOpt->data));
	}

	if(p_payload != NULL && p_payload->length)
	{
		if(!coap_add_data(l_newPdu, p_payload->length, p_payload->s))
		{
			printf("createCoapNewRequest coap add payload failed\n");
		}
	}
	return l_newPdu;
}

void showPDU(char* p_usage, coap_pdu_t *p_pdu)
{
	char* c_messageType[4] =
	{
			"COAP_MESSAGE_CON", "COAP_MESSAGE_NON", "COAP_MESSAGE_ACK", "COAP_MESSAGE_RST"
	};
	char *methods[] = { 0, "get", "post", "put", "delete", 0};
	int i = 0;
	if(LOG_DEBUG <= coap_get_log_level() && p_pdu != NULL)
	{
		printf("[PUD] max size  %ld\n", p_pdu->max_size);
		if(p_pdu->hdr != NULL)
		{
			printf("[PUD header] token_length  %d\n", p_pdu->hdr->token_length);
			printf("[PUD header] type          %s\n", c_messageType[p_pdu->hdr->type]);
			printf("[PUD header] version       %d\n", p_pdu->hdr->version);
			if(p_pdu->hdr->code >= 1 && p_pdu->hdr->code <= 4)
			{
				printf("[PUD header] method        %s\n", methods[p_pdu->hdr->code]);
			}
			else
			{
				printf("[PUD header] method        %d.%02d\n", (p_pdu->hdr->code >> 5),
						p_pdu->hdr->code & 0x1F);
			}
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
	printf("%s / ", p_usage);
	coap_show_pdu(p_pdu);
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

int checkSendAndReceivedCoapPDUToken(coap_pdu_t *p_sent, coap_pdu_t *p_received)
{
	if(p_sent != NULL && p_received != NULL)
	{
		if(p_received->hdr->token_length == p_sent->hdr->token_length &&
				memcmp(p_received->hdr->token, p_sent->hdr->token,
						p_received->hdr->token_length) == 0)
		{
			return 0;
		}
	}
	return -1;
}

