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
#endif

#include "qctx.h"
#include "qobj-ctx.h"
#include "qctx-comp.h"
#include "core.h"
#include "util.h"

#include "strary.h"

bool qsUnreg = false;

bool qsInitialized = false;

void qsInit(void *module)
{
	if (!qsInitialized) {

#if defined(_WINDOWS) || defined(WIN32)
		system("del /q /f __db.??? >nul 2>nul");
		_setmaxstdio(2048);
		_set_sbh_threshold(0);
#endif
		qsInitialized = true;

	}
}

// compile a macro (currently just - mostly obfuscates it)
void EvalCompile(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("compile", 1, 2);
	CStr val = (*args)[0];
	if (val.Length() > 0) {
		bool randomize = ParseBool((*args)[1]);
		qStrReadBuf tmp(val);
		qCtxComp comp(ctx, randomize);
		out->PutS(CMAGIC_V1);
		comp.Parse(&tmp, out);
	}
}

void EvalErrorHandler(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("errorhandler", 1, 1);
	ctx->SetErrorHandler(args->GetAt(0));
}

void EvalDebugLogLevel(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        VALID_ARGC("debug-loglevel", 0, 1);
	if (args->Count()>0) {
	        if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}
	        int newlevel = ParseInt(args->GetAt(0));
		smx_log_level = newlevel;
	} else {
		out->PutN(smx_log_level);
	}
}

void EvalDebugLogFile(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        VALID_ARGC("debug-logfile", 0, 1);
        if (args->Count()>0) {
                if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}
                smx_log_file(args->GetAt(0));
        } else {
		out->PutS(smx_log_file(NULL));	
	}
}


void EvalStrict(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("strict", 1, 1);
	ctx->SetStrict(ParseBool(args->GetAt(0)));
}


// expand an unexpanded (quoted) macro
void EvalExpand(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("expand", 0, 1);
	CStr val = (*args)[0];
	if (val.Length() > 0) {
		if (val.Length() > 4 && 
			val.Data()[0] == CMAGIC_V1[0] && val.Data()[1] == CMAGIC_V1[1] && 
			val.Data()[2] == CMAGIC_V1[2] && val.Data()[3] == CMAGIC_V1[3]) {
			qStrReadBuf tmp(val.Data()+4,val.Length()-4);
			RunCompiled(ctx, &tmp, out);
		} else {
			ctx->Parse(val, out);
		}
	}
}

class qObjSafe : public qObj {
	qObj * myObj;
	qCtx * myCtx;

public:
	qObjSafe(qObj * obj, qCtx * ctx) : qObj() {
		myObj = obj;
		myCtx = ctx;
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		myObj->Eval(myCtx, out, args);
	}

	char *GetQmap() {
		return myObj->GetQmap();
	}
};

// expand an unexpanded (quoted) macro, exposing only the specified functions
void EvalSafeExpand(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStr val = (*args)[0];
	if (val.Length() > 0) {
		qCtxTmp tCtx(ctx->GetEnv());
		int i;
		CStr name;
		qObj *obj;
		for (i=1; i < args->Count(); ++i) {
			name = (*args)[i];
			if (!name.IsEmpty() && ctx->Find(&obj, name)) {
				tCtx.MapObj(new qObjSafe(obj, ctx), name);
			}
			
		}
		tCtx.Parse(val, out);
	}
}

// "invoke" an macro/variable by name
void EvalInvoke(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGM("invoke", 1);
	CStr var   = (*args)[0];
	qObj *obj;
	if (ctx->Find(&obj, var)) {
		if (args->Count() >= 2) {
			int i;
			for (i = 0; i < (args->Count()-1); ++i) {
				args->SetAt(i,args->GetAt(i+1));
			}
			args->Grow(args->Count()-1);
			obj->Eval(ctx,out,args);
		} else {
			obj->Eval(ctx,out,0);
		}
	}
}

void EvalSet(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("set", 1, 2);
	CStr name;
	
	if (data) 
		name = (char *) data;
	else 
		name = (*args)[0];

	if (name.IsEmpty()) {
		ctx->Throw(out, 121, "set: empty variable name");
		return;
	}

	//CStr val = (*args)[1];

	if (args->Count() >= 2) {
		CStr val = (*args)[1];
		ctx->MapObj(new qObjStr(val), name);
	} else {
		ctx->DelObj(name);
	}
}

