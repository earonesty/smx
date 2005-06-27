/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#ifdef USE_QWEB
#include "qwebm.h"
#else
#include "sock.h"
#include "base64.h"
#include "qstrsock.h"
#endif

#include "qctx.h"

#ifdef WIN32
#include "qstr-pipe.h"
#endif

#include "pstime.h"

#include "util.h"

#include "qfopen.h"

#ifdef USE_QWEB
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
#else

static char* EMPTY = "";

bool ParseHTTPURI(const char *orig_url,char **proto, char **host, char **path, char **uid, char **pwd, int *port)
{
	CStr buf_url = orig_url;
	char *url = buf_url.Data();
	char *p;

	*proto = url;
	p = strstr(url, ":");
	if (!p) return false;
	*p++ = '\0';
	strlwr(*proto);

	if (stricmp(*proto, "http")
#ifdef HAVE_OPENSSL
		&& stricmp(*proto, "https")
#endif
	) return false;

	if (*p++ != '/') return false;
	if (*p++ != '/') return false;

	*host = p;
	if (p = strchr(*host, '/')) {
		*p++ = '\0';
		*path = p;
	} else { 
		*path = EMPTY;
	}

        if (p = strchr(*host, '@'))
        {
	    *uid = *host;
            *p++ = '\0';
            *host = p;
	    p = strchr(*uid, ':');
	    if (p) {
		*p++ = '\0';
		*pwd = p;
	    } else {
		*pwd = NULL;
	    }
        } else {
		*uid = *pwd = NULL;
	}


	*port = 0;

	if (p = strchr(*host, ':'))
	{
	    *p = 0;
	    *port = atoi(p + 1);
	}

	if (*port <= 0) {
		if (!stricmp(*proto, "https"))
			*port = 443;
		else
			*port = 80;
	}
	
	return true;
}

qStr *OpenURL(qCtx *ctx, const CStr &arg_path)
{
	const char *p = arg_path;
	char *proto, *host, *path, *uid, *pwd;
	int port;
	
	if (!ParseHTTPURI(p, &proto, &host, &path, &uid, &pwd, &port))	{
		ctx->Throw(ctx->GetEnv(), 558, "Invalid URL");
		return NULL;
	}

	int timeout = ctx->EvalInt("http-timeout");

	if (timeout <= 0 || timeout > 3600) 
		timeout = 20;

	int sock_err;
	Sock sock;
	sock.SetTimeout(timeout);
	sock_err = sock.Open(host, port);

	// todo, open an SSLSock if the proto is "https"

	if (sock_err) {
		if (sock_err == Sock::ERR_GETHOST)
			ctx->ThrowF(ctx->GetEnv(), 571, "Unable to lookup host %s", host);
		else if (sock_err == Sock::ERR_CONNECT)
			ctx->ThrowF(ctx->GetEnv(), 572, "Unable to connect to host %s", host);
		else if (sock_err == Sock::ERR_TIMEOUT)
			ctx->ThrowF(ctx->GetEnv(), 572, "Timeout while connect to host %s", host);
		else
			ctx->ThrowF(ctx->GetEnv(), 572, "Error %d/%d while connecting to host %s", sock_err, errno, host);
		return NULL;
	}

	CStr request = "GET /";

	request << path;
	request << ' '; 
	request << "HTTP/1.0\r\n";

	if (sock.Write(request, request.Length()) < 0) {
		ctx->ThrowF(ctx->GetEnv(), 572, "Error %d while writing request to host %s", errno, host);
		return NULL;
	}

	request.Format("Host: %s\r\n", host);

	if (uid && *uid) {
		CStr tmp = uid;
		tmp += ':';
		if (pwd)
			tmp += pwd;

		CStr result;
		base64(
				(char *) tmp.Data(), 
				tmp.Length(), 
				result.Grow(tmp.Length() * 2).GetBuffer()
		);
		result.Shrink();
		request << "Authorization:Basic " << result << "\r\n";
	}


	request << "Connection: close\n";
	request << "User-Agent: smx\n";
	request << "\r\n";

	if (sock.Write(request, request.Length()) < 0) {
		ctx->ThrowF(ctx->GetEnv(), 572, "Error %d while writing request to host %s", errno, host);
		return NULL;
	}

	bool noparse = ParseBool(ctx->Eval("http-noparse"));

	qStrSockI *qss = new qStrSockI(sock.GetSocket(true), true);

	if (!noparse) {
		bool prev_nl = false;
		char c;
		while (qss->GetS(&c, 1) > 0) {
			if (!prev_nl) {
				if (c == '\n') prev_nl = true;
			} else {
                                if (c == '\r') prev_nl = true;	// do nothing
                                else if (c == '\n') break;
                                else prev_nl = false;
			}

		}
	}

	return qss;
}

#endif

qStr *OpenFileFQ(qCtx *ctx, const char *p)
{
	if (p && *p) {
		CStr orig_path = p;
		CStr path = p; 
		path.Trim();
		fix_path(path.GetBuffer());

		if (!strnicmp(path, "http", 4) 
			&& (path[4] == ':' || (path[4] == 's' && path[5] == ':'))
			) {
			return OpenURL(ctx, path);
		}
    else
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
