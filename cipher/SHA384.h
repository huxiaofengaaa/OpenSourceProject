#ifndef SHA384_H
#define SHA384_H


#include "SHA512.h"
#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

void SHA384_Init(SHA512_State * s);
void SHA384_Bytes(SHA512_State * s, const void *p, int len);
void SHA384_Final(SHA512_State * s, unsigned char *output);
void SHA384_Simple(const void *p, int len, unsigned char *output);

#ifdef  __cplusplus
}
#endif /* __cplusplus */
#endif // SHA384_H
