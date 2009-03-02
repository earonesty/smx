/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _PSIMPL_H

class smxExContextImpl;
class smxExStreamOutImpl;
class smxExObject;

static char S_QMAP[2] = "A";

class smxExObjectWrap : public qObj {
	smxExObject *myObj;

public:
	smxExObjectWrap(smxExObject* obj) {
		myObj = obj;
	}

	~smxExObjectWrap() {
		myObj->Delete();
	}

	void Delete() {
		if (this) 
			delete this;
	}
	
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

	char *GetQmap() {
		return S_QMAP;
	}
};

class smxExFuncWrap : public qObj {
	void *myData;
	SMXUSERFUNC myFunc;
public:
	smxExFuncWrap(void *data, SMXUSERFUNC func) {
		myData = data;
		myFunc = func;
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

	virtual char *GetQmap() 
		{return S_QMAP;}
};

class smxExStreamOutWrap : public qStr {
	smxExStreamOut *myStr;
	bool myFree;

public:
	void Delete() {
		if (this)
			delete this;
	}

	smxExStreamOutWrap(smxExStreamOut* str, bool free) {
		myStr = str;
		myFree = free;
	}

	~smxExStreamOutWrap() {
		if (myFree)
			myStr->Delete();
	}

// writing
	virtual void PutS(const char *s)
		{if (s) myStr->PutS(s, strlen(s));}

	virtual void PutS(const CStr &s)
		{if (s) myStr->PutS(s, s.Length());}

	virtual void PutC(char c)
		{myStr->PutC(c);}

	virtual void PutS(const char *s, int len)
		{if (s) myStr->PutS(s, len);}
};

class smxExObjectImpl : public smxExObject {
	bool  myFree;
	qObj *myObj;

public:
	smxExObjectImpl(qObj *obj, bool free) {
		myObj  = obj;
		myFree = free;
	}

	virtual ~smxExObjectImpl() {
		if (myFree)
			myObj->Free();
	}

	qObj *STDCALL GetObj() {
		return myObj;
	}

    void STDCALL Eval(smxExContext *pCtx, smxExStreamOut *pOut, const char *pArgs[], smxArgType pArgType[], int nArgs);

    smxExContext *STDCALL GetContext() {
		// put context wrapper in here!
		return NULL;
	}

	void STDCALL Delete() {
		delete this;
	}
};

class smxExStreamOutImpl : public smxExStreamOut {
	bool  myFree;
	qStr *myStr;
	CStr  myTmp;

public:
	smxExStreamOutImpl(qStr *str, bool free) {
		myStr  = str;
		myFree = free;
	}

	virtual ~smxExStreamOutImpl() {
		if (myFree)
			delete myStr;
	}

	smxExStreamOut * STDCALL New() {
		return new smxExStreamOutImpl(new qStrBuf, true); 
	}

	void STDCALL Delete() {
		if (this) 
			delete this;
	}

	int STDCALL GetVersion() {
		return SMXEXTVER;
	}

	void STDCALL PutS(const char *pStr, int nLen) {
		myStr->PutS(pStr, nLen);
	}

	void STDCALL PutS(const char *pStr) {
		myStr->PutS(pStr);
	}

	void STDCALL PutC(char c) {
		myStr->PutC(c);
	}

	void STDCALL Clear() {
		myStr->Clear();
	}

	const char * STDCALL GetBuf() {
		myTmp = myStr->GetS();
		return myTmp;
	}
};

class smxExContextImpl : public smxExContext {
	bool  myFree;
	qCtx *myCtx;

public:
	smxExContextImpl(qCtx *ctx, bool free) {
		myCtx  = ctx;
		myFree = free;
	}

	virtual ~smxExContextImpl() {

		if (myFree)
			myCtx->Free();
	}

	qCtx * STDCALL GetCtx() {
		return myCtx;
	}

	int STDCALL GetVersion() {
		return SMXEXTVER;
	}

	virtual smxExContext * STDCALL New() {
		return new smxExContextImpl(new qCtxRef(myCtx), true);
	}
	virtual void STDCALL Delete() {
		delete this;
	}

	// HACK ALERT... NEED TO WRAP WITH POSSIBLE EXTENDED OBJECTS!
	virtual void STDCALL Parse(const char *pIn, int nLen, smxExStreamOut *pOut) {
		smxExStreamOutWrap wrap(pOut, false);
		qStrReadBuf rb(CStr(pIn, nLen));
		myCtx->Parse(&rb, &wrap);
	}

	virtual void STDCALL ParseString(const char *pIn, smxExStreamOut *pOut) {
		smxExStreamOutWrap wrap(pOut, false);
		qStrReadBuf rb(CStr(pIn, strlen(pIn)));
		myCtx->Parse(&rb, &wrap);
	}

	virtual char * STDCALL ParseArg(int nIndex, const char *pArgs[], smxArgType pArgType[], int nNumArgs) {
		if (nIndex < nNumArgs && pArgs) {
			if (!pArgType || pArgType[nIndex] != smxQuotedString) {
				qStrBuf out;
				myCtx->Parse(pArgs[nIndex], &out);
				((CStr *)pArgs)[nIndex] = out;
			}
			return ((CStr *)pArgs)[nIndex].SafeP();
		} else 
			return "";
	}

	virtual smxExObject * STDCALL GetObj(const char *name) {
		qObj *obj;
		if (myCtx->Find(&obj, name))
			return new smxExObjectImpl(obj, false);
		else
			return NULL;
	}

	virtual void STDCALL Eval(const char *name, int nLen, smxExStreamOut *pOut, const char *pArgs[], smxArgType pArgType[], int nNumArgs) {
		qObj *obj;
		if (myCtx->Find(&obj, name)) {
			qArgAry ary;
			int i;
			for (i = 0; i < nNumArgs; ++i) {
				ary.Add(pArgs[i]);
				ary.SetQuot(i, pArgType ? (pArgType[i] == smxQuotedString) : false);
			}
			smxExStreamOutWrap wrap(pOut, false);
			obj->Eval(myCtx, &wrap, &ary);
		}
	}

// map objects
	virtual void STDCALL MapObj(smxExObject *pObj, const char *name) {
		myCtx->MapObj(new smxExObjectWrap(pObj), name);
	}

	virtual void STDCALL MapFunc(void *pData, SMXUSERFUNC pFunc, const char *name) {
		myCtx->MapObj(new smxExFuncWrap(pData, pFunc), name);
	}

	virtual void STDCALL MapString(const char *pStr, const char *name) {
		myCtx->MapObj(pStr, name);
	}

	virtual void STDCALL MapInt(int pInt, const char *name) {
		myCtx->MapObj(pInt, name);
	}

// allocate memory
        virtual void * STDCALL Alloc(int pInt);

// kill objects
	virtual void STDCALL DelObj(const char *name) {
		myCtx->DelObj(name);
	}
	virtual void STDCALL Clear() {
		myCtx->Clear();
	}

// enumerate objects
	virtual SMXENUMPOS STDCALL First() {
		SMXENUMPOS pos;
		*((MAPPOS *)(&pos)) = myCtx->GetMap()->First();
		return pos;
	}

	virtual bool STDCALL Next(SMXENUMPOS *ppos, const char **name) {
		qObj *obj;
		const char *key;
		if (myCtx->GetMap()->Next(((MAPPOS *)ppos), &key, &obj)) {
			*name = key;
			return true;
		} else 
			return false;
	}

	virtual smxExStreamOut * STDCALL NewBuffer() {
		return new smxExStreamOutImpl(new qStrBuf, true); 
	}
};

#endif // #ifndef _PSIMPL_H
