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

char CMAGIC_V1[5] = "\xF2\x1A\x08\xE2";

#define BC_FUNC  1		// function
#define BC_OUT   2		// string

#define ARG_STR  0		// string needs to be parsed
#define ARG_QSTR 1		// string-quoted
#define ARG_CMP  2		// pre-compliled arg

typedef struct {
	char  bc;
	char  rn;
	int   ln;
} C_BASE;

typedef struct {
	short cnt;
} C_ARGS;

typedef struct {
	unsigned int at :  2;
	unsigned int ln : 30;
} C_ARG;


bool ParseArgQuoted(qStr *in, qArg &cur, char c);

void Decompile(qStr *in, qStr *out)
{
	int i, j;
	char *p; 

	C_BASE b;
	while (in->GetS((char *) &b, sizeof(b)) == sizeof(b)) {
		CStr dt(b.ln);
		if (in->GetS(dt.GetBuffer(), b.ln) == b.ln) {
			if (b.bc == BC_FUNC) {
				C_ARGS v;
				C_ARG a;
				CStr cur;

				p = dt.GetBuffer();

				if (b.rn) {
					for (j = 0; j < b.ln; ++j)
						p[j] = p[j] ^ b.rn;
				}

				out->PutC('%');
				out->PutS(dt);

				if (in->GetS((char *) &v, sizeof(v)) == sizeof(v)) {
					if (v.cnt > 0)
						out->PutC('(');
					else
						out->PutC('%');
					for (i = 0; i < v.cnt; ++i) {
						if (in->GetS((char *) &a, sizeof(a)) == sizeof(a)) {
							cur.Grow(a.ln);
							if (in->GetS(cur.GetBuffer(), a.ln) == (int) a.ln) {
								p = cur.GetBuffer();
								if (b.rn) {
									for (j = 0; j < (int) a.ln; ++j)
										p[j] = p[j] ^ b.rn;
								}
								if (a.at == ARG_CMP) {
									qStrReadBuf rTmp(cur);
									Decompile(&rTmp, out);
								} else {
									if (a.at == ARG_QSTR)
										out->PutC('\'');
									out->PutS(cur);
								}
								if (i < (v.cnt-1)) {
									out->PutC(',');
								}
							}
						}
					}
					if (v.cnt > 0)
						out->PutC(')');
				}
			} else if (b.bc == BC_OUT) {
				out->PutS(dt, b.ln);
			}
		}
	}
}

