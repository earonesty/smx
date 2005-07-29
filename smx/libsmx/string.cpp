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

#include "regx.h"

#include "open-enc.h"

#include "tabfmt.h"

#include "util.h"

#include <new>
using namespace std;

// sha hash
void EvalSha(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr out = HEX_encode(SHA1_string((*args)[0]));
		if (args->Count() > 1)
			pStream->PutS(out, max(0,min(ParseInt((*args)[1]), out.Length())));
		else 
			pStream->PutS(out);
	}
}

// string position
void EvalPos(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStr sub = (*args)[0];
	CStr str = (*args)[1];
	if (sub.Data() && str.Data()) {
		const char *p = strstr((const char *)str, (const char *)sub);
		if (p) {
			out->PutN(p - str.Data());
		}
	}
}

// set search
void EvalIn(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStr str = (*args)[0];

	CStr cur;
	int len = str.Length(), i;
	for (i = 1; i < args->Count(); ++i) {
		cur = (*args)[i];
		if (cur.Length() == len && !strnicmp(str, cur, len)) {
			out->PutN(i);
			return;
		}
	}
}

// set search
void EvalXin(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStr str = (*args)[0];
	CStr cur;
	int len = str.Length(), i;
	for (i = 1; i < args->Count(); ++i) {
		cur = (*args)[i];
		if (cur.Length() == len && !strncmp(str, cur, len)) {
			out->PutN(i);
			return;
		}
	}
}

void EvalEncrypt(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 1) {
		CStr cipher = (*args)[2];
		CStr encoding = (*args)[3];
		CStr out;

		if (!encoding.IsEmpty() && !stricmp(encoding, "hex")) {
			out = HEX_encode(EVP_encrypt((*args)[1],(*args)[0],cipher));
		} else {
			out = B64_encode(EVP_encrypt((*args)[1],(*args)[0],cipher));
		}
		pStream->PutS(out);
	}
}

#ifdef unix
void EvalCrypt(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
        if (args->Count() > 1) {
		pStream->PutS(crypt((*args)[0].SafeP(),(*args)[1].SafeP()));
        }
}
#endif


void EvalDecrypt(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 1) {
		CStr cipher = (*args)[2];
		CStr encoding = (*args)[3];
		CStr out;

		if (!encoding.IsEmpty() && !stricmp(encoding, "hex")) {
			out = EVP_decrypt((*args)[1].SafeP(),HEX_decode((*args)[0].SafeP()),cipher);
		} else {
			out = EVP_decrypt((*args)[1].SafeP(),B64_decode((*args)[0].SafeP()),cipher);
		}
		pStream->PutS(out);
	}
}

// HEX/ENCODE
void EvalEncode(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr encoding = (*args)[1];
		CStr out;

		if (!encoding.IsEmpty() && !stricmp(encoding, "hex")) {
			out = HEX_encode((*args)[0].SafeP());
		} else {
			out = B64_encode((*args)[0].SafeP());
		}
		pStream->PutS(out);
	}
}

// blowfish decrypt
void EvalDecode(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr encoding = (*args)[1];
		CStr out;

		if (!encoding.IsEmpty() && !stricmp(encoding, "hex")) {
			out = HEX_decode((*args)[0].SafeP());
		} else {
			out = B64_decode((*args)[0].SafeP());
		}
		pStream->PutS(out);
	}
}

void EvalObfuscate(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr out = B64_encode(EVP_encrypt("PSX",(*args)[0].SafeP(), 0));
		pStream->PutS(out);
	}
}

// blowfish decrypt
void EvalDeobfuscate(const void *data, qCtx *ctx, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr out = EVP_decrypt("PSX",B64_decode((*args)[0].SafeP()),0);
		pStream->PutS(out);
	}
}

