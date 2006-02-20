/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#include "mapstr.h"

#include "open-enc.h"
#include "openssl/evp.h"

#define CIPHER_BLOCK 512
#define ENCRYPT		1
#define DECRYPT     0
int EVP_cipher(const char *passw, int cbpass, char *strin, int cbstr, int op, const char *cipher);

void SHA1_string(const char *strin, int cbstr, char *strout)
{
	SHA1((unsigned char *)strin, cbstr, (unsigned char *)strout);
}

void MD5_string(const char *strin, int cbstr, char *strout)
{
	MD5((unsigned char *)strin, cbstr, (unsigned char *)strout);
}

void SHA0_string(const char *strin, int cbstr, char *strout)
{
	SHA((unsigned char *)strin, cbstr, (unsigned char *)strout);
}

int EVP_encrypt(const char *passw, int cbpass, char *strin, int cbstr, const char *cipher)
{
	return EVP_cipher(passw, cbpass, strin, cbstr, ENCRYPT, cipher);
}

int EVP_decrypt(const char *passw, int cbpass, char *strin, int cbstr, const char *cipher)
{
	return EVP_cipher(passw, cbpass, strin, cbstr, DECRYPT, cipher);
}

static class qEvpMap : public CMapStr<const EVP_CIPHER *>
{
public:
	qEvpMap();
	~qEvpMap();
} myEvpMap;

qEvpMap::qEvpMap() {
	Set("des",  EVP_des_ede_cbc());
	Set("des3", EVP_des_ede3_cbc());
#if !defined(NO_IDEA) && !defined(OPENSSL_NO_IDEA)
	Set("idea", EVP_idea_cbc());
#endif
	Set("cast", EVP_cast5_cbc());
	Set("rc2",  EVP_rc2_cbc());
#if !defined(NO_RC5) && !defined(OPENSSL_NO_RC5)
	Set("rc5",  EVP_rc5_32_12_16_cbc());
#endif
	Set("bf",   EVP_bf_cbc());
}
qEvpMap::~qEvpMap()
{
	Clear();
}

int EVP_cipher(const char *passw, int cbpass, char *strin, int cbstr, int op, const char *cipher)
{
	const EVP_CIPHER * enc;
	if (cipher && *cipher) {
		if (!myEvpMap.Find(cipher,enc))
			return CIPHER_INVALID;
	} else 
		enc = EVP_des_ede_cbc();

	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH];
	EVP_CIPHER_CTX ctx;

	EVP_BytesToKey(enc,EVP_md5(),NULL,(unsigned char *)passw,cbpass,1,key,iv);
	EVP_CIPHER_CTX_init(&ctx);
	EVP_CipherInit(&ctx,enc,key,iv,op);
	
	unsigned char out[CIPHER_BLOCK+EVP_MAX_IV_LENGTH];
	int outl;

	unsigned char *in = (unsigned char *) strin;
	unsigned char *endp = in + cbstr - CIPHER_BLOCK;
	unsigned char *outp = in;

	while (in < endp)	{
		EVP_CipherUpdate(&ctx,out,&outl,in,CIPHER_BLOCK);
		memcpy(outp, out, outl); outp += outl;
		in += CIPHER_BLOCK;
	}
	EVP_CipherUpdate(&ctx,out,&outl,in,endp+CIPHER_BLOCK-in);
	memcpy(outp, out, outl); outp += outl;

	EVP_CipherFinal(&ctx,out,&outl);
	memcpy(outp, out, outl); outp += outl;

	EVP_CIPHER_CTX_cleanup(&ctx);
	return (char *)outp-strin;
}

int B64_decode(const char *strin, char *strout, int cbstr)
{
	CStr altin;
	if ((cbstr % 4 == 3) || (cbstr % 4 == 2)) {
		altin.Copy(strin, cbstr);
		if (cbstr % 4 <= 3) altin << '=';
		if (cbstr % 4 == 2) altin << '=';
		strin = altin.Data();
		cbstr = altin.Length();
	}

	int len = EVP_DecodeBlock((unsigned char*)strout, (unsigned char*)strin, cbstr);
	if ( cbstr && (len > 0) )
		{
		if (strin[cbstr-1] == '=') 
			{
			if (strin[cbstr-2] == '=')
				return(len-2);
			else 
				return(len-1);
			}
		}
	return len;
}

