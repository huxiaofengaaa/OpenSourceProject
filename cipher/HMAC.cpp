#include "MD5.h"
#include "HMAC.h"
#include "SHA.h"
#include "SHA1.h"
#include "SHA224.h"
#include "SHA256.h"
#include "SHA384.h"
#include "SHA512.h"
#include <string.h>

/*
unsigned char*  text;                pointer to data stream
int             text_len;            length of data stream
unsigned char*  key;                 pointer to authentication key
int             key_len;             length of authentication key
unsigned char*  digest;              caller digest to be filled in
*/
#define KEY_IOPAD_SIZE          64
#define KEY_IOPAD_SIZE128       128

void hmac_md5(unsigned char *key, int key_len, unsigned char *text, int text_len, unsigned char *hmac)
{
    MD5_CTX context;
    unsigned char k_ipad[KEY_IOPAD_SIZE];    /* inner padding - key XORd with ipad  */
    unsigned char k_opad[KEY_IOPAD_SIZE];    /* outer padding - key XORd with opad */
    int i;

    /*
    * the HMAC_MD5 transform looks like:
    *
    * MD5(K XOR opad, MD5(K XOR ipad, text))
    *
    * where K is an n byte key
    * ipad is the byte 0x36 repeated 64 times

    * opad is the byte 0x5c repeated 64 times
    * and text is the data being protected
    */

    /* start out by storing key in pads */
    memset( k_ipad, 0, sizeof(k_ipad));
    memset( k_opad, 0, sizeof(k_opad));
    memcpy( k_ipad, key, key_len);
    memcpy( k_opad, key, key_len);

    /* XOR key with ipad and opad values */
    for (i = 0; i < KEY_IOPAD_SIZE; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    // perform inner MD5
    MD5Init(&context);                    /* init context for 1st pass */
    MD5Update(&context, k_ipad, KEY_IOPAD_SIZE);      /* start with inner pad */
    MD5Update(&context, (unsigned char*)text, text_len); /* then text of datagram */
    MD5Final(hmac, &context);             /* finish up 1st pass */

    // perform outer MD5
    MD5Init(&context);                   /* init context for 2nd pass */
    MD5Update(&context, k_opad, KEY_IOPAD_SIZE);     /* start with outer pad */
    MD5Update(&context, hmac, MD5_DIGEST_SIZE);     /* then results of 1st hash */
    MD5Final(hmac, &context);          /* finish up 2nd pass */
}

static void normalize_key_sha1(const char* input_key, const int input_length, unsigned char* output_key)
{
    if( input_length <= KEY_IOPAD_SIZE )
    {
        memcpy( output_key, input_key, input_length );
        memset( output_key + input_length, '\0', KEY_IOPAD_SIZE - input_length );
    }
    else
    {
        SHA1_CTX s;
        SHA1Init( &s );
        SHA1Update( &s, (const unsigned char*)input_key, input_length );
        SHA1Final( output_key, &s);
        memset( output_key + SHA1_DIGEST_SIZE, '\0', KEY_IOPAD_SIZE - SHA1_DIGEST_SIZE );
    }
}

void hmac_sha1(unsigned char *key, int key_len, unsigned char *text, int text_len, unsigned char *hmac)
{
    unsigned char new_key[ KEY_IOPAD_SIZE ] = { 0 };
    normalize_key_sha1((char*)key, key_len, new_key);

    unsigned char k_ipad[KEY_IOPAD_SIZE];
    unsigned char k_opad[KEY_IOPAD_SIZE];
    int i;

    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, new_key, KEY_IOPAD_SIZE);
    memcpy(k_opad, new_key, KEY_IOPAD_SIZE);

    for (i = 0; i < KEY_IOPAD_SIZE; i++)
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    SHA1_CTX context;
    SHA1Init(&context);
    SHA1Update(&context, k_ipad, KEY_IOPAD_SIZE);
    SHA1Update(&context, text, text_len);
    SHA1Final(hmac, &context);

    SHA1Init(&context);
    SHA1Update(&context, k_opad, KEY_IOPAD_SIZE);
    SHA1Update(&context, hmac, SHA1_DIGEST_SIZE);
    SHA1Final(hmac, &context);
}

static void normalize_key_sha224(const char* input_key, const int input_length, unsigned char* output_key)
{
    if( input_length <= KEY_IOPAD_SIZE )
    {
        memcpy( output_key, input_key, input_length );
        memset( output_key + input_length, '\0', KEY_IOPAD_SIZE - input_length );
    }
    else
    {
        SHA256_State s;
        SHA224_Init( &s );
        SHA224_Bytes( &s, (const unsigned char*)input_key, input_length );
        SHA224_Final( &s, output_key);
        memset( output_key + SHA224_DIGEST_SIZE, '\0', KEY_IOPAD_SIZE - SHA224_DIGEST_SIZE );
    }
}

void hmac_sha224(unsigned char *key, int key_len, unsigned char *text, int text_len, unsigned char *hmac)
{
    unsigned char new_key[ KEY_IOPAD_SIZE ] = { 0 };
    normalize_key_sha224((char*)key, key_len, new_key);

    SHA256_State context;
    unsigned char k_ipad[KEY_IOPAD_SIZE];
    unsigned char k_opad[KEY_IOPAD_SIZE];
    int i;

    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, new_key, KEY_IOPAD_SIZE);
    memcpy(k_opad, new_key, KEY_IOPAD_SIZE);

    for (i = 0; i < KEY_IOPAD_SIZE; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    SHA224_Init(&context);
    SHA224_Bytes(&context, k_ipad, KEY_IOPAD_SIZE);
    SHA224_Bytes(&context, (unsigned char*)text, text_len);
    SHA224_Final(&context, hmac);

    SHA224_Init(&context);
    SHA224_Bytes(&context, k_opad, KEY_IOPAD_SIZE);
    SHA224_Bytes(&context, hmac, SHA224_DIGEST_SIZE);
    SHA224_Final(&context, hmac);
}

