/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _QCTX_BASED_
#define _QCTX_BASED_

#include "qctx.h"

// context "based on another" (inherits from another)

class qCtxBased : public qCtx 
{
	qCtx *myBase;
public:
	qCtxBased() : qCtx() {myBase = 0;}
	bool FindL(qObj **obj, const char *name) \
		{return ((*obj = myMap.Find(name)) != myMap.Nf) ? true : \
		(myBase ? myBase->FindL(obj, name) : false);}
	void SetBase(qCtx* base) {myBase = base;}
};

class qCtxBasedTS : public qCtxBased {
	CCrit myCrit;
public:
	virtual qObj *MapObj(qObj *obj, char *name) \
		{	CLock lock= myCrit.Enter(); \
			return qCtxBased::MapObj(obj, name);
		}

	virtual qObj *MapObjG(qObj *obj, char *name) \
		{	CLock lock= myCrit.Enter(); \
			return qCtxBased::MapObjG(obj, name);
		}

	virtual bool FindL(qObj **obj, const char *name) \
		{	CLock lock= myCrit.Enter(); \
			return qCtxBased::Find(obj, name);
		}
};

#endif //#ifndef _QCTX_BASED_