void EvalGSet(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("gset", 1, 2);
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		if (var.IsEmpty()) {
			ctx->Throw(out, 121, "gset: empty variable name");
			return;
		}
		if (args->Count() >= 2) {
			CStr val = (*args)[1];
			if (ctx->GetEnv() && ctx->GetEnv()->GetSessionCtx()) {
				ctx->MapObjTop(ctx->CreateObj(val), var, ctx->GetEnv()->GetSessionCtx());
			} else {
				ctx->MapObjTop(ctx->CreateObj(val), var, NULL);
			}
		}
	}
}

void EvalLet(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("let", 1, 2);
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
                if (var.IsEmpty()) {
                        ctx->Throw(out, 121, "let: empty variable name");
                        return;
                }
		if (args->Count() >= 2) {
			CStr val = (*args)[1];
			ctx->MapObjLet(ctx->CreateObj(val), var);
		}
	}
}

class qObjParsed : public qObj
{
	CStr  myStr;

public:
	qObjParsed(CStr str) {
		myStr = str;
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		ctx->Parse(myStr, out);
	}
};

class qObjByRef : public qObj
{
	qObj *myObj;
	bool  myOwn;

public:
	qObjByRef(qObj *obj, bool own = true) {
		myObj = obj;
		myOwn = own;
	}

	~qObjByRef() {
		if (myOwn) 
			myObj->Free();
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		myObj->Eval(ctx, out, args);
	}

	qCtx *GetCtx() {
		return myObj->GetCtx();
	}
};


class qObjDefArgs : public qObj
{
	qObjDef *myDef;
	qArgAry *myArgs;
public:
	qObjDefArgs(qObjDef *def, qArgAry *args) {
		myDef = def; myArgs = args;
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		int index = ParseInt((*args)[0]);
		if (index >= 0 && index < myArgs->Count()) {
			if (myDef->IsAuto(index)) {
				ctx->Parse(myArgs->GetAt(index), out);
			} else if (myDef->IsRef(index)) {
				qObj *obj;
				if (ctx->Find(&obj, myArgs->GetAt(index))) {
					obj->Eval(ctx, out, args);
				}
			} else {
				out->PutS(myArgs->GetAt(index));
			}
		}
	}
};

void qObjDef::Eval(qCtx *ctx, qStr *out, qArgAry *args)
{
	AddRef();

	try {
		qCtxTmp tmpCtx(ctx);
		int i, an = myArgNames.Count();
		int mapcnt = min(an,args->Count());
		
		for (i = 0; i < mapcnt; ++i) {
			if (myArgNames[i].Length() > 0) {
				CStr s = args->GetAt(i);
				CStr n = myArgNames[i]; //n.Change();
				if (!n.IsEmpty()) {
				if (myQuoted[i] == dParsed ) {
					qObjParsed *p = new qObjParsed(s);
					tmpCtx.MapObj(p, n);
				} else if (myQuoted[i] == dQuoted) {
					tmpCtx.MapObj(s, n);
					args->SetAt(i, s);
				} else if (myQuoted[i] == dObjRef) {
					qObj *obj;
					args->SetAt(i,s);
					if (ctx->Find(&obj, s)) {
						tmpCtx.MapObj(new qObjByRef(obj, false), n);
					}
				} else {
					args->SetAt(i, s);
					tmpCtx.MapObj(s, n);
				}} else {
					args->SetAt(i, s);
				}
			}
		}

		for (; i < an; ++i) {
			if (myArgNames[i].Length() > 0) {
				CStr n = myArgNames[i]; n.Change();
				if (!n.IsEmpty()) 
					tmpCtx.MapObj(CStr::Null, n);
			}
		}

		for (; i < args->Count(); ++i) {
			args->SetAt(i,(*args)[i]);
		}


		qObjDefArgs *inst = new qObjDefArgs(this, args);

		tmpCtx.MapObj(inst, "arg");
		tmpCtx.MapObj(args->Count(), "num-args");
		tmpCtx.MapObj(args->Count(), "argc");

		// *** comp
		//tmpCtx.Parse(&qStrReadBuf(myBody), out);
		qStrReadBuf rTmp(myBody);
		RunCompiled(&tmpCtx, &rTmp, out);
	} catch (qCtxExAbort ex) {
		throw ex;
	} catch (qCtxEx ex) {
		throw ex;
	} catch (...) {
		ctx->Throw(out, 999, "Core Unhandled exception");
	}

	Free();
}