void RunCompiled(qCtx *ctx, qStr *in, qStr *out)
{
	int i, j;
	char *p; 

	C_BASE b;
	while (in->GetS((char *) &b, sizeof(b)) == sizeof(b) && b.ln > 0) {
		CStr dt(b.ln);
		if (in->GetS(dt.GetBuffer(), b.ln) == b.ln) {
			if (b.bc == BC_FUNC) {

				p = dt.GetBuffer();
				if (b.rn) {
					for (j = 0; j < b.ln; ++j)
						p[j] = p[j] ^ b.rn;
				}
				qObj *obj;
				if (ctx->Find(&obj, (const CStr &) dt)) {
					C_ARGS v;
					C_ARG a;
					CStr cur;
					qArgAry ary;
					char qmode;
					if (in->GetS((char *) &v, sizeof(v)) == sizeof(v)) {
						char *map = obj->GetQmap();
						if (!map || *map == 'A') {
							qmode = !map ? '0' : '1';
							for (i = 0; i < v.cnt; ++i) {
{
	if (in->GetS((char *) &a, sizeof(a)) == sizeof(a)) {
		cur.Grow(a.ln);
		if (in->GetS(cur.GetBuffer(), a.ln) == (int) a.ln) {
			p = cur.GetBuffer();
			if (b.rn) {
				for (j = 0; j < (int) a.ln; ++j)
					p[j] = p[j] ^ b.rn;
			}
			ary.Add(cur);
			ary.SetQuot(i, a.at == ARG_QSTR);
			if (!ary.GetQuot(i) && qmode == '0') {
        qStrBuf tmp;
				if (a.at == ARG_CMP) {
          qStrReadBuf rTmp(ary[i]);
					RunCompiled(ctx, &rTmp, &tmp);
				} else {
          qStrReadBuf rTmp(ary[i]);
					ctx->Parse(&rTmp, &tmp);
				}
				ary[i] = tmp;
			} else {
				if (a.at == ARG_CMP) {
					if (qmode != '2') {
            qStrBuf tmp;
            qStrReadBuf rTmp(ary[i]);
						Decompile(&rTmp, &tmp);
						ary[i] = tmp;
					} else {
						ary.SetQuot(i, ARG_CMP);
					}
				}
			}
		}
	}
}
							}
						} else {
							qmode = (*map == '1' ? '1' : '0');
							++map;
							for (i = 0; i < v.cnt; ++i) {
{
	if (in->GetS((char *) &a, sizeof(a)) == sizeof(a)) {
		cur.Grow(a.ln);
		if (in->GetS(cur.GetBuffer(), a.ln) == (int) a.ln) {
			p = cur.GetBuffer();
			if (b.rn) {
				for (j = 0; j < (int) a.ln; ++j)
					p[j] = p[j] ^ b.rn;
			}
			ary.Add(cur);
			ary.SetQuot(i, a.at == ARG_QSTR);
			if (!ary.GetQuot(i) && qmode == '0') {
        qStrBuf tmp;
				if (a.at == ARG_CMP) {
          qStrReadBuf rTmp(ary[i]);
					RunCompiled(ctx, &rTmp, &tmp);
				} else {
          qStrReadBuf rTmp(ary[i]);
					ctx->Parse(&rTmp, &tmp);
				}
				ary[i] = tmp;
			} else {
				if (a.at == ARG_CMP) {
					if (qmode != '2') {
            qStrBuf tmp;
            qStrReadBuf rTmp(ary[i]);
						Decompile(&rTmp, &tmp);
						ary[i] = tmp;
					} else {
						ary.SetQuot(i, ARG_CMP);
					}
				}
			}
		}
	}
}
								if (*map) {
									if (*map != 'A') {
										qmode = *map;
										++map;
									}
								} else {
									qmode = '0';
								}
							}
						}
						obj->Eval(ctx, out, v.cnt ? &ary : NULL);
					}
				} else {
					if (ctx->GetStrict()) {
						ctx->ThrowF(out, 98, "Function '%s' was not found.", (const char *) dt);
					} else {
						C_ARGS v;
						C_ARG a;
						CStr cur;
						if (in->GetS((char *) &v, sizeof(v)) == sizeof(v)) {
							for (i = 0; i < v.cnt; ++i) {
								if (in->GetS((char *) &a, sizeof(a)) == sizeof(a)) {
									cur.Grow(a.ln);
									if (in->GetS(cur.GetBuffer(), a.ln) != (int) a.ln) {
										break;
									}
								}
							}
						}
						out->PutC('%');
						out->PutS(dt);
						if (v.cnt > 0) {
							out->PutC(T_LP);
							out->PutS("...");
							out->PutC(T_RP);
						} else {
							out->PutC('%');
						}
					}
				}
			} else if (b.bc == BC_OUT) {
				out->PutS(dt, b.ln);
			}
		}
	}
}

void qCtxComp::OutFunc(qStr *out, CStr &name, qArgAry *ary)
{
 	int i, j;
	char *p;

	C_BASE b;
	b.bc = BC_FUNC;
	b.rn = myRn ? rand()%256 : 0;
	b.ln = name.Length();
	out->PutS((char *)&b, sizeof(b));

	p = name.GetBuffer();
	if (b.rn) {
		for (j = 0; j < b.ln; ++j)
			p[j] = p[j] ^ b.rn;
	}
	out->PutS(name);

	C_ARGS v;
	v.cnt = ary->Count();
	out->PutS((char *)&v, sizeof(v));

	C_ARG a;
	for (i = 0; i < ary->Count(); ++i){
		a.ln = ary->GetAt(i).Length();
		a.at = ary->GetQuot(i);
		out->PutS((char *)&a, sizeof(a));

		p = ary->GetAt(i).GetBuffer();
		if (b.rn) {
			for (j = 0; j < (int) a.ln; ++j)
				p[j] = p[j] ^ b.rn;
		}
		out->PutS(ary->GetAt(i));
	}
}

void qCtxComp::OutFlush(qStr *out)
{
	if (myOutBuf.Length()) {
		C_BASE b;
		b.bc = BC_OUT;
		b.ln = myOutBuf.Length();
		b.rn = 0;
		out->PutS((char *) &b, sizeof(b));
		out->PutS(myOutBuf);
		myOutBuf.Grow(0);
	}
}

inline void qCtxComp::ParseC(qStr *in, qStr *out, char c)
{
	if (c == T_PCT) {
		ParseFunc(in, out);
	} else {
		OutC(c);
	}
}

void qCtxComp::Parse(qStr *in, qStr *out)
{
	char c;
	while ( (c = in->GetC()) != EOF ) {
		ParseC(in, out, c);
	}
	OutFlush(out);
}

#define isqsymf(c) ( ((c) > ')' && (c) < ';') || ((c) > '?' && (c) < '{') )
#define isqsym(c)  ((c) > ')' && (c) < 127)


bool qCtxComp::ParseFunc(qStr *in, qStr *out)

