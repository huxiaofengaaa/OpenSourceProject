/***********************************************************************************

 * 文 件 名   : aes_cbc.c
 * 负 责 人   : 朱俊杰
 * 创建日期   : 2017年7月26日
 * 版 本 号   : 
 * 文件描述   : 加密模式：CBC 填充方式：PKCS5Padding 偏移量：”0000000000000000”(16
                个 0) 输出格式：base64 字符集：utf-8
 * 版权说明   : Copyright (C) 2000-2017   烽火通信科技股份有限公司
 * 其    他   : 
 * 修改日志   : 

***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openssl/aes.h"
#include "base64.h"
#include "md5.h"

#ifdef AES_DEBUG

// a simple hex-print routine. could be modified to print 16 bytes-per-line
static void hex_print(const void* pv, size_t len)
{
    const unsigned char * p = (const unsigned char*)pv;
    
	printf("\n|^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|");
    if (NULL == pv)
        printf("NULL");
    else
    {		
        size_t i = 0;
        for (; i<len;++i)
        {
			if (0 == i % 16) printf("\n");
            printf("%02X ", *p++);
		}
    }
    printf("\n|vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv|\n");
}
#endif

/*****************************************************************************
 * 函 数 名  : PadData
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年8月10日
 * 函数功能  : 计算pad实际长度
 * 输入参数  : unsigned char *ibuf  buffer
               unsigned int ilen    length
               int blksize    blksize 
 * 输出参数  : 无
 * 返 回 值  : unsigned  return length
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static unsigned int paddata (unsigned char *ibuf, unsigned int ilen, int blksize)
{
	unsigned int i; /* loop counter*/
	unsigned char pad; /* pad character (calculated)*/
	unsigned char *p; /*pointer to end of data*/

	/* calculate pad character*/
	pad = (unsigned char) (blksize - (ilen % blksize));

	/* append pad to end of string*/
/*	p = ibuf + ilen;
	for (i = 0; i < (int) pad; i++) 
	{
		*p = pad;
		++p;
	}*/

	return (ilen + pad);
}

/*****************************************************************************
 * 函 数 名  : NoPadLen
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年8月10日
 * 函数功能  : 计算实际长度
 * 输入参数  : unsigned char *ibuf  buffer
               unsigned int ilen    length
 * 输出参数  : 无
 * 返 回 值  : unsigned  return length
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static unsigned int nopadlen (unsigned char *ibuf, unsigned int ilen)
{
	unsigned int i; /* adjusted length*/
	unsigned char *p; /* pointer to last character*/

	p = ibuf + (ilen - 1);
	i = ilen - (unsigned int) *p;

	return (i);
}

/*****************************************************************************
 * 函 数 名  : ez_AES_cbc_encrypt_PKCS5Padding
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年7月26日
 * 函数功能  : AES-128 CBC PKCS5Padding off-16-0 base64 utf-8加密实现
 * 输入参数  : const char *in            input string before encrypt
               unsigned char *out        output string before encrypt
               const unsigned char *key  key = md5(loid)
               const int keybits         128
               int *outlen               out len
 * 输出参数  : 无
 * 返 回 值  :   返回值
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
int ez_AES_cbc_encrypt_PKCS5Padding(const char *in,
    unsigned char *out,
    const unsigned char *key,
    const int keybits,
    int *outlen)
{
    AES_KEY aes;
    unsigned char *input_string;
    unsigned char *encrypt_string;
    unsigned int surplus_len;
    char b, *tmp_out;
    const char *tmp_in;
    int encrypt_quit = 0;
    int i = 0;
    unsigned char iv[AES_BLOCK_SIZE];

    if (in == NULL || out == NULL || outlen == NULL)
    {
#ifdef AES_DEBUG
        printf("[%s] [%d] \"in\" or \"out\" point error!!\n", __FILE__, __LINE__);
#endif
    }

    for (i = 0; i < AES_BLOCK_SIZE; ++i)
    {
        iv[i] = '0';
    }

    input_string = (unsigned char *)calloc(AES_BLOCK_SIZE, sizeof(unsigned char));
    if (input_string == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for input_string\n");
        exit(-1);
    }

    encrypt_string = (unsigned char *)calloc(AES_BLOCK_SIZE, sizeof(unsigned char));
    if (encrypt_string == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for encrypt_string\n");
        return -1;
    }
    memset(encrypt_string, 0, AES_BLOCK_SIZE);

    // set encrypt key
    memset( &aes, 0x00, sizeof(AES_KEY));
#ifdef AES_DEBUG
	printf("#####key =%s:%d\n", key, keybits);
#endif
    if (AES_set_encrypt_key(key, keybits, &aes) < 0)
    {
        fprintf(stderr, "Unable to set encryption key in AES\n");
        return -1;
    }
    tmp_in = in;
    tmp_out = out;

    while (!encrypt_quit)
    {
        if (strlen(tmp_in) == 0)
        {
			break;

/*            surplus_len = AES_BLOCK_SIZE;
            b = '\0' + surplus_len;
            memset(input_string, 0, AES_BLOCK_SIZE);
            encrypt_quit = 1;*/
        }
        else if (strlen(tmp_in) / AES_BLOCK_SIZE > 0)
        {
            memcpy(input_string, tmp_in, AES_BLOCK_SIZE);
        }
        else
        {
            surplus_len = strlen(tmp_in) % AES_BLOCK_SIZE;
            b = '\0' + AES_BLOCK_SIZE - surplus_len;
            memset(input_string, b, AES_BLOCK_SIZE);
            memcpy(input_string, tmp_in, strlen(tmp_in));
            encrypt_quit = 1;
        }