void qObjDef::AddArgName(const CStr arg, int quoted)
{
	if (!arg.IsEmpty()) 
		myArgNames.Add(arg); 
	else 
		myArgNames.Add(0);
	
	myQuoted.Add(quoted);
	
	if (quoted==dQuoted && myQmap.IsEmpty()) {
		int i;
		for (i = 0; i < myQuoted.Count(); ++i) {
			myQmap << ((myQuoted[i] == (int) dQuoted) ? '1' : '0');
		}
	} else {
		myQmap << ((quoted == (int) dQuoted) ? '1' : '0');
	}
}

qObjDef *CreateDef(qCtx *ctx, qStr *out, qArgAry *args, CStr &name) 
{
 	if (args->Count() < 1)
		return NULL;

	if (name.IsEmpty())
		name = (*args)[0];

	if (name.Length()) {
		int i; 
		if (args->Count() >= 2) {

			CStr body = (*args)[1];

			// *** comp
			qStrBuf out;
      qStrReadBuf rTmp(body);
			qCtxComp comp(ctx); comp.Parse(&rTmp, &out);
			body = out;

			qArg arg;
			qObjDef *obj = new qObjDef(body);
			for (i = 2; i < args->Count(); ++i) {
				arg = args->GetAt(i);
				if (!arg.IsEmpty()) {
					if (args->GetQuot(i)) {
						obj->AddArgName(arg,   qObjDef::dQuoted);
					} else if (arg[0] == '*') {
						obj->AddArgName(arg+1, qObjDef::dParsed);
					} else if (arg[0] == '&') {
						obj->AddArgName(arg+1, qObjDef::dObjRef);
					} else {
						obj->AddArgName(arg, qObjDef::dNormal);
					}
				}
			}
			return obj;
		}
	}

	return NULL;
}

void EvalDefined(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("defined", 1, 1);
	qObj *obj;
	if (ctx->Find(&obj, (*args)[0])) {
		out->PutC('T');
	}
}

void EvalDefine(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGM("define", 1);

	CStr name;
	qObj *obj = CreateDef(ctx, out, args, name);

	if (!name.Length()) {
		if (obj) 
			obj->Free();
		return;
	}

	if (obj) 
		ctx->MapObj(obj, name);
	else
		ctx->DelObj(name);
}

void EvalGDefine(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGM("gdefine", 1);

	CStr name;
	qObj *obj = CreateDef(ctx, out, args, name);

	if (!name.Length()) {
		if (obj) 
			obj->Free();
		return;
	}

	if (obj) 
		if (ctx->GetEnv() && ctx->GetEnv()->GetSessionCtx()) {
			ctx->MapObjTop(obj, name, ctx->GetEnv()->GetSessionCtx());
		} else {
			ctx->MapObjTop(obj, name, NULL);
		}
	else
		if (ctx->GetEnv() && ctx->GetEnv()->GetSessionCtx()) {
			ctx->DelObjTop(name, ctx->GetEnv()->GetSessionCtx());
		} else {
			ctx->DelObjTop(name, NULL);
		}
}


void EvalNilOut(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		int i; for (i = 0; i < args->Count(); ++i) {
			((qStr *) data)->PutS(args->GetAt(i));
		}
	}
}

void EvalNil(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		qStrNull myOut;
		qCtxTmp myCtx(ctx);
		myCtx.MapObj(out,EvalNilOut,"out");
		int i; for (i = 0; i < args->Count(); ++i) {
			myCtx.Parse(args->GetAt(i), &myOut);
		}
	}
}

void EvalNull(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		qStrNull myOut;
		int i; for (i = 0; i < args->Count(); ++i) {
			if (!args->GetQuot(i)) {
				ctx->Parse(args->GetAt(i), &myOut);
			}
		}
	}
}



