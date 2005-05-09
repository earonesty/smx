/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


// loads data from the resource fork
#ifndef _RES_H_
#define _RES_H_

long resLoad(long hres, long id, char **pBuf);
long resLoad(long hres, long id, char *buf, int buf_len);
long resInit(char *file);
void resTerm(long handle);

// global resource handle
extern long ghRes;

// resource errors
#define RES_ERRNOTFOUND -1
#define RES_ERRVERSION -2
#define RES_ERRREAD -3

#ifdef RES_INIT
	#undef RES_INIT
#endif

#if defined _WINDOWS || defined WIN32
	#ifdef DLL_NAME
		#define RES_INIT ghRes ? ghRes : (ghRes = resInit(DLL_NAME));
	#else
		#define RES_INIT ghRes ? ghRes : (ghRes = resInit(NULL));
	#endif
#else
	#define RES_INIT ghRes ? ghRes : (ghRes = resInit(argv[0]));
#endif

#if defined _WINDOWS || defined _WIN32
	#ifndef LITTLE_ENDIAN
		#define LITTLE_ENDIAN
	#endif	
#endif

#endif //#ifndef _RES_H_
