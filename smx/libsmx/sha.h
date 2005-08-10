/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef SHA_H
#define SHA_H

/* NIST Secure Hash Algorithm */
/* heavily modified from Peter C. Gutmann's implementation */

/* Useful defines & typedefs */

#ifndef _WINDOWS_
typedef unsigned char BYTE;
typedef long int LONG;
#endif

#define SHA_BLOCKSIZE       64
#define SHA_DIGESTSIZE      20

typedef struct {
    LONG digest[5];     /* message digest */
    LONG count_lo, count_hi;    /* 64-bit bit count */
    LONG data[16];      /* SHA data buffer */
} SHA_INFO;

void sha_init(SHA_INFO *);
void sha_update(SHA_INFO *, BYTE *, int);
void sha_final(SHA_INFO *);

void sha_string(SHA_INFO *, const char *, int);
void sha_print(SHA_INFO *);

#define USE_MODIFIED_SHA 1

#endif /* SHA_H */