void EvalFor(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("for", 4, 5);
	if (args->Count() >= 4) {
		CStr var   = (*args)[0];
		if (var.IsEmpty()) {
			ctx->Throw(out, 121, "for: empty variable name");
			return;
		}
		int  from = ParseInt((*args)[1]);
		int  to = ParseInt((*args)[2]);
		int  step = ParseInt((*args)[4]);
		qCtxTmp myCtx(ctx);
		int i;
		myCtx.MapObj(&i, var);
		bool ok = true;
		myCtx.MapObj(&ok, (QOBJFUNC) EvalBreak, "break");
#ifdef _DEBUG
//	CMemoryState State;
//	State.Checkpoint();
#endif
		if (!step) 
			step = 1;

		if (step > 0) {
			for (i = from; ok && i <= to; i+=step) {
				myCtx.Parse(args->GetAt(3), out);
			}
		} else {
			for (i = from; ok && i >= to; i+=step) {
				myCtx.Parse(args->GetAt(3), out);
			}
		}

#ifdef _DEBUG
//	State.DumpAllObjectsSince();
#endif
	}
}

void EvalWhile(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("while", 1, 2);
	if (args->Count() > 0) {
		
		qObj *obj;
		int i = 0;
		bool ok = true;



		if (!ctx->Find(&obj, "break"))
			ctx->MapObj(&ok, (QOBJFUNC) EvalBreak, "break");
		
		int loopMax = ctx->EvalInt("max-iteration");
			
		while (ok && ParseBool(ctx->ParseStr(args->GetAt(0))) && (loopMax <= 0 || i++ < loopMax)) {
			if (args->Count() > 1) {
				ctx->Parse(args->GetAt(1), out);
			}
		}
	}
}

void EvalDo(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("do", 1, 2);
	if (args->Count() > 0) {
		int loopMax = ctx->EvalInt("max-iteration");
		qObj *obj;
		int i = 0;
		bool ok = true;
		if (!ctx->Find(&obj, "break"))
			ctx->MapObj(&ok, (QOBJFUNC) EvalBreak, "break");
		do {
			if (args->Count() > 0) {
				ctx->Parse(args->GetAt(0), out);
			}
		} while (ok && ParseBool(ctx->ParseStr(args->GetAt(1))) && (loopMax <= 0 || ++i < loopMax));
	}
}


void EvalIf(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGM("if", 2);
	int i = 1; CStr cond; 
	if (args->Count() > 1) {
		while (i < args->Count()) {
			if (ParseBool(ctx->ParseStr(args->GetAt(i-1)))) {
				ctx->Parse(args->GetAt(i), out);
				return;
			}
			i += 2;
		}
		if (i-1 < args->Count()) {
			ctx->Parse(args->GetAt(i-1), out);
		}
	}
}

void EvalAnd(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGM("and", 2);
	int i; 
	for (i = 0; i < args->Count(); ++i) {
		if (!ParseBool(ctx->ParseStr(args->GetAt(i)))) {
			return;
		}
	}
	out->PutC('T');
}

void EvalOr(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{

	VALID_ARGM("or", 2);
	int i; 
	for (i = 0; i < args->Count(); ++i) {
		if (ParseBool(ctx->ParseStr(args->GetAt(i)))) {
			out->PutC('T');
			return;
		}
	}
}

void EvalNot(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("not", 1, 1);
	if (ParseBool((*args)[0])) {
		return;
	}
	out->PutC('T');
}

void EvalDebugStop(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}

	// apache environment only...  should be GetDebugEnv(ctx) ... etc

	char *p;
	bool ok = true;
	
	char cmd[256];
	qArgAry cmdArgs;

	qCtxTmp debCtx(ctx);
	qStrFileO debout(stdout);

	debCtx.MapObj(&ok, (QOBJFUNC) EvalBreak, "go");

	printf("Press enter to continue\n");

	while (ok && fgets(cmd, 255, stdin)) {
		p = cmd;
		while (*p && !isspace(*p) && *p != '(')
			++p;
		*p++ = '\0';
		if (cmd && *cmd) {
			qObj *obj; debCtx.Find(&obj, cmd);
			cmdArgs.Grow(0);
      qStrReadBuf rTmp(p);
			debCtx.ParseArgsQmap(&rTmp, &cmdArgs, obj->GetQmap());
			debCtx.Eval(&debout, cmd, &cmdArgs);
			debout.PutC('\n');
		} else 
			break;
	}
}

void EvalDebugTrace(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}

	// apache environment only...  should be GetDebugEnv(ctx) ... etc
	qStrFileO debout(stdout);
	debout.PutS((*args)[0]);
}

#if defined(WIN32) && defined(MEMDEBUG)
#ifdef _DEBUG
class qObjMemState : public qObjTS
{
public:
	CMemoryState State;
};

