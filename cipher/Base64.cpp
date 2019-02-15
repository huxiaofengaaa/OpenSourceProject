#include "Base64.h"
#include <string.h>

static const unsigned char base64char[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int getBase64Index(unsigned char base64)
{
    for(unsigned char k = 0 ; k < 63 ; k++)
    {
        if(base64 == base64char[k])
        {
            return k;
        }
    }
    return 0xFF;
}

int encode_require_size(const int input_size)
{
    if(input_size % 3 == 0)
    {
        return input_size / 3 * 4;
    }
    else
    {
        return (input_size / 3) * 4 + 4;
    }
}

int decode_require_size(const int input_size)
{
    if(input_size % 4 == 0)
    {
        return input_size / 4 * 3;
    }
    else
    {
        return -1;
    }
}

/**
 * base64_encode - Base64 encode
 * @src:         - Data to be encoded
 * @len:         - Length of the data to be encoded
 * @out_len:     - Pointer to output length variable, or %NULL if not used
 * Returns:      - Allocated buffer of out_len bytes of encoded data, or %NULL on failure
 */
unsigned char * base64_encode(const unsigned char *input, size_t input_size, size_t *out_len)
{
    int outsize = encode_require_size(input_size);
    unsigned char* output = (unsigned char*)new char[outsize];
    if(output == NULL)
    {
        *out_len = 0;
        return NULL;
    }
    memset(output, 0, outsize);

    int i = 0;
    int j = 0;
    for(i = 0 ; (i + 2) < input_size ; i += 3)
    {
        output[j++] = base64char[(input[i] >> 2) & 0x3F];
        output[j++] = base64char[((input[i] << 4) | (input[i+1] >> 4)) & 0x3F];
        output[j++] = base64char[((input[i+1] << 2) | input[i+2] >> 6) & 0x3F];
        output[j++] = base64char[(input[i+2]) & 0x3F];
    }
    switch(input_size % 3)
    {
    case 1:
    {
        output[j++] = base64char[(input[i] >> 2) & 0x3F];
        output[j++] = base64char[(input[i] << 4) & 0x3F];
        output[j++] = '=';
        output[j++] = '=';
        break;
    }
    case 2:
    {
        output[j++] = base64char[(input[i] >> 2) & 0x3F];
        output[j++] = base64char[((input[i] << 4) | (input[i+1] >> 4)) & 0x3F];
        output[j++] = base64char[(input[i+1] << 2) & 0x3F];
        output[j++] = '=';
        break;
    }
    }
    *out_len = outsize;
    return output;
}


/**
 * base64_decode - Base64 decode
 * @src:         - Data to be decoded
 * @len:         - Length of the data to be decoded
 * @out_len:     - Pointer to output length variable
 * Returns:      - Allocated buffer of out_len bytes of decoded data, or %NULL on failure
 */
unsigned char * base64_decode(const unsigned char *input, size_t input_size, size_t *out_len)
{
    if(input_size % 4 != 0)
    {
        return NULL;
    }

    int outsize = decode_require_size(input_size);
    unsigned char* output = (unsigned char*)new char[outsize];
    if(output == NULL)
    {
        *out_len = 0;
        return NULL;
    }
    memset(output, 0, outsize);

    int i = 0;
    int j = 0;
    for(; (i + 3) < input_size; i += 4)
    {
        unsigned char ch1 = getBase64Index(input[i + 0]);
        unsigned char ch2 = getBase64Index(input[i + 1]);
        unsigned char ch3 = getBase64Index(input[i + 2]);
        unsigned char ch4 = getBase64Index(input[i + 3]);

        if(input[i + 0] == '=')
        {
            break;
        }

        if(input[i + 1] == '=')
        {
            break;
        }
        else
        {
            output[j++] = (((ch1 << 2) & 0xFC) | ((ch2 >> 4) & 0x3)) & 0xFF;
        }

        if(input[i + 2] == '=')
        {
            break;
        }
        else
        {
            output[j++] = (((ch2 << 4) & 0xF0) | ((ch3 >> 2) & 0x0F)) & 0xFF;
        }

        if(input[i + 3] == '=')
        {
            break;
        }
        else
        {
            output[j++] = (((ch3 << 6) & 0xC0) | ((ch4 >> 0) & 0x3F)) & 0xFF;
        }
    }
    *out_len = outsize;
    return output;
}
