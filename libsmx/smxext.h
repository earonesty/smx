/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _PSEXT_H
#define _PSEXT_H

#define SMXEXTVER 1

#ifdef WIN32
	#ifndef STDCALL
		#define STDCALL _stdcall
	#endif
#endif

#ifndef STDCALL 
	#define STDCALL __attribute__((stdcall)) 
#endif

#ifndef DWORD
        #define DWORD unsigned short
#endif

struct SMXENUMPOS {
	long p1;
	long p2;
};

class smxExObject;
class smxExContext;
class smxExStreamOut;

enum smxArgType {
	smxString = 0,
	smxQuotedString = 1
};

class smxExObject {
public:
    virtual void STDCALL Eval(smxExContext *pContext, smxExStreamOut *pOutput, const char *pArgs[], smxArgType pArgType[], int nArgs) = 0;
    virtual smxExContext *STDCALL GetContext() = 0;
	virtual void STDCALL Delete() = 0;
};

typedef void (STDCALL smxExObject::* SMXUSERMETH)
    (smxExContext *pContext, smxExStreamOut *pOutput, const char * pArgs[], smxArgType pArgType[], int nNumArgs);

typedef void (STDCALL * SMXUSERFUNC)
	(const void *pObject,smxExContext *pContext, smxExStreamOut *pOutput, const char *pArgs[], smxArgType pArgType[], int nNumArgs);

class smxExStreamOut {
public:
	virtual int STDCALL GetVersion() = 0;

	virtual smxExStreamOut * STDCALL New() = 0;
	virtual void STDCALL Delete() = 0;

	virtual void STDCALL PutS(const char *pStr, int nLen) = 0;
	virtual void STDCALL PutS(const char *pStr) = 0;
	virtual void STDCALL PutC(char c) = 0;

	virtual void STDCALL Clear() = 0;

	virtual const char * STDCALL GetBuf() = 0;
};

class smxExStreamIn {
public:
	virtual int  STDCALL GetVersion() = 0;
	virtual int  STDCALL GetS(const char **pStr) = 0;
	virtual char STDCALL GetC() = 0;
};

class smxExContext {
public:
// version number
	virtual int STDCALL GetVersion() = 0;

	virtual smxExContext * STDCALL New() = 0;
	virtual void STDCALL Delete() = 0;

	virtual void STDCALL Parse(const char *pIn, int nLen, smxExStreamOut *pOut) = 0;
	virtual void STDCALL ParseString(const char *pIn, smxExStreamOut *pOut) = 0;
	virtual char * STDCALL ParseArg(int nIndex, const char *pArgs[], smxArgType pArgType[], int nNumArgs) = 0;

// get info from context
	virtual smxExObject *STDCALL GetObj(const char *name) = 0;
	virtual void STDCALL Eval(const char *pFunc, int nLen, smxExStreamOut *pOut, const char *pArgs[], smxArgType pArgType[], int nNumArgs) = 0;

// map objects
	virtual void STDCALL MapObj(smxExObject *pObj, const char *name) = 0;
	virtual void STDCALL MapFunc(void *data, SMXUSERFUNC pFunc, const char *name) = 0;
	virtual void STDCALL MapString(const char *pStr, const char *name) = 0;
	virtual void STDCALL MapInt(int pInt, const char *name) = 0;

// allocate a bit of memory
	virtual void * STDCALL Alloc(int pInt) = 0;

// kill objects
	virtual void STDCALL DelObj(const char *name) = 0;
	virtual void STDCALL Clear() = 0;

// enumerate objects
	virtual SMXENUMPOS STDCALL First() = 0;
	virtual bool STDCALL Next(SMXENUMPOS *ppos, const char **name) = 0;

// create a new output buffer (must call Delete() or there will be memory leaks!)
	virtual smxExStreamOut * STDCALL NewBuffer() = 0;
};

typedef void (STDCALL * SMXLOADFUNC) (smxExContext *pContext);
typedef void (STDCALL * SMXTERMFUNC) ();

// This structure defines the SMX extension library
typedef struct _SMXEXTLIB {
    DWORD dwSize;           // Size of this structure
    SMXLOADFUNC pLoadLib;   // Called to load the library, given a context
    SMXTERMFUNC pTermLib;   // Called to terminate the library
} SMXEXTLIB, *PSMXEXTLIB;

// helper macros
#define DECL_SMXUSERFUNC(name) void name(const void *pObject, smxExContext *pContext, smxExStreamOut *pOutput, const char *pArgs[], smxArgType pArgType[], int nNumArgs)

#ifdef WIN32
#define SMXLIB_EXPORT extern "C" __declspec(dllexport) 
#else
#define SMXLIB_EXPORT extern "C"
#endif


#endif // #ifndef _PSEXT_H
