#ifndef _QARG_H
#define _QARG_H

class qObj;
class qCtx;

#include "strary.h"

typedef long qArgOpt;
typedef CStr qArg;

enum qArgOpt
{
	ARG_QUOTED = 1
}

class qArgAry : public CStrAry
{
	qArgOpt *myOpts;
	int      myOptsN;
	enum {
		myGrowBy = 8
	};
public:
	qArgAry() {
		myOpts = 0;
		myOptsN = 0;
	}

	void SetOpt(int n, qArgOpt opt, bool value) {
		if (n > myOptsN) {
			realloc(myOpts, n + myGrowBy);
			memset(myOpts + myOptsN, 0, n + myGrowBy - myOptsN);
			myOptsN = n + myGrowBy;
		}
		if (value) 
			myOpts[n] |= opt;
		else
			myOpts[n] &= ~opt;
	}

	bool GetOpt(int n, qArgOpt opt) {
		return (n < myOptsN) && (myOpts[n] & opt);
	}
};


#endif //#ifndef _QARG_H