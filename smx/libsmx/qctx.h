/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QCTX_
#define _QCTX_

class qObj;
class qStr;

#include "qstr.h"
#include "qobj.h"
#include "qenv.h"

#include "mapstr.h"

typedef CMapStr<qObj *> qObjMap;
typedef CLst<qObj *> qObjLst;

#ifdef WIN32 
	#ifndef TLS
		#define TLS _declspec(thread) 
	#endif
#endif

class qCtxExAbort { // just something to throw
	void *nada;
public:
	qCtxExAbort() {
		nada = NULL;
	}
	qCtxExAbort(const qCtxExAbort &de) {
		nada = de.nada;
	}
	qCtxExAbort & operator =(const qCtxExAbort &de) {
		nada = de.nada; return *this;
	}
};

#define T_PCT  '%'
#define T_LPAR '('
#define T_RPAR ')'
#define T_ESC  '\\'
#define T_SQ   '\''
#define T_BQ1  '"'
#define T_EQ1  '"'
#define T_BQ2  '{'
#define T_EQ2  '}'
#define T_MQ   '"'
#define T_DEREF "->"

#define S_MAGIC1  ((const char *) "%expand%")
#define S_MAGIC2  ((const char *) "<!--%expand%-->")
#define S_MAGIC1_N  8
#define S_MAGIC2_N  15

class qCtx;
class qCtxTS;
class qCtxEx;
class qCtxExData;

class qCtxExData {
friend class qCtxEx;


	int   myRefs;
	int   myID;
	char *myMsg;
	int   myLineNum;

public:
	~qCtxExData() {
		if (myMsg)
			free(myMsg);
	}

	qCtxExData(int id, const char *msg) {
		myRefs = 1; 
		myID = id;
		myMsg = msg ? strcpy((char*)malloc(strlen(msg)+1),msg) : 0;
		myLineNum = 0;
	}

	void FormatV(const char *msg, void *vargs);

	void AddRef() {++myRefs;}
	void Free() { if (!--myRefs) delete this;}
};

class qCtxEx {
	qCtxExData *myData;

public:
	qCtxEx(int id, const char * msg = NULL) {
		myData = new qCtxExData(id, msg);
	}

	qCtxEx(const qCtxEx &copy) {
		operator =(copy);
	}

	qCtxEx& operator = (const qCtxEx &copy) {
		myData = copy.myData;
		myData->AddRef();
		return *this;
	}

	~qCtxEx() {
		myData->Free();
	}

	int GetID() {
		return myData->myID;
	}

	int GetLineNum() {
		return myData->myLineNum;
	}

	void SetLineNum(int lineNum) {
		myData->myLineNum = lineNum;
	}

	const char *GetMsg() {
		return myData->myMsg;
	}

	void FormatV(const char *msg, void *vargs) {
		myData->FormatV(msg, vargs);
	}
};

class qCtx {
friend class qObjClass;

protected:
	
	static char sBQ2;
	static char sEQ2;

	qObjMap  myMap;
	qCtx *   myParent;
	qEnv *   myEnv;
	short    myTry;
	short    myLevel;
	bool     myStrict;
	int	 mySafeUID;
	bool	 mySafeMode;
	CStr     myErrorHandler;

private:
	inline void ParseC(qStr *in, qStr *out, char c);
	inline bool ParseFunc(qStr *in, qStr *out);

//	void ParseArg(qStr *in, qStr *out, int quot);
//	void ParseArg(qStr *in, qStr *out, bool quot);
	inline bool ParseArg(qStr *in, qStrBuf &cur, char c);
	void ParseTrim(qStr *in, qStr *out);

protected:
#ifdef _DEBUG
	inline bool SafeParent(qCtx *parent) { 
		return ( !parent   || 
				 !myParent || 
				 (this != parent && SafeParent(parent->myParent))
			   );
	}
#else
	#define SafeParent(p) true
#endif

