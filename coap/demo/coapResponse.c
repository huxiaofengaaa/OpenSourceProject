#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GROUP_RESPONSE	64

typedef struct
{
	char m_netAddr[32];
	char* m_response;
}CoapResponse;

static CoapResponse g_coapRespBuffer[MAX_GROUP_RESPONSE];

void initResponseBuffer()
{
	int i = 0;
	for(i = 0; i < MAX_GROUP_RESPONSE; i++)
	{
		memset(g_coapRespBuffer[i].m_netAddr, 0, sizeof(g_coapRespBuffer[i].m_netAddr));
		g_coapRespBuffer[i].m_response = NULL;
	}
}

void cleanGlobalResource()
{
	int i = 0;
	for(i = 0; i < MAX_GROUP_RESPONSE; i++)
	{
		memset(g_coapRespBuffer[i].m_netAddr, 0, sizeof(g_coapRespBuffer[i].m_netAddr));
		if(g_coapRespBuffer[i].m_response != NULL)
		{
			free(g_coapRespBuffer[i].m_response);
			g_coapRespBuffer[i].m_response = NULL;
		}
	}
}

int getResponseNumber()
{
	int l_respNumber = 0;
	int i = 0;
	for(i = 0; i < MAX_GROUP_RESPONSE; i++)
	{
		if(g_coapRespBuffer[i].m_response != NULL)
		{
			l_respNumber++;
		}
	}
	return l_respNumber;
}

int getResponse(char* p_buffer, int p_bufSize, char* p_netAddr)
{
	if(p_buffer == NULL || p_bufSize <= 0 || p_netAddr == NULL)
	{
		printf("get response failed, input is NULL\n");
		return -1;
	}
	int i = 0;
	for(i = 0; i < MAX_GROUP_RESPONSE; i++)
	{
		if(strlen(g_coapRespBuffer[i].m_netAddr) > 0
				&& strcmp(g_coapRespBuffer[i].m_netAddr, p_netAddr) == 0
				&& g_coapRespBuffer[i].m_response != NULL)
		{
			snprintf(p_buffer, p_bufSize, "%s", g_coapRespBuffer[i].m_response);
			free(g_coapRespBuffer[i].m_response);
			g_coapRespBuffer[i].m_response = NULL;
			memset(g_coapRespBuffer[i].m_netAddr, 0, sizeof(g_coapRespBuffer[i].m_netAddr));
			return 0;
		}

	}
	printf("get response for %s failed\n", p_netAddr);
	return -1;
}

int saveResponseIntoGlobalBuffer(unsigned char* p_dataPtr, size_t p_dataSize, char* p_netAddr)
{
	if(p_dataPtr == NULL || p_dataSize <= 0 || p_netAddr == NULL)
	{
		printf("saveResponseIntoGlobalBuffer failed, input is NULL\n");
		return -1;
	}

	int i = 0;
	for(i = 0; i < MAX_GROUP_RESPONSE; i++)
	{
		if(g_coapRespBuffer[i].m_response == NULL
				&& strlen(g_coapRespBuffer[i].m_netAddr) <= 0)
		{
			g_coapRespBuffer[i].m_response = (char*)malloc(p_dataSize + 1);
			if(g_coapRespBuffer[i].m_response == NULL)
			{
				printf("saveResponseIntoGlobalBuffer malloc failed\n");
				return -1;
			}
			memcpy(g_coapRespBuffer[i].m_response, p_dataPtr, p_dataSize);
			g_coapRespBuffer[i].m_response[p_dataSize] = '\0';
			snprintf(g_coapRespBuffer[i].m_netAddr, sizeof(g_coapRespBuffer[i].m_netAddr),
					"%s", p_netAddr);
			break;
		}
	}
	return 0;
}