void EvalLt(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1 && 
		ParseDbl((*args)[0])<ParseDbl((*args)[1])
		)
		out->PutC('T');
}
void EvalGt(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1 && 
		ParseDbl((*args)[0])>ParseDbl((*args)[1])
		)
		out->PutC('T');
}
void EvalLte(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1 && 
		ParseDbl((*args)[0])<=ParseDbl((*args)[1])
		)
		out->PutC('T');
}
void EvalGte(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1 && 
		ParseDbl((*args)[0])>=ParseDbl((*args)[1])
		)
		out->PutC('T');
}
void EvalEqN(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 2) {
		if (fabs(ParseDbl((*args)[0])-ParseDbl((*args)[1])) < ParseDbl((*args)[2]))
			out->PutC('T');	
	} else if (args->Count() > 1 && 
			ParseDbl((*args)[0])==ParseDbl((*args)[1]) )
		out->PutC('T');
}

void EvalEqS(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr a, b;
	a = (*args)[0];
	b = (*args)[1];
	if (args->Count() > 1 && 
		( a.IsEmpty() &&  b.IsEmpty()) ||
		(!a.IsEmpty() && !b.IsEmpty() && a.Length() == b.Length() && !strnicmp(a, b, a.Length()))
	   )
		out->PutC('T');
}
void EvalEqB(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr a, b;
	a = (*args)[0];
	b = (*args)[1];
	if (args->Count() > 1 && 
		( a.IsEmpty() &&  b.IsEmpty()) ||
		(!a.IsEmpty() && !b.IsEmpty() && a.Length() == b.Length() && !strncmp(a, b, a.Length()))
	   )
		out->PutC('T');
}

int IsWcmX(char *a, char *w) {
	for (; *w; a++, w++) {
		if (*w == '*') {
			while(*(++w) =='*');
			if (!*w) return 1;
			while (*a) {
				if(IsWcmX(a++,w))
					return 1;
			}
			return 0;
		} else if (*w == '?') {
			if (!*a) 
				return 0;
		} else if (*a != *w) {
			return 0;
		}
	}
	return !*a;
}

int IsWcm(char *a, char *w) {
	for (; *w; a++, w++) {
		if (*w == '*') {
			while(*(++w) =='*');
			if (!*w) return 1;
			while (*a) {
				if(IsWcm(a++,w))
					return 1;
			}
			return 0;
		} else if (*w == '?') {
			if (!*a) 
				return 0;
		} else if (tolower(*a) != tolower(*w)) {
			return 0;
		}
	}
	return !*a;
}

void EvalWcm(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1) {
		CStr a = (*args)[0];
		CStr b = (*args)[1];
		if (IsWcm(a.SafeP(),b.SafeP())) {
			out->PutC('T');
		}
	}
}

void EvalWcmX(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1) {
		CStr a = (*args)[0];
		CStr b = (*args)[1];
		if (IsWcmX(a.SafeP(),b.SafeP())) {
			out->PutC('T');
		}
	}
}

void EvalConcat(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; for (i = 0; i < args->Count(); ++i) {
		out->PutS(args->GetAt(i));
	}
}

void EvalReplace(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i = 1;
	CStr in = (*args)[0];
	while (i < args->Count()) {
		CStr from = (*args)[i];
		if (
    !in.IsEmpty()
    &&
    !from.IsEmpty()
    ) {
			CStr to = (*args)[i+1];
			in = ReplaceStrI(in, from, to);
		}
		i += 2;
	}
	out->PutS(in);
}

void EvalXReplace(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i = 1;
	CStr in = (*args)[0];
	while (i < args->Count()) {
		CStr from = (*args)[i];
		if (!in.IsEmpty() && !from.IsEmpty()) {
			CStr to = (*args)[i+1];
			in = ReplaceStr(in, from, to);
		}
		i += 2;
	}
	out->PutS(in);
}

void EvalLTrim(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("ltrim", 1, 2);
	if (args->Count() > 1) {
		CStr str = (*args)[0];
		if (str.IsEmpty()) 
			return;
		CStr toks = (*args)[1];
		if (toks.Length() == 0)
			out->PutS(str);
		else if (toks.Length() == 1)
			out->PutS(str.LTrim(*toks));
		else {
			const char *p = str;
			size_t i = strspn(p,toks); 
			out->PutS(p+i, str.Length() - i); 
		}
	} else
		out->PutS((*args)[0].LTrim());
}

