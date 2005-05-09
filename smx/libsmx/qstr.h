/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QSTR_H
#define _QSTR_H

#ifndef _STR_H
	#include "qlib.h"
	#include "str.h"
	#include "map.h"
	#include "lst.h"
#endif

#include "util.h"
#include "pstime.h"

inline CStr Dbl2Str(double num) {
	CStr tmp(32);
	_gcvt(num, 20, tmp.GetBuffer());
	// TODO: TAKE THIS OUT AND MAKE IT A COMPILE-TIME OPTION
	tmp.Shrink();
#ifdef SMX_DOT_COMPAT
	return tmp;
#else
	return tmp.RTrim('.');
#endif
}

class qStr {
public:
	virtual ~qStr() {}

// public instances
	virtual qStr *New()	
		{return NULL;}

	virtual void Delete()
		{if (this) delete this;}

// reading
	virtual char GetC() 
		{return EOF;}

	virtual bool UngetC(char c) 
		{return false;}

	virtual CStr GetS() 
		{return (const char *) NULL;}

	virtual int GetS(char *b, int n) 
		{return 0;}

// writing
	virtual void PutS(const char *s)				// override for efficiency (if any)
		{if (s) PutS(s, strlen(s));}

	virtual void PutS(const CStr &s)				// highly reccomended
		{if (s) PutS(s, s.Length());}

	virtual void PutC(char c) 						// highly reccomended
		{PutS(&c, 1);}

	virtual void PutS(const char *s, int len) = 0; // required override, or linker error!

	virtual time_t GetLastModified()
		{return -1;}

	virtual void Clear()
		{}

	virtual int GetLineNum()
		{return 0;}

// write helpers

	virtual void PutS(qStr &s)
	    {CStr t; while ((t = s.GetS()).Length() > 0) PutS(t);}

	virtual void PutN(int i)
		{char tmp[33]; _itoa(i, tmp, 10); PutS(tmp); }

	virtual void PutN(long int i)
		{char tmp[33]; _ltoa(i, tmp, 10); PutS(tmp); }

	virtual void PutN(unsigned int i)
		{char tmp[33]; _ultoa(i, tmp, 10); PutS(tmp); }

	virtual void PutN(double d)
		{PutS(Dbl2Str(d)); }
};

// safety wrapper for putting refs on the stack
class qStrRef {
protected:
	qStr  *myStr;

public:
	qStrRef(qStr *str = NULL) {
		myStr = str; 
	}
	~qStrRef() {
		if (myStr)
			delete myStr;
	}
	qStrRef &operator =(qStr *str) {
		myStr = str; 
		return *this;
	}

	operator bool () {
		return myStr != NULL;
	}

	bool operator ! () {
		return myStr == NULL;
	}

	operator qStr & () { 
		return *myStr;
	}

	operator qStr * () { 
		return myStr;
	}
};

class qStrApp : public qStr {
	CStr *myBuf;
	char *myP;

public:
	qStrApp(CStr &buf)
		{myBuf = &buf; myP = (myBuf->GetBuffer() + myBuf->Length());}

	char GetC()
		{if (myP < ((const char *) *myBuf) + myBuf->Length()) return *myP++; else return -1;}

	bool UngetC(char c) 
		{if (myP > (const char *)*myBuf) {*(--myP)=c; return true; } else return false;}

	int GetS(char *b, int n) {
		if (myP < ((const char *) *myBuf) + myBuf->Length()) {
			n = min(n, (myBuf->Data() + myBuf->Length()) - myP);
			memcpy(b, myP, n);
			myP+=n;
			return n;
		} else 
			return 0;
	}

	CStr GetS()
		{CStr tmp = myP; *myBuf = 0; return tmp;}

	void PutS(const char *s)
		{myBuf->Append(s);}

	void PutS(const char *s, int len)
		{myBuf->Append(s, len);}

	void PutS(const CStr &s)
		{if (s) PutS(s, s.Length());}

	void PutC(char c)
		{myBuf->Append(c);}
};

class qStrBuf : public qStr, public CStr {
	int myP;

public:
	qStrBuf(const CStr &buf) : CStr(buf)
		{myP = 0;}

	qStrBuf() : CStr()
		{myP = 0; CStr::operator =("");}

	virtual qStr *New()	
		{return new qStrBuf();}

	char GetC()
		{if (myP < Length()) return Data()[myP++]; else return -1;}

