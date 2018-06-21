#ifndef _AES_CBC_H_
#define _AES_CBC_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
   #include <stdio.h>
   #include <string.h>
   #include <stdlib.h>
   #include <errno.h>
 */
void encrypt_cbcaes(char *in, char *out, char *key);
int decrypt_cbcaes(char *in, char *out, char *key);

char *decrypt_cbcaes_base64(char *in, char *key);
char *encrypt_cbcaes_base64(char *in, char *key);
#if __cplusplus
}
#endif

#endif /* _AES_CBC_H_ */
