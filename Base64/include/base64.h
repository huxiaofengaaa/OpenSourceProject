#ifndef __BASE64_H_
#define __BASE64_H_

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

char * base64_encode( const unsigned char * bindata, char * base64, int binlength );
int base64_decode( const char * base64, unsigned char * bindata );

char* base64_encode_v2(const char* input, const int size);
char* base64_decode_v2(const char* base64, const int size);

#ifdef __cplusplus
}
#endif

#endif