	bool UngetC(char c) 
		{if (myP > 0) { --myP; Data()[myP] = c; return true; } else return false;}

	CStr GetS() 
		{CStr tmp = (myP < Length()) ? (Data()+myP) : 0; Clear(); return tmp;}

	int GetS(char *b, int n) {
		if (myP < Length()) {
			n = min(n,Length() - myP);
			memcpy(b, &(Data()[myP]), n);
			myP+=n;
			return n;
		} else 
			return 0;
	}

	void PutS(const char *s)
		{Append(s);}

	void PutS(const char *s, int len)
		{Append(s, len);}

	void PutS(const CStr &s)
		{if (s) PutS(s, s.Length());}

	void PutC(char c)
		{Append(c);}

	void Clear()
		{Grow(0);}

	int GetLineNum();
};

class qStrReadBuf : public qStr {
	const char *myB;
	const char *myE;
	const char *myP;

public:
	qStrReadBuf(const CStr &buf)
		{myB = myP = buf.Data(); myE = myP + buf.Length();}
	qStrReadBuf(const char *p, int n)
		{myB = myP = p; myE = p + n;}
	qStrReadBuf(const char *p)
		{myB = myP = p; myE = p + strlen(p);}

	char GetC()
		{if (myP < myE) return *myP++; else return -1;}

	bool UngetC(char c) 
		{if (myP > myB && c == myP[-1]) { --myP; return true; } else return false;}


	int GetS(char *b, int n) {
		if (b && myP < myE) {
			n = min(n, myE - myP);
			memcpy(b, myP, n);
			myP+=n;
			return n;
		} else 
			return 0;
	}

	CStr GetS()
		{CStr tmp(myP,myE-myP); myP = myE; return tmp;}

	int GetLineNum();

	void PutS(const char *s)
		{assert(0);}

	void PutS(const char *s, int len)
		{assert(0);}

	void PutS(const CStr &s)
		{assert(0);}

	void PutC(char c)
		{assert(0);}
};

class qStrFileI : public qStr {
	FILE *myFile;
	bool myFree;

public:
	qStrFileI() {
		myFile = NULL;
	}

	~qStrFileI() {
		if (myFree && myFile)
			fclose(myFile);
	}

	qStrFileI(FILE *fp, bool free = false) {
		SetFile(fp, free);
	}

	void SetFile(FILE *fp, bool free = false) {
		assert(fp!=NULL); 
		myFile = fp; 
		myFree = free;
	}

	char GetC() {
		assert(myFile != NULL);
		return fgetc(myFile);
	}

	bool UngetC(char c) {
		return EOF != ungetc(c, myFile);
	}

	CStr GetS() {
		assert(myFile != NULL);
		CStr tmp(1024); 
		tmp.Grow(fread(tmp.GetBuffer(), 1, 1024, myFile)); 

		return tmp;
	}

	int GetS(char *b, int n) {
		return fread(b, 1, n, myFile);
	}

	time_t GetLastModified() {
		assert(myFile != NULL);
		return GetFileModified(myFile);
	}


	int GetLineNum();
	
	void PutS(const char *s) {assert(false);};
	void PutC(char c) {assert(false);};
	void PutS(const char *s, int len) {assert(false);};
};

class qStrFileO : public qStr {
	FILE *myFile;
	bool myFree;

public:
	qStrFileO(FILE *fp, bool free = false)
		{myFile = fp; myFree = free;}
	~qStrFileO()
		{if (myFree && myFile) fclose(myFile);}

	int GetS(char *b, int n) {assert(false); return 0;}

	void PutS(const char *s)
		{if (s) fputs(s, myFile);}
	void PutS(const char *s, int n)
		{if (s) fwrite(s, 1, n, myFile);}
	void PutS(const CStr &s)
		{if (s) PutS(s, s.Length());}
	void PutC(char c)
		{fputc(c, myFile);}
};

class qStrNLTrim : public qStr {
	qStr *myStr;
	bool myNL;

public:
	qStrNLTrim(qStr *str)
		{myStr = str; myNL = true;}

	void PutS(const char *s);

	void PutS(const char *s, int n);

	void PutC(char c)
		{PutS(&c, 1);}
};

class qStrNull : public qStr {
	virtual void PutS(const char *s, int len)  {}
};


#endif //#ifndef _QSTR_H