void EvalRTrim(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("ltrim", 1, 2);
	if (args->Count() > 1) {
		CStr str = (*args)[0];
		if (str.IsEmpty()) 
			return;
		CStr toks = (*args)[1];




		if (toks.Length() == 0)
			out->PutS(str);
		else if (toks.Length() == 1)
			out->PutS(str.RTrim(*toks));
		else {
			const char *p, *b = str;
			p = b + str.Length() - 1; 
			while(p >= b && strchr((const char *)toks, *p)) 
				--p; 
			++p; 
			out->PutS(b, p - b); 
		}
	} else
		out->PutS((*args)[0].RTrim());
}

void EvalTrim(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("trim", 1, 2);
	if (args->Count() > 1) {
		CStr str = (*args)[0];
		if (str.IsEmpty()) 
			return;

		CStr toks = (*args)[1];
		if (toks.Length() == 0)
			out->PutS(str);
		else if (toks.Length() == 1)
			out->PutS(str.Trim(*toks));
		else {
			const char *b, *p = str;
			size_t i = strspn(p,toks); 
			b = p += i;
			p += str.Length() - 1 - i; 
			while(p >= b && strchr(toks.Data(), *p)) 
				--p; 
			++p; 
			out->PutS(b, p - b); 
		}
	} else
		out->PutS((*args)[0].Trim());
}

void EvalLen(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int acc = 0, i; for (i = 0; i < args->Count(); ++i) {
		acc += (*args)[i].Length();
	}
	out->PutN(acc);
}

void EvalFmt(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr val = (*args)[0];

	ColFmt col; 
	memset(&col, 0, sizeof(col));

	col.dec = args->Count() > 1 ? ParseInt((*args)[1]) : 2;

	if (args->Count() > 2) {
		CStr dch = (*args)[2];
		col.dch = (dch.IsEmpty()) ? 0 : *dch;
	} else
		col.dch = '.';

	if (args->Count() > 3) {
		CStr tho = (*args)[3];
		col.tho = (tho.IsEmpty()) ? 0 : *tho;
	} else
		col.tho = ',';

	CStr res(256);
	double dval;
	const char *p = CTabFmt::NumFmt(col, val.SafeP(), res.GetBuffer(), 256, &dval);
	out->PutS(p);
}

void PadParams(qCtx *ctx, qArgAry *args, CStr &str, int &pad, CStr &fill, int &len)
{
	str = (*args)[0];
	pad = (args->Count() > 1) ? ParseInt((*args)[1]) : 0;
	
	if (args->Count() > 2)
		fill = (*args)[2];
	else
		fill = " ";
	len = fill.Length();
}

void EvalLPad(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr s, f;
	int p, i, j = 0, l;

	PadParams(ctx, args, s, p, f, l);
	if (l > 0)
		for (i = s.Length(); i < p; ++i) {
			out->PutC(f[j++ % l]);
		}

	out->PutS(s);
}

void EvalRPad(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr s, f;
	int p, i, j = 0, l;

	PadParams(ctx, args, s, p, f, l);

	out->PutS(s);
	if (l > 0)
		for (i = s.Length(); i < p; ++i) {
			out->PutC(f[j++ % l]);
		}
}

void EvalCPad(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr s, f;
	int p, i, j = 0, l;

	PadParams(ctx, args, s, p, f, l);

	if (l > 0)
		for (i = s.Length(); i < ((p - s.Length()) / 2); ++i) {
			out->PutC(f[j++ % l]);
		}
	out->PutS(s);
	if (l > 0)
		for (; i < p; ++i) {
			out->PutC(f[j++ % l]);
		}
}

void EvalLeft(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 2) {
		CStr tmp   = (*args)[0];
		int  index = ParseInt((*args)[1]);
		if (index > 0) { 
			if (index < tmp.Length()) {
				out->PutS(tmp.SafeP(), index);
			} else {
				out->PutS(tmp);
			}
		}

	}
}