	void Construct(qCtx *parent, qEnv *env) { 
		myTry    = 0;
		myStrict = 0;
		myEnv    = 0; 
		myLevel  = 0;
		mySafeMode = false;
		mySafeUID = -1;
#ifdef _DEBUG
		myParent = 0;
#endif
		SetParent(parent);
		if (env) 
			SetEnv(env);
	}


private:
// recursive find, returns where found, *top is blocked from search
	bool FindC(qObj **obj, const char *name, qCtx **where, qCtx *top) {
		const char *p = name;
		if (!p) 
			p = "";
		if (*p == '<' && GetSafeMode())
			return false;					// cache and internal object names
		if (*p == ':' && !GetSafeMode()) {
			int pop = 2;
			while (*++p == ':') {
				++pop;
			}
			qObj *tobj = NULL;
			qCtx *ctx = this, *first = NULL;
			*where = 0;
			while (ctx && pop && this != top) {
				if (ctx->FindL(&tobj, p)) {
					*obj = tobj;
					*where = ctx;

					if (!first)

						first = ctx;
					--pop;
				}
				ctx = ctx->GetParent();
			}
			return (first != ctx) && *where ? true : 0;
		} else {
			return false;
		}
	}

protected:
   virtual ~qCtx();

    qCtx() {
	   Construct(NULL, NULL);
   }

public:

	qEnv *   GetEnv()                {return myEnv;}
	void     SetEnv(qEnv *env)       {assert(myEnv == NULL || env != NULL); myEnv = env;}

        virtual void SetSafeMode(bool mode);
        bool GetSafeMode()                              {return mySafeMode;}

        virtual void SetSafeUID(int uid)		{mySafeUID=uid;}
        int GetSafeUID()                                {return mySafeUID;}

	qCtx*    GetParent()             {return myParent;}
	void     SetParent(qCtx* parent) {

		assert(SafeParent(parent)); 
		myParent = parent; 
		if (myParent) {
			if (!myEnv)
				myEnv = myParent->GetEnv();
			myTry = myParent->GetTry();
			myStrict = myParent->myStrict;
			mySafeMode = myParent->mySafeMode;
			mySafeUID = myParent->mySafeUID;
			myErrorHandler = myParent->myErrorHandler;
			myLevel = myParent->GetLevel() + 1;
		}
	}

	 int GetTry() {return myTry;}
	void AddTry() {++myTry; assert(myTry >= 1);}
	void RemTry() {--myTry; assert(myTry >= 0);}

	int GetLevel()   { return myLevel; }
	void SetStrict(bool strict)  { myStrict = strict;}
	bool GetStrict()  { return myStrict;}
	
	void SetErrorHandler(CStr str);

	void SetQ2(char bq, char eq) {sBQ2 = bq; sEQ2 = eq;}

	virtual void Clear(bool force = false);
	qObjMap *GetMap() {return &myMap;}


	void Abort() {
		qCtxExAbort t;
		throw t;
	}

	void Throw(qStr *out, qCtxEx &ex) {
		smx_log(SMXLOGLEVEL_SCRIPT, "%s\t%s", GetEnv()->GetName(), ex.GetMsg());
		if (myTry) {
			throw (ex);
		} else {
			out->PutS('[');
			out->PutS(ex.GetMsg());
			out->PutS(']');
		}
	}

	void Throw(qStr *out, int id, const char *msg) {
		qCtxEx ex(id, msg);
		Throw(out, ex);
	}

	void ThrowF(qStr *out, int id, const char *msg, ...) {
		va_list vargs;
		va_start( vargs, msg );
		qCtxEx ex(id); ex.FormatV(msg, vargs);
		va_end( vargs );
		Throw(out, ex);
	}

// map an object
	virtual qObj *MapObj(qObj *obj, const char *name) {
    assert(name != 0);
		qObj *old = NULL;
		myMap.Set(name, obj, &old);
		if (old) old->Free();
		return obj;
	}

	virtual qObj *MapObj(qObj *obj, const CStr &name) {
    assert(!name.IsEmpty());
		qObj *old = NULL;
		myMap.Set(name, obj, &old);
		if (old) old->Free();
		return obj;
	}

// find an object
	virtual bool FindL(qObj **obj, const char *name) 
		{ return myMap.Find(name, *obj) && *obj ? true : ((*obj = 0), false); }
	virtual bool FindL(qObj **obj, const CStr &name) 
		{ return myMap.Find(name, *obj) && *obj ? true : ((*obj = 0), false); }

// kill an object
	virtual qCtx *DelObj(const char *name) 
		{ qObj *obj; if (RemObj(&obj, name)) {if (obj) obj->Free(); return this;} return NULL; }
	virtual qCtx *DelObj(const CStr &name) 
		{ qObj *obj; if (RemObj(&obj, name)) {if (obj) obj->Free(); return this;} return NULL; }

// maintain references (by default don't)
	virtual void AddRef() {
		assert(0);
	}

