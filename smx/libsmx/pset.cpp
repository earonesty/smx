/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifdef WIN32
// including stdafx before db_cxx breaks on WIN32, VS.NET
#include <db_cxx.h>
#endif

#include "stdafx.h"

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "hset.h"
#include "core.h"

#include "util.h"

class qObjPCtx : public qObjHCtx {
public:

	qObjPCtx::qObjPCtx(qCtx *parent);
	~qObjPCtx();

	bool SetFile(const char *file);
	const char *GetFile();

	void Cleanup(bool aborted=false);

	virtual void Free() {
		qObjTS::Free();

	}

	void PFile(qCtx *ctx, qStr *out, qArgAry *args);
	void Counter(qCtx *ctx, qStr *out, qArgAry *args);
};

void qObjPCtx::Cleanup(bool aborted)
{
                myHash.Close();
#ifdef WIN32
                if (myTemp) {
                        remove(myHash.GetPath());
                }
#endif
                if (myParent && !aborted) {
                        myParent->DelObj("pset");
                        myParent->DelObj("pget");
                        myParent->DelObj("pdel");
                        myParent->DelObj("pexists");
                        myParent->DelObj("penumvalues");
                        myParent->DelObj("counter");
                        myParent->DelObj("penumkeys");
                        myParent->DelObj("penumtree");
                        myParent->DelObj("pdbfile");
                }
}

CStr ChangeExt(CStr fname, const char *ext) {
	CStr tmp = fname;

	if (*ext == '.')
		++ext;

	int le = strlen(ext), lb;

	if ((lb = (tmp.RFindC('.') - tmp.Data()))) {
		tmp.Grow(lb + le + 1);
		memcpy(tmp.Data() + lb + 1, ext, le);
	} else {
		tmp << '.' << ext;
	}

	return tmp;
}

void qObjPCtx::Counter(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1) {
		int val;
		CStr var = (*args)[0];
		if (!var.IsEmpty()) {
      HTRANS trans = NULL;
      
			var  = "/counters/" << var;
/*
  			trans = myHash.BeginTrans();
*/
			try {
				if (args->Count() > 1) {
					val  = ParseInt((*args)[1]);
					switch(val) {
					case 0:
						myHash.Del(var);
						break;
					case -1: {
						CStr was = myHash.Get(var);
						int val;

						if (!was.IsEmpty())
							val = *((int*)was.Data());
						else 
							val = 1;
						break;
							 }
					default:
						myHash.Set(var, (char *) &val, sizeof(val));
					}
				} else {
					CStr was = myHash.Get(var);

					if (!was.IsEmpty())
						val = *((int*)was.Data()) + 1;
					else 
						val = 1;
					myHash.Set(var, (char *) &val, sizeof(val));
				}
			} catch (...) {
				ctx->ThrowF(out, 902, "Unexpected error during counter transaction");
        return;
			}
/*
  			myHash.Commit(trans);
        		return;
*/
			char tmp[32]; _itoa(val, tmp, 10);
			out->PutS(tmp);
		}
	}
}

void qObjPCtx::PFile(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (ctx->GetSafeMode()) {
		ctx->ThrowF(out, 909, "Permission denied");
		return;
	}
	if (args->Count() == 1) {
		SetFile((*args)[0]);
	} else if (args->Count() == 0) {
		out->PutS(myHash.GetPath());
	}
}

bool qObjPCtx::SetFile(const char *file)
{
	return myHash.SetPath(file);
}

const char *qObjPCtx::GetFile()
{
	return myHash.GetPath();
}

qObjPCtx::qObjPCtx(qCtx *parent) : qObjHCtx(parent)
{
}

qObjPCtx::~qObjPCtx()
{
}

void LoadPSet(qCtx *ctx) {
    CStr db_dir;

    db_dir = getenv("SMXHOME");

    if (db_dir.IsEmpty()) {
//  get some standard or environment controlled directory as 'appropriate' to the O/S
#ifdef WIN32
    db_dir = getenv("HOMEDRIVE");
    if (db_dir.IsEmpty())
	db_dir.Grow(GetSystemDirectory(db_dir.Grow(MAX_PATH).Data(), MAX_PATH));
    else {
	db_dir += getenv("HOMEPATH");
	db_dir.RTrim('\\'); db_dir.RTrim('/');
        db_dir = db_dir + "\\.smx";
	CreateDirectory(db_dir,NULL);
    }
#else
//  in linux, it will be home directory/.smx or "/etc" - whichever is available
    db_dir = getenv("HOME");
    if (db_dir.IsEmpty())
	    db_dir = ".";
    db_dir = db_dir + "/.smx";
    mkdir(db_dir, 0755);
#endif
    }

    if (db_dir.IsEmpty()) 
	db_dir = ".";

    qObjPCtx *pCtx = new qObjPCtx(ctx);

#ifdef WIN32
    pCtx->SetFile(db_dir << "\\pset.db");
#else
    pCtx->SetFile(db_dir << "/pset.db");
#endif


    ctx->MapObj(pCtx, (QOBJMETH) &qObjPCtx::HSet,	"pset");
    ctx->MapObj(pCtx, (QOBJMETH) &qObjPCtx::HSet,	"pdel");
    ctx->MapObj(pCtx, (QOBJMETH) &qObjPCtx::HGet,	"pget");
    ctx->MapObj(pCtx, EvalHEnumValues,			"penumvalues");
    ctx->MapObj(pCtx, EvalHEnumKeys,			"penumkeys");
    ctx->MapObj(pCtx, EvalHEnumTree,			"penumtree");

    ctx->MapObj(pCtx, (QOBJMETH) &qObjPCtx::Counter,	"counter");

    ctx->MapObj(pCtx, (QOBJMETH) &qObjPCtx::PFile,	"pdbfile");

    ctx->MapObj(pCtx, "<pset>");
}
