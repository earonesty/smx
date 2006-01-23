/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// modsmx.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"

#ifdef unix
#include <arpa/inet.h>
#endif

#include "crit.h"
#include "qctx.h"
#include "qobj-cache.h"

#ifndef WIN32
#define NO_LOAD_TSET
#endif

#include "libs.h"

#ifdef WIN32
	#include "ole2.h"
	#define _MSWSOCK_
	#include <winsock.h>
#endif

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_vhost.h"
#include "http_request.h"
#include "http_core.h"

#ifdef APACHE2
	#include "apr_strings.h"
#endif
// #define _DWINMEM // define if you want to test memory allocation

#include "modsmx.h"

/* -------------------------------------------------------------- */
static CMutex gCrit("mod_smx.1917000012");

static qEnvApacheServer *gEnv = NULL;

void ServerReInit(qEnvApacheServer *s, qCtx *ctx, qStr *out, qArgAry *args);

void qEnvApacheServer::ReInitialize()
{
	CMutexLock lock(gCrit);

	if (myInitMacro) {
		qStrNull null;

		try {
			GetCtx()->Parse(myInitMacro, &null);
		} catch (qCtxEx ex) {
			ap_log_error(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, AP2STATUS myServer,
						"init failed: %s", ex.GetMsg());
		} catch (...) {
			ap_log_error(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, AP2STATUS myServer,
						"init failed with a fatal error.");
		}

	}

	myNoMagic = !CStr(GetCtx()->Eval("http-nomagic")).IsEmpty();
	myUserMacro = GetCtx()->Eval("http-user");
	myBadMacro = GetCtx()->Eval("http-badurl");
	myPageMacro = GetCtx()->Eval("http-init");
	myWrapMacro = GetCtx()->Eval("http-wrap");
}

void qEnvApacheServer::Initialize(request_rec *r)
{
	CMutexLock lock(gCrit);

	if (IsReady())
		return;

	if (myInInit)
		return;

	myInInit = true;

	if (!gEnv->IsReady())
		gEnv->Initialize(r);

	if (!myServer)
		SetServer(r->server);

	GetCtx()->MapObj(ap_document_root(r), "httproot");
	GetCtx()->MapObj(ap_document_root(r), "http-root");

	ReInitialize();

	GetCtx()->MapObj(this, (QOBJFUNC) ServerReInit, "server-reinit");

	myReady = true;
}

static void * create_psx_config(ap_pool *p, server_rec *s);

static ap_inline qEnvApacheServer *get_psx_srv_env(request_rec *r) 
{ 
	if (!gEnv) {
		smx_log_str(SMXLOGLEVEL_DEBUG,"server environment created during get_psx_srv_env");
		ap_set_module_config(r->server->module_config, &smx_module, create_psx_config(r->pool, r->server));
	}

	if (!gEnv)
		return NULL;

	qEnvApacheServer *ps = ((qEnvApacheServer *) 
		ap_get_module_config(r->server->module_config, &smx_module));

	if (!ps->IsReady())
		ps->Initialize(r);

	return ps;
}

static qEnvApache *get_psx_req_env(request_rec *r, bool create = true)
{
	if (!gEnv) {
		smx_log_str(SMXLOGLEVEL_DEBUG,"server environment created during get_psx_req_env");
		ap_set_module_config(r->server->module_config, &smx_module, create_psx_config(r->pool, r->server));
	}

	if (!gEnv)
		return NULL;
		

	qEnvApache *renv = (qEnvApache *)
		ap_get_module_config(r->request_config, &smx_module);

	if (!renv) {
		if (create) {
			renv = new qEnvApache(r);
			ap_set_module_config(r->request_config, &smx_module, renv);
		}
	} else if (create)
		renv->AddRef();

	return renv;
}