#ifdef AES_DEBUG
        printf("\n");
        printf("input_string: %s\n", input_string);
        for (i = 0; i < AES_BLOCK_SIZE; i++)
        {
            printf("%2.2x ", input_string[i]);
        }
        printf("\n\n");

#endif

        memset(encrypt_string, 0, AES_BLOCK_SIZE);

        //AES_ecb_encrypt(input_string, encrypt_string, &aes, AES_ENCRYPT);
        AES_cbc_encrypt(input_string, encrypt_string, AES_BLOCK_SIZE, &aes, iv, AES_ENCRYPT);

#ifdef AES_DEBUG
        printf("encrypt_string length:%d\n", AES_BLOCK_SIZE);
        printf("encrypt_string:");
        for (i = 0; i < AES_BLOCK_SIZE; i++)
        {
            printf("%2.2x ", encrypt_string[i]);
        }
        printf("\n");

#endif
        memcpy(tmp_out, encrypt_string, AES_BLOCK_SIZE);
        tmp_out += AES_BLOCK_SIZE;
        tmp_in += AES_BLOCK_SIZE;
        *outlen += AES_BLOCK_SIZE;
    }
    free(input_string);
    free(encrypt_string);
#ifdef AES_DEBUG
    printf("strlen:%d outlen=%d ", strlen(out), *outlen);
    printf("out_string:");
    for (i = 0; i < strlen(out); i++)
    {
        printf("%2.2x ", out[i]);
    }
    printf("\n");

#endif
    return 0;
}