static void normalize_key_sha256(const char* input_key, const int input_length, unsigned char* output_key)
{
    if( input_length <= KEY_IOPAD_SIZE )
    {
        memcpy( output_key, input_key, input_length );
        memset( output_key + input_length, '\0', KEY_IOPAD_SIZE - input_length );
    }
    else
    {
        SHA256_State s;
        SHA256_Init( &s );
        SHA256_Bytes( &s, input_key, input_length );
        SHA256_Final( &s, output_key );
        memset( output_key + SHA256_DIGEST_SIZE, '\0', KEY_IOPAD_SIZE - SHA256_DIGEST_SIZE );
    }
}

void hmac_sha256(unsigned char *key, int key_len, unsigned char *text, int text_len, unsigned char *hmac)
{
    unsigned char new_key[ KEY_IOPAD_SIZE ] = { 0 };
    normalize_key_sha256((char*)key, key_len, new_key);

    SHA256_State context;
    unsigned char k_ipad[KEY_IOPAD_SIZE];
    unsigned char k_opad[KEY_IOPAD_SIZE];
    int i;

    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, new_key, KEY_IOPAD_SIZE);
    memcpy(k_opad, new_key, KEY_IOPAD_SIZE);

    for (i = 0; i < KEY_IOPAD_SIZE; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    SHA256_Init(&context);
    SHA256_Bytes(&context, k_ipad, KEY_IOPAD_SIZE);
    SHA256_Bytes(&context, text, text_len);
    SHA256_Final(&context, hmac);

    SHA256_Init(&context);
    SHA256_Bytes(&context, k_opad, KEY_IOPAD_SIZE);
    SHA256_Bytes(&context, hmac, SHA256_DIGEST_SIZE);
    SHA256_Final(&context, hmac);
}

static void normalize_key_sha384(const char* input_key, const int input_length, unsigned char* output_key)
{
    if( input_length <= KEY_IOPAD_SIZE128 )
    {
        memcpy( output_key, input_key, input_length );
        memset( output_key + input_length, '\0', KEY_IOPAD_SIZE128 - input_length );
    }
    else
    {
        SHA512_State s;
        SHA384_Init( &s );
        SHA384_Bytes( &s, input_key, input_length );
        SHA384_Final( &s, output_key );
        memset( output_key + SHA384_DIGEST_SIZE, '\0', KEY_IOPAD_SIZE128 - SHA384_DIGEST_SIZE );
    }
}

void hmac_sha384(unsigned char *key, int key_len, unsigned char *text, int text_len, unsigned char *hmac)
{
    unsigned char new_key[ KEY_IOPAD_SIZE128 ] = { 0 };
    normalize_key_sha384((char*)key, key_len, new_key);

    SHA512_State context;
    unsigned char k_ipad[KEY_IOPAD_SIZE128];
    unsigned char k_opad[KEY_IOPAD_SIZE128];
    int i;

    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, new_key, KEY_IOPAD_SIZE128);
    memcpy(k_opad, new_key, KEY_IOPAD_SIZE128);

    for (i = 0; i < KEY_IOPAD_SIZE128; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    SHA384_Init(&context);
    SHA384_Bytes(&context, k_ipad, KEY_IOPAD_SIZE128);
    SHA384_Bytes(&context, text, text_len);
    SHA384_Final(&context, hmac);

    SHA384_Init(&context);
    SHA384_Bytes(&context, k_opad, KEY_IOPAD_SIZE128);
    SHA384_Bytes(&context, hmac, SHA384_DIGEST_SIZE);
    SHA384_Final(&context, hmac);
}

static void normalize_key_sha512(const char* input_key, const int input_length, unsigned char* output_key)
{
    if( input_length <= KEY_IOPAD_SIZE128 )
    {
        memcpy( output_key, input_key, input_length );
        memset( output_key + input_length, '\0', KEY_IOPAD_SIZE128 - input_length );
    }
    else
    {
        SHA512_State s;
        SHA512_Init( &s );
        SHA512_Bytes( &s, input_key, input_length );
        SHA512_Final( &s, output_key );
        memset( output_key + SHA512_DIGEST_SIZE, '\0', KEY_IOPAD_SIZE128 - SHA512_DIGEST_SIZE );
    }
}

void hmac_sha512(unsigned char *key, int key_len, unsigned char *text, int text_len, unsigned char *hmac)
{
    unsigned char new_key[ KEY_IOPAD_SIZE128 ] = { 0 };
    normalize_key_sha512((char*)key, key_len, new_key);

    SHA512_State context;
    unsigned char k_ipad[KEY_IOPAD_SIZE128];
    unsigned char k_opad[KEY_IOPAD_SIZE128];
    int i;

    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, new_key, KEY_IOPAD_SIZE128);
    memcpy(k_opad, new_key, KEY_IOPAD_SIZE128);

    for (i = 0; i < KEY_IOPAD_SIZE128; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    SHA512_Init(&context);
    SHA512_Bytes(&context, k_ipad, KEY_IOPAD_SIZE128);
    SHA512_Bytes(&context, text, text_len);
    SHA512_Final(&context, hmac);

    SHA512_Init(&context);
    SHA512_Bytes(&context, k_opad, KEY_IOPAD_SIZE128);
    SHA512_Bytes(&context, hmac, SHA512_DIGEST_SIZE);
    SHA512_Final(&context, hmac);
}
