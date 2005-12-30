/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#include "qctx.h"
#include "qobj-ctx.h"
#include "qobj-cache.h"
#include "qfopen.h"
#include "util.h"

#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#endif

#include <errno.h>

#ifdef WIN32
class DIRSTATE {
public:
	CStr path;
	WIN32_FIND_DATA data;
	DIRSTATE() {
		memset(&data, 0, sizeof(data));
	}
};
#else

enum DIRSTATUS
{
  DIR_EMPTY = 0,
  DIR_FULL = 1,
  DIR_ERR = 2
};


struct DIRSTATE {
	char *found;
	DIRSTATUS status;
	struct stat data;
};
#endif

#ifdef WIN32
#define DIRSEP ((char) '\\')
#else
#define DIRSEP ((char) '/')
#endif

#define ISDIRSEP(c) ((c)=='/'||(c) =='\\')
#define ISPATHSEP(c) ((c)=='/'||(c) =='\\'||(c) ==':')

bool ScanDir(CStr path, int mask, CStr body, qCtx *ctx, qStr *out);
bool _ScanDir(CStr path, int mask, CStr body, qCtx *ctx, qStr *out, DIRSTATE &st);

void EvalExt(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("ext", 1, 1);
	CStr path = (*args)[0];
	if (path) { 
	char *r = strrchr((const char *)path, '.');
	if (r) {
		out->PutS(r+1);
	}}
}

#ifndef WIN32
void EvalUmask(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        VALID_ARGC("umask", 2, 2);
        CStr mask_str = (*args)[0];
        CStr body = (*args)[1];

        int mask = strtol(mask_str.SafeP(),(char **)NULL, 0);

	if (args->Count() > 1) {
	        mode_t prev_mask;
	        prev_mask = umask((mode_t)mask);
		try {
			ctx->Parse(body, out);
		} catch (qCtxEx ex) {
			umask(prev_mask);
			throw ex;
		} 
		umask(prev_mask);
	} else {
	        if (!ctx->GetSafeMode()) {
			umask((mode_t)mask);
		} else {
			ctx->ThrowF(out, 619, "Permission denied");
		}
	}
}
#endif

void EvalBase(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("base", 1, 1);
	CStr path = (*args)[0];
	if (path){
	char *r = strrchr((const char *)path, '.');
	if (r) {
		*r = '\0';
		path.Grow(r - (const char *)path);
	}
	out->PutS(path);
	}
}

void EvalFileName(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("filename", 1, 1);
	CStr path = (*args)[0];

	if (path) {
	const char *b = path;

	const char *r = b + path.Length() - 1;
	while (r >= b && !(ISPATHSEP(*r))) {
		--r;
	}
	++r;
	out->PutS(r, path.Length() - (r - b));
	}
}
void EvalFilePath(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("filepath", 1, 1);
	CStr path = (*args)[0];
	char *b = path.GetBuffer();

	if (b) {
	char *r = b + path.Length() - 1;
	while (r >= b && !(ISPATHSEP(*r))) {
		--r;
	}
	if (r > b) {
		*r++ = DIRSEP;
		*r = '\0';
		path.Grow(r - b);
		out->PutS(path);
	}
	}
}

void EvalMakePath(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("makepath", 2, 2);
	CStr path1 = (*args)[0];
	CStr path2 = (*args)[1];
	fslash(path1.SafeP());
	fslash(path2.SafeP());
#ifdef WIN32
	path1.RTrim(DIRSEP);
	path2.LTrim(DIRSEP);
	path1.RTrim('/');
	path2.LTrim('/');
#else
	path1.RTrim(DIRSEP);
	path2.LTrim(DIRSEP);
#endif

	out->PutS(path1);

#ifdef WIN32
	if (path1.Length() > 0 && !ISPATHSEP(path1.SafeP()[path1.Length()-1]))
#endif
		out->PutC(DIRSEP);
	out->PutS(path2);
}

