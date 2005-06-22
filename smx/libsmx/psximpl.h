/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _PSIMPL_H

class psxExContextImpl;
class psxExStreamOutImpl;
class psxExObject;

static char S_QMAP[2] = "A";

class psxExObjectWrap : public qObj {
	psxExObject *myObj;

public:
	psxExObjectWrap(psxExObject* obj) {
		myObj = obj;
	}

	~psxExObjectWrap() {
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

class psxExFuncWrap : public qObj {
	void *myData;
	PSXUSERFUNC myFunc;
public:
	psxExFuncWrap(void *data, PSXUSERFUNC func) {
		myData = data;
		myFunc = func;
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

	virtual char *GetQmap() 
		{return S_QMAP;}
};

class psxExStreamOutWrap : public qStr {
	psxExStreamOut *myStr;
	bool myFree;

public:
	void Delete() {
		if (this)
			delete this;
	}

	psxExStreamOutWrap(psxExStreamOut* str, bool free) {
		myStr = str;
		myFree = free;
	}

	~psxExStreamOutWrap() {
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

class psxExObjectImpl : public psxExObject {
	bool  myFree;
	qObj *myObj;

public:
	psxExObjectImpl(qObj *obj, bool free) {
		myObj  = obj;
		myFree = free;
	}

	virtual ~psxExObjectImpl() {
		if (myFree)
			myObj->Free();
	}

	qObj *STDCALL GetObj() {
		return myObj;
	}

    void STDCALL Eval(psxExContext *pCtx, psxExStreamOut *pOut, const char *pArgs[], psxArgType pArgType[], int nArgs);

    psxExContext *STDCALL GetContext() {
		// put context wrapper in here!
		return NULL;
	}

	void STDCALL Delete() {
		delete this;
	}
};

class psxExStreamOutImpl : public psxExStreamOut {
	bool  myFree;
	qStr *myStr;
	CStr  myTmp;

public:
	psxExStreamOutImpl(qStr *str, bool free) {
		myStr  = str;
		myFree = free;
	}

	virtual ~psxExStreamOutImpl() {
		if (myFree)
			delete myStr;
	}

	psxExStreamOut * STDCALL New() {
		return new psxExStreamOutImpl(new qStrBuf, true); 
	}

	void STDCALL Delete() {
		if (this) 
			delete this;
	}

	int STDCALL GetVersion() {
		return PSXEXTVER;
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

class psxExContextImpl : public psxExContext {
	bool  myFree;
	qCtx *myCtx;

public:
	psxExContextImpl(qCtx *ctx, bool free) {
		myCtx  = ctx;
		myFree = free;
	}

	virtual ~psxExContextImpl() {

		if (myFree)
			myCtx->Free();
	}

	qCtx * STDCALL GetCtx() {
		return myCtx;
	}

	int STDCALL GetVersion() {
		return PSXEXTVER;
	}

	virtual psxExContext * STDCALL New() {
		return new psxExContextImpl(new qCtxRef(myCtx), true);
	}
	virtual void STDCALL Delete() {
		delete this;
	}

	// HACK ALERT... NEED TO WRAP WITH POSSIBLE EXTENDED OBJECTS!
	virtual void STDCALL Parse(const char *pIn, int nLen, psxExStreamOut *pOut) {
		psxExStreamOutWrap wrap(pOut, false);
		qStrReadBuf rb(CStr(pIn, nLen));
		myCtx->Parse(&rb, &wrap);
	}

	virtual void STDCALL ParseString(const char *pIn, psxExStreamOut *pOut) {
		psxExStreamOutWrap wrap(pOut, false);
		qStrReadBuf rb(CStr(pIn, strlen(pIn)));
		myCtx->Parse(&rb, &wrap);
	}

	virtual char * STDCALL ParseArg(int nIndex, const char *pArgs[], psxArgType pArgType[], int nNumArgs) {
		if (nIndex < nNumArgs && pArgs) {
			if (!pArgType || pArgType[nIndex] != psxQuotedString) {
				qStrBuf out;
				myCtx->Parse(pArgs[nIndex], &out);
				((CStr *)pArgs)[nIndex] = out;
			}
			return ((CStr *)pArgs)[nIndex].SafeP();
		} else 
			return "";
	}

	virtual psxExObject * STDCALL GetObj(const char *name) {
		qObj *obj;
		if (myCtx->Find(&obj, name))
			return new psxExObjectImpl(obj, false);
		else
			return NULL;
	}

	virtual void STDCALL Eval(const char *name, int nLen, psxExStreamOut *pOut, const char *pArgs[], psxArgType pArgType[], int nNumArgs) {
		qObj *obj;
		if (myCtx->Find(&obj, name)) {
			qArgAry ary;
			int i;
			for (i = 0; i < nNumArgs; ++i) {
				ary.Add(pArgs[i]);
				ary.SetQuot(i, pArgType ? (pArgType[i] == psxQuotedString) : false);
			}
			psxExStreamOutWrap wrap(pOut, false);
			obj->Eval(myCtx, &wrap, &ary);
		}
	}

// map objects
	virtual void STDCALL MapObj(psxExObject *pObj, const char *name) {
		myCtx->MapObj(new psxExObjectWrap(pObj), name);
	}

	virtual void STDCALL MapFunc(void *pData, PSXUSERFUNC pFunc, const char *name) {
		myCtx->MapObj(new psxExFuncWrap(pData, pFunc), name);
	}

	virtual void STDCALL MapString(const char *pStr, const char *name) {
		myCtx->MapObj(pStr, name);
	}

	virtual void STDCALL MapInt(int pInt, const char *name) {
		myCtx->MapObj(pInt, name);
	}


// kill objects
	virtual void STDCALL DelObj(const char *name) {
		myCtx->DelObj(name);
	}
	virtual void STDCALL Clear() {
		myCtx->Clear();
	}

// enumerate objects
	virtual PSXENUMPOS STDCALL First() {
		PSXENUMPOS pos;
		*((MAPPOS *)(&pos)) = myCtx->GetMap()->First();
		return pos;
	}

	virtual bool STDCALL Next(PSXENUMPOS *ppos, const char **name) {
		qObj *obj;
		const char *key;
		if (myCtx->GetMap()->Next(((MAPPOS *)ppos), &key, &obj)) {
			*name = key;
			return true;
		} else 
			return false;
	}

	virtual psxExStreamOut * STDCALL NewBuffer() {
		return new psxExStreamOutImpl(new qStrBuf, true); 
	}
};

#endif // #ifndef _PSIMPL_H