	virtual void Free() {
		assert(0);
	}

// map local only if not found
	qObj *MapObjLet(qObj *obj, const char *name, qCtx *top) 
		{ qCtx *where = DelObjLet(name, top); 
		  return where ? where->MapObj(obj, name) : MapObj(obj, name); }
	qObj *MapObjLet(qObj *obj, const CStr &name, qCtx *top) 
		{ qCtx *where = DelObjLet((const CStr &)name, top); 
		  return where ? where->MapObj(obj, name) : MapObj(obj, name); }
// auto env
	qObj *MapObjLet(qObj *obj, const char *name) 
		{ return MapObjLet(obj, name, GetEnv() ? GetEnv()->GetSessionCtx() : this); }
	qObj *MapObjLet(qObj *obj, const CStr &name) 
		{ return MapObjLet(obj, name, GetEnv() ? GetEnv()->GetSessionCtx() : this); }

// remove an object from the mapm but dont free it
	virtual bool RemObj(qObj **obj, const char * name) 
		{return (myMap.Del(name, obj) ?true : false);}
	virtual bool RemObj(qObj **obj, const CStr name) 
		{return (myMap.Del(name, obj) ?true : false);}

// helper maps
	qObj *MapArgv(int argc, char **argv, const CStr & name)
		{return MapObj(CreateArgv(argc, argv), name);}

	qObj *MapObj(int *val, const CStr & name) 
		{return MapObj(CreateObj(val), name);}

	qObj *MapObj(double val, const CStr & name) 
		{return MapObj(CreateObj(val), name);}

	qObj *MapObj(const char **charpp, const CStr & name) 
		{return MapObj(CreateObj(charpp), name);}

	qObj *MapObj(CStr *strp, const CStr & name) 
		{return MapObj(CreateObj(strp), name);}

	qObj *MapObj(CStr str, const CStr & name) 
		{return MapObj(CreateObj(str), name);}

	qObj *MapObjLet(CStr str, const CStr & name, qCtx *top) 
		{return MapObjLet(CreateObj(str), name, top);}

	qObj *MapObj(const char *str, const CStr & name) 
		{return MapObj(CreateObj(str), name);}

	qObj *MapObj(const void *ext_obj, QOBJFUNC func, const CStr & name, char *qmap = NULL) 
		{return MapObj(CreateObj(ext_obj, func, qmap), name);}

	void SetValue(const char *name, QOBJFUNC func, void *ext_obj)
		{MapObj(CreateObj(ext_obj, func, NULL), name);}

	qObj *MapObj(QOBJFUNC func, const CStr & name, char *qmap = NULL) 
		{return MapObj(CreateObj(func, qmap), name);}

	qObj *MapObj(qObj* obj,QOBJMETH meth, const CStr & name, char *qmap = NULL) 
		{return MapObj(CreateObj(obj, meth, qmap), name);}
// end helper maps

// create helpers
	qObj *CreateArgv(int argc, char **argv)
		{qObj *obj; return obj = new qObjArgv(argc, argv);}

	qObj *CreateObj(int *val) 
		{qObj *obj; return obj = new qObjIntP(val);}
	qObj *CreateObj(double val) 
		{qObj *obj; return obj = new qObjDbl(val);}

	qObj *CreateObj(const char *str) 
		{qObj *obj; return obj = new qObjStr(CStr(str));}

	qObj *CreateObj(CStr str) 
		{qObj *obj; return obj = new qObjStr(str);}

	qObj *CreateObj(CStr *strp) 
		{qObj *obj; return obj = new qObjStrP(strp);}
	qObj *CreateObj(const char **charpp) 
		{qObj *obj; return obj = new qObjCharPP(charpp);}

	qObj *CreateObj(QOBJFUNC func, char *qmap) 
		{qObj *obj; return obj = new qObjFunc(0, func, qmap);}

	qObj *CreateObj(const void *ext_obj, QOBJFUNC func, char *qmap) 
		{qObj *obj; return obj = new qObjFunc(ext_obj, func, qmap);}

