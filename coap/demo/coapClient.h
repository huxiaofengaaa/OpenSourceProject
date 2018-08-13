/*
 * coapClient.h
 *
 *  Created on: 2018Äê8ÔÂ13ÈÕ
 *      Author: Administrator
 */

#ifndef COAP_DEMO_COAPCLIENT_H_
#define COAP_DEMO_COAPCLIENT_H_

typedef struct
{
	char* m_dstUrl;
	int m_port;
	char* m_method;
	char* m_token;
	char* m_payload;
	char* p_respBuffer;
	int p_bufferSize;
}CoapRequestList;

/*
 * coap response message used for up layer would save into global buffer
 */
void initResponseBuffer();
void cleanGlobalResource();
int getResponseNumber();
int getResponse(char* p_buffer, int p_bufSize, char* p_netAddr);
int saveResponseIntoGlobalBuffer(unsigned char* p_dataPtr, size_t p_dataSize, char* p_netAddr);

/*
 * below function used to handle with coap optlist
 */
int order_opts(void *a, void *b);
coap_list_t *new_option_node(unsigned short p_key, unsigned int p_length, unsigned char *p_data);
char* getOptKeyName(int p_optKey);
void showOptList(coap_list_t * p_currentOptList);
void createOptList(int p_port, str* p_host, str* p_path, str* p_query, coap_list_t ** p_saveOptList);

/*
 * below function used to resolve input coap URL,
 * A coap URL look like: coap://[10.96.17.50]/sensors/temperature?value
 *     protocol: coap
 *     host    : 10.96.17.50
 *     port    : 5683(default)
 *     path    : sensors/temperature
 *     query   : value
 */
int resolveURLResource(char* p_URL, str** p_host, str** p_path, str** p_query, int* p_port);
void releaseURLResource(str* p_host, str* p_path, str* p_query);
void showURLResource(char* p_dstUrl, str* p_dstHost, str* p_dstPath, str* p_dstQuery,  int p_dstPort);

/*
 * below function used to resolve network address
 */
int resolveDstAddress(str* p_host, int p_port, coap_address_t* p_dstAddr);
void showDstAddr(coap_address_t* p_dstAddr);
int getAddressString(struct sockaddr* p_sa, char* p_buffer, int p_bufferSize, int* p_port);
void showLocalAddress(int p_socket, int p_netType);

/*
 * below function used to resolve and build coap PDU.
 */
coap_pdu_t *createCoapNewRequest(coap_context_t *p_coapContext, char* p_method,
		coap_list_t *p_optlist, unsigned char p_msgType, str* p_Token, str* p_payload);
void showPDU(char* p_usage, coap_pdu_t *p_pdu);
int sendCoapPdu(coap_context_t  *p_coapContext, coap_pdu_t * p_pdu, coap_address_t* p_clientDstAddr);
int checkSendAndReceivedCoapPDUToken(coap_pdu_t *p_sent, coap_pdu_t *p_received);

/*
 * below function used to create an client coap context, which used by many other function.
 */
coap_context_t  *getClientCoapContext(int p_netType, coap_response_handler_t p_handler);
coap_context_t *get_context(const char *node, const char *port);

int sendCoapRequestMsg(char* p_dstUrl, int p_port, char* p_method, char* p_token, char* p_payload,
		char* p_respBuffer, int p_bufferSize);
int sendGroupCoapRequestMsg(CoapRequestList* p_reqList, int p_reqSize, int p_timeout);

#endif /* COAP_DEMO_COAPCLIENT_H_ */