void EvalRight(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 2) {
		CStr tmp   = (*args)[0];
		int  index = ParseInt((*args)[1]);
		if (index > 0) { 
			if (index < tmp.Length()) {
				out->PutS(tmp + tmp.Length() - index, index);
			} else {
				out->PutS(tmp);
			}
		}
	}
}

void EvalMid(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 2) {
		CStr tmp   = (*args)[0];
		int  index = ParseInt((*args)[1]);
		if (args->Count() >= 3) {
			int len = ParseInt((*args)[2]);
			if (abs(index) < tmp.Length()) {
				if (index >= 0) {
					out->PutS(tmp.SafeP() + index, min(tmp.Length()-index, len));
				} else {
					out->PutS(tmp.SafeP() + max(tmp.Length() + index - abs(len), 0), min(tmp.Length(), len));
				}
			}
		} else {
			if (abs(index) < tmp.Length()) {
				if (index >= 0) {
					out->PutS(tmp.SafeP() + index, tmp.Length()-index);
				} else {
					out->PutS(tmp.SafeP(), tmp.Length() + index);
				}
			}

		}
	}
}

void EvalDup(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 2) {
		CStr str = (*args)[0];
		int dup;
		for (dup = ParseInt((*args)[1]); dup > 0; --dup) {
			out->PutS(str);
		}
	}
}

void EvalGetToken(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 3) {
		// todo, ParseFork arg zero into a blocking i/o stream...
		CStr dat = (*args)[0];
		CStr toks = (*args)[1];

		int  ind = ParseInt((*args)[2]);

		qCtxTmp sub(ctx);

		char *b = dat.SafeP();
		char *t = toks.SafeP();
		const char *p = b + strspn(b, t);
		char *e;

		sub.MapObj(&p, "token");

		while (p && *p) {
			e = strpbrk(p, t);
			if (e) {
				*e = 0;
				if (--ind < 0)
					break;
				p = (++e) + strspn(e, t);
			} else {
				if (ind > 0)
					return;
				else
					break;
			}
		}
		if (p) {
			out->PutS(p);
		}
	}
}

void EvalSkipTokens(int *skip, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 0) {
		int tmp = ParseInt((*args)[0]);
		if (tmp >= 0) {
			*skip = tmp;
		}
	}
}

void EvalEnumToken(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 3) {
		// todo, ParseFork arg zero into a blocking i/o stream...
		CStr dat = (*args)[0];
		CStr toks = (*args)[1];

		qCtxTmp sub(ctx);

		char *b = dat.SafeP();
		char *t = toks.SafeP();
		const char *p = b + strspn(b, t);
		char *e;

		bool ok = true;
		int skip = 0;
		sub.MapObj(&p, "token");
		sub.MapObj(&ok, (QOBJFUNC) EvalBreak, "break");
		sub.MapObj(&skip, (QOBJFUNC) EvalSkipTokens, "skip-tokens");

		while (p && *p && ok) {
			e = strpbrk(p, t);
			if (e) {
				*e = 0;
				if (!skip)
					sub.Parse(args->GetAt(2),out);
				else
					--skip;
				p = (++e) + strspn(e, t);
			} else {
				if (!skip)
					sub.Parse(args->GetAt(2),out);
				else
					--skip;
				break;
			}
		}
	}
}


class WNCompX {
public:
	CStr alg;
	qCtx *ctx;
	qStrBuf *out;
	const char *a;
	const char *b;
	WNCompX() {
		ctx = NULL;
		out = NULL;
		a = b = NULL;
	}
};

// TODO: change this to store only a pointer to a string instead of a copy of a string
class WNAryX {
public:
	CStr v;
	WNCompX *c;
};

int EvalWNCom(const void *a, const void *b)
{
	WNCompX *c = ((WNAryX *) a)->c;
	c->a = ((WNAryX *) a)->v;
	c->b = ((WNAryX *) b)->v;

	CStr r = c->ctx->ParseStr(c->alg);

	if (!r.IsEmpty()) {
		return atoi(r);
	} else
		return 0;
}

