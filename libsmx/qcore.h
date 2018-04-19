/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _QCORE_
#define _QCORE_

/////// %hset/%hget object, uses a threadsafe (TS) context

class qObjHCtx : public qObj 
{
	qCtxTS myCtx;
	CStr   myStr;
public:
// construct / destruct
	~qObjHCtx() {};
	qObjHCtx() : qObj() {}
	qObjHCtx(CStr &str) : qObj() {myStr = str;}

// access
	qCtx *GetCtx() {return &myCtx;}
	CStr &GetStr() {return  myStr;}

// helpers - used internally and also by %pset, %fset  & other hierarchical storage macros
	bool HGet(char *path, CStr &val);
	bool HDel(CStr &path);
	qObjHCtx *HSet(char *path, CStr &val);
	qObjHCtx *HFind(char *path);

// override
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

// mapped macros
	int HGet(qCtx *ctx, qStr *out, qArgAry *args);
	int HSet(qCtx *ctx, qStr *out, qArgAry *args);
	int HOpen(qCtx *ctx, qStr* out, qArgAry *args);
	int HEnum(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalHEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QCORE_
