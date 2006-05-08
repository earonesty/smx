/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "strary.h"

#include "pstime.h"

#include "util.h"

#ifdef WIN32
#include <winsock.h>
#endif

#ifdef unix
#include <netdb.h>
extern int h_errno;
#endif

class CStrLst : public CLst<CStr>
{
	CStrLst *m_last;
public:
	CStrLst *Next() {
		return (CStrLst *) CLst<CStr>::Next();
	}
	CStrLst *Link(CStr val) {
		CStrLst *t = (CStrLst *) new CStrLst(val);
		m_last->CLst<CStr>::Next() = t;
		m_last = t;
		return this;
	}
	CStrLst(const CStr &e) : CLst<CStr>(e) {
		m_last = this;
	}
	CStrLst(const CStr &e, CStrLst *n) : CLst<CStr>(e, (CLst<CStr>*)n) {
		m_last = this;
	}
	~CStrLst() {
		if (Next()) {
			delete Next();
			CLst<CStr>::Next()=NULL;
		}
	}
};

class CMapStrLst : public CMapStr<CStrLst*>
{
public:
	bool Set(const CStr &key, CStr val) {
		CStrLst **top; 
		if (Find(key, &top)) {
			*top = (*top)->Link(val);
			return true;
		} else 
			CMapStr<CStrLst*>::Set(key, new CStrLst(val)); 
		return false; 
	}
	~CMapStrLst() {
		MAPPOS pos; const char *key; CStrLst *data; 
		for (pos=First(); Next(&pos, &key, &data); ) 
			delete data;
	}
};


#define TAG_NS(T) myTags.Set(T, false)
#define TAG_S(T)  myTags.Set(T, true)

class qObjCGI : public qObj {

	CMapStrLst myFormMap;
	CMapStrLst myFileMap;

	CStr       myClientBodyCopy;
	CStr       myClientBody;
	bool       myClientBodyRead;

	CMapStr<bool> myTags;

	qObjCGI *DoFormParse(qCtx *ctx, qStr *out, qArgAry *args);

	void Parse(qEnvHttp *env);
	void ParseFormString(char *string);
	void ParseFormStringMulti(char *string, char *bound, int bound_len);
	void ParseMulti(char *string, int cb, const char *bound, int bound_len);
	long ParseMultiBody(char *&p, int cb, const char *bound, int bound_len);
	void ReadClientBody(qEnvHttp *env);

	bool myFormAutoVar;

public:
	qObjCGI() {
		myFormAutoVar = true;
		myClientBodyRead = 0;
		EvalFormEnumP = &qObjCGI::EvalFormEnumPFast;
		
		TAG_NS("br");
		TAG_NS("dd");
		TAG_NS("dt");
		TAG_NS("hr");
		TAG_NS("img");
		TAG_NS("li");
		TAG_NS("p");
		TAG_NS("input");

		TAG_S("a");
		TAG_S("address");
		TAG_S("b");
		TAG_S("blink");
		TAG_S("center");
		TAG_S("code");
		TAG_S("dl");
		TAG_S("em");
		TAG_S("font");
		TAG_S("form");
		TAG_S("h1");
		TAG_S("h2");
		TAG_S("h3");
		TAG_S("h4");
		TAG_S("h5");
		TAG_S("h6");
		TAG_S("i");
		TAG_S("ol");
		TAG_S("pre");
		TAG_S("select");
		TAG_S("strong");
		TAG_S("textarea");
		TAG_S("tt");
		TAG_S("u");
		TAG_S("ul");
	}

