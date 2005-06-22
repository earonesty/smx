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
#include "qctx-comp.h"

#include <stdarg.h>

char qCtx::sBQ2 = T_BQ2;
char qCtx::sEQ2 = T_EQ2;

#define T_LP '('
#define T_RP ')'
#define T_LC '{'
#define T_RC '}'

bool ParseArgQuoted(qStr *in, qArg &cur, char c);
inline char ParseArgsLPar(qStr *in, qStr *out, bool quoted = true);
char ParseArgsLPar(qStr *in, CStr &out, bool quoted = true);
char ParseArgEscaped(qStr *in, qArg &cur, char eq);

qCtx::~qCtx() 
{
	Clear();
}

void qCtx::Clear(bool force) 
{
	MAPPOS pos;
	const char * key=NULL;
	qObj **obj;
  try {
  	for (pos = myMap.First(); myMap.Next(&pos, &key, &obj); ) {
  		if (*obj) {
#ifdef _CTX_DEBUG
      fprintf(stderr, "qctx free: %s\n", (char *) key);
#endif
        if ((const char *)key == NULL)
          break;
	try {
	        (*obj)->Free(force);
	} catch (...) {
	}
      }
  		*obj = NULL;
  	}
    myMap.Clear();
  } catch (...) {
  }
}

void qCtx::ParseTry(qStr *in, qStr *out)
{
	try {
		ParseTrim(in, out);
	} catch (qCtxExAbort) {
		out->Clear();
		return;
	} catch (qCtxEx ex) {
		if (!ex.GetLineNum())
			ex.SetLineNum(in->GetLineNum());

		if (!myErrorHandler.IsEmpty()) {
			try {
				qCtxTmp sub(this);

				sub.MapObj(ex.GetLineNum(), "exception-line");
				sub.MapObj(ex.GetID(),      "exception-num");
				sub.MapObj(ex.GetMsg(),     "exception-msg");

				sub.Parse(myErrorHandler,out);
				return;
			} catch (qCtxEx ex) {
				out->PutS("Handler Error# ");
				out->PutN(ex.GetID());
				if (ex.GetLineNum()) {
					out->PutS(" @");

					out->PutS(ex.GetLineNum());
				}
				out->PutS(": ");
				out->PutS(ex.GetMsg());
				out->PutC('.');
			}
		}

		out->PutS("Unhandled Error# ");
		out->PutN(ex.GetID());
		out->PutS(": ");
		out->PutS(ex.GetMsg());
//		out->PutS(": ");
//		out->PutS(GetEnv() && GetEnv()->GetSessionCtx() ? GetEnv()->GetSessionCtx()->GetLNum() : GetLNum());
		out->PutC('.');
	}
}

bool qCtx::ParseMagic(qStr *in, qStr *out, bool bProcessUnhandled)
{
	CStr tmp;
	char c;

	c = in->GetC();

	if ( c != EOF ) {
		if (c == S_MAGIC1[0]) {
			tmp << c;
			int i = 0;
			while ( (S_MAGIC1[++i]) && (c = in->GetC()) != EOF ) {
				tmp << c;
				if (c != (S_MAGIC1[i]) ) {
					break;
				}
			}
			if (c != EOF && (!S_MAGIC1[i])) {
				ParseTry(in, out);
				return true;
			}
			else {
				if (bProcessUnhandled)
					out->PutS(tmp);
				return false;
			}
		} else if (c == S_MAGIC2[0]) {
			tmp << c;
			int i = 0;
			while ( (S_MAGIC2[++i]) && (c = in->GetC()) != EOF ) {
				tmp << c;
				if (c != (S_MAGIC2[i]) ) {
					break;
				}
			}
			if (c != EOF && (!S_MAGIC2[i])) {
				ParseTry(in, out);
				return true;
			} else {
				if (bProcessUnhandled)
					out->PutS(tmp);
				return false;
			}
		} else if (c == CMAGIC_V1[0]) {
			tmp << c;
			int i = 0;
			while ( (CMAGIC_V1[++i]) && (c = in->GetC()) != EOF ) {
				tmp << c;
				if (c != (CMAGIC_V1[i]) ) {
					break;
				}
			}
			if (c != EOF && (!CMAGIC_V1[i])) {
				RunCompiled(this, in, out);
				return true;
			} else {
				if (bProcessUnhandled)
					out->PutS(tmp);
				return false;
			}
		} else {
			if (bProcessUnhandled) {
				out->PutC(c);
				out->PutS(*in);
			}
			return false;
		}
	} else {
		if (bProcessUnhandled)
			out->PutS(tmp);
		return false;
	}
}