/*****************************************************************************
 * 函 数 名  : encrypt_cbcaes
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年7月26日
 * 函数功能  : AES-128 CBC加密
 * 输入参数  : char *in   string before encrypted
               char *out  string after encrypted
               char *key  key
 * 输出参数  : 无
 * 返 回 值  :   返回值
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void encrypt_cbcaes(char *in, char *out, char *userKey)
{
    int keybits = 128;
    AES_KEY key;
    unsigned char ivec[16];
    memset(ivec, '0', sizeof(ivec));
    int res = AES_set_encrypt_key(userKey, keybits, &key);
   // printf("AES_set_encrypt_key result : %d\n", res);

   // printf("before encrypt_cbcaes : %s\n", in);
    AES_cbc_encrypt(in, out, strlen(in), &key, ivec, 1);
    //printf("after encrypt_cbcaes : %s\n", out);

#if 0
    int keybits = 128, outlen = 0, i = 0;
    unsigned char aesout[256] = {0};
    char *outtmp = out;
    printf("in:%s	key:%s", in, key);
    ez_AES_cbc_encrypt_PKCS5Padding(in, aesout, key, keybits, &outlen);

    for (i = 0; i < outlen; i++)
    {
        sprintf(out, "%2.2x", aesout[i]);
        out += 2;
    }
    while (*outtmp != '\0')
    {
        if ((*outtmp >= 'a') && (*outtmp <= 'z'))
        {
            *outtmp -= 32;
        }
        outtmp++;
   }
#endif
}

/*****************************************************************************
 * 函 数 名  : encrypt_cbcaes_base64
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年8月10日
 * 函数功能  : aes-128-cbc pck5spadding iv="0000000000000000" base64
 * 输入参数  : char *in   待加密字串、
               char *key  加密密钥
 * 输出参数  : 无
 * 返 回 值  : char  返回加密后字串
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
char *encrypt_cbcaes_base64(char *in, char *key)
{
	char *out = NULL;
	int inlen = 0;
	int outlen = 0;
	char *ret = NULL;

	inlen = strlen(in);

	outlen = paddata(in, inlen, AES_BLOCK_SIZE);
	out = malloc(outlen + 1);
	if (NULL == out) return NULL;

	encrypt_cbcaes(in, out, key);
	
	ret = base64_encode2((const char *)out, outlen);
    printf("encrypt_cbcaes_base64 : %s\n", ret);
	free(out);

	return ret;
}


/*****************************************************************************
 * 函 数 名  : ez_AES_cbc_decrypt_PKCS5Padding
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年7月26日
 * 函数功能  : AES-128 CBC PKCS5Padding off-16-0 base64 utf-8解密实现
 * 输入参数  : const char *in            input string before decrypt
               unsigned char *out        output string before decrypt
               const unsigned char *key  key = md5(loid)
               const int keybits         128
               int *outlen               out len
 * 输出参数  : 无
 * 返 回 值  :   返回值
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
int ez_AES_cbc_decrypt_PKCS5Padding(char *in,
    unsigned char *out,
    const unsigned char *key,
    const int keybits,
    int *outlen)
{
    unsigned char decrypt_step_string[AES_BLOCK_SIZE] = {0};
    AES_KEY aes;
    char *p = in;
    int i = 0, j = 0, decrypt_len = 0;
    unsigned int totallen = 0;
    unsigned char iv[AES_BLOCK_SIZE];
#ifdef AES_DEBUG
    unsigned char *outp = out;
#endif
    
    *outlen = 0;

    for (i = 0; i < AES_BLOCK_SIZE; ++i)
    {
        iv[i] = '0';
    }

    AES_set_decrypt_key(key, keybits, &aes);

    decrypt_len = strlen(p);
#ifdef AES_DEBUG
    printf("decrypt_len:%d\n", decrypt_len);
	hex_print(p, decrypt_len);
#endif

    while (decrypt_len / AES_BLOCK_SIZE > 0)
    {
        AES_cbc_encrypt(p, decrypt_step_string, sizeof(decrypt_step_string), &aes, iv, AES_DECRYPT);
#ifdef AES_DEBUG
		for (i = 0; i < AES_BLOCK_SIZE; i++)
		{
			printf("%2.2x ",p[i]);
		}
#endif
        p += AES_BLOCK_SIZE;
        decrypt_len -= AES_BLOCK_SIZE;

		printf("\n");
        for (i = 0; i < AES_BLOCK_SIZE; i++)
        {
            if (decrypt_step_string[i] > AES_BLOCK_SIZE)
            {
                sprintf(out, "%c", decrypt_step_string[i]);
                out += 1;
                *outlen += 1;
            }
#ifdef AES_DEBUG
			else
				printf("maybe padding\n");
			printf("%2.2x",decrypt_step_string[i]);
			printf("-%c ",outp[i]);
#endif
        }
    }

    return 0;
}

/*****************************************************************************
 * 函 数 名  : decrypt_cbcaes
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年7月26日
 * 函数功能  : AES-128 CBC解密
 * 输入参数  : char *in   string before decrypted
               char *out  string after decrypted
               char *key  key
 * 输出参数  : 无
 * 返 回 值  :   返回值
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
int decrypt_cbcaes(char *in, int inlen, char *out, char *userKey)
{

    int keybits = 128;
    AES_KEY key;
    unsigned char ivec[16];
    memset(ivec, '0', sizeof(ivec));
    int res = AES_set_decrypt_key(userKey, keybits, &key);
   // printf("AES_set_encrypt_key result : %d\n", res);

   // printf("before encrypt_cbcaes : %s\n", in);
    AES_cbc_encrypt(in, out, inlen, &key, ivec, 0);
    //printf("after encrypt_cbcaes : %s\n", out);

    int eIdx = strlen(out) - 1;

  /*  int i;
    for(i=0; i < 5; i++)
        printf("%d\n", out[eIdx-i]); */

    int num = 16;
    while( (out[eIdx] == 10 || out[eIdx] == 12) && num > 0)
    {
        eIdx--;
        num--;
    }
    out[eIdx + 1] = '\0';


