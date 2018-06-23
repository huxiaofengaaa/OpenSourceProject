#include "base64.h"
#include <stdlib.h>

static const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char getBase64Index(unsigned char base64)
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

char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode( const char * base64, unsigned char * bindata )
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}

char* base64_encode_v2(const char* input, const int size)
{
	int i , j;
	if(input == NULL || size <= 0)
	{
		return NULL;
	}
	int base64_block = ((int)(size * 8 / 6)) + ((size * 8 % 6 == 0) ? 0 : 1);
	int base64_size = ((int)(base64_block / 4) + ((base64_block % 4 == 0) ? 0 : 1)) * 4;

	char* base64 = (char*)malloc(base64_size + 1);
	if(base64 == NULL)
	{
		return NULL;
	}
	memset(base64, 0 ,base64_size + 1);
	
	for(i = 0, j = 0 ; i < size, j < base64_size ; i += 3)
	{
		unsigned char ch1 = 0;
		unsigned char ch2 = 0;
		unsigned char ch3 = 0;
		if(i < size)
		{
			ch1 = (unsigned char)input[i];
			base64[j++] = base64char[(unsigned char)((ch1 >> 2) & 0x3F)];
		}
		else
		{
			break;
		}

		if(i + 1 < size)
		{
			ch2 = (unsigned char)input[i + 1];
			base64[j++] = base64char[(unsigned char)(((ch1 << 4) | (ch2 >> 4)) & 0x3F)];
		}
		else
		{
			base64[j++] = base64char[(unsigned char)((ch1 << 4) & 0x3F)];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}

		if(i + 2 < size)
		{
			ch3 = (unsigned char)input[i + 2];
			base64[j++] = base64char[(unsigned char)(((ch2 << 2) | (ch3 >> 6)) & 0x3F)];
			base64[j++] = base64char[(unsigned char)(ch3 & 0x3F)];
		}
		else
		{
			base64[j++] = base64char[(unsigned char)((ch2 << 2) & 0x3F)];
			base64[j++] = '=';
			break;
		}
	}
	base64[j] = '\0';
	return base64;
}

char* base64_decode_v2(const char* base64, const int size)
{
    int i , j;
    if(base64 == NULL || size <= 0)
    {   
        return NULL;
    }

	int base64_size = ((int)(size / 4) + (size % 4 == 0 ? 0 : 1)) * 4;
	int src_size = base64_size * 6 / 8 + 1;
	
	char* srcptr = (char*)malloc(src_size + 1);
	if(srcptr == NULL)
	{
		return NULL;
	}
	memset(srcptr, 0, src_size + 1);
	
	for(i = 0, j = 0 ; i < base64_size, j < src_size; i += 4)
	{
		unsigned char temp[4];
		unsigned char temp_index;
		memset( temp, 0, sizeof(temp) );

		temp_index = 0;
		if(i + temp_index < size && base64[i + temp_index] != '=')
		{
			temp[temp_index] = getBase64Index(base64[i + temp_index]);
		}
		else
		{
			break;
		}

		temp_index = 1;
		if(i + temp_index < size && base64[i + temp_index] != '=')
		{
			temp[temp_index] = getBase64Index(base64[i + temp_index]);
			srcptr[j++] = (unsigned char)(((temp[0] << 2) | (temp[1] >> 4)) & 0xFF);
		}
		else
		{
			srcptr[j++] = (unsigned char)(((temp[0] << 2)) & 0xFF);
			break;
		}

		temp_index = 2;
		if(i + temp_index < size && base64[i + temp_index] != '=')
		{
			temp[temp_index] = getBase64Index(base64[i + temp_index]);
			srcptr[j++] = (unsigned char)(((temp[1] << 4) | (temp[2] >> 2)) & 0xFF);
		}
		else
		{
			srcptr[j++] = (unsigned char)(((temp[1] << 4)) & 0xFF);
			break;
		}

		temp_index = 3;
		if(i + temp_index < size && base64[i + temp_index] != '=')
		{
			temp[temp_index] = getBase64Index(base64[i + temp_index]);
			srcptr[j++] = (unsigned char)(((temp[2] << 6) | (temp[3] >> 0)) & 0xFF);
		}
		else
		{
			srcptr[j++] = (unsigned char)(((temp[2] << 6)) & 0xFF);
			break;
		}
	} 
	srcptr[j] = '\0';
	return srcptr;
}