inline void qCtx::ParseC(qStr *in, qStr *out, char c)
{
	if (c == '%') {
		ParseFunc(in, out);
	} else {
		out->PutC(c);
	}
}

void qCtx::ParseTrim(qStr *in, qStr *out)
{
	char c;
	while ( (c = in->GetC()) != EOF ) {
		if (!isspace(c))
			break;
		if (c == '\n') {
			c = in->GetC();
			break;
		}
	}
	if (c != EOF)
		ParseC(in, out, c);
	while ( (c = in->GetC()) != EOF ) {
		ParseC(in, out, c);
	}
}

void qCtx::Parse(qStr *in, qStr *out)
{
	if (GetLevel() > myEnv->GetMaxLevels()) {
		CStr s;
		while (!(s = in->GetS()).IsEmpty()) {
			out->PutS(s);
		}
		Throw(out, 99, "Maximum nesting level reached.");
	} else {
		char c;
		while ( (c = in->GetC()) != EOF ) {
			ParseC(in, out, c);
		}
	}
}


#define isqsymf(c) ( ((c) > T_RP && (c) < ';') || ((c) > '?' && (c) < T_LC) )
#define isqsym(c)  ((c) > T_RP && (c) < 127)

bool qCtx::ParseFunc(qStr *in, qStr *out)
{
	char *map;
	char c;
//	int pop = 0;

	c = in->GetC();
	
	if (isqsymf(c)) {
		qObj *obj;
		CStr name(c,1);
		while ( (c = in->GetC()) != EOF ) {
			if (c == T_LP) {
				qArgAry ary;
				if (Find(&obj, (const CStr &) name)) {
					if ((map = obj->GetQmap())) {
						ParseArgsQmap(in, &ary, map);
					} else {
						ParseArgs(in, &ary);
					}
					if (myTry)
						obj->Eval(this, out, &ary);
					else {
						try {
							obj->Eval(this, out, &ary);
						} catch (qCtxEx ex) {
							Throw(out, ex);
						} catch (qCtxExAbort ex) {
							throw ex;
						} catch (...) {
							Throw(out, 999, "Ctx Unhandled exception.");
						}
					}
					return true;
				} else {
					if (myStrict) {
						ThrowF(out, 98, "Function '%s' was not found.", (const char *) name);
					} else {
						out->PutC('%');
						out->PutS(name);
						out->PutC(T_LP);
						ParseArgsLPar(in, out);
						return false;
					}
				}
			} else if (c == '%') {
				if (name.Length()==0) {
					break;
				} else if (Find(&obj, name)) {
					obj->Eval(this, out, NULL);
					return true;
				} else {
					if (myStrict) {
						ThrowF(out, 98, "Function '%s' was not found.", (const char *) name);
					}
					break;
				}
			} else if (isqsym(c)) {
				name << c;
			} else 
				break;
		}

		if (name.Length()!=0) {
			out->PutC('%');
			out->PutS(name);
		} else if (c == T_LP)
			out->PutC('%');

		if (c != EOF)
			out->PutC(c);
	} else {
		out->PutC('%');
		if (c != EOF && c != '%') {
			if ((c == T_RP || c == ',' || c == T_LP || c == '"')) {
				if (!in->UngetC(c) || myStrict) {
					ThrowF(out, 98, "Invalid syntax '%%%c'.", c);
				}
			} else {
				out->PutC(c);
			}
		}
	}
	return false;
}

void qCtx::ParseArgs(qStr *in, qArgAry *out)
{
	bool quot = false;
	bool more = true;
	qStrBuf cur;
//	bool trim = true;
	char c;

	do {
		while ( isspace((c = in->GetC())) );

		if (c == T_SQ) {
			quot = true; 
			c = in->GetC();
			more = ParseArgQuoted(in, cur, c);
		} else {
			quot = false;
			more = ParseArg(in, cur, c);
		}

		out->Add(cur);
		cur.Grow(0);
		out->SetQuot(out->Count()-1,quot);
	} while (more && c != EOF);
}