static int psx_auth_fail(request_rec *r, qEnvApache *renv)
{
// this is kinda wrong.... but for now we keep it....
	
	if (!r->header_only) {
		CStr resp = renv->GetCtx()->Eval("http-noauth");
		if (!resp.IsEmpty()) {
			qStrBuf bufin(resp);
			qStrBuf bufout;
			renv->GetCtx()->ParseTry(&bufin, &bufout);
			ap_custom_response(r, HTTP_UNAUTHORIZED, bufout.GetBuffer());
		}
	}

	if (!ap_auth_type(r)) {
		ap_table_setn(r->err_headers_out,
			  r->proxyreq ? "Proxy-Authenticate" : "WWW-Authenticate",
			  ap_pstrcat(r->pool, "Basic realm=\"", ap_auth_name(r), "\"",
				  NULL));
	} else {
		ap_note_basic_auth_failure(r);
	}
	renv->Free();

	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, AP2STATUS r, "returning unauthorized: %d, status : %d", HTTP_UNAUTHORIZED, r->status);
	return HTTP_UNAUTHORIZED;
}

static int psx_log(request_rec *r)
{
	qEnvApache *renv = get_psx_req_env(r, false);

	if (renv)
		renv->Done();

	return DECLINED;
}

static int psx_user(request_rec *r)
{
	qEnvApache *renv = NULL;
	try {
		qEnvApacheServer *senv = get_psx_srv_env(r);
		if (!senv)
			return DECLINED;

		const char *macro = senv->GetUserMacro();
		if (!macro || !*macro)
			return DECLINED;

		renv = get_psx_req_env(r);
		if (!renv)
			return DECLINED;


		if (renv->IsAuth == 1) {
			if (r->main)
				renv->Free();
			return OK;
		} else if (renv->IsAuth == -1) {
			return psx_auth_fail(r, renv);
		}

		qCtxTmp tmpCtx(renv->GetCtx());

		char *user = NULL;
		const char *pw = NULL;

		int res=ap_get_basic_auth_pw(r,&pw);

		if (!res) {
#ifdef APACHE2
			user = r->user;
#else
			user = r->connection->user;
#endif
			
		}

		if (!user) user = "";
		tmpCtx.MapObj(user, "username");

		if (!pw) pw = "";
		tmpCtx.MapObj(pw, "password");

		CStr out = tmpCtx.ParseStr(macro);
		out.Trim();

		// ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, AP2STATUS r, "macro: %s, username: %s, password: %s, macro output: %s, macro length: %d, macro char: %x", (const char *) macro, (const char *)user, (const char *)pw, (const char *)out, out.Length(), (unsigned int)*(const char *)out);

		if (out.IsEmpty()) {
			renv->IsAuth = -1;
			return psx_auth_fail(r, renv);
		} else {
			if (r->main)
				renv->Free();
			renv->IsAuth = 1;
			return OK;
		}
	} catch (...) {
		// login code redirected things
		if (r->status == HTTP_MOVED_TEMPORARILY) {
			return OK;
		}

		if (renv)
			renv->Free();

		smx_log_str(SMXLOGLEVEL_ERROR, "unhandled exception during authentication");

		return DECLINED;
	}
}