int EvalWNComSimple(const void *a, const void *b)
{
	return stricmp(((WNAryX *)a)->v, ((WNAryX *)b)->v);
}


void EvalStricmp(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 1) {
		CStr a = (*args)[0];;
		CStr b = (*args)[1];;

		char buf[63];
		out->PutS(_itoa(stricmp(a,b), buf, 10));
	}
}

void EvalIsUpper(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		const char * a = (*args)[0];;
		if (a && isupper(*a)) {
			out->PutS("T", 1);
		}
	}
}

void EvalIsLower(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		const char * a = (*args)[0];;
		if (a && islower(*a)) {
			out->PutS("T", 1);
		}
	}

}

void EvalEnumSort(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 2) {
		CStr datax = (*args)[0];;
		CStr body = args->GetAt(2).GetBuffer();
		if (!body)
			return;
		char * data = datax.GetBuffer();

		CStr delim = "\n\r|,;";
		CStr alg;

		if (args->Count() > 1 && args->GetAt(1))
			delim = (*args)[1];;

		if (args->Count() > 3)
			alg = args->GetAt(3);

		int num = 0;
		WNAryX *ary = NULL;
		char *p;


		WNCompX compx;

		if ((p = strtok(data, delim))) {
			int len;
			do {
				len = strlen(p);
				ary = (WNAryX*) realloc(ary, ++num * sizeof(WNAryX));
				memset(&(ary[num-1]), 0, sizeof(WNAryX));
				new(&ary[num-1]) WNAryX;
				ary[num-1].v = p;
				ary[num-1].c=&compx;
			} while ((p = strtok(NULL, delim)));
		}

		if (!alg.IsEmpty()) {
			qCtxTmp tmpCtx(ctx);
			qStrBuf tmpOut;
			compx.alg=alg;
			compx.out=&tmpOut;
			compx.ctx=&tmpCtx;
			tmpCtx.MapObj(&compx.a, "a");
			tmpCtx.MapObj(&compx.b, "b");
			qsort(ary, num, sizeof(WNAryX), EvalWNCom);
		} else 
			qsort(ary, num, sizeof(WNAryX), EvalWNComSimple);


		const char *curw;
		qCtxTmp tmpCtx(ctx);
		tmpCtx.MapObj(&curw, "token");

		bool ok = true;
		tmpCtx.MapObj(&ok, (QOBJFUNC) EvalBreak, "break");

		int i; for (i = 0; ok && i < num; ++i) {
			curw = ary[i].v;
			tmpCtx.Parse(body, out);
			ary[i].~WNAryX();
		}
		free(ary);
	}
}

// proper case
const char *uclist[] = {"LLC","LLP","LP","SCOR",0};
void pcasew(char *&c, char *&p)
{
	const char **s;
	if (!isalnum(*p)) {
		for(s = uclist; *s; ++s) {
			if (!strnicmp(c, *s, p-c))
				return;

		}
		if (!*s) {
			*c = toupper(*c);
			while(++c < p)
				*c = tolower(*c);
		}
	}
}

void EvalPcase(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr s = (*args)[0];
		char *p = s.SafeP();
		char *c=0;
		while (*p) {
			if (c) {
				if (!isalnum(*p)) {
					pcasew(c, p);
					c = 0;
				}
			} else {
				if (isalpha(*p))
					c = p;
			}
			++p;
		}
		if (c) {
			pcasew(c,p);
		}
		out->PutS(s,s.Length());
	}
}

inline static void CsvQuoteQ(const char *&pi, char *&po)
{
	while (*pi) {
		if (*pi == '\"')
			*po++ = '\"';
		*po++ = *pi++;
	}
}

