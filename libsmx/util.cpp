/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "stdafx.h"
#include "qstr.h"
#include "qctx.h"
#include "qfopen.h"
#include "util.h"
#include "crit.h"

#include <errno.h>

static void _log_debug(const char *msg);
static char *stristr(char *a, int la, const char *b, int lb);

int smx_log_level = SMXLOGLEVEL_WARNING;

static char *stristr(char *a, int la, const char *b, int lb)
{
	if (la <= 0 || la < lb || !a || !b)
		return NULL;

	char *e = a + la - lb + 1;
	while (a < e) {
  if (!memicmp(a,b,lb)) {
			return a;
		}
		++a;
	}
	return NULL;
}

CStr ReplaceStr(CStr in, CStr from, CStr to)
{
	int fl = from.Length(), tl = to.Length(), il = in.Length();

	if (fl <= 0 || fl > il)
		return in;

	char *p;
	if (fl==tl) {
		while ((p = strstr(in.SafeP(), from))) {
			memcpy(p, to.Data(), tl);
		}
		return in;
	} else {
		CStr res;
		char *b = (char *) in.SafeP();
		char *i = b;
		while ((p = strstr(i, from))) {
			res.Append(i, p - i);
			if (tl > 0)
				res << to;
			i = p + fl;
		}
		if (i != in.Data() && ((il - (i - b)) > 0)) {
			res.Append(i, il - (i - b));
			return res;
		} else {
			return in;
		}
	}
}

CStr ReplaceStrI(CStr in, CStr from, CStr to)
{
	char *p;
	int fl = from.Length(), tl = to.Length(), il = in.Length();

	if (fl <= 0 || fl > il)
		return in;


	if (fl == tl) {
		char *i = in.SafeP();
		char *b = i;
		while ((p = stristr(i, (il-(i-b)), from, fl))) {
			memcpy(p, to.Data(), tl);
			i = p + fl;
		}
		return in;
	} else {
		CStr res;
		char *i = in.SafeP();
		while ((p = stristr(i, il-(i-in.Data()), from, fl))) {
			res.Append(i, p - i);
			if (tl > 0)
				res << to;
			i = p + fl;
		}
		if (i != in.Data()) {
			res.Append(i, il - (i - in.Data()));
			return res;
		} else {
			return in;
		}
	}
}

char *bslash(char *f) {
	char *s = f;
	if (s) while(*s) {
		if (*s=='/') *s='\\'; ++s;
	}
	return s;

}

char *fslash(char *f) {
	char *s = f;
	if (s) while(*s) {
		if (*s=='\\') *s='/'; ++s;
	}
	return s;
}

int ParseInt(const CStr &str) {
	return atoi(str.SafeP());
}

double ParseDbl(const CStr &str) {
	char *ep;
	return strtod(str.SafeP(), &ep); 
}


#if defined(_WINDOWS) || defined(WIN32)
time_t GetFileModified(const FILE *fp)
{
        HANDLE hF = (HANDLE) _get_osfhandle(fp->_file);
        FILETIME ft; FILETIME lt;
        if (GetFileTime(hF,NULL, NULL, &ft)) {
                FileTimeToLocalFileTime(&ft,&lt);
                return ConvWin32Time(lt);
        }
        return 0;
}
#else
time_t GetFileModified(const FILE *fp)
{
        struct stat sb;
        if (!fstat(fileno((FILE *)fp), &sb)) {
                return sb.st_mtime;
        }
        return 0;
}
#endif

static CStr set_default_log();
static FILE * gDebugLog = NULL;
static CCrit  gDebugLogCrit;
static CStr   gDebugLogFile = set_default_log();

static class gDebugLogTermClass {
public:
	~gDebugLogTermClass() {
		if (gDebugLog) fclose(gDebugLog); gDebugLog = NULL;
	 }
} gDebugLogTerm;

static void (*gDebugLogFunc)(const char *msg) = _log_debug;

