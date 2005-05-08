/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _OPEN_ENC_H
#define _OPEN_ENC_H

#define BLOCK_SIZE  8192
#define CIPHER_INVALID -1

void SHA1_string(const char *strin, int cbstr, char *strout);
void SHA0_string(const char *strin, int cbstr, char *strout);
int  EVP_encrypt(const char *passw, int cbpass, char *strin, int cbstr, const char *cipher);
int  EVP_decrypt(const char *passw, int cbpass, char *strin, int cbstr, const char *cipher);
void HEX_string(const char *strin, char *strout, int cbstr);

#ifdef _STR_H
	CStr EVP_encrypt(CStr passw, CStr strin, const char *cipher);
	CStr EVP_decrypt(CStr passw, CStr strin, const char *cipher);
	CStr SHA1_string(CStr strin);

	CStr HEX_encode(CStr strin);
	CStr HEX_decode(CStr strin);
	CStr B64_encode(CStr strin);
	CStr B64_decode(CStr strin);
#endif // #ifdef _STR_H

#endif /* #ifndef _OPEN_ENC_H */