void EvalDebugMemSnap(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}
	qObjMemState *obj = new qObjMemState();
	ctx->MapObj(obj, (*args)[0]);
	obj->State.Checkpoint();
}

void EvalDebugMemDiff(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}
	qObjMemState *obj;
	if (ctx->Find((qObj **)&obj, (*args)[0])) {
		obj->State.DumpAllObjectsSince();
	}
}
#endif
#endif

void EvalTry(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count()>0) {
		try {
			ctx->AddTry();
			ctx->Parse(args->GetAt(0),out);
			ctx->RemTry();
		} catch (qCtxExAbort ex) {
			ctx->RemTry();
			throw ex;
		} catch (qCtxEx ex) {
			ctx->RemTry();
			if (args->Count()>1) {
				qCtxTmp sub(ctx);


				sub.MapObj(ex.GetLineNum(),  "exception-line");
				sub.MapObj(ex.GetID(),       "exception-num");
				sub.MapObj(ex.GetMsg(),      "exception-msg");

				sub.Parse(args->GetAt(1),out);
			}
		} catch (...) {
			ctx->RemTry();
			if (args->Count()>1) {
				qCtxTmp sub(ctx);

				sub.MapObj(0,                   "exception-line");
				sub.MapObj(0,                   "exception-num");
				sub.MapObj("Unknown Exception", "exception-msg");

				sub.Parse(args->GetAt(1),out);
			}
		}
	}
}

void EvalThrow(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	VALID_ARGC("throw", 2, 2);
	if (args->Count()>1) {
		ctx->Throw(out, ParseInt((*args)[0]), (*args)[1]);
	}
}

void EvalAbort(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	ctx->Abort();
}

void EvalDefctx(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	VALID_ARGC("defctx", 2, 3);
	if (args->Count() > 1) {
		CStr name = (*args)[0];
                if (name.IsEmpty()) {
                        ctx->Throw(out, 121, "defctx: empty variable name");
                        return;
                }
		CStr bname = (*args)[1];
		CStr body;
		if (args->Count() > 2)
			body = args->GetAt(2);
	
		qObj *base = 0;
		if (!bname.IsEmpty())
			ctx->Find(&base, bname);

		qObjClass *obj = new qObjClass((qObjClass *)base);

		// now load in methods and variables
		obj->GetCtx()->SetParent(ctx);
		qStrNull tmpOut;
		obj->GetCtx()->Parse(body, &tmpOut);
		ctx->MapObj(obj, name);
		obj->GetCtx()->SetParent(NULL);
	}
}

void EvalSafeMode(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count()>0) {
		bool mode = ParseBool((*args)[0]);
		if (mode)
			ctx->SetSafeMode(true);
	} else {
		if (ctx->GetSafeMode())
			out->PutC('T');
	}
}
void EvalSafeUID(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count()>0) {
		if (!ctx->GetSafeMode()) {
			int uid = ParseInt((*args)[0]);
			ctx->SetSafeUID(uid);
		}
	}	
	int uid = ctx->GetSafeUID();
	out->PutN(uid);
}

void EvalSmxQuote(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	int i;
	for(i=0;i<args->Count();++i) {
		if (i!=0) out->PutC(',');
		out->PutS(SmxQuote((*args)[i]));
	}
}

CStr SmxQuote(CStr in) {
	CStr r;
	const char *p, *b = in;
	const char *e = in.Length()  + (p = b);
	bool quot = false;

	while (p < e) {
		if (*p == '\'' || *p == '%' || *p == '\"' || *p == ',' || *p == '\\' || *p ==')' || *p == '(' || *p == 0 || *p == EOF || isspace(*p)) {
			quot = true;
			r = "\"" << CStr(b,p-b);
			while (p < e) {
				if (*p == '"' || *p =='\\' || *p =='(' || *p ==')') {
					r << '\\';	
					r << *p++;
				} else if (*p == '%') {
					r << '%';
					r << '%';
					++p;
				} else { 
					r << *p++;
				}
			}
			r << '"';
			break;
		} else
			++p;
	}

	if (quot)
		return r;
	else
		return in;
}

#ifdef WIN32