	CMapStrLst *GetFormMap() {return &myFormMap;}
	void EvalEnumHeader(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalAllHeaders(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalHeader(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalSetHeader(qCtx *ctx, qStr *out, qArgAry *args);
	
	void EvalAppendHeader(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalContentType(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalExpires(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalAuthenticate(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalHttpReply(qCtx *ctx, qStr *out, qArgAry *args);
	
	void EvalClientIP(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalProtocol(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientHttp(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientHost(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientHostName(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientAuth(qCtx *ctx, qStr *out, qArgAry *args); 
	void EvalClientMethod(qCtx *ctx, qStr *out, qArgAry *args); 

	void EvalClientQuery(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientURL(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalClientState(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientStateP(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientBody(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalClientBodyP(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalServerName(qCtx *ctx, qStr *out, qArgAry *args); 
	void EvalServerAddr(qCtx *ctx, qStr *out, qArgAry *args); 
	void EvalServerPort(qCtx *ctx, qStr *out, qArgAry *args); 

	void EvalFormCollate(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormAutoVar(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalForm(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormP(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormSet(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormSetP(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormSaveAs(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormSaveAsP(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormParse(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormParseP(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalFormEnum(qCtx *ctx, qStr *out, qArgAry *args);
	void (qObjCGI::*EvalFormEnumP)(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormEnumPFast(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFormEnumPCollated(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalFqPPath(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalFqURL(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalHttproot(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalHtmlClean(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalHtmlLen(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalHtmlQuote(qCtx *ctx, qStr *out, qArgAry *args);

	static CStr UrlEncode(CStr url);
	void EvalUrlEncode(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalUrlDecode(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalRedirect(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalMimeType(qCtx *ctx, qStr *out, qArgAry *args);
};

qObjCGI *qObjCGI::DoFormParse(qCtx *ctx, qStr *out, qArgAry *args)
{
	qObjCGI *cgi_f = new qObjCGI;
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env && env->GetSessionCtx() && env->EnableCache()) {
		cgi_f->Parse(env);

		env->GetSessionCtx()->MapObj(cgi_f, "<cgif>");
		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH) &qObjCGI::EvalClientStateP, "client-state", NULL);
		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH) &qObjCGI::EvalClientBodyP, "client-body", NULL);

		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH) &qObjCGI::EvalFormSetP, "fset", NULL);
		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH) &qObjCGI::EvalFormParseP, "form-parse", NULL);
		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH) &qObjCGI::EvalFormP, "form", NULL);
		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH) &qObjCGI::EvalFormSaveAsP, "form-saveas", NULL);

		env->GetSessionCtx()->MapObj(cgi_f, (QOBJMETH)  qObjCGI::EvalFormEnumP, "enumform", "1");

		if (myFormAutoVar) {
			MAPPOS pos;
			const char * key=NULL; CStrLst *val;
			for (pos = cgi_f->GetFormMap()->First(); cgi_f->GetFormMap()->Next(&pos, &key, &val);) {
				if (key && *key) {
					qObj *nil;
					if (!env->GetSessionCtx()->Find(&nil, key)) {
						env->GetSessionCtx()->MapObj(val->Data(), key);
					}
				}
			}
		}
	}
	return cgi_f;
}

void qObjCGI::EvalFormEnum(qCtx *ctx, qStr *out, qArgAry *args)
{
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		(obj->*EvalFormEnumP)(ctx, out, args);
	}
}

void qObjCGI::EvalFormCollate(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (ParseBool((*args)[0]))
		EvalFormEnumP = &qObjCGI::EvalFormEnumPCollated;
	else
		EvalFormEnumP = &qObjCGI::EvalFormEnumPFast;
}

void qObjCGI::EvalFormAutoVar(qCtx *ctx, qStr *out, qArgAry *args)
{
	myFormAutoVar =  ParseBool((*args)[0]);
}


void qObjCGI::EvalFormEnumPCollated(qCtx *ctx, qStr *out, qArgAry *args)
{
	
	MAPPOS pos;
	CStrLst*list;
	qCtxTmp tmpCtx(ctx);

	int  i, index = 0;
	CStr body = args->GetAt(0);
	const char *name;
	CStr value;

	tmpCtx.MapObj(&name,  "name");
	tmpCtx.MapObj(&value, "value");

	bool ok = true;
	while (ok) {
		ok = false;
		for (pos = myFormMap.First(); myFormMap.Next(&pos, &name, &list); ) {
			i = 0;
			while (list && i++ < index) {
				list = list->Next();
			}
			if (list) {
				ok = true;
				value = list->Data();
				tmpCtx.Parse(body, out);
			}
		}
		++index;
	}
}

void qObjCGI::EvalFormEnumPFast(qCtx *ctx, qStr *out, qArgAry *args)
{
	
	MAPPOS pos;
	CStrLst*list;
	qCtxTmp tmpCtx(ctx);

	CStr body = args->GetAt(0);
	const char * name;
	CStr value;

	tmpCtx.MapObj(&name,  "name");
	tmpCtx.MapObj(&value, "value");

	for (pos = myFormMap.First(); myFormMap.Next(&pos, &name, &list); ) {
		while (list) {
			value = list->Data();
			tmpCtx.Parse(body, out);
			list = list->Next();
		}
	}
}

void qObjCGI::EvalForm(qCtx *ctx, qStr *out, qArgAry *args)
{
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		obj->EvalFormP(ctx, out, args);
	}
}

void qObjCGI::EvalFormP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStrLst *val;
	if ( myFormMap.Find((*args)[0], val) ) {
		if (args->Count() > 1) {
			int inst = ParseInt((*args)[1]);
			while (val && inst--) {
				val = val->Next();
			}
			if (val && val->Data().Length()) {
				out->PutS(val->Data());
			}
		} else {
			while (val) {
				out->PutS(val->Data());
				val = val->Next();
				if (val && val->Data().Length()) {
					out->PutC(',');
				}
			}
		}
	}
}

void qObjCGI::EvalFormParse(qCtx *ctx, qStr *out, qArgAry *args) {
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		obj->EvalFormParseP(ctx, out, args);
	}
}

void qObjCGI::EvalFormParseP(qCtx *ctx, qStr *out, qArgAry *args) {
	ParseFormString((*args)[0].GetBuffer());
}


void qObjCGI::EvalFormSet(qCtx *ctx, qStr *out, qArgAry *args)
{
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		obj->EvalFormSetP(ctx, out, args);
	}
}

void qObjCGI::EvalFormSetP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStrLst *list;
	CStr name = (*args)[0];

	CStrAry vals;
	CStr val; int i;
	for (i = 1; i < args->Count(); ++i) {
		val = (*args)[i];
		if (!val.IsEmpty()) {
			vals.Add(val);
		}
	}

	if (!name.IsEmpty())
		if ( myFormMap.Find(name, list) ) {
			delete list;
			myFormMap.Del(name);
		}

	for (i = 0; i < vals.Count(); ++i) {
		myFormMap.Set(name, vals[i]);
	}

}

void qObjCGI::EvalFormSaveAs(qCtx *ctx, qStr *out, qArgAry *args)
{
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		obj->EvalFormSaveAsP(ctx, out, args);
	}
}

void qObjCGI::EvalFormSaveAsP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStrLst *val;
	if (args->Count() > 1) {
		CStr name = (*args)[1];
		if (myFileMap.Find(name, val)) {
			if (args->Count() > 2) {
				int inst = ParseInt((*args)[2]);
				while (val && inst--) {
					val = val->Next();
				}
			}
			if (val && val->Data().Length()) {
				CStr file = (*args)[0];
				FILE *fp = safe_fopen(ctx, file, "wb");
				if (fp) {
					fwrite((void *)val->Data().Data(), 1, val->Data().Length(), fp);
					fclose(fp);
				} else {
                			ctx->ThrowF(out, 601, "Failed to open file for writing. %y", GetLastError());
				}
			} else {
				smx_log(SMXLOGLEVEL_DEBUG, "no data in form-saveas, %s", name.Data());	
			}
		} else {
			smx_log(SMXLOGLEVEL_DEBUG, "form-saves name not found, %s", name.Data());	
		}
	}
}


inline int hex2int(char c) 
{
	return (c <= '9' ? (c - '0') : c < 'Z' ? (c - 'A' + 10) : (c - 'a' + 10));
}

void qObjCGI::ReadClientBody(qEnvHttp *env)
{
	if (!myClientBodyRead) {
		CStr t;
		while (!(t = env->GetS()).IsEmpty()) {
			myClientBody << t;
		}
		myClientBodyCopy = myClientBody;
		myClientBodyCopy.Change();
	}
}

void qObjCGI::Parse(qEnvHttp *env)
{
	const char *method = env->GetRequestMethod();
	if (method && !stricmp(method, "post")) {
		CStr ctype = env->GetContentType();
		CStr boundary;
		ctype.LTrim();
		if (!strnicmp(ctype, "multipart/form-data", 19)) {
			char *p = strchr((const char *)ctype+19, ';');
			if (p) {
				while(isspace(*++p));
				if (!strnicmp(p, "boundary", 8)) {
					p = strchr((const char *)ctype+8, '=');
					boundary = p+1;
					ReadClientBody(env);
					ParseMulti(myClientBody.GetBuffer(), myClientBody.Length(), boundary, boundary.Length());
				}
			}
		} else {
			ReadClientBody(env);
			ParseFormString(myClientBody.GetBuffer());
		}
	}
	const char *query = env->GetQueryString();
	ParseFormString(CStr(query).GetBuffer());
}

long qObjCGI::ParseMultiBody(char *&p, int cb, const char *bound, int bound_len)
{
	char *b = p;
	char *e = p + cb;
	while (p < e) {
		// scan for required and non duplicated "line dash dash" sequence
		if (*p == '\n' && p[1] == '-' && p[2] == '-') {
			// check for boundary
			if (!strncmp(p+3, bound, bound_len)) {
				long lof = p - b;
				if (*(p-1) == '\r') {		// 'optional' crlf support for unix programmers who can't read
					--lof;
				}
				p+=bound_len+3;				// skip past \n--[bound]
				return lof;
			}
		}
		++p;
	}

	return 0;
}

void ParseCDisp(char *str, char **vname, char **fname)
{
	char *v2 = 0;
	char *v1 = 0;
	char *p  = str;

	while(isspace(*p)) ++p;

	char *n  = p;
	while (*p) {
		if (*p == '=') {
			*p = '\0';
			++p;
			while(isspace(*p)) ++p;
			v1 = p;

			if (*p == '"') {
				v1 = ++p;
				while (*p) {
					if (*p == '"') {
						v2 = p - 1;
						*p++='\0';
						break;
					}
					++p;
				}
			} else {
				while (*p) {
					if (*p == ';') {
						v2 = p-1;
						*p='\0';
						break;
					}
					++p;
				}
			}
			
			while (isspace (*v2))
				--v2;
			*++v2 = '\0';

			if (!stricmp(n, "name")) {
				*vname = v1;
			} else if (!stricmp(n, "filename")) {
				*fname = v1;
			}

			if (*p) {
				while (isspace(*++p));
			}
			n = p;
		} else if (*p == ';') {
			while (isspace (*++p));
			n = p;
		}
		else {
			++p;
		}
	}
}


CStr &UrlDecode(CStr &str)
{
	if (str.Length()) {
		unsigned char *p = (unsigned char *) (char *) str.GetBuffer();
		unsigned char *v2 = p;
		while (*p) {
			if (*p == '%' && isxdigit(p[1]) && isxdigit(p[2])) {
				*v2++ = (hex2int(p[1])<<4) | (hex2int(p[2]));
				p += 3;
			} else if (*p == '+') {
				*v2++ = ' ';
				++p;
			} else {
				*v2++ = *p++;
			}
		}
		str.Grow(v2-(unsigned char *)(char *) str.GetBuffer());
	}
	return str;
}

void qObjCGI::ParseMulti(char *str, int cb, const char *bound, int bound_len)
{
	if (str) {
		char *p = str;
		char *e = p + cb;
		char *n;
		char *v1 = 0, *v2 = 0;

		char *c_disp;
		char *c_type;

		if (*p == '-' && p[1] == '-' && !strncmp(p+2, bound, bound_len)) {
			p+=bound_len+2;
		}

		while (isspace(*p)) ++p;
		n = p;
		while (p < e) {
			if (*p == '\n') {
				if ((p[1] == '\n') || (p[1] == '\r' && (p[2] == '\n'))) {
					if (p[1] == '\r') ++p;
					p += 2;
					long lof;
					char *b = p;
					if ((lof = ParseMultiBody(p, (cb - (p-str)), bound, bound_len)) > 0) {
						char *fname = 0;
						char *vname = 0;
						ParseCDisp(c_disp, &vname, &fname);
						if (fname) {
							myFormMap.Set(vname, CStr(fname));
							myFileMap.Set(vname, CStr(b, lof));
						} else {
							myFormMap.Set(vname, CStr(b, lof));
						}
					}
				} else {
					while (isspace(*++p));
					n = p;
				}
			} else if (*p == ':') {
				*p++ = '\0';
				while (isspace(*p)) ++p;
				v1 = p;
				while (*p) {
					if (*p == '\n')
						break;
					++p;
				}
				v2 = p - 1;
				while (isspace (*v2))
					--v2;
				*++v2 = '\0';
				if (!stricmp(n, "content-disposition")) {
					c_disp = v1;
				} else if (!stricmp(n, "content-type")) {
					c_type = v1;
				}
				n = p = v2 + 1;
			} else {
				++p;
			}
		}
	}
}

void qObjCGI::ParseFormString(char *str)
{
	if (str) {
		char *p = str;
		char *n = p;
		char *v1 = 0, *v2 = 0;
		while (*p) {
			if (*p == '=') {
				*p++ = '\0';
				v1 = v2 = p;
				while (*p) {
					if (*p == '=' || *p == '&') {
						CStr tmp(n);
						myFormMap.Set(UrlDecode(tmp), CStr(v1, v2 - v1));
						n = ++p;
						break;
					} else if (*p == '%' && isxdigit(p[1]) && isxdigit(p[2])) {
						*v2++ = (hex2int(p[1])<<4) | (hex2int(p[2]));
						p += 2;
					} else if (*p == '+') {
						*v2++ = ' ';
					} else {
						*v2++ = *p;
					}
					++p;
				}
 			} else if (*p == '&' || *p == '?') {
				n = p + 1;
				++p;
			} else {
				++p;
			}
		}
		if (*n && v1 && (v2 > v1)) { 
			CStr tmp(n);
			myFormMap.Set(UrlDecode(tmp), CStr(v1, v2 - v1));
		}
	}
}

class qEnvHttpHeaderFixed : public qEnvHttpHeaderCB 
{
public:
	qCtx* ctx;
	qStr* out;
	virtual bool Callback(const char *name, const char *valu) {
		out->PutS(name);
		out->PutC(':');
		out->PutS(valu);
		out->PutC('\n');
		return true;
	}
};

class qEnvHttpHeaderOut : public qEnvHttpHeaderCB 
{
public:
	qCtx* ctx;
	CStr* bod;
	qStr* out;
	CStr  name; 
	CStr  valu;
	bool  ok;
	virtual bool Callback(const char *name, const char *valu) {
		this->name = name;
		this->valu = valu;
		ctx->Parse(*bod, out);
		return ok;
	}
};

void qObjCGI::EvalEnumHeader(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env && args->Count() >0) {
		qCtxTmp sub(ctx);
		qEnvHttpHeaderOut looper;

		sub.MapObj(&looper.name, "header");
		sub.MapObj(&looper.name, "name");
		sub.MapObj(&looper.valu, "value");
		sub.MapObj(&looper.ok, (QOBJFUNC) EvalBreak, "break");

		looper.ok = true;
		looper.ctx = &sub;
		looper.out = out;
		looper.bod = &(args->GetAt(0));

		env->GetHeaders(&looper);
	}
}

void qObjCGI::EvalAllHeaders(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		qEnvHttpHeaderFixed looper;
		looper.ctx = ctx;
		looper.out = out;
		env->GetHeaders(&looper);
	}
}

void qObjCGI::EvalHeader(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		out->PutS(env->GetHeader((*args)[0]));
	}
}

void qObjCGI::EvalAppendHeader(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env && args->Count()>1) {
		env->AppendHeader((*args)[0],(*args)[1]);
	}
}

void qObjCGI::EvalSetHeader(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env && args->Count()>1) {
		env->SetHeader((*args)[0],(*args)[1]);
	}
}

void qObjCGI::EvalContentType(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env && args->Count()>0) {
		CStr s = (*args)[0];
		if (strchr((const char *)s, '/')) {
			env->SetHeader("content-type",s);
		}
	}
}

void qObjCGI::EvalExpires(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env && args->Count()>0) {
		time_t t = ParseInt((*args)[0]);
		time_t now = time(0);
		if (t && t != -1) {
			t = min(now + 315576000, t);
		} else {
			t = now + 315576000;
		}

		qStrBuf buf;
		struct tm *tms = gmtime(&t);
		FmtTime(tms, "www, dd mmm yyyy HH:nn:ss GMT", &buf);
		env->SetHeader("Expires",buf);
	}
}

void qObjCGI::EvalAuthenticate(qCtx *ctx, qStr *out, qArgAry *args)
{
	qEnvHttp *env = GetHttpEnv(ctx);
	bool ok = false;

	if (env && args->Count()>0) {
		CStr realm = (*args)[0];
		ok = env->SetHeader("WWW-Authenticate", "Basic realm=\"" << realm << '"');
		ok     |= env->SetReplyCode(401);
	}

	if (ok) {
		ctx->Abort();
	} else {
		ctx->Throw(out, 105,"authenticate failed/not supported");
	}
}

void EvalGetEnv(void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	const char *string;
	if (ctx->GetEnv() && (string = ctx->GetEnv()->GetEnvString((*args)[0].GetBuffer()))) {
		out->PutS(string);
	}
	return;
}

void EvalSetEnv(void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
 	if (ctx->GetSafeMode()) {
		ctx->Throw(out, 997, "operation not permitted");
		return;
	}
	if (ctx->GetEnv()) {
		CStr val = (*args)[0];
		if (args->Count() == 2) {
			val << "=";
			val << (*args)[1];
		}
		char * tmp = val;
		ctx->GetEnv()->PutEnvString(val.GetBuffer());
	}
	return;
}

void EvalFlush(void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (ctx->GetEnv()) {
		ctx->GetEnv()->Flush();
	}
	return;
}


void qObjCGI::EvalClientQuery(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *query = env->GetQueryString();
		if (query) {
			out->PutS(query);
		}
	}
}

void qObjCGI::EvalClientState(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		obj->EvalClientStateP(ctx, out, args);
	}
}

void qObjCGI::EvalClientStateP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		MAPPOS pos;
		CStrLst *list;
		const char *name;
		CStr value;
		bool first = true;
		for (pos = myFormMap.First(); myFormMap.Next(&pos, &name, &list); ) {
			while (list) {
				value = list->Data();
				out->PutS(name);
				out->PutC('=');
				out->PutS(UrlEncode(value));
				out->PutC('&');
				list = list->Next();
				first = false;
			}
		}
	}
}

void qObjCGI::EvalClientBody(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qObjCGI *obj = DoFormParse(ctx, out, args);
	if (obj) {
		obj->EvalClientBodyP(ctx, out, args);
	}
}

void qObjCGI::EvalClientBodyP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (!myClientBodyCopy.IsEmpty())
		out->PutS(myClientBodyCopy);
}

void qObjCGI::EvalClientURL(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *url = env->GetRequestURL();
		if (url && *url) {
			char *p;
			if ((p = strchr(url, '?')))
				out->PutS(url, p-url);
			else
				out->PutS(url);
		} else {
			ctx->Throw(out, 99,"%client-url% not supported");
		}
	}
}

void qObjCGI::EvalClientIP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *url = env->GetRemoteAddr();
		if (url) {
			out->PutS(url);
		} else {
			ctx->Throw(out, 100,"%client-ip% not supported");
		}
	}
}

void qObjCGI::EvalProtocol(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *proto = env->GetURLProtocol();
		if (proto) {
			out->PutS(proto);
		} else {
			ctx->Throw(out, 100,"%protocol% not supported by your server");
		}
	}
}

void qObjCGI::EvalClientHttp(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *proto = env->GetServerProtocol();
		if (proto) {
			out->PutS(proto);
		} else {
			ctx->Throw(out, 100,"%client-http% not supported");
		}
	}
}

void qObjCGI::EvalServerName(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *url = env->GetServerName();
		if (url && *url) {
			out->PutS(url);
		} else {
			ctx->Throw(out, 101,"%server% not supported");
		}
	}
}

