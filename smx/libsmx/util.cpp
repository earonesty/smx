/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#include "stdafx.h"
#include "qstr.h"
#include "qctx.h"
#include "qfopen.h"
#include "util.h"
#include "crit.h"

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
		char *b = in.SafeP();
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

static FILE * gDebugLog = NULL;
static CCrit  gDebugLogCrit;
static const char * gDebugLogFile = DEFAULT_LOG;

static class gDebugLogTermClass {
public:
	~gDebugLogTermClass() {
		if (gDebugLog) fclose(gDebugLog); gDebugLog = NULL;
	 }
} gDebugLogTerm;

static void (*gDebugLogFunc)(const char *msg) = _log_debug;

void _log_debug(const char *msg) {
  int pid = (int) getpid();

  if (gDebugLog && ferror(gDebugLog)) {
        CLock lock(&gDebugLogCrit);
        fclose(gDebugLog);
        gDebugLog = NULL;
  }

  if (gDebugLog == NULL) {
        CLock lock(&gDebugLogCrit);
	gDebugLog = fopen(gDebugLogFile,"a");
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
	if (path) gDebugLogFile = path;
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
	struct stat s;
	if (!stat(path, &s) && ((unsigned int) uid == s.st_uid))
                return true;
#else
	return true;
#endif

	errno = EPERM;
	return false;
}

bool safe_fcheck(qCtx *ctx, const char *path)
{
        if (!path) return false;
        if (!ctx->GetSafeMode()) return true;
	
        int uid = ctx->GetSafeUID();
        if (uid == 0) return true;
        if (uid == -1) return false;

        CStr dir = path;
        char *pbs = strrchr(dir,'/');

        if (pbs)
		*pbs='\0';
        else
		dir = ".";

#ifdef unix
        struct stat s;
        if (!stat(dir, &s) && ((unsigned int) uid == s.st_uid))
                return true;

        errno = EPERM;
	return false;
#else
        return true;
#endif
}

FILE *safe_fopen(qCtx *ctx, const char *path, const char *mode)
{
	if (safe_fcheck(ctx, path))
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
                int add, off = pmy - tmp;
                memcpy((*out_p), tmp, off);

#ifdef WIN32
                add = FormatMessage(
                        FORMAT_MESSAGE_FROM_SYSTEM, 0, dy, 0, &((*out_p)[off]), 1024, 0);
#else
                const char *tbuf = strerror_r((int) dy, &((*out_p)[off]), 1024);
    add = strlen(tbuf);
    if (tbuf != &((*out_p)[off])) {
      strcpy(&((*out_p)[off]),tbuf);
    }
#endif
                memcpy(&((*out_p)[add+off]), pmy+2, len-off-2);
                len = add + len - 2;
                if (len > 0 && (*out_p)[len-1] == '\n') --len;
                if (len > 0 && (*out_p)[len-1] == '\r') --len;
                (*out_p)[len] = '\0';
                strcpy(tmp, (*out_p));
        }
// end support for the "%y" format flag

        vsnprintf((*out_p), max, tmp, (va_list) vargs);

        free(tmp);
}