void EvalPriorityExpand(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}
	int priority = THREAD_PRIORITY_ERROR_RETURN;

	CStr pname = (*args)[0];

	if (!stricmp(pname,"low")) {
		priority = THREAD_PRIORITY_BELOW_NORMAL;
	}
	if (!stricmp(pname,"high")) {
		priority = THREAD_PRIORITY_ABOVE_NORMAL;

	}
	if (!stricmp(pname,"normal")) {
		priority = THREAD_PRIORITY_NORMAL;
	}
	if (!stricmp(pname,"lowest")) {
		priority = THREAD_PRIORITY_LOWEST;

	}
	if (!stricmp(pname,"highest")) {
		priority = THREAD_PRIORITY_HIGHEST;
	}

	if (priority == THREAD_PRIORITY_ERROR_RETURN)
		ctx->Throw(out, 353, "Priority must be one of low, high, normal, lowest, highest");
	else {

		DWORD was = GetThreadPriority(GetCurrentThread());

		SetThreadPriority(GetCurrentThread(), priority);

		try {
			ctx->Parse(args->GetAt(1),out);
		} catch (qCtxExAbort ex) {
			SetThreadPriority(GetCurrentThread(), was);
			throw ex;
		} catch (qCtxEx ex) {
			SetThreadPriority(GetCurrentThread(), was);
			throw ex;
		} catch (...) {
			qCtxEx ex(354, "Unknown exception during priority shift");
			SetThreadPriority(GetCurrentThread(), was);
			throw ex;
		}

		SetThreadPriority(GetCurrentThread(), was);
	}
}
#endif


void LoadCore(qCtx *ctx) {
	qsInit(NULL);

	ctx->MapObj(EvalNil,    CStr("nil"), QMAP_ALL);
	ctx->MapObj(EvalNull,   "null", QMAP_ALL);

	ctx->MapObj(EvalFor,    "for", "0001");
	ctx->MapObj(EvalWhile,  "while", "11");
	ctx->MapObj(EvalDo,     "do", "11");
	ctx->MapObj(5000,       "max-iteration");

	ctx->MapObj(EvalIf,     "if", QMAP_ALL);
	ctx->MapObj(EvalIf,     "switch", QMAP_ALL);

	ctx->MapObj(EvalOr,     "or", QMAP_ALL);
	ctx->MapObj(EvalAnd,    "and", QMAP_ALL);
	ctx->MapObj(EvalNot,    "not");

	ctx->MapObj(EvalSet,    "set" );
	ctx->MapObj(EvalLet,    "let" );
	ctx->MapObj(EvalGSet,   "gset");

	ctx->MapObj(EvalDefine, "macro");
	ctx->MapObj(EvalDefine, "define", "01");
	ctx->MapObj(EvalGDefine,"gmacro");
	ctx->MapObj(EvalGDefine,"gdefine", "01");

	ctx->MapObj(EvalDefctx, "defctx", "001");

	ctx->MapObj(EvalDefined, "defined");

	ctx->MapObj(EvalInvoke,"get");
	ctx->MapObj(EvalInvoke,"invoke");
	ctx->MapObj(EvalExpand,"expand");
	ctx->MapObj(EvalSafeExpand,"safe-expand");

	ctx->MapObj(EvalCompile,"compile");

	ctx->MapObj(EvalErrorHandler,"errorhandler", "1");
	ctx->MapObj(EvalDebugLogFile,"debug-logfile");
	ctx->MapObj(EvalDebugLogLevel,"debug-loglevel");
	ctx->MapObj(EvalStrict,"strict");

	ctx->MapObj(EvalDebugStop,"_stop");
	ctx->MapObj(EvalDebugTrace,"_trace");

#if defined(WIN32) && defined(MEMDEBUG)
#ifdef _DEBUG
	ctx->MapObj(EvalDebugMemSnap,"_memsnap");
	ctx->MapObj(EvalDebugMemDiff,"_memdiff");
#endif
#endif
	
	ctx->MapObj(EvalTry,    "try", "11");
	ctx->MapObj(EvalThrow,  "throw");
	ctx->MapObj(EvalAbort,  "abort");


	ctx->MapObj(EvalSmxQuote,  "macro-quote");

	ctx->MapObj(EvalSafeMode,  "safe-mode");
	ctx->MapObj(EvalSafeUID,  "safe-uid");

#ifdef WIN32
	ctx->MapObj(EvalPriorityExpand,  "priority-expand", "01");
#endif

	ctx->MapObj(PACKAGE_VERSION,  "script-version");
}