static int psx_handler(request_rec *r)
{

#ifdef _DWINMEM
	static CMemoryState myMs;
	myMs.Checkpoint();
#endif

#ifdef APACHE2
	if (!r->handler || (strcmp("psx-parsed",r->handler) && strcmp("smx-parsed",r->handler)))
		return DECLINED;
#endif

	qEnvApache *renv = get_psx_req_env(r);
	qEnvApacheServer *senv = get_psx_srv_env(r);

	if (!renv)
		return DECLINED;
	
	if (r->status == HTTP_MOVED_TEMPORARILY) {
		renv->Done();
		return r->status;
	}

	if (r->filename && *r->filename) {
		qStr *pIn = NULL;
		qStrBuf *badUrl = NULL;

		r->status = HTTP_OK;

		FILE *fp=NULL; qStrFileI inT;
		if (r->filename && *r->filename && (fp = fopen(r->filename,"rb"))) {
			inT.SetFile(fp, true);
			pIn = &inT;
		} else {
			if (senv->GetBadMacro() && *senv->GetBadMacro()) {
				badUrl = new qStrBuf(senv->GetBadMacro());
				pIn = badUrl;
			} else {
				renv->Done();
				return HTTP_NOT_FOUND;
			}
		}

#ifdef APACHE2
		renv->GetCtx()->MapObj((const char **)&r->user, "client-authname");
#else
		renv->GetCtx()->MapObj((const char **)&r->connection->user, "client-authname");
#endif

		if (r->prev && r->prev->unparsed_uri) {
			renv->GetCtx()->MapObj((const char **)&r->prev->unparsed_uri, "redirect-url");
		} else {
			renv->GetCtx()->MapObj(CStr::Null, "redirect-url");
		}

		bool noMagic = senv->GetNoMagic();
		int code = 0;

		qStr *out = renv;
                qStrBuf wrapTmp;

                if (senv->GetWrapMacro()) {
                        out = &wrapTmp;
                }

		if (senv->GetPageMacro() && !badUrl && fp) {
			bool ok = noMagic;
			if (!ok) {
				char c=fgetc(fp);
				ok = (c == '%');
				ungetc(c, fp);
			}
			if (ok) {
				qStrBuf pageTmp(senv->GetPageMacro());
				renv->GetCtx()->ParseTry(&pageTmp, renv);
			}
		}

		try {
			if (badUrl || noMagic)
				renv->GetCtx()->ParseTry(pIn, renv);
			else {
				if (!renv->GetCtx()->ParseMagic(pIn, renv, false)) {
					code = DECLINED;
				}
			}
		} catch (...) {
			try {
				renv->PutS("Unexpected error.");
				renv->Done();
				r->status = 500;
				return 500;
			} catch (...) {
				ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, AP2STATUS r,
					"PSX : Unexpected error, recovery impossible, possible memory leak.");
				r->status = 500;
				return 500;
			}
		}

                if (senv->GetWrapMacro()) {
                        renv->GetCtx()->MapObj(&wrapTmp, "page");
                        qStrBuf pageTmp(senv->GetWrapMacro());
                        renv->GetCtx()->ParseTry(&pageTmp, renv);
                }

		if (badUrl)
			delete badUrl;

		if (code == DECLINED)
			return code;

		code = r->status;

		if (badUrl || renv->GetContentLength() > 0 || code > 200) {
			code = renv->FinishReq(false);
		} else {
			code = HTTP_NOT_FOUND;
		}

		renv->Done();

#ifdef _DWINMEM
		myMs.DumpAllObjectsSince();
#endif

		return code;
	} else
		renv->Done();

	return DECLINED;
}


/* -------------------------------------------------------------- */
/* Setup configurable data */

static void * create_psx_config(ap_pool *p, server_rec *s)
{
	qEnvApacheServer *senv;

	{
		CMutexLock lock(gCrit);

		if (!gEnv) {		// first time
			gEnv = senv = new qEnvApacheServer(NULL);
			gEnv->SetServer(s);

			LoadAllLibs(gEnv->GetCtx());
				
//			gEnv->GetCtx()->MapObj(new qObjCtxP(gEnv->GetCtx()), "global-context");

			gEnv->GetCtx()->MapObj(gEnv, (QOBJFUNC) ServerReInit, "global-reinit");

		} else {
			senv = gEnv->NewServer();
		}
	}

    return senv;
}

static const char * set_psx_init(cmd_parms *parms, char *struct_ptr, char *arg)
{
    qEnvApacheServer *psf = (qEnvApacheServer *) 
		ap_get_module_config(parms->server->module_config, &smx_module);
	psf->SetInitMacro(arg);
    return NULL;
}

static const char * set_psx_name(cmd_parms *parms, char *struct_ptr, char *arg)
{
    qEnvApacheServer *psf = ((qEnvApacheServer *) 
		ap_get_module_config(parms->server->module_config, &smx_module));
	psf->SetName(arg);
    return NULL;
}

#ifdef APACHE2
static void register_psx_hooks(apr_pool_t *p)
{
    ap_hook_check_user_id(psx_user,NULL,NULL,APR_HOOK_MIDDLE);
    ap_hook_log_transaction(psx_log,NULL,NULL,APR_HOOK_MIDDLE);
    ap_hook_handler(psx_handler, NULL, NULL, APR_HOOK_MIDDLE);
}
#else
static const handler_rec psx_handlers[] =
{
    {"psx-parsed", psx_handler},
    {NULL}
};
#endif