int B64_encode(const char *strin, char *strout, int cbstr)
{
	return EVP_EncodeBlock((unsigned char*)strout, (unsigned char*)strin, cbstr);
}

#define hex2i(c) ((c) <= '9' ? ((c) - '0') : (c) <= 'Z' ? ((c) - 'A' + 10) : ((c) - 'a' + 10))
void HEX_decode(const char *strin, char *strout, int cbstr)
{
	const char *r = strin;
	const char *e = r + cbstr - (cbstr % 2);
	unsigned char *w = (unsigned char *) strout;
	while (r < e) {
		*w++ = (hex2i(r[0])<<4) | (hex2i(r[1]));
		r+=2;
	}
}

#define i2hex(c) (assert((c) < 16),((c) < 10 ? ('0'+(c)) : ((c) - 10 + 'a')))
void HEX_encode(const char *strin, char *strout, int cbstr)
{
	const unsigned char *r = (unsigned char *) strin;
	const unsigned char *e = (unsigned char *) r + cbstr;
	char *w = strout;
	while (r < e) {
		*w++ = i2hex((*r >> 4)); 
		*w++ = i2hex(*r & 0xF);
		++r;
	}
#ifdef _DEBUG
	char *t = (char *) malloc(cbstr * 2);
	HEX_decode(strout, t, cbstr * 2);
	assert(!memcmp(strin, t, cbstr));
	free(t);
#endif
}

#ifdef _STR_H

CStr EVP_encrypt(CStr passw, CStr strin, const char *cipher)
{
	int len = strin.Length();
	if (len > 0) {
		strin.Grow(len + EVP_MAX_IV_LENGTH);
		return strin.Grow(
			EVP_encrypt(passw.SafeP(), passw.Length(), strin.GetBuffer(), len, cipher)
		);
	} else
		return strin;
}

CStr EVP_decrypt(CStr passw, CStr strin, const char *cipher)
{
	int len = strin.Length();
	if (len > 0 && passw.Length() > 0) {
		strin.Grow(len + EVP_MAX_IV_LENGTH);
		return strin.Grow(
			EVP_decrypt(passw.SafeP(), passw.Length(), strin.GetBuffer(), len, cipher)
		);
	} else
		return strin;
}

CStr SHA0_string(CStr strin)
{
	CStr strout(SHA_DIGEST_LENGTH);
	SHA0_string(strin, strin.Length(), strout.GetBuffer());
	return strout;
}

CStr SHA1_string(CStr strin)
{
	CStr strout(SHA_DIGEST_LENGTH);
	SHA1_string(strin, strin.Length(), strout.GetBuffer());
	return strout;
}

CStr MD5_string(CStr strin)
{
	CStr strout(MD5_DIGEST_LENGTH);
	MD5_string(strin, strin.Length(), strout.GetBuffer());
	return strout;
}

CStr HEX_encode(CStr strin)
{
	CStr strout(strin.Length() * 2);
	HEX_encode(strin, strout.GetBuffer(), strin.Length());
	return strout;
}

CStr HEX_decode(CStr strin)
{
	CStr strout((int) strin.Length() / 2);
	HEX_decode(strin, strout.GetBuffer(), strin.Length());
	return strout;
}


CStr B64_encode(CStr strin)
{
	if (!strin.IsEmpty()) {
		CStr strout(strin.Length() * 2);
		int len = B64_encode(strin, strout.GetBuffer(), strin.Length());
		if (len < 0) len = 0;
		return strout.Grow(len);
	} else 
		return strin;
}

CStr B64_decode(CStr strin)
{
	if (!strin.IsEmpty()) {
		CStr strout((int) strin.Length());
		int len = B64_decode(strin, strout.GetBuffer(), strin.Length());
		if (len < 0) len = 0;
		return strout.Grow(len);
	} else 
		return strin;
}

#endif // #ifdef _STR_H