void EvalCsvQuote(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr s = (*args)[0];

		char *bi = s.SafeP();
		const char *pi = bi;

		while (*pi) {
			if (*pi == '\"' || *pi == ',') {
				CStr o(s.Length()*2 + 2);
				char *po = o.SafeP();
				*po++ = '\"';
				memcpy(po, s.Data(), pi-bi);
				CsvQuoteQ(pi, po);
				*po++ = '\"';
				o.Grow(po - o.Data());
				out->PutS(o,o.Length());
				return;
			}
			++pi;
		}

		out->PutS(s,s.Length());
	}
}

void EvalCsvQuoteQ(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr s = (*args)[0];
		CStr o(s.Length()*2 + 2);

		char *bi = s.SafeP();
		const char *pi = bi;
		char *po = o.SafeP();

		*po++ = '\"';
		CsvQuoteQ(pi, po);
		*po++ = '\"';

		o.Grow(po - o.Data());
		out->PutS(o,o.Length());
	}
}

void EvalUcase(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr tmp = (*args)[0];
		if (tmp)
			out->PutS(strupr(tmp.GetBuffer()), tmp.Length());
	}
}


void EvalGetAttrValue(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr tmp = (*args)[0];
		if (args->Count() > 1) {
			CStr m = (*args)[1];
			if (tmp && !m.IsEmpty()) {
				char *p = tmp.GetBuffer();
				char *n = p;
				char *v = 0;
				while (*p) {
					while (isspace(*p))
						++p;
					if (*p == '=') {
						*p++ = '\0';
						while (isspace(*p))
							++p;
						v = p;
						while (*p) {
							if (*p == ';') {
								*p = '\0';
								if (!stricmp(m, n)) {
									out->PutS(v);
									return;
								}
								while (isspace(*++p));
								n = p;
								v = NULL;
								break;
							}
							++p;
						}
					} else if (*p == ';')
						n = p + 1;
					++p;
				}
				if (v && *v && n && *n && !stricmp(m, n)) {
					out->PutS(v);
					return;
				}
			}
		}
	}
}


void EvalLcase(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CStr tmp = (*args)[0];
		if (tmp)
			out->PutS(strlwr(tmp.GetBuffer()), tmp.Length());
	}
}

void EvalNLTrim(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; for (i = 0; i < args->Count(); ++i) {
		qStrNLTrim nlout(out);
		ctx->Parse(args->GetAt(i), &nlout);
	}
}

void EvalSubExpr(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CRegX *myRx = (CRegX *)data;
	int myIndex = ParseInt((*args)[0]);
	const char *sp; const char *ep;
	if (myIndex >=0 && myRx->GetMatchInfo(sp, ep, myIndex))
		out->PutS(sp, ep-sp);
}

void EvalRxMatch(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("rxmatch", 2, 4);

	CRegX rx;
	CStr str  = (*args)[0];
	CStr regx = (*args)[1];
	CStr flags = args->GetAt(3);

	if (!rx.Compile(regx)) {
		ctx->ThrowF(out, 701, "Invalid regular expression (%s)", rx.GetError() ? rx.GetError() : (const char *) regx);
		return;
	}

	rx.SetCase(false);

	const char * p;
	
	p = flags;
	while (p && *p) {
		if ((*p) == 's') rx.SetCase(true);
		++p;
	}

	p = str;
	int loc = rx.Match(p);
	if (loc) {
		if (args->Count() >= 3) {
			CStr body = args->GetAt(2);

			qCtxTmp tmpCtx(ctx);
			tmpCtx.MapObj(&rx, EvalSubExpr, "subx");
			tmpCtx.Parse(body, out);
		} else {
			const char *sp; const char *ep;
			if (rx.GetMatchInfo(sp, ep, 0))
				out->PutS(sp, ep-sp);
		}
	}
}

