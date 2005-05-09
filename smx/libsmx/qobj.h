/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QOBJ_H
#define _QOBJ_H

#include "qstr.h"
#include "qarg.h"
#include "ary.h"

class qObj;
class qCtx;

typedef void (qObj::*QOBJMETH)(qCtx *,qStr *,qArgAry *);
typedef void (*QOBJFUNC) (const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#define QMAP_EVEN "E"
#define QMAP_ALL "A"

class qObj {
protected:
	virtual ~qObj()
		{} 
public:
// override
	virtual void Free(bool force = false)
		{delete this;}
/*	virtual qObj *AddRef()
		{return this;} */

	virtual void Eval(qCtx *ctx, qStr *out, qArgAry *args) 
		{}
	virtual char *GetQmap() 
		{return NULL;}
	virtual bool EvalSet(qCtx *ctx, qStr *out, qArgAry *args)
		{return false;}
	virtual void Dump(qStr *out)
		{}
	virtual qCtx *GetCtx()
		{return NULL;}

// helpers
	void Eval(qCtx *ctx, qStr *out)
		{Eval(ctx, out, NULL);}
	void Eval(qCtx *ctx, qStr *out, CStr arg1)
		{qArgAry args(&arg1, 1); Eval(ctx, out, &args);}
	void Eval(qCtx *ctx, qStr *out, CStr arg1, CStr arg2)
		{qArgAry args(&arg1, 2); Eval(ctx, out, &args);}

	CStr Eval(qCtx *ctx, qArgAry *args)
		{qStrBuf tmp; Eval(ctx, &tmp, args); return tmp;}
	CStr Eval(qCtx *ctx)
		{qStrBuf tmp; Eval(ctx, &tmp, NULL); return tmp;}
	CStr Eval(qCtx *ctx, CStr arg1)
		{qStrBuf tmp; Eval(ctx, &tmp, arg1); return tmp;}
	CStr Eval(qCtx *ctx, CStr arg1, CStr arg2)
		{qStrBuf tmp; Eval(ctx, &tmp, arg2); return tmp;}
};

class qObjTSRef;

class qObjTS : public qObj {
	int   myRefs;

protected:
	int GetRefs() {
		return myRefs;

	}

	~qObjTS() { 
		assert(myRefs == 0); 
	}
public:
	qObjTS() {
		myRefs = 1;
	}
	virtual void Free(bool force = false) {
		assert(myRefs > 0);
#ifdef WIN32
		if (!InterlockedDecrement((LONG *)&myRefs))
#else
		if (!--myRefs || force)
#endif
			delete this;
	}

	virtual void AddRef() {
#ifdef WIN32
		InterlockedIncrement((LONG *)&myRefs);
#else
		++myRefs;
#endif
	}

	qObjTSRef GetRef();
};

class qObjTSRef {
	friend class qObjTS;

	qObjTS *myTS;

	qObjTSRef(qObjTS *ts) {
		myTS = ts;
		if (myTS)
			myTS->AddRef();
	}

public:
	qObjTSRef() {
		myTS = NULL;
	}

	qObjTSRef & operator =(const qObjTSRef &ref) {
		if (myTS != ref.myTS) {
			qObjTS *ts = myTS;
			myTS = ref.myTS;
			if (myTS)
				myTS->AddRef();
			if (ts)
				ts->Free();
		}
    return *this;
	}

	~qObjTSRef() {
		if (myTS) 
			myTS->Free();
	}
};

class qObjStr : public qObj {
	CStr myStr;
public:
	qObjStr(const CStr &str) {myStr = str;}
	void Eval(qCtx *ctx, qStr *out, qArgAry *args)
		{out->PutS(myStr);}
	CStr GetStr()
		{return myStr;}
	virtual void Dump(qStr *out)
		{out->PutS(myStr);}
};

class qObjStrP : public qObj {
	CStr *myStrP;
public:
	qObjStrP(CStr *strp) {myStrP = strp;}
	void Eval(qCtx *ctx, qStr *out, qArgAry *args)
		{out->PutS(*myStrP);}
	CStr GetStr()
		{return *myStrP;}
	virtual void Dump(qStr *out)
		{out->PutS(*myStrP);}
};

class qObjCharP : public qObj {
	char *myCharP;
public:
	qObjCharP(char *CharP) {myCharP = CharP;}
	void Eval(qCtx *ctx, qStr *out, qArgAry *args)
		{out->PutC(*myCharP);}
	virtual void Dump(qStr *out)
		{out->PutC(*myCharP);}
};

class qObjCharPP : public qObj {
	const char **myCharPP;
public:
	qObjCharPP(const char **CharPP) {myCharPP = CharPP;}
	void Eval(qCtx *ctx, qStr *out, qArgAry *args)
		{if (*myCharPP) out->PutS(*myCharPP);}
	virtual void Dump(qStr *out)
		{if (*myCharPP) out->PutS(*myCharPP);}
};

class qObjArgv : public qObj {
	char **myArgv;
	int myArgc;
public:
	qObjArgv(int Count, char **CharPP) {
		myArgv = CharPP; 
		myArgc = Count;
	}
	
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

	virtual void Dump(qStr *out) {
		int index; for (index = 0; index < myArgc; ++index) {
			out->PutS(myArgv[index]);
			out->PutC(',');
		}
	}
};

class qObjIntP : public qObj {
	int *myIntP;
public:
	qObjIntP(int *val) {myIntP = val;}
	inline void Eval(qCtx *ctx, qStr *out, qArgAry *args)
		{out->PutN(*myIntP);}
	virtual void Dump(qStr *out)
		{out->PutS("<int *>=");out->PutN(*myIntP);}
};

class qObjDbl : public qObj {
	double myDbl;
public:
	qObjDbl(double val) {myDbl = val;}
	inline void Eval(qCtx *ctx, qStr *out, qArgAry *args)
		{out->PutN(myDbl);}
	double Set(double val) {return myDbl = val;}
	operator double () const {return myDbl;}
	virtual void Dump(qStr *out)
		{out->PutN(myDbl);}
};

class qObjFunc : public qObj {
	const void *myData;
	char *myQmap;
	QOBJFUNC myFunc;
public:
	qObjFunc(const void *newData, QOBJFUNC newFunc, char *qmap = NULL)
		{myData = newData; myFunc = newFunc; myQmap = qmap;}
	inline void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		myFunc(myData, ctx, out, args);
	}
	inline char *GetQmap() {
		return myQmap;
	}
	virtual void Dump(qStr *out)
		{out->PutS("<function>");}
};