void EvalFName(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fname", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;
#ifdef WIN32
	out->PutS(state->data.cFileName);
#else
  char *p = strrchr(state->found,'/');
  if (p)
    out->PutS(p+1);
  else
    out->PutS(state->found);
#endif
}

void EvalFPath(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fpath", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;

#ifdef WIN32
	out->PutS(state->path);
	out->PutS(state->data.cFileName);
#else
	out->PutS(state->found);
#endif
}

#ifdef unix
bool GetDirStatus(DIRSTATE *state)
{
  if (state->status==DIR_EMPTY) {
    if (stat(state->found, &state->data))
      state->status = DIR_ERR;
    else
      state->status = DIR_FULL;
  }
  return (state->status == DIR_FULL);
}
#endif

void EvalFAttr(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fattr", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;
#ifdef WIN32
	out->PutN((int)state->data.dwFileAttributes);
#else
	if (GetDirStatus(state)) {
    out->PutN((int)state->data.st_mode);
  }
#endif
}

void EvalFIsDir(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("isdir", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;
#ifdef WIN32
  	if((int)state->data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
#else
	  if (GetDirStatus(state) && S_ISDIR(state->data.st_mode))
#endif
		out->PutC('T');
    
}

void EvalFExt(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fattr", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;
	qArgAry name(1);
#ifdef WIN32

	name[0] = state->data.cFileName;
#else
	name[0] = state->found;
#endif

	EvalExt(data, ctx, out, &name);
}

void EvalFSize(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fsize", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;

#ifdef WIN32
  if (state->data.nFileSizeHigh)
		out->PutN((double) state->data.nFileSizeHigh * (double) MAXDWORD + (double) state->data.nFileSizeLow);
	else
		out->PutN((unsigned int)state->data.nFileSizeLow);
#else
	if (GetDirStatus(state)) {

    out->PutN((int)state->data.st_size);
  }
#endif
}

void EvalFMtime(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fmtime", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;
#ifdef WIN32
	if (state->data.ftLastWriteTime.dwLowDateTime>0) {
		out->PutN(ConvWin32Time(state->data.ftLastWriteTime));
	}
#else
	if (GetDirStatus(state)) {
    out->PutN((int)state->data.st_mtime);
  }
#endif
}

void EvalFCtime(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        VALID_ARGC("fctime", 0, 0);
        DIRSTATE *state = (DIRSTATE *) data;
#ifdef WIN32
        if (state->data.ftCreationTime.dwLowDateTime>0) {
                out->PutN(ConvWin32Time(state->data.ftCreationTime));
        }
#else
        if (GetDirStatus(state)) {
    out->PutN((int)state->data.st_ctime);
  }
#endif
}


#ifdef unix
void EvalFMode(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("fmtime", 0, 0);
	DIRSTATE *state = (DIRSTATE *) data;
	if (GetDirStatus(state)) {
    out->PutN((int)state->data.st_mode);
  }
}
#endif

#ifdef WIN32
void EvalVolumes(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("volumes", 1, 1);
	CStr body = args->GetAt(1);
}
#endif

void EvalDir(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("dir", 2, 3);

	CStr path = (*args)[0];

	if (!path) {
		ctx->ThrowF(out, 601, "Empty directory path is invalid");
		return;
	}

	if (!safe_fcheck(ctx, path)) {
		ctx->ThrowF(out, 601, "Failed to open directory. %y", GetLastError());
		return;
	}

	CStr body = args->GetAt(1);
	int mask = 0;
	if (args->Count() > 2) {
		mask = ParseInt((*args)[2]);
	}

	if (!mask)
		mask = -1;

	ScanDir(path, mask, body, ctx, out);
}

void EvalFileFlush(const void *file, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("flush", 0, 0);
	fflush((FILE*)file);
}

void WriteCSV(const void *mode, bool forceQuoted, qCtx *ctx, qStr *out, qArgAry *args);


void EvalFileWriteCSVq(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	WriteCSV(mode, true, ctx, out, args);
}

void EvalFileWriteCSV(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	WriteCSV(mode, false, ctx, out, args);
}

void WriteCSV(const void *mode, bool forceQuoted, qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr path = ctx->ParseStr((*args)[0]);

	if (path.IsEmpty())
		return;
	
	FILE *fp = safe_fopen(ctx, path, (const char *) mode);

	if (!fp) {
		ctx->ThrowF(out, 601, "Failed to open file for writing. %y", GetLastError());
		return;
	}

	qStrFileO fo(fp, true);
	CStr bo;
	qStrBuf quot;

	qCtxTmp sub(ctx);
	sub.MapObj(fp, EvalFileFlush,"flush");

	int i;
	char *p, *b;
	for (i = 1; i < args->Count(); ++i) {
		bo = sub.ParseStr((*args)[i]);
		quot.Clear();

		b = bo.GetBuffer();
		if ((p = strchr((const char *)b, '"'))) {
			quot.PutC('"');
			do {
				quot.PutS(b, p - b + 1);
				quot.PutS('"');
			} while ((p = strchr((const char *)b, '"')));
			quot.PutC('"');
			fo.PutS(quot);
		} else if (forceQuoted || (p = strchr((const char *)b, ','))) {
			quot.PutC('"');
			quot.PutS(bo);
			quot.PutC('"');
			fo.PutS(quot);
		} else
			fo.PutS(bo);
		if (i < (args->Count()-1) ) {
			fo.PutC(',');
		}
	}
	fo.PutC('\n');
}

void EvalFileWrite(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr path = ctx->ParseStr((*args)[0]);
	FILE *fp;
	int err = 0;

#ifndef WIN32
	CStr perm_str = ctx->ParseStr((*args)[2]);
	int perms = strtol(perm_str.SafeP(),(char **)NULL, 0);
	mode_t prev_perms;
	if (perms) {
		prev_perms = umask((mode_t)~perms);
		printf("umask: %d (%d)\n", perms, prev_perms);
	}
	try {	
#endif

	if (path.IsEmpty())
		return;

	fp = safe_fopen(ctx, path, (const char *) mode);

	if (!fp) err = GetLastError();

#ifndef WIN32
	} catch (qCtxEx ex) {
	        if (perms) umask(prev_perms);
		throw ex;
	}
        if (perms) umask(prev_perms);
#endif

	if (!fp) {
		ctx->ThrowF(out, 601, "Failed to open file for writing. %y", err);
		return;
	}

	qStrFileO fo(fp, true);

	qCtxTmp sub(ctx);
	sub.MapObj(fp, EvalFileFlush,"flush");
	sub.Parse(args->GetAt(1), &fo);
}

void EvalFileDelete(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr path = (*args)[0];
	if (path.IsEmpty())
		return;

	if (!safe_fcheck(ctx, path)) {
		ctx->ThrowF(out, 632, "Error deleting file, %y", GetLastError());
		return;
	}
	remove(path);
}

void EvalFileCopy(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("copy", 2, 2);

	CStr from = (*args)[0];
	CStr to = (*args)[1];

	FILE *fpi = safe_fopen(ctx, from, "rb");

	if (!fpi) {
		ctx->ThrowF(out, 603, "Failed to open file for reading.");
		return;
	}
	FILE *fpo = safe_fopen(ctx, to, "wb");
	if (!fpo) {
		ctx->ThrowF(out, 602, "Failed to open file for copy.");
		return;
	}

	qStrFileO fo(fpo, true);
	qStrFileI fi(fpi, true);
	CStr s;
	while (!(s = fi.GetS()).IsEmpty()) {
		fo.PutS(s);
	}
}

void EvalFileRename(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("rename", 2, 3);
	CStr path = (*args)[0];
	CStr path_new = (*args)[1];

	if (!safe_fcheck(ctx, path)) {
		ctx->ThrowF(out, 632, "Access denied to source");
		return;
	}

	if (!safe_fcheck(ctx, path_new)) { 
		ctx->ThrowF(out, 632, "Access denied to destination");
		return;
	}

#ifdef WIN32
	bool clobber = ParseBool((*args)[2]);
	if (clobber) {
		if (!MoveFileEx(path, path_new, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
			ctx->ThrowF(out, 632, "Rename failed, %y", GetLastError());
			return;
		}	
	}
  else
#endif
  {
		if (rename(path, path_new)) {
			if (errno == ENOENT) {
				ctx->ThrowF(out, 632, "Rename failed, file or path specified not found.");
				return;
			}
			if (errno == EACCES) {
				ctx->ThrowF(out, 633, "Rename failed, file already exists.");
				return;
			}
			if (errno == EINVAL) {
				ctx->ThrowF(out, 634, "Rename failed, invalid filename.");
				return;
			}
			ctx->ThrowF(out, 635, "Rename failed.");
			return;
		}
	}
}

void EvalFileExists(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("exists", 1, 1);
	CStr path = (*args)[0];
	if (path.IsEmpty()) return;
#ifdef WIN32
	if (_access(path, 00) == 0) {
		out->PutC('T');
	}
#else
  struct stat s;
	if (stat(path, &s) == 0) {
		out->PutC('T');
  }
#endif
}

void EvalDirMake(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("mkdir", 1, 1);
	char * path = (*args)[0].SafeP();
#ifdef WIN32
	_mkdir(path);
#else
 	 mkdir(path, 0777);
#endif
}

void EvalDirRemove(const void *mode, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("rmdir", 1, 1);
	CStr path = (*args)[0];
	if (path) {
	bslash(path.GetBuffer());
#ifdef WIN32
	_rmdir(path);
#else
  rmdir(path);
#endif
	}
}


void EvalInclude(const void *nada, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("include", 1, 1);

	if ((*args)[0].IsEmpty())
		return;

	qStrRef in = OpenFile(ctx, (*args)[0]);
	if (in)
		out->PutS(in);
}

void LoadFile(qCtx *ctx) {
//string
	qObjCache *modules;

	if (!ctx->Find((qObj**)&modules, "<modules>")) {
		modules = new qObjCache(ctx);
		ctx->MapObj(modules, "<modules>");
	}

	ctx->MapObj(NULL, EvalInclude, "include");

	ctx->MapObj(modules, (QOBJMETH) &qObjCache::EvalModule, "module");
#if defined(WIN32) && !defined(NOACTIVEX)
	ctx->MapObj(modules, (QOBJMETH) &qObjCache::EvalActiveX, "activex");
#endif

	ctx->MapObj(EvalExt,			"ext");
	ctx->MapObj(EvalBase,			"base");
	ctx->MapObj(EvalFileName,		"filename");
	ctx->MapObj(EvalFilePath,		"filepath");
	ctx->MapObj(EvalMakePath,		"makepath");

#ifndef WIN32
	ctx->MapObj(EvalUmask,			"umask", "01");
#endif

	ctx->MapObj("wb", EvalFileWrite, "create-file",QMAP_ALL);
	ctx->MapObj("ab", EvalFileWrite, "text-append",QMAP_ALL);
	ctx->MapObj("a",  EvalFileWriteCSV, "csv-append",QMAP_ALL);
	ctx->MapObj("a",  EvalFileWriteCSVq, "csv-appendq",QMAP_ALL);

	ctx->MapObj(EvalFileDelete, "delete");
	ctx->MapObj(EvalFileRename, "rename");
	ctx->MapObj(EvalFileRename, "move");
	ctx->MapObj(EvalFileExists, "exists");
	ctx->MapObj(EvalFileCopy,   "copy");

	ctx->MapObj(EvalDir,		"dir","01");
	ctx->MapObj(EvalDirMake,   "mkdir");
	ctx->MapObj(EvalDirRemove, "rmdir");

#ifdef WIN32
	ctx->MapObj(EvalVolumes,		"volumes");
#endif
}

bool ScanDir(CStr path, int mask, CStr body, qCtx *ctx, qStr *out)
{
	DIRSTATE st;

	qCtxTmp tmpCtx(ctx);

	tmpCtx.MapObj(&st, EvalFName, "fname");
	tmpCtx.MapObj(&st, EvalFPath, "fpath");
	tmpCtx.MapObj(&st, EvalFAttr, "fattr");
	tmpCtx.MapObj(&st, EvalFExt,  "fext");
	tmpCtx.MapObj(&st, EvalFSize, "fsize");
	tmpCtx.MapObj(&st, EvalFMtime,"fmtime");
	tmpCtx.MapObj(&st, EvalFCtime,"fctime");

#ifdef unix
	tmpCtx.MapObj(&st, EvalFMode,"fmode");
#endif

	tmpCtx.MapObj(&st, EvalFIsDir,"isdir");
  
  return _ScanDir(path, mask, body, &tmpCtx, out, st);
}

#ifdef WIN32
bool _ScanDir(CStr path, int mask, CStr body, qCtx *ctx, qStr *out, DIRSTATE &st)
{
	BOOL bMore; 
	HANDLE hFind; 
	BOOL showdot = false;
  
  	// truncate trailing slashes
	char *b = path.GetBuffer();

	if (!b || !*b) return false;

	char *p = path+path.Length() - 1;
	while (p >= b && ISDIRSEP(*p))
		--p;

	if (p-b+1 > 0) {
		if (*p == ':') {
			showdot = true;
			if (!ISDIRSEP(p[1])) {
				path << '.';
				b = path.GetBuffer();
				p = path+path.Length() - 1;
			} else
				++p;
		}

		st.path = path;

		// truncate path to parent
		while (p >= b && !ISPATHSEP(*p))
			--p;

		if (p >= b) {
			st.path.Grow(p-b+1);
		} else {
			st.path.Grow(0);
		}
	} else {
		st.path = path;
	}

  /* read all entries in the directory */
	WIN32_FIND_DATA *r = &st.data;
    hFind = FindFirstFile(path, r); 
    bMore = (hFind != (HANDLE) -1); 
    while (bMore) { 
    if ((mask & r->dwFileAttributes)
			&& !(r->cFileName[0]=='.'&&r->cFileName[1]=='\0')
			) {
			ctx->Parse(body, out);
		} else if (showdot && r->cFileName[0]=='.'&&r->cFileName[1]=='\0') {
			ctx->Parse(body, out);
		}
		bMore = FindNextFile(hFind, r);
    }
    FindClose(hFind); 

	return true;
} /* dir_scan */
#else

#include <glob.h>

class safe_glob_t : public glob_t
{
public:
  safe_glob_t() {memset(this, 0, sizeof(glob_t));}
  ~safe_glob_t() {if (gl_pathv) globfree(this);}
};

bool _ScanDir(CStr path, int mask, CStr body, qCtx *ctx, qStr *out, DIRSTATE &st)
{
  safe_glob_t gb;
  
  int err = glob(path, GLOB_NOSORT | GLOB_TILDE, NULL, &gb);
  
  if (err) {
    ctx->ThrowF(out, 631, "Error during directory read. %y", GetLastError());
    return false;
  }
  
  char **curp = gb.gl_pathv;
  
  while (*curp) {
    if ( !( (*curp)[0]=='.'&&(*curp)[1]=='\0') ) {
      st.found = *curp;
      st.status = DIR_EMPTY;
			ctx->Parse(body, out);
		}
    ++curp;
  }
  
	return true;
} /* dir_scan */
#endif
