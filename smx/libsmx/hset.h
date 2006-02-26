/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalHEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumTree(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QHSET_