void qObjCGI::EvalServerAddr(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *url = env->GetServerAddr();
		if (url && *url) {
			out->PutS(url);
		} else {
			ctx->Throw(out, 101,"%server-ip% not supported");
		}
	}
}

void qObjCGI::EvalServerPort(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		int port = env->GetServerPort();
		if (port) {
			out->PutN(port);
		} else {
			ctx->Throw(out, 102,"%port% not supported");
		}
	}
}

CStr GetHostName(qEnvHttp *env)
{
	CStr host = env->GetRemoteHost();

	if (host) {

	if (isdigit(*(const char *)host)) {
		host = env->GetHeader("Host");

		if (isdigit(*(const char *)host))
			return 0;
	}

	}

	return host;
}

void qObjCGI::EvalClientHost(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		CStr host = GetHostName(env);
		
		if (!host)
			host = env->GetRemoteHost();

		if (host)
			out->PutS(host);
		else
			ctx->Throw(out, 103,"%client-host% not supported");
	}
}

void qObjCGI::EvalClientHostName(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *host = GetHostName(env);
		
		const char *p = host;

		while (*p) {
			if (!isdigit(*p) && *p != '.') {
				break;
			}
			++p;
		}

		if (!*p) {
			struct hostent * he = gethostbyname(host);
			if (he && he->h_name)
				host = he->h_name;
		}

		if (host)
			out->PutS(host);
		return;
	}
	return;
}

