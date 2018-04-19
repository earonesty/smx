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
	#include "str.h"
	#include "map.h"
	#include "lst.h"
#endif

	#include "pstime.h"

inline CStr Dbl2Str(double num) {
	CStr tmp(50);
	_gcvt(num, 48, tmp);
	tmp.Shrink();
	return tmp.RTrim('.');
}

class qStr {
public:
	virtual ~qStr() {};

// public instances
	virtual qStr *New(int p1, int p2)	
		{return 0;}

	virtual Delete()					
		{if (this) delete this;}

// reading
	virtual char GetC() 
		{return EOF;}

	virtual CStr GetS() 
		{return NULL;}

// writing
	virtual void PutS(const char *s)				// override for efficiency (if any)
		{if (s) PutS(s, strlen(s));}

	virtual void PutS(const CStr &s)				// highly reccomended
		{if (s) PutS(s, s.Length());}

	virtual void PutC(char c) 						// highly reccomended
		{PutS(&c, 1);}

	virtual void PutS(const char *s, int len)		// required
		{}

	virtual time_t GetLastModified()
		{return -1;}

	virtual void Clear()
		{}

// write helpers

	virtual void PutS(qStr &s)
	    {CStr t; while ((t = s.GetS()).Length() > 0) PutS(t);}

	virtual void PutN(int i)
		{char tmp[33]; _itoa(i, tmp, 10); PutS(tmp); }

	virtual void PutN(double d)
		{PutS(Dbl2Str(d)); }
};

class qStrApp : public qStr {
	CStr *myBuf;
	char *myP;

public:
	qStrApp(CStr &buf)
		{myBuf = &buf; myP = ((char *) *myBuf) + myBuf->Length();}
	qStrApp()
		{myP = *myBuf;};

	char GetC()
		{if (myP < ((char *) *myBuf) + myBuf->Length()) return *myP++; else return -1;}

	CStr GetS()
		{CStr tmp = myP; *myBuf = 0; return tmp;}

	void PutS(const char *s)
		{*myBuf << s;}

	void PutS(const char *s, int len)
		{myBuf->Append(s, len);}

	void PutS(const CStr &s)
		{if (s) PutS(s, s.Length());}

	void PutC(char c)
		{*myBuf << c;}
};

class qStrBuf : public qStr {
	char * myX;
	char * myP;
	char * myE;
	char * myBuf;

public:
	qStrBuf(const CStr &buf) {
		memcpy(myBuf = (char *) malloc(myX = buf.Count()), buf.Data(), buf.Count());
	}

	qStrBuf() {
		myP   = 0; 
		myE   = 0; 
		myX   = 0; 
		myBuf = 0;
	}

	virtual qStr *New()	
		{return new qStrBuf();}

	char GetC()
		{if (myP < myE) return *myP++; else return -1;}

	CStr GetS() 
		{CStr tmp = (myP < myE) ? CStr(myP, myE-myP) : 0; myBuf.Clear(); return tmp;}

	void PutS(const char *s)
		{PutS(s, strlen(s));}

	void PutS(const char *s, int len)
		{myBuf.Append(s, len);}

	void PutS(const CStr &s)
		{if (s) PutS(s, s.Length());}

	void PutC(char c)
		{myBuf << c;}

	void Clear()
		{myBuf.Grow(0);}
};

class qStrFileI : public qStr {
	FILE *myFile;
	bool myFree;

public:
	qStrFileI() {
		myFile = NULL;
	}

	~qStrFileI() {
		if (myFree)
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

	CStr GetS() {
		assert(myFile != NULL);
		CStr tmp(1024); 
		tmp.Grow(fread((char *) tmp, 1, 1024, myFile)); 
		return tmp;
	}

	time_t GetLastModified() {
		assert(myFile != NULL);
		return qTime::GetFileModified(myFile);
	}
	
	void PutS(const char *s) {}
	void PutC(char c) {}
};

class qStrFileO : public qStr {
	FILE *myFile;
	bool myFree;

public:
	qStrFileO(FILE *fp, bool free = false)
		{myFile = fp; myFree = free;}
	~qStrFileO()
		{if (myFree && myFile) fclose(myFile);}

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

inline qStrBuf::qStrBuf(const CStr &buf)
{
	Copy(buf);
	myP = 0;
}

class qStrNull : public qStr {
};


#endif //#ifndef _QSTR_H