inline bool qCtx::ParseArg(qStr *in, qStrBuf &cur, char c)
{
	bool more = true;
	int numsp = 0;
	char quoted = '\x0';
	int qcnt = 0;
	int lpc = 0;

	if (c == '"')
		{ quoted = '"'; c = in->GetC(); }
	else if (c == T_LC)
		{ quoted = T_LC; c = in->GetC(); qcnt = 1; }

	do {
		if (c == T_ESC) {
			c = in->GetC();
			if (c != EOF)
				cur << c;
			numsp = 0;
		} else if (c == '%') {
			ParseFunc(in, &cur);
			numsp = 0;
		} else if (quoted) {
			if (c == T_LC && quoted == T_LC) {
				++qcnt;
			} else if (c == '"' && quoted == '"') {
				quoted = '\x0';
				lpc = 0;
				numsp = 0;
			} else if ( (c == T_RC) && (quoted == T_LC) && (--qcnt <= 0) ) {
				quoted = '\x0';
				numsp = 0;
			} else {
				cur << c;
			}
		} else if (lpc) {
			if (c == T_RP) {
				--lpc;
				cur << c;
				numsp = 0;
			} else if (c == T_LP) {
				++lpc;
				cur << c;
				numsp = 0;
			} else if (isspace(c)) {
				cur << c;
				++numsp;
			} else {
				numsp = 0;
				cur << c;
			}
		} else {
			if (c == ',') {
				if (numsp) 
					cur.Grow(cur.Length() - numsp);
				break;
			} else if (c == T_LP) {
				cur << c;
				++lpc;
				numsp = 0;
			} else if (c == T_RP) {
				if (numsp) 
					cur.Grow(cur.Length() - numsp);
				more = false;
				break;
			} else if (isspace(c)) {
				++numsp;
				cur << c;
			} else {
				numsp = 0;
				cur << c;
			}
		}
	} while (EOF != (c = in->GetC()));

	return c == EOF ? false : more;
}

void qCtx::ParseArgsQmap(qStr *in, qArgAry *out, char *qmap)
{
	bool quot = false;
	bool more = true;
//	bool trim = true;
//	char quoted = '\x0';
//	int qcnt = 0;
	char c;
//	int acnt = 0;

	if (!qmap) {
		ParseArgs(in, out);
		return;
	}

	char qmode = *qmap == 'A' ? '1' : *qmap == '1' ? '1' : '0';

	if (*qmap != 'A' )
		++qmap;

	do {
		qStrBuf cur;
		while ( isspace((c = in->GetC())) );

		if (c == T_SQ) {
			quot = true; 
			c = in->GetC();
		} else {
			quot = false;
		}

		if (qmode == '1' || quot) {
			more = ParseArgQuoted(in, cur, c);
		} else {
			more = ParseArg(in, cur, c);
		}

		out->Add(cur);
		out->SetQuot(out->Count()-1,quot);

		if (*qmap) {
			if (*qmap == '1') {
				qmode = '1';
				++qmap;
			} else if (*qmap == '0') {
				qmode = '0';
				++qmap;
			}
		} else {
			qmode = '0';

		}
	} while (more && c != EOF);
}


bool ParseArgQuoted(qStr *in, qArg &cur, char c)
{
	bool trim;

	if (c == '"') {
		if (ParseArgEscaped(in, cur, T_EQ1) == EOF)
			return false;
		while ( isspace((c = in->GetC())) );
		trim = false;
	} else if (c == T_BQ2) {
		if (ParseArgEscaped(in, cur, T_EQ2) == EOF)
			return false;
		while ( isspace((c = in->GetC())) );
		trim = false;
	} else {
		trim = true;
	}

	do {
		if (c == T_ESC) {
			if ( (c = in->GetC()) != EOF ) {
				cur << c;

			}
		} else if (c == ',') {
			if (trim) cur.RTrim();
			return true;
		} else if (c == T_LP) {
			cur << T_LP;
			if (ParseArgsLPar(in, cur) == EOF) {
				if (trim) cur.RTrim();
				return false;
			}
		} else if (c == T_RP) {
			if (trim) cur.RTrim();
			return false;
		} else 
			cur << c;
	} while ( (c = in->GetC()) != EOF );

	if (trim) cur.RTrim();
	return false;
}