void qObjCGI::EvalClientAuth(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *url = env->GetRemoteUser();
		if (url) {
			out->PutS(url);
		} else {
			ctx->Throw(out, 104,"%client-authname% not supported");
		}
	}
}

void qObjCGI::EvalClientMethod(qCtx *ctx, qStr *out, qArgAry *args) 
{
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		const char *method = env->GetRequestMethod();
		if (method) {
			out->PutS(method);
		} else {
			ctx->Throw(out, 104,"%method% not supported");
		}
	}
}

void qObjCGI::EvalRedirect(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->GetAt(0)) {
		qEnvHttp *env = GetHttpEnv(ctx);
		if (env) {
			bool ok = env->SetHeader("Location", (*args)[0]);
			ok     |= env->SetReplyCode(302);
			if (ok) {
				ctx->Abort();
			} else {
				ctx->Throw(out, 98,"redirect not supported after %flush%");
			}
		}
	}
}

void qObjCGI::EvalHttpReply(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->GetAt(0)) {
		int code = ParseInt((*args)[0]);
		qEnvHttp *env = GetHttpEnv(ctx);
		if (env) {
			bool ok = env->SetReplyCode(code);
			if (!ok) {
				ctx->Throw(out, 98,"http-reply: not supported after %flush%");
			}
		}
	}
}

