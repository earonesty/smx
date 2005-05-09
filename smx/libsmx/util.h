/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _UTIL_H
#define _UTIL_H

#include <time.h>

#ifdef WIN32 
	#ifndef TLS
		#define TLS _declspec(thread) 
	#endif
#endif

#ifdef unix
        #define DEFAULT_LOG "/tmp/smx_debug.log";
#else
        #define DEFAULT_LOG "c:/smx_debug.log"
#endif

#define SMXLOGLEVEL_NONE    0		// not logged
#define SMXLOGLEVEL_SEVERE  1		// severe error
#define SMXLOGLEVEL_ERROR   2		// error
#define SMXLOGLEVEL_WARNING 3		// warning
#define SMXLOGLEVEL_SCRIPT  4		// script-level error messages
#define SMXLOGLEVEL_DEBUG   5		// debugging messages
#define SMXLOGLEVEL_DEBUGV  6		// verbose debugging messages

CStr ReplaceStr(CStr in, CStr from, CStr to);
CStr ReplaceStrI(CStr in, CStr from, CStr to);

// reallocates the out_p and format's it using vsprintf with the addition of a %y which is GetLastError string
void exvsprintf(char **out_p, const char *msg_p, va_list vargs);

char *bslash(char *f);
char *fslash(char *f);
int    ParseInt(const CStr &str);
double ParseDbl(const CStr &str);
inline bool ParseBool(const char *value) {return (value && *value);}
time_t GetFileModified(const FILE *fp);

void smx_log_str(int level, const char *msg);
void smx_log(int level, const char *msg, ...);
void smx_log_v(int level, const char *msg, va_list vargs);
void smx_log_pf(int level, int err_id, const char *s1, const char *s2 = NULL, const char *s3 = NULL);
const char *smx_log_file(const char *file);
void *smx_log_func(void (*func)(const char *msg));
extern int smx_log_level;

class qCtx;

// get the uid for a script
int safe_getfileuid(const char *path);
// open a file, checking with safe_fcheck first
FILE *safe_fopen(qCtx *ctx, const char *path, const char *mode);
// check a directory for faux-ownership
bool safe_dcheck(qCtx *ctx, const char *path);
// check a file for faux-ownersip
bool safe_fcheck(qCtx *ctx, const char *path);

#endif // #ifndef _UTIL_H