static CStr set_default_log() {
	CStr log = getenv("SMX_LOG");

#ifdef unix
	if (log.IsEmpty()) {
		const char * varlog = "/var/log/smx_debug.log";
		FILE * t = fopen(varlog, "a");
		if (t) {
			log = varlog;
			fclose(t);
		}
	}
#endif

	if (log.IsEmpty()) {
                const char * tmp = getenv("tmp");
                tmp = getenv("temp");
		if (tmp) {
			log = tmp;
			log += "/smx_debug.log";
		}
        }

        if (log.IsEmpty()) {
                const char * tmp = getenv("HOME");
                if (tmp) {
                        log += "/smx_debug.log";
                }
        }

	return log;
}

void _log_debug(const char *msg) {
  int pid = (int) getpid();

  if (gDebugLog && ferror(gDebugLog)) {
        CLock lock(&gDebugLogCrit);
        fclose(gDebugLog);
        gDebugLog = NULL;
  }

  if (gDebugLog == NULL) {
	if (gDebugLogFile) {
	        CLock lock(&gDebugLogCrit);
#ifdef unix
  		int um = umask(0027);
#endif
		gDebugLog = fopen(gDebugLogFile,"a");
#ifdef unix
  		umask(um);
#endif
	}
  }

  if (gDebugLog) {
	time_t tmt = time(0);
	struct tm * tms = localtime(&tmt);
	qStrBuf buf;
	FmtTime(tms, "yyyy-mm-dd hh:nn:ss", &buf);
	fprintf(gDebugLog, "%d\t%s\t%s\n", pid, (const char *) buf, msg);
	fflush(gDebugLog);
  }
}

void smx_log_str(int level, const char *msg) {
        if (level > smx_log_level)
                return;
	gDebugLogFunc(msg);
}

void smx_log(int level, const char *msg, ...) {
	if (level > smx_log_level) 
		return;
	va_list vargs;
	va_start(vargs, msg);
	smx_log_v(level, msg, vargs);
	va_end(vargs);
}

void smx_log_v(int level, const char *msg, va_list vargs) {
        if (level > smx_log_level)
                return;
        char *tmp=NULL;
        exvsprintf(&tmp, msg, vargs);
        gDebugLogFunc(tmp);
        free(tmp);
}

#ifdef WIN32
#define snprintf _snprintf
#endif

void smx_log_pf(int level, int err_id, const char *s1, const char *s2, const char *s3) {
        if (level > smx_log_level)
                return;
        char tmp[2048];
        snprintf(tmp, 2048, "%d\t%s\t%s\t%s", err_id, s1?s1:"", s2?s2:"", s3?s3:"");
        gDebugLogFunc(tmp);
}


const char *smx_log_file(const char *path)
{
	const char *ret = gDebugLogFile;
	if (path && *path) gDebugLogFile = path;
	return ret;
}

void *smx_log_func(void (*func)(const char *path))
{
	void *ret = (void *) gDebugLogFunc;
	if (func) gDebugLogFunc = func;
	return ret;
}

int safe_getfileuid(const char *path)
{
        if (!path) return false;

        CStr dir = path;
        char *pbs = strrchr(dir,'/');

        if (pbs)
                *pbs='\0';
        else
                dir = ".";

#ifdef unix
        struct stat s;
        if (!stat(dir, &s) && s.st_uid >= 500)
		return s.st_uid;
        else 

        return -1;
#else
        return -1;
#endif
}

bool safe_dcheck(qCtx *ctx, const char *path)
{
	if (!path) return false;
	if (!ctx->GetSafeMode()) return true;

	int uid = ctx->GetSafeUID();
	if (uid == 0) return true;
	if (uid == -1) return false;

#ifdef unix
        if (!strnicmp(path,"/tmp",4)) {
                errno = EPERM;
                return false;
        }
	struct stat s;
	if (!stat(path, &s) && ((unsigned int) uid == s.st_uid))
                return true;
#else
	return true;
#endif

	errno = EPERM;
	return false;
}