char ParseArgEscaped(qStr *in, qArg &cur, char eq)
{
	char c;
	bool pct = false;
	while ( (c = in->GetC()) != EOF ) {
		if (c == T_ESC) {
			if ( (c = in->GetC()) != EOF ) {
				cur << c;
			}
		} else if (c == '%') {
			pct = !pct;
			cur << c;
		} else if (c == eq) {
			return c;
		} else if (c == T_LP && pct) {


			cur << c;
			if (ParseArgsLPar(in, cur) == EOF)
				return EOF;
			pct = false;
		} else 
			cur << c;
	}
	return c;
}

inline char ParseArgsLPar(qStr *in, qStr *out, bool quoted)
{
	CStr tmp;
	char c = ParseArgsLPar(in, tmp, quoted);
	out->PutS(tmp);
	return c;
}

char ParseArgsLPar(qStr *in, CStr &out, bool quoted)
{
	char c;
	int lpc = 0;

	do {
		while ( isspace((c = in->GetC())) );
		do {
			if (c == T_ESC) {
				if (quoted)

					out << c;
				c = in->GetC();
				out << c;
			} else if (c == T_LP) { 
				++lpc;
				out << c;
				break;
            } else if (c == T_RP) {
				out << c;
				if (!lpc) {
					return c;
				} else {
					--lpc;
				}
			} else {
				out << c;
			}
		} while ( (c = in->GetC()) != EOF );

	} while (c != EOF);

	return c;
}


void qCtx::Dump(qStr *out)
{
	MAPPOS pos;
	const char * key;
	qObj *obj;
	for (pos = myMap.First(); myMap.Next(&pos, &key, &obj); ) {
		out->PutS(key);
		out->PutS(": ");
		obj->Dump(out);
		out->PutS("\n");
	}
}

qCtx *qCtx::DelObjTop(const char *name, qCtx *top)
{
	qCtx *where = this;
	while (where != top  && where->GetParent()) {
		where->DelObj(name);
		where = where->GetParent();
	}
	return where;
}

qCtx *qCtx::DelObjTop(const CStr &name, qCtx *top)
{
	qCtx *where = this;
	while (where != top  && where->GetParent()) {
		where->DelObj(name);
		where = where->GetParent();
	}
	return where;

}


qCtx *qCtx::MapObjTop(qObj *obj, const CStr &name, qCtx *top)
{
	qCtx *where = DelObjTop(name, top);
	if (!where) 
		where = this;
	where->MapObj(obj, name);
	return where;
}

void qCtx::SetErrorHandler(CStr str) {
	if (!myErrorHandler.IsEmpty()) RemTry();
	myErrorHandler = str;
	if (!myErrorHandler.IsEmpty()) AddTry();

}

void qCtxExData::FormatV(const char *msg_p, void *pvargs)
{
	exvsprintf(&myMsg, msg_p, (va_list) pvargs);
}

/*

int qCtx::ParseInt(const CStr &str, bool quot) {
	if (quot) {
		return atoi(str.SafeP());
	} else {
		qStrBuf myTmp;
		Parse(&qStrReadBuf(str), &myTmp); 
		return atoi(myTmp.SafeP()); 
	}
}

double qCtx::ParseDbl(const CStr &str, bool quot) {
	char *ep;
	if (quot) {
		return strtod(str.SafeP(), &ep); 
	} else {
		qStrBuf myTmp;
		Parse(&qStrReadBuf(str), &myTmp); 
		return strtod(myTmp.SafeP(), &ep);  
	}
}
*/

CStr qCtx::ParseStr(const CStr &str) {
	qStrBuf myTmp;
	qStrReadBuf myIn(str);
	Parse(&myIn, &myTmp); 
	return myTmp ? (CStr) myTmp : CStr::Empty; 
}

void qCtx::SetSafeMode(bool mode)
{
        mySafeMode = mode;
	qEnvHttp *env;
	if (env = (qEnvHttp*) myEnv->GetInterface(ENV_HTTP)) {
#ifdef unix
        const char *p = env->GetScriptPath();
        // todo: make this pre-safe-mode configurable
	SetSafeUID(safe_getfileuid(p));
#endif
	}
}

