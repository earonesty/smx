// Copyright (C) 1999 by Prime Data Corp. All Rights Reserved.
// For information contact erik@primedata.org

#include "stdafx.h"

#ifdef WIN32
#include "qwebm.h"
#endif

#include "qctx.h"

#ifdef WIN32
#include "qstr-pipe.h"
#endif

#include "pstime.h"

#include "util.h"

#include "qfopen.h"

#ifdef WIN32
class qObjHttpRequest : public qObjTS
{
public:
	qCtx *ctx;
	qStrPipe *pipe;
	CStrAry *ary;
	HANDLE hThread;
	qObjHttpRequest() {
		ctx = NULL; pipe = NULL; hThread = NULL; ary = NULL;
	}
	~qObjHttpRequest() {
		if (hThread && WaitForSingleObject(hThread, 3000)!= WAIT_OBJECT_0) {
			TerminateThread(hThread, 1045);
		}
		if (pipe) {
			pipe->CloseWrite();
			delete pipe;
			pipe = 0;
		}
		if (ary)
			delete ary;
		if (ctx)
			ctx->Free();
	}
};

void OpenHttpRequest(void *varg)
{
	try {
		qObjHttpRequest *obj = (qObjHttpRequest *)varg;

		CQWebOpts opts;

		int argc = obj->ary->Count();

		char **argv = new char *[argc+1];

		int i; for (i = 0; i < argc; ++i) {
			argv[i] = obj->ary->GetAt(i).GetBuffer();
		}
		argv[argc]=NULL;

		http_options(&opts, argc, argv);
		delete [] argv;
		opts.quiet = TRUE;

		FILE *fp = _fdopen(obj->pipe->GetWriteH(), "wb");
		CStr fpx('\x1',1);
		fpx << ((long) fp);
		try {
			int rval = http_request(&opts, (*(obj->ary))[1], fpx, fpx);
			obj->hThread = 0;
		} catch (CEx2 ex) {
			obj->hThread = 0;
			if (obj->pipe) {
				if (!obj->ctx->GetTry())
					obj->ctx->Throw(obj->pipe, ex.ID+500, strchr(ex.Msg,':')+2);
			}
		} catch(...) {
			obj->hThread = 0;
			if (obj->pipe) {
				if (!obj->ctx->GetTry())
					obj->ctx->Throw(obj->pipe, 559, "Pipe Unhandled exception");
			}
		}
		http_options_free(&opts);

		if (obj->pipe) {
			obj->pipe->CloseWrite();
		}

		obj->Free();
	} catch(...) {
		assert(0);
	}
}



long gUID = 10000;
qStr *OpenURL(qCtx *ctx, const CStr &path) 
{
	// parse args
	CStrAry *ary = new CStrAry;
	
	ary->Add("psx");

	const char *rp = path;
	char *wp;
	
	CStr tmp(max(MAX_PATH, path.Length()));
	while (*rp) {
		wp = tmp.GetBuffer();
		while (isspace(*rp))
			++rp;

		if (*rp == '"') {
			++rp;
			while (*rp) {
				if (*rp == '"') {
					if (rp[1] != '"') {
						fslash(ary->Add((const char *)tmp.Grow(wp-tmp.Data())).GetBuffer());
						++rp;
						break;
					}
				}
				*wp++ = *rp++;
			}
		} else {
			while (*rp) {
				if (isspace(*rp)) {
					const char * sp = rp;
					while (isspace(*sp))
						++sp;
					



					if (*sp == '/' || *sp == '\\' || *sp == '-') {
						fslash(ary->Add((const char *)tmp.Grow(wp-tmp.Data())).GetBuffer());
						++rp;
					} else {
						*wp++ = '+';
						++rp;
					}
					break;
				}
				*wp++ = *rp++;
			}
		}
	}
	if (!*rp && wp > tmp.Data()) {
		fslash(ary->Add(tmp.Grow(wp-tmp.Data())).GetBuffer());
	}

	qStrPipe *fip1 = new qStrPipe;
	qStrPipe *fip2 = new qStrPipe(*fip1);

	qObjHttpRequest *rargs = new qObjHttpRequest;

	rargs->ctx = new qCtxRef(ctx);
	rargs->ary = ary;
	rargs->pipe = fip1;
	fip1->CloseRead();
	fip2->CloseWrite();

	/*if (timed) {
		qStrBuf wbuf;
		time_t was = timed->GetTimeGMT();
		wbuf.PutS("If-Modified-Since: ");
		FmtTime(gmtime(&was), "www, dd mmm yyyy hh:nn:ss GMT", &wbuf);
		ary.Add("/other=" << wbuf);
	}*/

	rargs->AddRef();
	++gUID;
	ctx->MapObj(rargs, CStr("<http-req-")<<gUID<<">");
	if (ary->Count() > 0) {
		_beginthread(OpenHttpRequest, 0, rargs);
		return fip2;
	} else {
		delete rargs;
		delete fip1;
		delete fip2;
		return NULL;
	}
}

#endif

qStr *OpenFileFQ(qCtx *ctx, const char *p)
{
	if (p && *p) {
		CStr orig_path = p;
		CStr path = p; 
		path.Trim();
		fix_path(path.GetBuffer());

#ifdef WIN32
		if (!strnicmp(path, "http", 4) 
			&& (path[4] == ':' || (path[4] == 's' && path[5] == ':'))
			) {
			return OpenURL(ctx, path);
		}
    else
#endif
		{
			FILE *fp = fopen(path, "rb");
			if (fp) {
				return new qStrFileI(fp,true);
			} else {
				if (ctx->GetTry()>0) {
					CStr strict = ctx->Eval("strict-includes");
					if (!strict.IsEmpty()) {
						ctx->ThrowF(ctx->GetEnv(), 558, "Open file failed: %y", GetLastError());
					}
				}
			}
		}
	}
	return NULL;
}

qStr *OpenFile(qCtx *ctx, CStr &path)
{
	if (!path.IsEmpty()) {
		ResolvePath(ctx, path);
		return OpenFileFQ(ctx, path);
	}
	return NULL;
}

void ResolvePath(qCtx *ctx, CStr &path)
{
	if (path) {
		fix_path(path.GetBuffer());

		qEnvHttp *env = GetHttpEnv(ctx);
		if (env && !strchr((const char *)path,':')) {
			CStr pdir = env->GetScriptPath();

			if (!pdir.IsEmpty()) {
				char *p = strrchr((const char *)pdir, '/');
				if (p) {
					*p = '\0';
					pdir.Grow(p - pdir.Data());
					path.Grow(pdir.Length() + path.Length());
				}
			} else
				pdir = env->GetServerRoot();

				char *fp;
				path.Grow(MAX_PATH);
				path.Grow(
					SearchPath(pdir, path, 0, path.Length(), path.GetBuffer(), &fp)
				);
		}
	}
}
