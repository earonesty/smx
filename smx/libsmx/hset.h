/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QHSET_
#define _QHSET_

/////// %hset/%hget object, uses a threadsafe (TS) context

#include "dbpset.h"

class qObjHCtx : public qObjTS
{
protected:

#ifdef WIN32
	bool myTemp;
#endif
	CDBHash myHash;
	qCtx *myParent;

public:
// construct / destruct
	~qObjHCtx() {
		Cleanup();
	}

	virtual void Cleanup(bool aborted = false);
public:
	qObjHCtx(qCtx *parent) {
#ifdef WIN32
		myTemp = false;
#endif
		myParent = parent;
	}

	void SetPath(char *path, bool temp = false) {
#ifdef WIN32
		myTemp = temp;
#endif
		myHash.SetPath(path);
	}

	void HGet(qCtx *ctx, qStr *out, qArgAry *args);
	void HExists(qCtx *ctx, qStr *out, qArgAry *args);
	void HSet(qCtx *ctx, qStr *out, qArgAry *args);
	void HDel(qCtx *ctx, qStr *out, qArgAry *args);
	void HEnum(qCtx *ctx, qStr *out, qArgAry *args);
	void HFile(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalHEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumTree(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QHSET_
