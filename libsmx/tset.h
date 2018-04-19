/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QTSET_
#define _QTSET_

/////// %tset/%hget object, uses a threadsafe (TS) context

class qObjTCtx : public qObjTS 
{
protected:
	qCtxTS *myCtx;
	CStr    myStr;

	qObjTCtx * TSet(char *path, CStr &val, qObjTSRef &ref);

// construct / destruct
	~qObjTCtx() {
		if (myCtx)
			myCtx->Free(); 
		myStr.Free();
	}

public:
	qObjTCtx() {myCtx = new qCtxTS();}
	qObjTCtx(CStr &str) {myStr = str; myCtx=NULL;}

// access
	qCtx *GetCtx() {return myCtx;}
	CStr  GetStr() {return myStr;}
	void SetStr(CStr str) {myStr = str;}

// helpers - used internally and also by %pset, %fset  & other hierarchical storage macros
	bool TGet(const char *path, CStr &val);
	bool TExists(const char *path);
	bool TDel(CStr &path);
	qObjTCtx *TSet(char *path, CStr &val);

	qObjTCtx *TFind(const char *path, qObjTSRef &lock);

// override
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

// mapped macros (_ is to resolve gcc problem)
	void _TGet(qCtx *ctx, qStr *out, qArgAry *args);
	void _TExists(qCtx *ctx, qStr *out, qArgAry *args);
	void _TSet(qCtx *ctx, qStr *out, qArgAry *args);
	void TOpen(qCtx *ctx, qStr* out, qArgAry *args);

	int TEnum(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalTEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalTEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QTSET_