void EvalRxSplit(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("rxsplit", 2, 4);

	CRegX rx;
	CStr str  = (*args)[0];
	CStr regx = (*args)[1];
	CStr body = args->GetAt(2);
	CStr flags = args->GetAt(3);

	rx.SetCase(false);

	const char * p;
	
	p = flags;
	while (p && *p) {
		if ((*p) == 's') rx.SetCase(true);
		++p;
	}

	if (!rx.Compile(regx)) {
		ctx->ThrowF(out, 701, "Invalid regular expression (%s)", rx.GetError() ? rx.GetError() : (const char *) regx);
		return;
	}

	p = str;
	int loc = rx.Match(p);
	int cnt = 0;

	const char *sp; const char *ep = p;

	qCtxTmp tmpCtx(ctx);
	tmpCtx.MapObj(&rx, EvalSubExpr, "subx");
	tmpCtx.MapObj("", "last");
	tmpCtx.MapObj(&cnt, "rxnum");

	while (loc && *p) {
		++cnt;

		rx.GetMatchInfo(sp, ep, 0);
		tmpCtx.MapObj(CStr(p, sp-p), "elem");
		tmpCtx.MapObj(CStr(p, sp-p), "token");

		if (ep && !*ep)
			tmpCtx.MapObj("T", "last");

		tmpCtx.Parse(body, out);

		if (ep <= p)
			break;
		else
			loc = rx.Match(p=ep);
	}
	
	if ((loc && !(ep && !*ep)) || (ep && *ep)) {
		++cnt;
		tmpCtx.MapObj("T", "last");

		if (ep && *ep)
			str = ep;
		tmpCtx.MapObj(str, "elem");
		tmpCtx.MapObj(str, "token");
		tmpCtx.Parse(body, out);
	}

	if (cnt)
		ctx->MapObj(cnt, "rxcount");
	else
		ctx->MapObj("", "rxcount");
}


void EvalPack(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGM("pack", 2);
	CStr templ = (*args)[0];
	CStr sarg;

	char t;
	const char * p = templ;
	char *endp = 0;
	if (!p) return;

	int nlen, plen, i = 1, j;

	while (*p) {
		switch(*p) {
		case 'a':
		case 'A':
			if (isdigit(p[1])) {
				nlen = strtol(p+1, &endp, 0);
				p = endp-1;
				if (nlen <= 0) nlen =1;
			} else
				nlen =1;
			 sarg = (*args)[i++];
			 plen = sarg.Length();
			 sarg.Grow(nlen);
			 if (nlen > plen)
				 memset(sarg.Data()+plen,(*p == 'a' ? ' ' : '\0'),nlen-plen); 
			 out->PutS(sarg);
		case 'l':
		case 'L':
			t = *p;
			if (isdigit(p[1])) {
				nlen = strtol(p+1, &endp, 0);
				p = endp-1;
				if (nlen <= 0) nlen =1;
			} else
				nlen =1;
			for(j = 0; j < nlen; ++j) {
				DWORD dw;
				sarg = (*args)[i++];
				if (t == 'l')
					dw = (DWORD) strtoul(sarg.Data(), &endp, 0);
				else if (t == 'L')
					dw = (DWORD) strtol(sarg.Data(), &endp, 0);
				out->PutS((char *)&dw,sizeof(dw));
				
			}
		}
		++p;
	}
}

void EvalUnpack(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGM("unpack", 2);

	CStr templ = (*args)[0];
	CStr sdata = (*args)[1];

	char t;
	const char * p = templ;
	char *endp = 0;
	if (!p) return;


	const char * d = sdata;
	int dlen = sdata.Length();

	int nlen, j;

	while (*p) {
		switch(*p) {
		case 'a':
		case 'A':
			if (isdigit(p[1])) {
				nlen = strtol(p+1, &endp, 0);
				p = endp-1;
				if (nlen <= 0) nlen =1;
			} else
				nlen =1;
			 nlen = min(nlen, dlen);
			 out->PutS(d, nlen);
			 d += nlen;
			 dlen -=4;
		case 'l':
		case 'L':
			if (dlen >= 4) {
				t = *p;
				if (isdigit(p[1])) {
					nlen = strtol(p+1, &endp, 0);
					p = endp-1;
					if (nlen <= 0) nlen =1;
				} else
					nlen =1;
				for(j = 0; j < nlen; ++j) {
					if (t == 'l')
						out->PutN(*(long *) d);
					else if (t == 'L')
						out->PutN(*(unsigned int *) d);
				}
			}
		}
		++p;
	}
}