typedef const char *(*command_rec_func) ();

static const command_rec psx_cmds[] =
{
    {"PSXInit", (command_rec_func) set_psx_init, NULL, RSRC_CONF, RAW_ARGS,
     "Global server-initialization script."},
    {"SMXInit", (command_rec_func) set_psx_init, NULL, RSRC_CONF, RAW_ARGS,
     "Global server-initialization script."},
    {"PSXServer", (command_rec_func) set_psx_name, NULL, RSRC_CONF, TAKE1,
     "Named (virtual) server."},
    {"SMXServer", (command_rec_func) set_psx_name, NULL, RSRC_CONF, TAKE1,
     "Named (virtual) server."},
    {NULL}
};

extern "C" {
#ifdef APACHE2
module AP_MODULE_DECLARE_DATA smx_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,						/* create per-directory config structure */
    NULL,						/* merge per-directory config structures */
    create_psx_config,		    /* create per-server config structure */
    NULL,	                    /* merge per-server config structures */
    psx_cmds,				    /* command table */
    register_psx_hooks
};
#else
module MODULE_VAR_EXPORT smx_module =
{
    STANDARD_MODULE_STUFF,
    NULL,				        /* initializer */
    NULL,						/* create per-directory config structure */
    NULL,						/* merge per-directory config structures */
    create_psx_config,		    /* create per-server config structure */
    NULL,	                    /* merge per-server config structures */
    psx_cmds,				    /* command table */
    psx_handlers,			    /* handlers */
    NULL,						/* translate_handler */
    psx_user,				    /* check_user_id */
    NULL,				        /* check auth */
    NULL,				        /* check access */
    NULL,						/* type_checker */
    NULL,						/* pre-run fixups */
    psx_log,					/* logger */
#if MODULE_MAGIC_NUMBER >= 19970103
    NULL,      /* [3] header parser */
#endif
#if MODULE_MAGIC_NUMBER >= 19970719
    NULL,         /* process initializer */
#endif
#if MODULE_MAGIC_NUMBER >= 19970728
    NULL,         /* process exit/cleanup */
#endif
#if MODULE_MAGIC_NUMBER >= 19970902
    NULL/* [1] post read_request handling */
#endif
};
#endif

}

struct ap_table_cb_header
{
	qEnvHttpHeaderCB *cb;
	int count;
};

static int ap_table_cb(void *a, const char *k, const char *v)
{
	if (a) {
		ap_table_cb_header *hd = (ap_table_cb_header *)a;
		hd->cb->Callback(k, v);
		++(hd->count);
	}
	return 1;
}


#ifdef _DBUGMEM
long gApCnt = 0;
long gCxCnt = 0;
#endif

qEnvApache::qEnvApache(request_rec *req) 
{
#ifdef _DBUGMEM
	InterlockedIncrement(&gApCnt);
#endif

	myRefs = 1;
	myReq = req;
	myBufSize = AP_IOBUFSIZE; 
	mySetupChunkThing = false; 
	myCurStr = &myStrBuf;
	myBuf.Grow(myBufSize);

	qEnvApache *prevEnv = PrevEnv();
	qEnvApache *mainEnv = MainEnv();

	if (prevEnv) {
		myCtx = prevEnv->myCtx;
		prevEnv->myCtxFree = false;
		myCtxFree = true;
		myCtx->SetEnv(this);
		IsAuth  = 0;
	} else if (mainEnv) {
		myCtx = mainEnv->myCtx;
		myCtxFree = false;
		IsAuth  = mainEnv->IsAuth;
	} else {
#ifdef _DBUGMEM
		InterlockedIncrement(&gCxCnt);
#endif
		myCtx = new qCtxRef(get_psx_srv_env(req)->GetCtx());
		myCtx->SetEnv(this);
		myCtxFree = true;
		IsAuth  = 0;
	}
}

void qEnvApache::Free()
{
	if (--myRefs == 0)
		delete this;
}

void qEnvApache::Done()
{
	delete this;
}