void qObjCGI::EvalMimeType(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->GetAt(0)) {
		qEnvHttp *env = GetHttpEnv(ctx);
		if (env) {
			CStr ext = (*args)[0];
			if (ext) {
				if (!strchr((const char *)ext, '.'))
					ext = (CStr(".") << ext);
				CStr mime = env->GetMimeType(ext);
				if (!mime.IsEmpty()) {
					out->PutS(mime);
				}
			}
		}
	}
}

#define i2hex(c) (assert((c) >= 0 && (c) < 16),((c) < 10 ? ('0'+(c)) : ((c) - 10 + 'A')))
#define u2hex(c) (assert((c) < 16),((c) < 10 ? ('0'+(c)) : ((c) - 10 + 'A')))

CStr qObjCGI::UrlEncode(CStr url)
{
	if (!url.IsEmpty()) {
		CStr buf(url.Length() * 3);
		unsigned char *u = (unsigned char *) (char *) url.GetBuffer();
		char *b = buf.GetBuffer();

		while(*u) {
			switch (*u) {
			case ' ':
				*b++ = '+'; break;
			case '/':
			case '\\':
			case '+':
			case ',':
			case '`':
				*b++ = '%'; 
				*b++ = u2hex((*u >> 4)); 
				*b++ = u2hex(*u & 0xF);
				break;
			default:
				if (   (*u <= 41)
				    || (*u >= 123)
					|| (*u >= 58 && *u <= 64)
					)
				{
					*b++ = '%'; 
					*b++ = u2hex((*u >> 4)); 
					*b++ = u2hex(*u & 0xF);
				} else 
					*b++ = *u;
			}
			++u;
		}
		*b = 0;
		buf.Grow(b - (char *) buf.GetBuffer());
		return buf;
	} else
		return CStr(0);
}

