 /*
 * aes128.h
 *
 *  Created on: 2022Äê10ÔÂ19ÈÕ
 *      Author: lwp edit
 */
#ifndef __AES128_H__
#define __AES128_H__

#ifdef __cplusplus
extern "C" {
#endif
/*
********************************************************************************
********************************************************************************
*/
void aes_init(unsigned int* key);

void aes_encrypt(unsigned int* in, unsigned int* key, unsigned int* out);

void aes_decrypt(unsigned int* in, unsigned int* key, unsigned int* out);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __AES128_H__