	qObj *CreateObj(qObj* obj, QOBJMETH meth, char *qmap) 
		{qObj *mobj; return mobj = new qObjMeth(obj, meth, qmap);}

// end create helpers

// public find
	template <class STR_TYPE>
	bool Find(qObj **obj, const STR_TYPE name) {
		qCtx *where;
		return Find(obj, name, &where);
	}

// public find/where

	template <class STR_TYPE>

	bool Find(qObj **obj, const STR_TYPE name, qCtx **where, qCtx *top = NULL) {
		if (FindC(obj, name, where, top))
			return true;
		else if (FindL(obj, name)) {
			*where=this;
			return true; 
		} else if (myParent && this!=top)
			return myParent->Find(obj, name, where, top) ;
		else 
			return false;
	}

// macro parsing
	void Parse(qStr *in, qStr *out);

// "safe" document parsing with "magic strings" support
	bool ParseMagic(qStr *in, qStr *out, bool bProcessUnhandled = true);

// "safe" parsing without magic 
	void ParseTry(qStr *in, qStr *out);

// argument array parsing
	void   ParseArgs(qStr *in, qArgAry *out);
	void   ParseArgsQmap(qStr *in, qArgAry *out, char *qmap);

// argument parsing
//	int    ParseInt(const CStr &str, bool quot);
//	int    ParseInt(int index, qArgAry *args) {return ParseInt(args->GetAt(index),args->GetQuot(index));}
//	double ParseDbl(const CStr &str, bool quot);
//	double ParseDbl(int index, qArgAry *args) {return ParseDbl(args->GetAt(index),args->GetQuot(index));}
	CStr   ParseStr(const CStr &str);
//	CStr   ParseStr(int index, qArgAry *args) {return ParseStr(args->GetAt(index),args->GetQuot(index));}

	void Dump(qStr *out);

	void Parse(const CStr &in, qStr *out) {
		qStrReadBuf tmp(in);
		Parse(&tmp,out);
	}

	void Parse(const CStr &in) {
		qStrNull out; 
		qStrReadBuf tmp(in);
		Parse(&tmp,&out);
	}

// eval helpers
	void   Eval(qStr *out, const char *func, CStr arg1, CStr arg2) {
		qObj *obj; if (Find(&obj, func)) {qArgAry tmp; tmp.Add(arg1);tmp.Add(arg2);obj->Eval(this, out, &tmp);} 
	}
	void   Eval(qStr *out, const char *func, CStr arg1) { 
		qObj *obj; if (Find(&obj, func)) {qArgAry tmp; tmp.Add(arg1); obj->Eval(this, out, &tmp);} 
	}
	void   Eval(qStr *out, const char *func, qArgAry *args) { 
		qObj *obj; if (Find(&obj, func)) {obj->Eval(this, out, args);} 
	}
	CStr Eval(const char *func, qArgAry *args = 0) 
		{qStrBuf tmp; Eval(&tmp, func, args); return tmp;}
	CStr Eval(const char *func, const char *arg1) 
		{qStrBuf tmp; Eval(&tmp, func, arg1); return tmp;}
	CStr Eval(const char *func, const char *arg1, const char *arg2) 
		{qStrBuf tmp; Eval(&tmp, func, arg1, arg2); return tmp;}

	int EvalInt(const char *func, qArgAry *args = 0) 
		{CStr tmp = Eval(func, args); return tmp.IsEmpty() ? 0 : atoi(tmp);}

// delete all the way to the top
	qCtx *DelObjTop(const char *name, qCtx *top);
	qCtx *DelObjTop(const CStr &name, qCtx *top);

// map and KILL (using this is not in the spirit of "context" coding)
	qCtx *MapObjTop(qObj *obj, const CStr &name);
	qCtx *MapObjTop(qObj *obj, const CStr &name, qCtx *top);

	template <class STR_TYPE>
	qCtx *DelObjLet(const STR_TYPE name, qCtx *top)
	{
		qCtx *where = this;
		while (where) {
			if (where->DelObj(name))
				return where;
			if (top == where)
				return NULL;
			where = where->GetParent();
		}
		return NULL;
	}