void qObjCGI::EvalUrlEncode(qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStr url = (*args)[0];
	out->PutS(UrlEncode(url));
}

void qObjCGI::EvalUrlDecode(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count()) {
		CStr url = (*args)[0];
		out->PutS(UrlDecode(url));
	}
}

int ParseAry(const char *p, CStrAry &ary, char *toks)
{
	if (!p)
		return 0;

	char *t;
	const char *o = p;
	while (*p) {
		t = toks;
		while (*t) {
			if (*p == *t) {
				ary.Add(CStr(o, p-o));
				++p;
				while (*p) {
					t = toks;
					while (*t) {
						if (*p != *t) {
							break;
						}
					}
					if (*t) 
						break;
				}
				o = p;
				break;
			}
			++t;
		}
		++p;
	}
	if (p > o) {
		ary.Add(CStr(o, p-o));
	}
	return ary.Count();
}

void qObjCGI::EvalHtmlClean(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count()) {
		CStr html       = (*args)[0];
		CStr ok_strip   = (*args)[2];
		CStr ok_nostate = (*args)[3];
		CStr ok_state   = (*args)[4];

		CMapStr<bool> tags;
		CMapStr<int> state;

		if (ok_nostate) {
			CStrAry ary; int i; int l = ParseAry(ok_nostate, ary, ",;");
			for (i = 0; i < l; ++i)
				tags.Set(ary[i], false);
		}

		if (ok_state) {
			CStrAry ary; int i; int l = ParseAry(ok_nostate, ary, ",;");
			for (i = 0; i < l; ++i)
				tags.Set(ary[i], true);
		}

		CStr tmp;
		char *b;
		char *n;
		char *p = html.GetBuffer();
		if (!p)	return;
		bool cl;
		bool st;

		while (*p) {
			if (*p == '<') {
				b = p;
				while (isspace(*p)) 
					++p;

				++p;
				if (cl = ((*p) == '/')) {
					while (isspace(*++p)) {}
					n = p;
				}
				n=p;

				while (!isspace(*p) && *p != '>')
					++p;

				tmp = CStr(n, p - n);
				if (!myTags.Find(tmp, st) && !tags.Find(tmp, st)) {
					while (*p && *p != '>')
						++p;
					if (*p == '>') 
						++p;
				} else {
					while (b < p)
						out->PutC(*b++);
					while (*p && *p != '>')
						out->PutC(*p++);
					if (*p)
						out->PutC(*p++);
					if (st) {
						int *i;
						if (!state.Find(tmp, &i)) {
							state.Add(tmp)=cl?-1:1;
						} else {
							*i+=(cl?-1:1);
						}
					}
				}
			} else {
				out->PutC(*p++);
			}
		}

		MAPPOS pos;
		int left=0;
		const char *tc;
		for (pos = state.First(); state.Next(&pos, &tc, &left);) {
			while (left-- > 0) {
				out->PutC('<');
				out->PutC('/');
				out->PutS(tc);
				out->PutC('>');
			}
		}

	}
}