void LoadString(qCtx *ctx) {
//string
	ctx->MapObj(EvalLTrim, "trimleft");
	ctx->MapObj(EvalRTrim, "trimright");

	ctx->MapObj(EvalLTrim, "ltrim");
	ctx->MapObj(EvalRTrim, "rtrim");

	ctx->MapObj(EvalTrim,  "trim");

	ctx->MapObj(EvalLPad,  "lpad");
	ctx->MapObj(EvalRPad,  "rpad");
	ctx->MapObj(EvalCPad,  "cpad");

	ctx->MapObj(EvalLen,  "len");
	ctx->MapObj(EvalFmt,  "fmt");

	ctx->MapObj(EvalLeft,  "left");
	ctx->MapObj(EvalRight, "right");
	ctx->MapObj(EvalMid,   "mid");
	ctx->MapObj(EvalDup,   "dup");

	ctx->MapObj(EvalGetToken,   "gettoken");
	ctx->MapObj(EvalEnumToken,  "enumtoken", "001");
	ctx->MapObj(EvalEnumSort,  "enumsort", "0011");

	ctx->MapObj(EvalLcase, "lcase");
	ctx->MapObj(EvalUcase, "ucase");
	ctx->MapObj(EvalPcase, "pcase");

	ctx->MapObj(EvalCsvQuote, "csv-quote");
	ctx->MapObj(EvalCsvQuoteQ, "csv-quoteq");

	ctx->MapObj(EvalLcase, "tolower");
	ctx->MapObj(EvalUcase, "toupper");
	ctx->MapObj(EvalIsLower, "islower");
	ctx->MapObj(EvalIsUpper, "isupper");
	ctx->MapObj(EvalStricmp, "stricmp");

	ctx->MapObj(EvalGetAttrValue, "getattrvalue");

	ctx->MapObj("\r",    "cr");
	ctx->MapObj("\n",    "lf");
	ctx->MapObj("\r\n",  "crlf");

//comp
	ctx->MapObj(EvalLt,    "lt");
	ctx->MapObj(EvalLte,   "lte");
	ctx->MapObj(EvalGt,    "gt");
	ctx->MapObj(EvalGte,   "gte");
	ctx->MapObj(EvalEqN,   "equ");
	ctx->MapObj(EvalEqN,   "==");
	ctx->MapObj(EvalEqS,   "equal");
	ctx->MapObj(EvalEqB,   "xequal");
	ctx->MapObj(EvalWcm,   "wcmatch");
	ctx->MapObj(EvalWcmX,  "wcxmatch");

	ctx->MapObj(EvalReplace,       "replace");
	ctx->MapObj(EvalXReplace,      "xreplace");
	ctx->MapObj(EvalConcat,        "concat");

	ctx->MapObj(EvalPos,	       "pos");
	ctx->MapObj(EvalXin,	       "xin");
	ctx->MapObj(EvalIn,	           "in");

// encrypt
	ctx->MapObj(EvalSha,	       "sha");

	ctx->MapObj(EvalEncrypt,       "encrypt");
	ctx->MapObj(EvalDecrypt,       "decrypt");

#ifdef unix
	ctx->MapObj(EvalCrypt,         "crypt");
#endif

	ctx->MapObj(EvalEncode,        "encode");
	ctx->MapObj(EvalDecode,        "decode");

	ctx->MapObj(EvalObfuscate,     "obfuscate");
	ctx->MapObj(EvalDeobfuscate,   "deobfuscate");

	ctx->MapObj(EvalRxMatch,       "rxmatch","0011");
	ctx->MapObj(EvalRxSplit,       "rxsplit","0011");
	ctx->MapObj("",                "rxcount");

	ctx->MapObj(EvalPack,          "pack");

	ctx->MapObj(EvalUnpack,        "unpack");
}