class qObjMeth : public qObj {
	void (qObj::*myMeth)(qCtx *,qStr *,qArgAry *);
	qObj *myObj;
	char *myQmap;

public:
	qObjMeth(qObj* newObj, void (qObj::*newMeth)(qCtx *,qStr *,qArgAry *), char *qmap = NULL)
		{myObj = newObj; myMeth = newMeth; myQmap = qmap;}

	inline void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		(myObj->*myMeth)(ctx, out, args);
	}
	inline char *GetQmap() {
		return myQmap;
	}
	virtual void Dump(qStr *out)
		{out->PutS("<method>");}
};

class qObjDef : public qObjTS
{
	CStr myBody;
	CStrAry myArgNames;
	CAry<int> myQuoted;
	CStr myQmap;

public:
	enum qObjDefArg
	{
		dNormal,
		dQuoted,
		dParsed,
		dObjRef
	};

	qObjDef(const CStr &body) {myBody = body;}

	void AddArgName(const CStr arg, int quoted);
	
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

	char *GetQmap() {return myQmap.GetBuffer();}

	bool IsQuoted(int i)
		{ return (i >= 0 && i < myQuoted.Count() && myQuoted[i] == dQuoted); }

	bool IsAuto(int i)
		{ return (i >= 0 && i < myQuoted.Count() && myQuoted[i] == dParsed); }

	bool IsRef(int i)
		{ return (i >= 0 && i < myQuoted.Count() && myQuoted[i] == dObjRef); }
};

#endif //#ifndef _QOBJ_H