#if 0
    char *decrypt_string = NULL;
    unsigned char decrypt_step_string[AES_BLOCK_SIZE] = {0};
    AES_KEY aes;
    char *p = in;
    unsigned char p4[3] = {0};
    int i = 0;
    int j = 0;
    int outlen = 0;
    unsigned char *p2 = NULL;

    if (strlen(in) % AES_BLOCK_SIZE != 0)
    {
        return -1;
    }

    decrypt_string = malloc(strlen(p) + 1);
    if (NULL == decrypt_string) return -1;
    memset(decrypt_string, 0, strlen(p) + 1);
    p2 = decrypt_string;

    for (i = 0, j = 0; i < strlen(p); i += 2, j++)
    {
        p4[0] = p[i];
        p4[1] = p[i + 1];
        p4[2] = 0;
        decrypt_string[j] = strtoll(p4, (char * *)NULL, 16); //convert input
    }

	ez_AES_cbc_decrypt_PKCS5Padding(p2, out, key, 128, &outlen);
	free(decrypt_string);
#endif
    return 0;
}

/*****************************************************************************
 * 函 数 名  : decrypt_cbcaes_base64
 * 负 责 人  : 朱俊杰
 * 创建日期  : 2017年8月10日
 * 函数功能  : aes-128-cbc pck5spadding iv="0000000000000000" base64
 * 输入参数  : char *in   待加密字串、
               char *key  加密密钥
 * 输出参数  : 无
 * 返 回 值  : char  返回加密后字串
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
char *decrypt_cbcaes_base64(char *in, char *key)
{
	char *out = NULL;
	char *decout = NULL;
	int inlen = 0;

	out = base64_decode2((const char *)in, &inlen);
	if (NULL == out) return NULL;

	//inlen = strlen(in);
	decout = malloc(inlen + 1);
	if (NULL == decout)
	{
		free(out);
		return NULL;
	}
	memset(decout, 0, inlen + 1);

	decrypt_cbcaes(out, inlen, decout, key);
	free(out);

	return decout;
}

#if 1
#define aes_printf	printf
#else
#define aes_printf
#endif

static void aes_cbc_disp(const char *str, const void *pbuf, const int size)
{
	int i=0;
	if(str != NULL)
	{
		aes_printf("%s:\n", str);
	}
	if(pbuf != NULL && size > 0)
	{
		for(i=0;i<size;i++)
			aes_printf("%02x ", *((unsigned char *)pbuf+i));
		aes_printf("\n");
	}
	aes_printf("\n");
}


/**************************************************************************
功能:aes cbc 加密或解密
参数:
userKey： 密钥数值；
bits：密钥长度，以bit为单位，如果密钥数字是16个字节，则此参数值应为128；
in： 需要加密/解密的数据；
out： 计算后输出的数据；
length： 数据长度
en_ro_de:	1--加密      0--解密
**************************************************************************/
int cbc_encrypt_or_decrypt(const unsigned char *userKey, const int bits,
						const unsigned char *in, unsigned char *out,
						size_t length, int en_ro_de)
{
	int returnval;
	AES_KEY key;
	unsigned char ivec[16];
	int enc;
		
	enc = en_ro_de;
	memset(ivec, 0, sizeof(ivec));

	if(1 == enc)
	{
		returnval = AES_set_encrypt_key(userKey, bits, &key);
		aes_printf("encrypt ! \n");
	}
	else
	{
		returnval = AES_set_decrypt_key(userKey, bits, &key);
		aes_printf("decrypt ! \n");
	}
	if(0 != returnval)
	{
		aes_printf("set key error !\n");
		return returnval;
	}

	aes_cbc_disp("aes_cbc in", in, length);
	AES_cbc_encrypt(in, out, length, &key, ivec, enc);
	aes_cbc_disp("aes_cbc out", out, length);

	return 0;
}



#if 0
void aes_main()
{
    unsigned char key[256] = {0};
    AES_KEY aes;
    getmd5("EPONSMT011", key);
    unsigned char *in = "123456789hjkhj";

    unsigned char out[256] = {0};
    unsigned char decrypt_out[256] = {0};

    getaes(in, out, key);
    printf("encrypt_out is %s\n", out);

    decrypt_aes(out, decrypt_out, key);
    printf("decrypt_out is %s\n", decrypt_out);
}

#endif

