#ifndef _QARG_H
#define _QARG_H

class qObj;
class qCtx;
class qArgAry;

typedef int (qObj::*QOBJMETH)(qCtx *,qStr *,qArgAry *);
typedef void (*QOBJFUNC) (const void *data, qCtx *ctx, qStr *out, qArgAry *args);

class qArg {
	bool myQuoted : 1;

	enum {
		tStr,
		tInt,
		tDbl,
		tObj
	} myType   : 3;

	union {
		void  *myStr;
		int    myInt;
		double myDbl;
		qObj  *myObj;
	};

public:
	static qArg Null;

	qArg &operator =(const qArg& newArg) {
		if (myType != newArg.myType) {
			Free();
			myType = newArg.myType;
		}
		switch(myType) {
		case tStr: myStr = newArg.myStr; break;
		case tInt: myInt = newArg.myInt; break;
		case tDbl: myDbl = newArg.myDbl; break;
		case tObj: myObj = newArg.myObj; break;
		}
	}

	qArg &operator =(const char *newStr) {
		if (myType != tStr) {
			Free();
			myType = tStr;
		}
		((CStr&)myStr) = newStr;
	}

	qArg &operator =(int newInt) {
		if (myType != tInt) {
			Free();
			myType = tInt;
		}
		myInt = newInt;
	}

	qArg &operator =(double newDbl) {
		if (myType != tDbl) {
			Free();
			myType = tDbl;
		}
		myDbl = newDbl;
	}

	void Free() {
		if (myType == tStr)
			((CStr&)myStr).Free();
	}

	qObj * GetObj() const   { if (myType == tObj) return myObj; else return NULL; }
	int    GetInt()	const   { if (myType == tInt) return myInt; else if (myType == tStr) return atoi(((CStr&)myStr)); else if (myType == tDbl) return myDbl; else return 0; }
	double GetDbl()			{ if (myType == tDbl) return myDbl; else if (myType == tStr) return atoi(((CStr&)myStr)); else if (myType == tInt) return myInt; else return 0; }
	CStr   GetStr()	        { if (myType == tInt) return CStr() << myInt; else if (myType == tStr) return ((CStr&)myStr); else if (myType == tDbl) return CStr() << tDbl; else return 0; }

	operator CStr ()		{ return GetStr(); }
	operator int ()		{ return GetInt(); }
	operator double ()	{ return GetDbl(); }
	operator qObj * ()	{ return GetObj(); }
};

#define CBufZ CBufChainZ
class qArgAry : public CBufZ<qArg> {
public:
	qArgAry(int n = 0) \
		{ m_buf = 0; Grow(n); }
	qArgAry(qArg *d, int n) \
		{ m_buf = 0; Grow(n); memcpy(m_buf, d, n * sizeof(qArg)); }

	qArg *Grow(int n) \
		{int o = Count(); CBufZ<qArg>::Grow(n); \
				if (Count() < o) \
				{int i; for (i = Count(); i < o; ++i) Data()[i].Free();}\
		return Data();}

	~qArgAry() \
		{int i; for (i = 0; i < (Alloc() / (int) sizeof(qArg)); ++i) Data()[i].Free();}

	int Count() const \
		{if (this) return CBufZ<qArg>::Count(); else return 0;}
	qArg &GetAt(int i) \
		{if (i < Count()) return Data()[i]; else return qArg::Empty;}
	qArg &SetAt(int i, const char *str) \
		{if (i > Count()) Grow(i); return Data()[i] = str;}
	qArg &Add(qArg str) \
		{Grow(Count()+1); return Data()[Count()-1] = str;}

	operator char **() \
		{return (char **) Data();}
	operator const char **() \
		{return (const char **) Data();}
	qArg &operator [](int index) \
		{return GetAt(index);}


	qArgAry &operator =(const qArgAry &newBuf) {
		int i; 
		for (i = 0; i < (Alloc() / (int) sizeof(qArg)); ++i) 
			Data()[i].Free();
		if (&newBuf && newBuf.Data()) {
			i = newBuf.Count();
			Grow(i);
			for (i = 0; i < Count(); ++i) 
				Data()[i] = newBuf.Data()[i];
		}
		return *this;
	}


	qArgAry &operator =(const char **newBuf) {
		int i; 
		for (i = 0; i < (Alloc() / (int) sizeof(qArg)); ++i) 
			Data()[i].Free();
		
		if (newBuf) {
			const char **t = newBuf;
			while (*t) ++t;
			Grow(t-newBuf);
			for (t = newBuf; *t; ++t) 
				Data()[i] = *t;
		}
		return *this;
	}
};

#endif //#ifndef _QARG_H