	template <class STR_TYPE>
	qCtx *DelObjRoot(STR_TYPE name)
	{
		qObj *obj;
		if (!name && !*name)
			return NULL;

		char *p = strstr(name, "->");

		if (p) {
			*p++ = '\0';
		} 
		if (FindL(&obj, name)) {
			if (obj->GetCtx()) {
				myMap.Del(name);
				obj->Free();
				return this;
			}
		}
		return NULL;
	}
};

class qCtxTmp : public qCtx {
protected:
//	qCtxTmp() { 
//		Construct(NULL, NULL);
//	}

public:	

	~qCtxTmp() {}

	qCtxTmp(qCtx *parent) {
		Construct(parent, parent ? parent->GetEnv() : NULL);
	}

	qCtxTmp(qEnv *env) { 
		assert(env!=NULL); 
		Construct(NULL, env);
		if (!env->GetSessionCtx())
			env->SetSessionCtx(this);

	}

	qCtxTmp(qCtx *parent, qEnv *env) { 
		assert(env!=NULL); 
		Construct(parent, env);
		if (!env->GetSessionCtx())
			env->SetSessionCtx(this);
	}
};

class qCtxRef : public qCtx {
protected:
	int   myRefs;

	~qCtxRef() {
		assert(myRefs == 0);
	}

public:

	qCtxRef(qCtx *parent = NULL) {
		Construct(parent, NULL);
		myRefs = 1;
	}

	void AddRef() {
		++myRefs;
	}

	void Free() {
		if (--myRefs == 0) {
			delete this;
		}
	}
	
	void Parse(qStr *in, qStr *out) {
		AddRef();
		qCtx::Parse(in, out);
		Free();
	}
};

class qCtxTS : public qCtx {

protected:
	CCrit myCrit;
	int   myRefs;

	~qCtxTS() {
		assert(myRefs == 0);
	}

public:

	qCtxTS(qCtxTS *parent = NULL) {
		Construct(parent, NULL);
		myRefs = 1;
	}

	void AddRef() {
		InterlockedIncrement((long *) &myRefs);
	}

	void Free() {
		if (InterlockedDecrement((long *) &myRefs) == 0) {
			delete this;
		}
	}

	CLock Enter() {
		return myCrit.Enter();
	}
	
	virtual void Clear(bool force = false) {

		CLock lock = Enter();
		qCtx::Clear(force);
	}

	void Parse(qStr *in, qStr *out) {
		AddRef();
		qCtx::Parse(in, out);
		Free();
	}

	virtual qObj *MapObj(qObj *obj, const CStr &name)  {	
		CLock lock = Enter();
		return qCtx::MapObj(obj, name);
	}

	virtual qObj *MapObj(qObj *obj, const char *name) {
		CLock lock = Enter();
		return qCtx::MapObj(obj, name);
	}

	virtual bool FindL(qObj **obj, const CStr &name) {
		AddRef();
		bool rVal = qCtx::FindL(obj, name);
		Free();
		return rVal;
	}

	virtual bool FindL(qObj **obj, const char *name) {
		AddRef();
		bool rVal = qCtx::FindL(obj, name);
		Free();
		return rVal;
	}

	virtual bool RemObj(qObj **obj, const char * name) {
		CLock lock = Enter();
		return qCtx::RemObj(obj, name);
	}

	virtual bool RemObj(qObj **obj, const CStr name) {
		CLock lock = Enter();
		return qCtx::RemObj(obj, name);
	}
};

// helpers should be in qsUtil.h
void EvalBreak(bool *ok, qCtx *ctx, qStr *out, qArgAry *args);
qEnvHttp *GetHttpEnv(qCtx *ctx);
void qsInit(void *module = NULL);
extern bool qsUnreg;

#define VALID_ARGM(name, min) {\
	if ((args ? (args->Count()) : 0) < min) {\
		ctx->ThrowF(out, 1000, "Too few arguments (%d) to function %%%s(), must be at least %d", args->Count(), name, min);\
		return;\
	}\
}

#define VALID_ARGC(name, min, max) {\
	if ((args ? (args->Count()) : 0) > max) {\
		ctx->ThrowF(out, 1000, "Too many arguments (%d) to function %%%s(), the maximum is %d", args->Count(), name, max);\
		return;\
	} else VALID_ARGM(name, min)\
}


#endif //#ifndef _QCTX_