void qObjCGI::EvalHtmlQuote(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count()) {
		CStr html       = (*args)[0];

		char *p = html.GetBuffer();
		if (p) while (*p) {
			switch (*p) {
			case '>':
				out->PutS("&gt;",4); break;
			case '<':
				out->PutS("&lt;",4); break;
			case '&':
				out->PutS("&amp;",5); break;
			case '"':
				out->PutS("&quot;",6); break;
			default:
				if (isprint((UCHAR)*p) || isspace((UCHAR)*p)) {
					out->PutC(*p);
				} else {
					out->PutC('&');
					out->PutC('#');
					out->PutN((unsigned char) *p);
					out->PutC(';');
				}
			}
			++p;
		}
	}
}

void qObjCGI::EvalHtmlLen(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count()) {
		CStr html       = (*args)[0];
		int l = 0;
		char *p = html.GetBuffer();
		while (*p) {
			if (*p == '<') {
				while (*p != '>')
					++p;
			} else if (*p == '&') {
				while (*p != ';')
					++p;
			} else {
				++l;
				++p;
			}
		}
		out->PutN(l);
	}
}

void qObjCGI::EvalFqPPath(qCtx *ctx, qStr *out, qArgAry *args) {
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		if (args->Count() >= 1) {
			CStr fqppath = env->MapFullPath((*args)[0]);
			out->PutS(fqppath);
		}
	} else {
		ctx->Parse(args->GetAt(0), out);
	}
}