bool safe_fcheck(qCtx *ctx, const char *path, char rw)
{
        if (!path) return false;
        if (!ctx->GetSafeMode()) return true;
	
        int uid = ctx->GetSafeUID();
        if (uid == 0) return true;
	//printf("safe_uid: %d\n", uid);
        if (uid == -1)  {
		// script owner user id is probably too low
		errno = EPERM;
		return false;
	}

        CStr dir = path;
        char *pbs = strrchr(dir,'/');

        if (pbs)
		*pbs='\0';
        else
		dir = ".";

#ifdef unix
/*
	char *fullp = (char *) malloc(PATH_MAX);
	realpath(path, fullp);
	if (!strnicmp(fullp,"/tmp",4)) {
		errno = EPERM;
		return false;
	}
	free(fullp);
*/
        struct stat s;
        if (!stat(dir, &s) && ((unsigned int) uid == s.st_uid))
                return true;

        if ( (rw == 'r') && ((s.st_mode & S_IROTH) && (s.st_mode & S_IXOTH)) )
		return true;

        errno = EPERM;
	return false;
#else
        return true;
#endif
}

FILE *safe_fopen(qCtx *ctx, const char *path, const char *mode)
{
	
	if (safe_fcheck(ctx, path, ((mode && (mode[0] == 'r')) ? 'r' : 'w')))
		return fopen(path, mode);
	else
		return NULL;
}

void exvsprintf(char **out_p, const char *msg_p, va_list vargs)
{
        int len = strlen(msg_p);
        int max = len + 1024;
        char *tmp = (char *) malloc(max);
        strcpy(tmp, msg_p);

// support for the "%y" format flag
        int   s;
        char *pm = tmp;
        char *pa = (char *) (void *) vargs;
        char *py = 0;

        char *pmy = 0;
        DWORD dy = 0;

        while (*pm) {
                if (*pm == '%') {
                        ++pm;
                        if (!py && (*pm == 'y')) {
                                pmy = pm-1;
                                py = pa;
                                dy = *((DWORD *) py);
                                pa += sizeof(DWORD);
                        } else {
                                while (*pm && !isalpha(*pm) && (*pm != 'L') && (*pm != 'h') && (*pm != 'I')) ++ pm;
                                s = 0;
                                switch (*pm) {
                                        case 'c':
                                                s = sizeof(char); break;
                                        case 'C':
                                                s = sizeof(wint_t); break;
                                        case 'd':case 'i':case 'o':case 'u':case 'x':case 'X':
                                                s = sizeof(int); break;
                                        case 'e':case 'E':case 'f':case 'g':case 'G':
                                                s = sizeof(double); break;
                                        case 'n':case 'p':case 'S':case 's':
                                                s = sizeof(void *); break;
                                }
                                if (py) {
                                        memcpy(py, pa, s);
                                        py += s;
                                }
                                pa += s;
                        }
                }
                pm++;
        }

        (*out_p) = (char *) realloc((*out_p), max);

        if (pmy) {
                int add = 0, off = pmy - tmp;
                memcpy((*out_p), tmp, off);

#ifdef WIN32
                add = FormatMessage(
                        FORMAT_MESSAGE_FROM_SYSTEM, 0, dy, 0, &((*out_p)[off]), 1024, 0);
#else
                char *obuf = &((*out_p)[off]); 
#ifdef STRERROR_R_CHAR_P
                char * ret = strerror_r((int) dy, obuf, 1024);
                if (ret) {
			if (ret != obuf) {
				strcpy(obuf, ret);
			}
                	add = strlen(obuf);
                }
#else
		int ret = strerror_r((int) dy, obuf, 1024);
		if (ret) {
			add = strlen(obuf);
		}
#endif
#endif
                memcpy(&((*out_p)[add+off]), pmy+2, len-off-2);
                len = add + len - 2;
                if (len > 0 && (*out_p)[len-1] == '\n') --len;
                if (len > 0 && (*out_p)[len-1] == '\r') --len;
                (*out_p)[len] = '\0';
                strcpy(tmp, (*out_p));
        }
// end support for the "%y" format flag

#ifdef WIN32
        _vsnprintf((*out_p), max, tmp, vargs);
#else 
        vsnprintf((*out_p), max, tmp, vargs);
#endif

		free(tmp);
}

#ifdef WIN32
int setenv(const char *name, const char *value, int overwrite)
{
	if (!overwrite &&  getenv(name)) return 0;
	char *buf=(char *) malloc(strlen(name)+strlen(value)+2);
	if (!buf) return errno;
	sprintf(buf, "%s=%s", name, value);
	int r = putenv(buf);
	free(buf);
	return r;
}
#endif