{
	char c;
//	int pop = 0;

	c = in->GetC();
	
	if (isqsymf(c)) {
		CStr name(c,1);
		while ( (c = in->GetC()) != EOF ) {
			if (c == T_LPAR) {
				OutFlush(out);
				qArgAry ary;
				qObj *obj;
				if (Find(&obj, name)) {
					ParseCompArgs(in, &ary, obj->GetQmap());
				} else {
					ParseCompArgs(in, &ary, NULL);
				}
				OutFunc(out, name, &ary);
				return true;
			} else if (c == T_PCT) {
				if (name.Length()==0) {
					break;
				} else {
					OutFlush(out);
					OutFunc(out, name, NULL);
					return true;
				}
			} else if (isqsym(c)) {
				name << c;
			} else 
				break;
		}
		if (name.Length()!=0) {
			OutS(T_PCT);
		} else if (c == T_LPAR)
			OutS(T_PCT);


		if (c != EOF)
			OutS(c);
	} else {
		OutC(T_PCT);
		if (c != EOF && c != T_PCT) {
			if ((c == T_RP || c == ',' || c == T_LP || c == '"')) {
				if (!in->UngetC(c) || myStrict) {
					ThrowF(out, 98, "Invalid syntax '%%%c'.", c);
				}
			} else {
				OutC(c);
			}
		}
	}
	return false;
}


// NOTE: this is identical to the regular one except for ParseCompArg

void qCtxComp::ParseCompArgs(qStr *in, qArgAry *out, char *qmap)
{
	int quot = false;
	bool more = true;
//	bool trim = true;

	qStrBuf cur;
//	char quoted = '\x0';
//	int qcnt = 0;
	char c;
//	int acnt = 0;

	char qmode;
	
	if (qmap) {
		qmode = *qmap == 'A' ? '1' : *qmap == '1' ? '1' : '0';
		if (*qmap != 'A' )
			++qmap;
		else
			qmap = NULL;
	} else {
		qmode = '0';
	}

	do {
		while ( isspace((c = in->GetC())) );

		if (c == T_SQ) {
			quot = ARG_QSTR; 
			c = in->GetC();
		} else {
			quot = ARG_STR;
		}

		if (qmode == '1' || quot) {
			more = ParseArgQuoted(in, cur, c);
		} else  {
//			more = ParseArgQuoted(in, cur, c);
			more = ParseCompArg(in, cur, c);
			quot = ARG_CMP;
		}

		out->Add(cur);
		cur.Grow(0);
		out->SetQuot(out->Count()-1,quot);

		if (qmap) {
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
		}
	} while (more && c != EOF);
}

bool qCtxComp::ParseCompArg(qStr *in, qStrBuf &cur, char c)
{
	bool more = true;
	int numsp = 0;
	char quoted = '\x0';
	int qcnt = 0;
	int lpc = 0;

	CStr tmp;

	if (c == '"')
		{ quoted = '"'; c = in->GetC(); }
	else if (c == T_LC)
		{ quoted = T_LC; c = in->GetC(); qcnt = 1; }

	do {
		if (c == T_ESC) {
			c = in->GetC();
			if (c != EOF)
				tmp << c;
			numsp = 0;
		} else if (c == '%') {
			if (tmp.Length()) {
				C_BASE b;
				b.bc = BC_OUT;
				b.ln = tmp.Length();
				b.rn = myRn ? rand()%256 : 0; // 2003-05-21 more encryption
				cur.PutS((char *) &b, sizeof(b));
				cur.PutS(tmp);
				tmp.Grow(0);
			}
			ParseFunc(in, &cur);
			OutFlush(&cur);
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
				tmp << c;
			}
		} else if (lpc) {
			if (c == T_RP) {
				--lpc;
				tmp << c;
				numsp = 0;
			} else if (c == T_LP) {
				++lpc;
				tmp << c;
				numsp = 0;
			} else if (isspace(c)) {
				tmp << c;
				++numsp;
			} else {
				numsp = 0;
				tmp << c;
			}
		} else {
			if (c == ',') {
				if (numsp) 
					tmp.Grow(tmp.Length() - numsp);
				break;
			} else if (c == T_LP) {
				tmp << c;
				++lpc;
				numsp = 0;
			} else if (c == T_RP) {
				if (numsp) 
					tmp.Grow(tmp.Length() - numsp);
				more = false;
				break;
			} else if (isspace(c)) {
				++numsp;
				tmp << c;
			} else {

				numsp = 0;
				tmp << c;
			}
		}
	} while (EOF != (c = in->GetC()));

	if (tmp.Length()) {
		C_BASE b;
		b.bc = BC_OUT;
		b.ln = tmp.Length();
		b.rn = myRn ? rand()%256 : 0;  // 2003-05-21 more encryption
		cur.PutS((char *) &b, sizeof(b));
		cur.PutS(tmp);
		tmp.Grow(0);
	}
	return c == EOF ? false : more;
}
