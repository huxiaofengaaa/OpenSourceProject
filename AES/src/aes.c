
/*
    in :  input string before encrypt
    out : output string after encrypt
    key : input key = md5(loid)
	keybits : = 128 can not modify //now use 256
*/


#include <stdio.h>
#include <stdlib.h>
#include "openssl/aes.h"
#include "md5.h"

int ez_AES_ecb_encrypt_PKCS5Padding(const char *in, unsigned char *out, const unsigned char *key, const int keybits,int *outlen)
{
	AES_KEY aes;
	unsigned char *input_string;
	unsigned char *encrypt_string;
	unsigned int surplus_len;
	char b, *tmp_out;
	const char *tmp_in;
	int encrypt_quit = 0;
	int i=0;
	if(in == NULL || out == NULL || outlen==NULL)
	{
#ifdef AES_DEBUG
		printf("[%s] [%d] \"in\" or \"out\" point error!!\n", __FILE__, __LINE__);
#endif
	}
	input_string = (unsigned char*)calloc(AES_BLOCK_SIZE, sizeof(unsigned char));
	if (input_string == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for input_string\n");
		exit(-1);
	}

	encrypt_string = (unsigned char*)calloc(AES_BLOCK_SIZE, sizeof(unsigned char));
	if (encrypt_string == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for encrypt_string\n");
		return -1;
	}
	memset(encrypt_string, 0, AES_BLOCK_SIZE);
	// set encrypt key
	if (AES_set_encrypt_key(key, keybits, &aes) < 0)
	{
		fprintf(stderr, "Unable to set encryption key in AES\n");
		return -1;
	}
	tmp_in = in;
	tmp_out = out;

	while(!encrypt_quit)
	{
		if(strlen(tmp_in) == 0)
		{
			surplus_len = AES_BLOCK_SIZE;
			b = '\0'+surplus_len;
			memset(input_string, b, AES_BLOCK_SIZE);
			encrypt_quit = 1;
		}
		else if(strlen(tmp_in)/AES_BLOCK_SIZE > 0)
		{
			memcpy(input_string, tmp_in, AES_BLOCK_SIZE);
		}
		else
		{
			surplus_len = strlen(tmp_in)%AES_BLOCK_SIZE;
			b = '\0'+AES_BLOCK_SIZE-surplus_len;
			memset(input_string, b, AES_BLOCK_SIZE);
			memcpy(input_string, tmp_in, strlen(tmp_in));
			encrypt_quit = 1;
		}
#ifdef AES_DEBUG
		printf("input_string: %sinput_string\n", input_string);
		for(i=0; i<AES_BLOCK_SIZE; i++)
			printf("%2.2x ",input_string[i]);
		printf("\n\n");

#endif

		memset(encrypt_string, 0, AES_BLOCK_SIZE);
		AES_ecb_encrypt(input_string, encrypt_string, &aes, AES_ENCRYPT);
#ifdef AES_DEBUG
		printf("encrypt_string:");
		for(i=0; i<AES_BLOCK_SIZE; i++)
			printf("%2.2x ",encrypt_string[i]);
		printf("\n");

#endif
		memcpy(tmp_out, encrypt_string, AES_BLOCK_SIZE);
		tmp_out += AES_BLOCK_SIZE;
		tmp_in += AES_BLOCK_SIZE;
		*outlen+=AES_BLOCK_SIZE;
	}
	free(input_string);
	free(encrypt_string);
	printf("outlen=%d ",strlen(out));
#ifdef AES_DEBUG
	printf("out_string:");
	for(i=0; i<strlen(out); i++)
		printf("%2.2x ",out[i]);
	printf("\n");

#endif
	return 0;
}

void getaes(char *in, char *out, char *key)
{
	int keybits=256, outlen=0, i=0;
	unsigned char aesout[256]= {0};
	char *outtmp=out;

	AppMgrLogLine("in:%s	key:%s", in, key);
	ez_AES_ecb_encrypt_PKCS5Padding(in, aesout, key, keybits, &outlen);
	for(i=0; i<outlen; i++)
	{
		sprintf(out,"%2.2x", aesout[i]);
		out+=2;
	}
	while(*outtmp != '\0')
	{
		if((*outtmp>='a')&&(*outtmp<='z'))
			*outtmp -= 32;
		outtmp++;
	}
}

int decrypt_aes(char *in, char *out, char *key)
{
	if(strlen(in)%AES_BLOCK_SIZE!=0 )
		return -1;

	unsigned char decrypt_string[256]= {0};
	unsigned char decrypt_step_string[AES_BLOCK_SIZE]= {0};
	AES_KEY aes;
	char *p=in;
	unsigned char p4[3]= {0};
	int i=0,j=0,decrypt_len=0;
	unsigned char *p2=decrypt_string;
	unsigned int totallen=0;

	for(i=0,j=0; i<strlen(p); i+=2,j++)
	{
		p4[0]=p[i];
		p4[1]=p[i+1];
		p4[2]=0;
		decrypt_string[j]=strtoll(p4, (char **)NULL, 16);//convert input
	}

	AES_set_decrypt_key(key, 256,&aes);

	decrypt_len=strlen(p)/2;
	//printf("decrypt_len=%d\n",j);

	while(decrypt_len/AES_BLOCK_SIZE>0)
	{

		AES_decrypt(p2,decrypt_step_string,&aes);
		p2+=AES_BLOCK_SIZE;
		decrypt_len-=AES_BLOCK_SIZE;

		for(i=0; i<AES_BLOCK_SIZE; i++)
		{
			//printf("%2.2x ",decrypt_step_string[i]);
			if(decrypt_step_string[i]>0x10)
			{
				sprintf(out,"%c", decrypt_step_string[i]);
				out+=1;
			}
			//else
			//printf("maybe padding\n");

		}
	}

	return 0;
}

#if 0
void aes_main()
{
	unsigned char key[256]= {0};
	AES_KEY aes;
	getmd5("EPONSMT011",key);
	unsigned char *in="123456789hjkhj";


	unsigned char out[256]= {0};
	unsigned char decrypt_out[256]= {0};

	getaes(in,out,key);
	printf("encrypt_out is %s\n",out);

	decrypt_aes(out,decrypt_out,key);
	printf("decrypt_out is %s\n",decrypt_out);
}
#endif