qEnvApache::~qEnvApache() 
{
#ifdef _DBUGMEM
	InterlockedDecrement(&gApCnt);
#endif

	if (myCtxFree) {
#ifdef _DBUGMEM
		InterlockedDecrement(&gCxCnt);
#endif
		myCtx->Free();
	}

	if (PrevEnv())
		PrevEnv()->Free();

	ap_set_module_config(myReq->request_config, &smx_module, NULL);
}

int qEnvApache::GetHeaders(qEnvHttpHeaderCB *fCB)
{
	ap_table_cb_header hd = {fCB, 0};
	ap_table_do(ap_table_cb, &hd, myReq->headers_in, NULL);
	return hd.count;
}

CStr qEnvApacheServer::MapFullPath(const char *path)
{
#ifdef APACHE2
    ap_pool * p; apr_pool_create_ex(&p,NULL,NULL,NULL);
#else
		ap_pool * p = ap_make_sub_pool(NULL);
#endif
	CStr r = ap_server_root_relative(p, (char *) path);
	ap_destroy_pool(p);
	return r;
}

CStr qEnvApache::GetMimeType(const char *ext)
{
	if (!ext)
		return 0;

	CStr type;
	request_rec *rr;

#ifdef APACHE2
	rr = ap_sub_req_lookup_file(ext, myReq, NULL);
#else
	rr = ap_sub_req_lookup_file(ext, myReq);
#endif

	type = rr->content_type;
	ap_destroy_sub_req(rr);

	return type;
}

/* Translate the URL into a 'filename' */
CStr qEnvApache::MapFullPath(const char *path)
{
	int was = IsAuth;

	IsAuth = 1;
  
#ifdef APACHE2
	request_rec *r = ap_sub_req_lookup_uri(path, myReq, NULL);
#else
	request_rec *r = ap_sub_req_lookup_uri(path, myReq);
#endif

	IsAuth = was;
	CStr mapped = r->filename;
	
	ap_destroy_sub_req(r);

	return mapped;
}

CStr qEnvApache::MapURL(const char *path)
{
	return ap_construct_url(myReq->pool, path, myReq);
}

bool qEnvApache::AppendHeader(const char *str, const char *val)
{
	if (!IsFlushed()) {
		ap_table_add(GetRequest()->headers_out, str, val);
		if (!stricmp(str,"set-cookie"))
			ap_table_add(GetRequest()->err_headers_out, str, val);
		return true;
	}
	return false;
}
	
bool qEnvApache::SetHeader(const char *str, const char *val)
{
	if (!IsFlushed()) {
		ap_table_set(GetRequest()->headers_out, str, val);
		if (!stricmp(str,"content-type")) {
			myReq->content_type = ap_pstrdup(myReq->pool, val);
		} else  if (!stricmp(str,"set-cookie"))
			ap_table_set(GetRequest()->err_headers_out, str, val);
		return true;
	}
	return false;
}

bool qEnvApache::SetReplyCode(int reply)
{
	if (!IsFlushed()) {
		GetRequest()->status = reply;
		return true;
	}
	return false;
}

const char *qEnvApache::GetServerAddr()		
{
	if (myServerAddr.IsEmpty()) {
		myServerAddr.Grow(16);
		char *p = myServerAddr.GetBuffer();
#ifdef APACHE2
		sprintf(p, "%s",inet_ntoa(myReq->connection->local_addr->sa.sin.sin_addr));
#else
	#ifdef WIN32
		sprintf(p, "%d.%d.%d.%d", 
				myReq->connection->local_addr.sin_addr.S_un.S_un_b.s_b1,
				myReq->connection->local_addr.sin_addr.S_un.S_un_b.s_b2,
				myReq->connection->local_addr.sin_addr.S_un.S_un_b.s_b3,
				myReq->connection->local_addr.sin_addr.S_un.S_un_b.s_b4);
	#else
		sprintf(p, "%s",inet_ntoa(myReq->connection->local_addr.sin_addr));
	#endif
#endif
		myServerAddr.Shrink();
	}
	return myServerAddr;
}

void ServerReInit(qEnvApacheServer *s, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (s && ctx->GetSafeMode())
		s->ReInitialize();
}