void qObjCGI::EvalFqURL(qCtx *ctx, qStr *out, qArgAry *args) {
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		if (args->Count() >= 1) {
			CStr sname = env->GetServerName();
			CStr uprot = env->GetURLProtocol();
			int sport = env->GetServerPort();
			CStr path = (*args)[0];
			fslash(path.GetBuffer());
			
			if (uprot.IsEmpty()) {
				if (sport == 443)
					uprot = "https";
				else
					uprot = "http";
				sport = 0;
			} else if (sport == 443 && !stricmp(uprot,"https"))
				sport = 0;
			else if (sport == 80 && !stricmp(uprot,"http"))
				sport = 0;

			out->PutS(uprot);
			out->PutS("://");
			out->PutS(sname);
			if (sport) {
				out->PutC(':');
				out->PutN(sport);
			}

			if (path.Data() && path.Data()[0] != '/') {
				CStr curl = env->GetRequestURL();
				CStr fqppath = env->MapFullPath(curl);
				CStr isdir = ctx->Eval("dir", fqppath, "%isdir%");
				if (isdir.IsEmpty()) {
					const char * p = curl.RFindC('/');
					if (p)
						curl.Grow(p - (const char *)curl);
					else
						curl.Grow(0);
				}
				if (!curl.Data())
					curl = "/";
				else if (curl.Data()[curl.Length()] != '/')
					curl << '/';

				out->PutS(curl);
			}
			out->PutS(path);
		}
	} else {
		ctx->Parse(args->GetAt(0), out);
	}
}

void qObjCGI::EvalHttproot(qCtx *ctx, qStr *out, qArgAry *args) {
	qEnvHttp *env = GetHttpEnv(ctx);
	if (env) {
		CStr fqppath = env->MapFullPath("/");
		char *p = fqppath.Data();
		fslash(p);
		if (p[fqppath.Length() - 1] == '/')
			fqppath.Grow(fqppath.Length() - 1);
		out->PutS(fqppath);
	}
}

void LoadCGI(qCtx *ctx) {
	qObjCGI *cgi = new qObjCGI();
	ctx->MapObj(cgi, "<cgi>");

	ctx->MapObj((QOBJFUNC) EvalGetEnv,	"getenv");
	ctx->MapObj((QOBJFUNC) EvalSetEnv,	"setenv");
	ctx->MapObj((QOBJFUNC) EvalFlush,	"flush");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalHeader,		"header"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalEnumHeader,	"enumheader", "1"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalAllHeaders,	"client-headers"); 

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalAppendHeader,	"append-header"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalAppendHeader,	"append-header:"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalSetHeader,	"set-header:");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalSetHeader,	"set-header");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalContentType,	"content-type:"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalExpires,		"expires:"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalAuthenticate,	"authenticate:"); 
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalHttpReply,	"return-code:"); 
	

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFormCollate,   "collate-enumform");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFormAutoVar,   "form-autovar");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalForm,			"form");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFormParse,	"form-parse");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFormSet,		"fset");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFormEnum,		"enumform", "1");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFqPPath,		"fqppath");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalFqURL,		 "fqurl");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalHttproot,		"httproot");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalRedirect,		"redirect");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalMimeType,		"mimetype");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalUrlEncode,		"url-encode");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalUrlDecode,		"url-decode");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalHtmlClean,		"html-clean");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalHtmlLen,		"html-len");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalHtmlQuote,		"html-quote");


	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientQuery,	"client-query");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientState,	"client-state");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientBody,	"client-body");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientURL,	    "client-url");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientHttp,	"client-http");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientIP,	    "client-ip");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientHost,	"client-host");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientHostName,"client-hostname");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientAuth,	"client-authname");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalClientMethod,  "client-method");

	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalServerName,	"server");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalServerAddr,	"server-ip");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalServerPort,	"port");
	ctx->MapObj(cgi, (QOBJMETH) &qObjCGI::EvalProtocol,	    "protocol");
	
}

	

	/*
	//client-body
	//client-headers
	//client-host
	//client-hostname

	client-ip

	//client-method
	//client-query
	//client-state
	//client-url
	//cl-fmt

	//Content-type:
	enumform
	Expires:
	flushlog
	form
	form-content-encoding
	form-saveas
	header
	log
	port
	protocol
	server
	server-ip
	sslport
*/
