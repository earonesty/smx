/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _OPEN_ENC_H
#define _OPEN_ENC_H

#define BLOCK_SIZE  8192
#define CIPHER_INVALID -1

int  EVP_encrypt(const char *passw, int cbpass, char *strin, int cbstr, const char *cipher);
int  EVP_decrypt(const char *passw, int cbpass, char *strin, int cbstr, const char *cipher);
void HEX_string(const char *strin, char *strout, int cbstr);
int MD5_string(const char *strin, int cbstr, char *strout);
int SHA1_string(const char *strin, int cbstr, char *strout);

#ifdef _STR_H
	CStr EVP_encrypt(CStr passw, CStr strin, const char *cipher);
	CStr EVP_decrypt(CStr passw, CStr strin, const char *cipher);
	CStr SHA1_string(CStr strin);
	CStr MD5_string(CStr strin);

	CStr HEX_encode(CStr strin);
	CStr HEX_decode(CStr strin);
	CStr B64_encode(CStr strin);
	CStr B64_decode(CStr strin);
#endif // #ifdef _STR_H

#endif /* #ifndef _OPEN_ENC_H */
