/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#include <ctype.h>
#include <math.h>
#include <time.h>

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"
#include "util.h"

#ifdef unix
#include <signal.h>
#include <setjmp.h>
#endif

class qObjLVal {

	qCtx *myCtx;
	qObjDbl *myObj;
	CStr myName;

public:
	qObjLVal(qCtx *ctx) {myObj = 0; myCtx = ctx;}
	double Set(double val) {
		if (this) if (!myObj) {
			if (myName) myObj = (qObjDbl*) (void *) myCtx->MapObj(val, myName);
		} else {
			myObj->Set(val);
		} return val;
	}
	void Map(CStr name) {if (this) {myName = name; myObj = 0;}}
};

double ParseExpr(const char *&p, qCtx *ctx, qObjLVal*lval);

inline void PutDbl(double num, qStr* out) {
	out->PutN(num);
}

void EvalAdd(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = 0;
	for (i = 0; i < args->Count(); ++i) {
		acc += ParseDbl((*args)[i]);
	}
	PutDbl(acc, out);
}

void EvalSub(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = ParseDbl((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc -= ParseDbl((*args)[i]);
	}
	PutDbl(acc, out);
}

void EvalDiv(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = ParseDbl((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc /= ParseDbl((*args)[i]);
	}
	PutDbl(acc, out);
}

void EvalMult(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = ParseDbl((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc *= ParseDbl((*args)[i]);
	}
	PutDbl(acc, out);
}

void EvalIncr(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = 0;
	for (i = 0; i < args->Count(); ++i) {
		qObj *obj;
		CStr name = args->GetAt(i);
		if (name) {
		if (ctx->Find(&obj, name)) {
			qStrBuf tmp;
			char *ep; 
			obj->Eval(ctx, &tmp);
			acc = (tmp.IsEmpty() ? 0 : strtod(tmp.GetS(), &ep)) + 1;
		} else 
			acc = 1;
		ctx->MapObjLet(ctx->CreateObj(acc), name);
		}
	}
}

void EvalDecr(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = 0;
	for (i = 0; i < args->Count(); ++i) {
		qObj *obj;
		CStr name = args->GetAt(i);
		if (name) {
		if (ctx->Find(&obj, name)) {
			qStrBuf tmp;
			char *ep; 
			obj->Eval(ctx, &tmp);
			acc = (tmp.IsEmpty() ? 0 : strtod(tmp.GetS(), &ep)) - 1;
		} else 
			acc = -1;
		ctx->MapObjLet(ctx->CreateObj(acc), name);
		}
	}
}

void EvalAddX(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc;
	qObj *obj = NULL;

	CStr var = (*args)[0];

	if (var) {
	if (ctx->Find(&obj, var)) {
		qStrBuf tmp;
		char *ep; 
		obj->Eval(ctx, &tmp);
		acc = (tmp.IsEmpty() ? 0 : strtod(tmp.GetS(), &ep));
	} else 
		acc = 0;
	
	if (args->Count() > 0) {
		
		for (i = 1; i < args->Count(); ++i) {
			acc += ParseDbl((*args)[i]);
		}

	}
	if (obj)
		ctx->MapObjLet(ctx->CreateObj(acc), var);
	else
		ctx->MapObj(ctx->CreateObj(acc), var);
	}
}

void EvalSubX(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc;
	qObj *obj;

	CStr var = (*args)[0];
	if (var) {
	if (ctx->Find(&obj, var)) {
		qStrBuf tmp;
		char *ep; 
		obj->Eval(ctx, &tmp);
		acc = (tmp.IsEmpty() ? 0 : strtod(tmp.GetS(), &ep));
	} else 
		acc = 0;
	
	if (args->Count() > 0) {
		
		for (i = 1; i < args->Count(); ++i) {
			acc -= ParseDbl((*args)[i]);
		}

	}
	if (obj)
		ctx->MapObjLet(ctx->CreateObj(acc), var);
	else
		ctx->MapObj(ctx->CreateObj(acc), var);
	}
}


void EvalIAdd(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	int acc = 0;
	for (i = 0; i < args->Count(); ++i) {
		acc += ParseInt((*args)[i]);
	}
	out->PutN(acc);
}

void EvalISub(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	int acc = ParseInt((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc -= ParseInt((*args)[i]);
	}
	out->PutN(acc);
}

#ifdef unix
jmp_buf sigenv;
static void signal_handler(int sig)
{
	longjmp(sigenv,sig);
}

#define BEGINFPE if (setjmp(sigenv)!=0) {return;} sigset(SIGFPE, signal_handler);
#define ENDFPE signal(SIGFPE, SIG_DFL);
#else
#define BEGINFPE
#define ENDFPE 
#endif

void EvalIDiv(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("idiv", 2, 2);
	BEGINFPE
	int i; 
	int acc = ParseInt((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc /= ParseInt((*args)[i]);
	}
	out->PutN(acc);
	ENDFPE
}

void EvalIMult(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	int acc = ParseInt((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc *= ParseInt((*args)[i]);
	}
	out->PutN(acc);
}

void EvalIMod(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("imod", 2, 2);
	BEGINFPE
	int v1 = ParseInt((*args)[0]);
	int v2 = ParseInt((*args)[1]);
	out->PutN(v1%v2);
	ENDFPE
}

void EvalMax(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() == 0 )
		return;
	int i; 
	double acc = ParseDbl((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc = max(acc, ParseDbl((*args)[i]));
	}
	PutDbl(acc, out);
}

void EvalMin(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() == 0 )
		return;
	int i; 
	double acc = ParseDbl((*args)[0]);
	for (i = 1; i < args->Count(); ++i) {
		acc = min(acc, ParseDbl((*args)[i]));
	}
	PutDbl(acc, out);
}

double ParseDbl(const char *&p) {
	char *e;
	double val = strtod(p, &e);	
	p = e;
	return val;
}

void ParseExprArgs(const char *&p, qCtx *ctx, qArgAry *ary) {
	++p;
	while (*p) {
		if (!isspace(*p)){
			const char *b = p;
			while (*p && *p != ',' && *p != ')') {
				++p;
			}
			ary->Add(CStr(b, p - b));
			if (*p ==')') {
				++p;
				return;
			} else if (*p ==',')
				++p;

		} else 
			++p;
	}
}

double ParseExprVar(const char *&p, qCtx *ctx, qObjLVal *lval) {
	const char *n = p;
	while ( *p ) {
		if (!(__iscsym(*p) || *p == '-'  || *p == '/')) {
			break;
		}
		++p;
	}
	qObj *obj;
	CStr name(n, p - n);
	lval->Map(name);

	if (ctx->Find(&obj, name)) {
		qArgAry ary;
		while(isspace(*p)) ++p;
		if (*p == '(') {
			ParseExprArgs(p, ctx, &ary);
		}
		qStrBuf out;
		obj->Eval(ctx, &out, &ary);
		const char *t = out;
		return ::ParseDbl(t);
	} else {
		return 0.0;
	}
}

double ParseExprTop(const char *&p, qCtx *ctx, qObjLVal *lval);
double ParseExprL2(const char *&p, qCtx *ctx, qObjLVal *lval);

double ParseExprBinOp2(const char *&p, qCtx *ctx, double v1, qObjLVal *lval) {
	while ( *p ) {
		if (isspace(*p))
			++p;
		else switch(*p) {
			case '/' : {
				if (p[1]=='=') {
					v1 /= ParseExprTop(p+=2,  ctx, 0);
					if (lval) lval->Set(v1);
				} else  {
					v1 /= ParseExprL2(p+=1,  ctx, 0);
				}
				break;
			}
			case '*' : {
				if (p[1]=='=') {
					v1 *= ParseExprTop(p+=2,  ctx, 0);
					if (lval) lval->Set(v1);
				} else  {
					v1 *= ParseExprL2(p+=1,  ctx, 0);
				}
				break;
			}
			case '^' : {
				if (p[1]=='=') {
					v1 = pow(v1,ParseExprTop(p+=2,  ctx, 0));
					if (lval) return lval->Set(v1);
				} else  {
					v1 = pow(v1,ParseExprL2(p+=1,  ctx, 0));
				}
				break;
			}
			default : {
				return v1;
			}
		}
	}
	return v1;
}

double ParseExprBinOp(const char *&p, qCtx *ctx, double v1, qObjLVal *lval) {
	while ( *p ) {
		if (isspace(*p))
			++p;
		else switch(*p) {
			case '=' : {
				if (p[1] == '=') {
					v1 = (v1 == ParseExprTop(p+=2,  ctx, 0));
				} else {
					v1 = ParseExprTop(p+=1,  ctx, 0);
					if (lval) lval->Set(v1);
				}
				break;
			}
			case '+' : {
				if (p[1]=='=') {
					v1 += ParseExprTop(p+=2,  ctx, 0);
					if (lval) lval->Set(v1);
				} else  {
					v1 += ParseExprL2(p+=1,  ctx, 0);
				}
				break;
			}
			case '-' : {
				if (p[1]=='=') {
					v1 -= ParseExprTop(p+=2,  ctx, 0);
					if (lval) lval->Set(v1);
				} else  {
					v1 -= ParseExprL2(p+=1,  ctx, 0);
				}
				break;
			}
			case ')' : {
				return v1;
			}
			default : {
				const char * t = p;
				v1 = ParseExprBinOp2(p,  ctx, v1, lval);
				if (t == p)
					return v1;
			}
		}
	}
	return v1;
}

double ParseExprTop(const char *&p, qCtx *ctx, qObjLVal *lval) {
	double val = ParseExpr(p, ctx, lval);
	return ParseExprBinOp(p, ctx, val, lval);
}

double ParseExprL2(const char *&p, qCtx *ctx, qObjLVal *lval) {
	double val = ParseExpr(p, ctx, lval);
	return ParseExprBinOp2(p, ctx, val, lval);
}

double ParseExprGroup(const char *&p, qCtx *ctx, qObjLVal *lval) {
	++p;
	double val = ParseExprTop(p, ctx, lval);
	while(isspace(*p)) ++p;
	if (*p == ')') {
		++p;
	}
	return val;
}

double ParseExprLV(const char *&p, qCtx *ctx, qObjLVal *lval) {
	while(isspace(*p)) ++p;
	if (*p == '(')
		return ParseExprGroup(p, ctx, lval);
	else 
		return ParseExprVar(p, ctx, lval);
}

double ParseExpr(const char *&p, qCtx *ctx, qObjLVal *lval) {
	double val;
	CStr name; 
	while ( *p ) {
		if (__iscsymf(*p)) {
			qObjLVal var(ctx);
			return val = ParseExprVar(p, ctx, &var);
		} else if (isdigit(*p)) {
			return val = ::ParseDbl(p);
		} else if (*p == '(') {
			return val = ParseExprGroup(p, ctx, lval);
		} else if (*p == '-') {
			if (p[1] == '-') {
				qObjLVal var(ctx);
				val = ParseExprLV(p+=2, ctx, &var);
				return var.Set(--val);
			} else {
				return -ParseExpr(++p, ctx, lval);
			}
		} else if (*p == '+') {
			if (p[1] == '+') {
				qObjLVal var(ctx);
				val = ParseExprLV(p+=2, ctx, &var);
				return var.Set(++val);
			} else {
				return ParseExpr(++p, ctx, lval);
			}
		} else if (*p == '!') {
			return ParseExpr(++p, ctx, lval) != 0.0;
		} 
		if (*p) {
			++p;
		}
	}
	return 0.0;
}

void EvalExpr(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("expr", 1, 1);
	CStr expr = (*args)[0];
	const char *p = (const char *) expr;
	double v = ParseExprTop(p, ctx, 0);
	PutDbl(v, out);
}

void EvalRand(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("rand", 0, 1);
	int range;
	if (args->Count() > 0) {
		range = (int) ParseInt((*args)[0]);
		out->PutN((int) ((((unsigned long)rand())<<16)|(((unsigned long)rand()))) % range);
	} else {
		out->PutN(rand());
	}
}

void EvalRound(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("round", 1, 2);
	double val = ParseDbl((*args)[0]);
	int num = 0;
	int dec; int sig;
	if (args->Count() == 2) {
		num = ParseInt((*args)[1]);
	}
	
	char *buf = _fcvt(val, num, &dec, &sig);
	
	if (sig)
		out->PutC('-');

	if (dec <= 0) {
		if (*buf) {
			out->PutC('.');
			for ( ; dec < 0; ++dec) {
				out->PutC('0');
			}
			out->PutS(buf);
		} else
			out->PutC('0');
	} else {
		for ( ; *buf && dec > 0; dec--) {
			out->PutC(*buf++);
		}
		if (!*buf) {
			for ( ; dec > 0; dec--) {
				out->PutC('0');
			}
		} else {
			out->PutC('.');
			out->PutS(buf);
		}
	}
}

void EvalTrunc(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("trunc", 1, 1);
	int r = ParseInt((*args)[0]);
	out->PutN(r);
}

void EvalAbs(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	out->PutN(fabs(ParseDbl((*args)[0])));
}

void EvalIAbs(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	out->PutN(abs(ParseInt((*args)[0])));
}

void EvalAsc(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	out->PutC(ParseInt((*args)[0]));
}

void EvalOrd(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr p = (*args)[0];
	if (!p.IsEmpty()) {
		out->PutN(*(unsigned char*)(p.Data()));
	}
}

void EvalAvg(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i; 
	double acc = 0;
	for (i = 0; i < args->Count(); ++i) {
		acc += ParseDbl((*args)[i]);
	}
	PutDbl(acc/args->Count(), out);
}

void EvalINot(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	out->PutN(~ParseInt((*args)[0]));
}
void EvalIXor(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int r = ParseInt((*args)[0]);
	int i; for (i = 1; i < args->Count(); ++i){
		r ^= ParseInt((*args)[i]);
	}
	out->PutN(r);
}
void EvalIOr(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int r = ParseInt((*args)[0]);
	int i; for (i = 1; i < args->Count(); ++i){
		r |= ParseInt((*args)[i]);
	}
	out->PutN(r);
}
void EvalIAnd(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int r = ParseInt((*args)[0]);
	int i; for (i = 1; i < args->Count(); ++i){
		r &= ParseInt((*args)[i]);
	}
	out->PutN(r);
}
void EvalLShift(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("lshift", 2, 2);
	int r = ParseInt((*args)[0]);
	int s = ParseInt((*args)[1]);
	out->PutN(r<<s);
}
void EvalRShift(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("rshift", 2, 2);
	int r = ParseInt((*args)[0]);
	int s = ParseInt((*args)[1]);
	out->PutN(r>>s);
}

void EvalRadix(qCtx *ctx, qStr *out, CStr s, int r, int b) {
  if (b <= 0) {
    ctx->Throw(out, 649, "Radix base must be > 0");
    return;
  }
	char *e; unsigned long n = strtoul(s, &e, b);
	char tmp[sizeof(long) + 2];
	out->PutS(_ultoa(n,tmp,r));
}

void EvalHex(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	EvalRadix(ctx, out, (*args)[0], 16, ParseInt((*args)[1]));
}

void EvalOct(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	EvalRadix(ctx, out, (*args)[0], 8, ParseInt((*args)[1]));
}

void EvalBin(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	EvalRadix(ctx, out, (*args)[0], 2, ParseInt((*args)[1]));
}

void EvalRadix(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("radix", 2, 2);
	EvalRadix(ctx, out, (*args)[0],ParseInt((*args)[1]),ParseInt((*args)[2]));
}

static bool gLoaded = false;
void LoadMath(qCtx *ctx) {
//math
	if (!gLoaded) {
		gLoaded = true;
		time_t t = time(&t);
#ifdef WIN32
		clock_t c = clock();
		DWORD pid = GetCurrentProcessId();
		DWORD tid = GetCurrentThreadId();
		srand(c + _rotl(t,24) + _rotl(pid,8) + _rotl(tid,16));
#else
		srand(t);
#endif
	}

	ctx->MapObj(EvalMax,   "max");
	ctx->MapObj(EvalMin,   "min");
	ctx->MapObj(EvalAvg,   "avg");

	ctx->MapObj(EvalAdd,   "add");
	ctx->MapObj(EvalAdd,   "+");
	ctx->MapObj(EvalSub,   "sub");
	ctx->MapObj(EvalSub,   "-");
	ctx->MapObj(EvalDiv,   "div");
	ctx->MapObj(EvalDiv,   "/");
	ctx->MapObj(EvalMult,  "mult");
	ctx->MapObj(EvalMult,  "*");

	ctx->MapObj(EvalIAdd,  "iadd");
	ctx->MapObj(EvalISub,  "isub");
	ctx->MapObj(EvalIDiv,  "idiv");
	ctx->MapObj(EvalIMult, "imult");

	ctx->MapObj(EvalIMod,  "mod");
	ctx->MapObj(EvalIMod,  "imod");

	ctx->MapObj(EvalIOr,   "ior");
	ctx->MapObj(EvalIOr,   "|");
	ctx->MapObj(EvalINot,  "inot");
	ctx->MapObj(EvalINot,  "~");
	ctx->MapObj(EvalIAnd,  "iand");
	ctx->MapObj(EvalIAnd,  "&");
	ctx->MapObj(EvalIXor,  "ixor");

	ctx->MapObj(EvalLShift,   "lshift");
	ctx->MapObj(EvalRShift,   "rshift");

	ctx->MapObj(EvalIncr,  "incr");
	ctx->MapObj(EvalIncr,  "++");

	ctx->MapObj(EvalAddX,  "+=");
	ctx->MapObj(EvalSubX,  "-=");

	ctx->MapObj(EvalDecr,  "decr");
	ctx->MapObj(EvalDecr,  "--");

	ctx->MapObj(EvalAbs,   "abs");
	ctx->MapObj(EvalIAbs,  "iabs");


	ctx->MapObj(EvalAsc,   "asc");
	ctx->MapObj(EvalOrd,   "ord");

	ctx->MapObj(EvalExpr,  "expr");

	ctx->MapObj(EvalRand,  "rand");


	ctx->MapObj(EvalRound,  "round");
	ctx->MapObj(EvalTrunc,  "trunc");

	ctx->MapObj(EvalHex,    "hex");
	ctx->MapObj(EvalOct,    "oct");
	ctx->MapObj(EvalBin,    "bin");
	ctx->MapObj(EvalRadix,  "radix");
}
