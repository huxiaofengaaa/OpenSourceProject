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
#include "coapClient.h"

static void set_timeout(coap_tick_t *timer, const unsigned int seconds)
{
	coap_ticks(timer);
	*timer += seconds * COAP_TICKS_PER_SECOND;
}

static void messageHandler(struct coap_context_t  *p_coapContext, const coap_address_t *p_remote,
		coap_pdu_t *p_sent, coap_pdu_t *p_received, const coap_tid_t p_id)
{
	if(p_coapContext == NULL || p_remote == NULL || p_sent == NULL || p_received == NULL
			|| p_id == COAP_INVALID_TID)
	{
		printf("message handler - failed, parameter is NULL\n");
		return;
	}

	char l_host[32] = { 0 };
	int l_port = 0;
	getAddressString((struct sockaddr*)&(p_remote->addr.sa), l_host, sizeof(l_host), &l_port);

	showPDU("messageHandler request  PDU", p_sent);
	showPDU("messageHandler response PDU", p_received);

	if(0 != checkSendAndReceivedCoapPDUToken(p_received, p_sent))
	{
		printf("messageHandler - token check failed\n");
		return;
	}

	if(p_sent->hdr->type == COAP_MESSAGE_CON && p_received->hdr->type == COAP_MESSAGE_ACK)
	{
		unsigned char* l_dataPtr = NULL;
		size_t l_dataSize = 0;
		if(coap_get_data(p_received, &l_dataSize, &l_dataPtr))
		{
			printf("messageHandler - received data %d, %s\n", l_dataSize, l_dataPtr);
			char l_netAddr[32] = { 0 };
			snprintf(l_netAddr, sizeof(l_netAddr), "%s:%d", l_host, l_port);
			saveResponseIntoGlobalBuffer(l_dataPtr, l_dataSize, l_netAddr);
		}
	}
}

