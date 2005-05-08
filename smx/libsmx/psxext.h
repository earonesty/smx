/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _PSEXT_H
#define _PSEXT_H

#define PSXEXTVER 1

#ifdef WIN32
#ifndef STDCALL
#define STDCALL _stdcall
#endif
#endif

struct PSXENUMPOS {
	long p1;
	long p2;
};

class psxExObject;
class psxExContext;
class psxExStreamOut;

enum psxArgType {
	psxString = 0,
	psxQuotedString = 1
};

class psxExObject {
public:
    virtual void STDCALL Eval(psxExContext *pContext, psxExStreamOut *pOutput, const char *pArgs[], psxArgType pArgType[], int nArgs) = 0;
    virtual psxExContext *STDCALL GetContext() = 0;
	virtual void STDCALL Delete() = 0;
};

typedef void (STDCALL psxExObject::* PSXUSERMETH)
    (psxExContext *pContext, psxExStreamOut *pOutput, const char * pArgs[], psxArgType pArgType[], int nNumArgs);

typedef void (STDCALL * PSXUSERFUNC)
	(const void *pObject,psxExContext *pContext, psxExStreamOut *pOutput, const char *pArgs[], psxArgType pArgType[], int nNumArgs);

class psxExStreamOut {
public:
	virtual int STDCALL GetVersion() = 0;

	virtual psxExStreamOut * STDCALL New() = 0;
	virtual void STDCALL Delete() = 0;

	virtual void STDCALL PutS(const char *pStr, int nLen) = 0;
	virtual void STDCALL PutS(const char *pStr) = 0;
	virtual void STDCALL PutC(char c) = 0;

	virtual void STDCALL Clear() = 0;

	virtual const char * STDCALL GetBuf() = 0;
};

class psxExStreamIn {
public:
	virtual int  STDCALL GetVersion() = 0;
	virtual int  STDCALL GetS(const char **pStr) = 0;
	virtual char STDCALL GetC() = 0;
};

class psxExContext {
public:
// version number
	virtual int STDCALL GetVersion() = 0;

	virtual psxExContext * STDCALL New() = 0;
	virtual void STDCALL Delete() = 0;

	virtual void STDCALL Parse(const char *pIn, int nLen, psxExStreamOut *pOut) = 0;
	virtual void STDCALL ParseString(const char *pIn, psxExStreamOut *pOut) = 0;
	virtual char * STDCALL ParseArg(int nIndex, const char *pArgs[], psxArgType pArgType[], int nNumArgs) = 0;

// get info from context
	virtual psxExObject *STDCALL GetObj(const char *name) = 0;
	virtual void STDCALL Eval(const char *pFunc, int nLen, psxExStreamOut *pOut, const char *pArgs[], psxArgType pArgType[], int nNumArgs) = 0;

// map objects
	virtual void STDCALL MapObj(psxExObject *pObj, const char *name) = 0;
	virtual void STDCALL MapFunc(void *data, PSXUSERFUNC pFunc, const char *name) = 0;
	virtual void STDCALL MapString(const char *pStr, const char *name) = 0;
	virtual void STDCALL MapInt(int pInt, const char *name) = 0;

// kill objects
	virtual void STDCALL DelObj(const char *name) = 0;
	virtual void STDCALL Clear() = 0;

// enumerate objects
	virtual PSXENUMPOS STDCALL First() = 0;
	virtual bool STDCALL Next(PSXENUMPOS *ppos, const char **name) = 0;

// create a new output buffer (must call Delete() or there will be memory leaks!)
	virtual psxExStreamOut * STDCALL NewBuffer() = 0;
};

typedef void (STDCALL * PSXLOADFUNC) (psxExContext *pContext);
typedef void (STDCALL * PSXTERMFUNC) ();

// This structure defines the PSX extension library
typedef struct _PSXEXTLIB {
    DWORD dwSize;           // Size of this structure
    PSXLOADFUNC pLoadLib;   // Called to load the library, given a context
    PSXTERMFUNC pTermLib;   // Called to terminate the library
} PSXEXTLIB, *PPSXEXTLIB;

// helper macros
#define DECL_PSXUSERFUNC(name) name(const void *pObject, psxExContext *pContext, psxExStreamOut *pOutput, const char *pArgs[], psxArgType pArgType[], int nNumArgs)
#define PSXLIB_EXPORT extern "C" __declspec(dllexport) 

#endif // #ifndef _PSEXT_H