static int receivedCoapResponseOnceWithTimeout(coap_context_t *p_coapContext, int p_timeout)
{
	if(p_coapContext == NULL)
	{
		return -1;
	}
	coap_tick_t l_maxWaitSecond;
	set_timeout(&l_maxWaitSecond, p_timeout);
	printf("timeout is set to %d seconds, current %ld\n", p_timeout, time(NULL));

	int ready = 1;
	while(!(ready && coap_can_exit(p_coapContext)))
	{
		fd_set l_readfds;
		FD_ZERO(&l_readfds);
		FD_SET(p_coapContext->sockfd, &l_readfds );

		coap_tick_t l_currentCoapTick;
		coap_ticks(&l_currentCoapTick);
		if(l_currentCoapTick >= l_maxWaitSecond)
		{
			printf("timeout, current %ld\n", time(NULL));
			return -1;
		}

		struct timeval l_selectTimeout;
		l_selectTimeout.tv_usec = ((l_maxWaitSecond - l_currentCoapTick)
				% COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
		l_selectTimeout.tv_sec = (l_maxWaitSecond - l_currentCoapTick) / COAP_TICKS_PER_SECOND;

		int result = select(p_coapContext->sockfd + 1, &l_readfds, 0, 0, &l_selectTimeout);
		if(result < 0)
		{
			perror("select error");
			continue;
		}
		else if(result > 0)
		{
			if (FD_ISSET(p_coapContext->sockfd, &l_readfds ))
			{
				coap_read(p_coapContext);	/* read received data */
				coap_dispatch(p_coapContext);	/* and dispatch PDUs from receivequeue */
			}
		}
		else
		{
			printf("timeout, current %ld\n", time(NULL));
			return -1;
		}
	}
	printf("received all response, current %ld\n", time(NULL));
	return 0;
}

static void releaseContext(coap_context_t  **p_coapContext)
{
	if(*p_coapContext)
	{
		coap_free_context(*p_coapContext);
		*p_coapContext = NULL;
	}
}

int sendGroupCoapRequestMsg(CoapRequestList* p_reqList, int p_reqSize, int p_timeout)
{
	if(p_reqList == NULL || p_reqSize <= 0)
	{
		return -1;
	}
	initResponseBuffer();
	coap_context_t  *l_coapContext = NULL;
	char* l_netAddr = (char*)malloc(32 * p_reqSize);
	if(l_netAddr == NULL)
	{
		printf("malloc for l_netAddr failed\n");
		return -1;
	}
	memset(l_netAddr, 0, 32 * p_reqSize);

	printf("==========================================================\n");
	int i = 0;
	for(i  = 0; i< p_reqSize ; i++)
	{
		/*
		 * resolve input cmd line URL
		 */
		str *l_dstHost, *l_dstPath, *l_dstPuery;
		int l_dstPort;
		if(0 != resolveURLResource(p_reqList[i].m_dstUrl, &l_dstHost, &l_dstPath, &l_dstPuery, &l_dstPort))
		{
			releaseContext(&l_coapContext);
			return -1;
		}
		l_dstPort = (l_dstPort != p_reqList[i].m_port && p_reqList[i].m_port != -1)
				? p_reqList[i].m_port :l_dstPort;
		showURLResource(p_reqList[i].m_dstUrl, l_dstHost, l_dstPath, l_dstPuery, l_dstPort);

		/*
		 * resolve destination address
		 */
		coap_address_t l_dstAddr;
		if(0 != resolveDstAddress(l_dstHost, l_dstPort, &l_dstAddr))
		{
			releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
			releaseContext(&l_coapContext);
			return -1;
		}
		showDstAddr(&l_dstAddr);

		/*
		 * print local socket net address
		 */
		char l_host[32] = { 0 };
		int l_port = 0;
		getAddressString((struct sockaddr*)&(l_dstAddr.addr.sa), l_host, sizeof(l_host), &l_port);
		if(LOG_DEBUG <= coap_get_log_level())
		{
			printf("host %s, port %d\n", l_host, l_port);
		}
		snprintf(l_netAddr + i * 32, 32, "%s:%d", l_host, l_port);

		/*
		 * create an coap client context used for send msg
		 */
		if(l_coapContext == NULL)
		{
			l_coapContext = getClientCoapContext(l_dstAddr.addr.sa.sa_family, messageHandler);
			if(l_coapContext == NULL)
			{
				releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
				return -1;
			}
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
		COAP_SET_STR(&l_token, strlen(p_reqList[i].m_token), p_reqList[i].m_token);
		str l_payload;
		COAP_SET_STR(&l_payload, strlen(p_reqList[i].m_payload), p_reqList[i].m_payload);
		const unsigned char c_msgType = COAP_MESSAGE_CON;
		coap_pdu_t * l_newPDU = createCoapNewRequest(l_coapContext, p_reqList[i].m_method,
				l_currentOptlist, c_msgType, &l_token, &l_payload);
		if(l_newPDU == NULL)
		{
			printf("create new coap request failed\n");
			releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
			releaseContext(&l_coapContext);
			return -1;
		}
		showPDU("Coap Request", l_newPDU);

		/*
		 * send pdu message
		 */
		if(0 != sendCoapPdu(l_coapContext, l_newPDU, &l_dstAddr))
		{
			printf("send coap pdu failed\n");
			releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
			releaseContext(&l_coapContext);
			return -1;
		}
		else
		{
			printf("send coap pdu successfully\n");
		}
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
	}

	if(0 != receivedCoapResponseOnceWithTimeout(l_coapContext, p_timeout))
	{
		printf("received coap response timeout\n");
	}
	for(i  = 0; i< p_reqSize ; i++)
	{
		getResponse(p_reqList[i].p_respBuffer, p_reqList[i].p_bufferSize, l_netAddr + 32 * i);
	}

	if(l_netAddr)
	{
		free(l_netAddr);
		l_netAddr = NULL;
	}
	releaseContext(&l_coapContext);
	cleanGlobalResource();
	return 0;
}

int sendCoapRequestMsg(char* p_dstUrl, int p_port, char* p_method, char* p_token, char* p_payload,
		char* p_respBuffer, int p_bufferSize)
{
	if(p_dstUrl == NULL || strlen(p_dstUrl) <= 0)
	{
		return -1;
	}
	initResponseBuffer();
	printf("==========================================================\n");
	/*
	 * resolve input cmd line URL
	 */
	str *l_dstHost, *l_dstPath, *l_dstPuery;
	int l_dstPort;
	if(0 != resolveURLResource(p_dstUrl, &l_dstHost, &l_dstPath, &l_dstPuery, &l_dstPort))
	{
		return -1;
	}
	l_dstPort = (l_dstPort != p_port && p_port != -1) ? p_port :l_dstPort;
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

	/*
	 * print local socket net address
	 */
	char l_host[32] = { 0 };
	int l_port = 0;
	getAddressString((struct sockaddr*)&(l_dstAddr.addr.sa), l_host, sizeof(l_host), &l_port);
	if(LOG_DEBUG <= coap_get_log_level())
	{
		printf("host %s, port %d\n", l_host, l_port);
	}

	/*
	 * create an coap client context used for send msg
	 */
	coap_context_t  *l_coapContext = getClientCoapContext(l_dstAddr.addr.sa.sa_family, messageHandler);
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
		releaseContext(&l_coapContext);
		return -1;
	}
	showPDU("Coap Request", l_newPDU);

	/*
	 * send pdu message
	 */
	if(0 != sendCoapPdu(l_coapContext, l_newPDU, &l_dstAddr))
	{
		printf("send coap pdu failed\n");
		releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
		releaseContext(&l_coapContext);
		return -1;
	}
	else
	{
		printf("send coap pdu successfully\n");
	}

	if(0 != receivedCoapResponseOnceWithTimeout(l_coapContext, 8))
	{
		printf("received coap response timeout\n");
	}
	else
	{
		char l_netAddr[32] = { 0 };
		snprintf(l_netAddr, sizeof(l_netAddr), "%s:%d", l_host, l_port);
		printf("get response %s\n", l_netAddr);
		getResponse(p_respBuffer, p_bufferSize, l_netAddr);
	}

	/*
	 * release resource and memory
	 */
	releaseURLResource(l_dstHost, l_dstPath, l_dstPuery);
	releaseContext(&l_coapContext);
	cleanGlobalResource();
	return 0;
}

int main(int argc, char** argv)
{
	//coap_set_log_level(LOG_DEBUG);

	char* l_inputUrl2 = "coap://[10.96.17.50]/sensors/temperature?value";
	char* l_token = "cafe";
	char* l_payload = "abcd";
	char l_respBuf[128] = { 0 };
	sendCoapRequestMsg(l_inputUrl2, 8899, "get", l_token, l_payload, l_respBuf, sizeof(l_respBuf));
	printf("resp - %s\n", l_respBuf);

	/////////////////////////////////////////////////////////////////////////////////////////////

	char l_response1[128] = { 0 };
	char l_response2[128] = { 0 };
	char l_response3[128] = { 0 };
	CoapRequestList l_reqlist[3] =
	{
			{"coap://[10.96.17.50]/sensors/temperature?value", -1, "get", "cafe", "abcd", l_response1, 128},
			{"coap://[10.96.17.50]/sensors/temperature?value", 8899, "get", "haha", "efgh", l_response2, 128},
			{"coap://[10.96.17.51]/sensors/temperature?value", -1, "get", "haha", "efgh", l_response3, 128}
	};
	sendGroupCoapRequestMsg(l_reqlist, 3, 3);

	printf("resp - %s %s %s\n", l_response1, l_response2, l_response3);

	return 0;
}
