/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"
#include "util.h"

#if defined(HAVE_LIBTDB) || defined(HAVE_SQLITE3) || defined(HAVE_DB_H)

#include "qctx.h"
#include "qobj-ctx.h"
#include "hset.h"

/////////////////// %hset/%hget //////////////////

#define H_KEYS   1
#define H_VALUES 2


void LoadHSet(qCtx *ctx, const char *tmpName);

void qObjHCtx::Cleanup(bool aborted) {
        myHash.Close();
#ifdef WIN32
        if (myTemp) {
                remove(myHash.GetPath());
        }
#endif
}

void qObjHCtx::HGet(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1 && (*args)[0]){
		CStr val;
		if (val = myHash.Get((*args)[0])) {
			out->PutS(val);
		}
	}
}

void qObjHCtx::HFile(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() == 1) {
		if (ctx->GetSafeMode()) {
			ctx->ThrowF(out, 909, "Permission denied");
			return;
		}
		myHash.SetPath((*args)[0]);
	} else if (args->Count() == 0) {
		out->PutS(myHash.GetPath());
	}
}

void qObjHCtx::HExists(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1){
		CStr var   = (*args)[0];
		if (var && myHash.Exists(var)) {
			out->PutC('T');
		}
	}
}

void qObjHCtx::HSet(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1 && (*args)[0]) {
		if (args->Count() >= 2 && (*args)[1]) {
			myHash.Set((*args)[0], (*args)[1]);
		} else {
			myHash.Del((*args)[0]);
		}
	}
}

void qObjHCtx::HDel(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1 && (*args)[0]) {
		myHash.Del((*args)[0]);
	}
}

void qObjHCtx::Eval(qCtx *ctx, qStr *out, qArgAry *args)
{
	out->PutS(myHash.GetPath());
}

void qObjHCtx::Counter(qCtx *ctx, qStr *out, qArgAry *args)
{
        if (args->Count() >= 1) {
                int val;
                CStr var = (*args)[0];
                if (!var.IsEmpty()) {
                        var  = "/counters/" << var;
/*
      			HTRANS trans = NULL;
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
*/
                        char tmp[32]; _itoa(val, tmp, 10);
                        out->PutS(tmp);
                }
        }
}

qObjHCtx *GetHCtx(qCtx *ctx, const void *data)
{
        qObjHCtx *hCtx=NULL;
	const char *name=(const char *)data;
	if (ctx->GetSafeMode()) {
        	if ( ctx->GetEnv() && ctx->GetEnv()->GetSessionCtx() )
                	ctx=ctx->GetEnv()->GetSessionCtx();
        	ctx->FindL((qObj **)&hCtx, name);
	} else {
        	ctx->Find((qObj **)&hCtx, name);
	}
	return hCtx;
}


void EvalHFile(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        qObjHCtx *hCtx;
        if ( ctx->GetEnv() && ctx->GetEnv()->GetSessionCtx() )
        	ctx=ctx->GetEnv()->GetSessionCtx();

        if (args->Count() == 1) {
                if (!safe_fcheck(ctx, (*args)[0])) {
                        ctx->ThrowF(out, 909, "Permission denied");
                        return;
                }
		hCtx = new qObjHCtx(ctx);
       		hCtx->SetPath((*args)[0], true);
		ctx->MapObj(hCtx, (const char *) data);
        } else if (args->Count() == 0) {
        	if (hCtx=GetHCtx(ctx, data)) 
			hCtx->Eval(ctx, out, args);
        }
}

void EvalHGet(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        qObjHCtx *hCtx;
        if (hCtx=GetHCtx(ctx, data)) 
		hCtx->HGet(ctx, out, args);
}
void EvalHExists(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        qObjHCtx *hCtx;
        if (hCtx=GetHCtx(ctx, data))
                hCtx->HExists(ctx, out, args);
}
void EvalHDel(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        qObjHCtx *hCtx;
        if (hCtx=GetHCtx(ctx, data))
                hCtx->HDel(ctx, out, args);
}
void EvalHSet(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        qObjHCtx *hCtx;
        if (hCtx=GetHCtx(ctx, data))
                hCtx->HSet(ctx, out, args);
}
void EvalCounter(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        qObjHCtx *hCtx;
        if (hCtx=GetHCtx(ctx, data)) 
		hCtx->Counter(ctx, out, args);
}


struct LOOPCTX {
	qCtx *ctx;
	qStr *out;
	CStr body;
	const char *key;
	const char *val;
	int n;
};

static bool HEnumLoop(void *obj, char *key, int klen, char *val, int vlen) {
	LOOPCTX * loop = (LOOPCTX *) obj;

	key[klen] = '\0';
	val[vlen] = '\0';

	loop->key = key;
	loop->val = val;

	loop->ctx->Parse(loop->body, loop->out); 
	return true;
}

void qObjHCtx::HEnum(qCtx *ctx, qStr *out, qArgAry *args)
{
// require body argument
	if (args->Count() < 1)
		return;

// get context
	CStr var   = (*args)[0];

// read filter
	int filter = 0;

	if (args->Count() > 2) {
		CStr tmp = (*args)[2];
		strlwr(tmp.GetBuffer());
		if (strchr((const char*)tmp, 'v'))
			filter |= HENUM_VALUES;
		if (strchr((const char*)tmp, 'k'))
			filter |= HENUM_KEYS; 
		if (strchr((const char*)tmp, 't'))
			filter |= HENUM_TREE; 
	} else 
		filter = HENUM_KEYS | HENUM_VALUES;

// loop through objects in my map

	qCtxTmp tmpCtx(ctx);

	LOOPCTX loop;

	loop.body = args->GetAt(1);
	loop.n = 0;
	loop.ctx = &tmpCtx;
	loop.out = out;
	
	loop.ctx->MapObj(&loop.key, "key");
	loop.ctx->MapObj(&loop.val, "value");
	
	ctx->MapObj(&loop.n,  "num");

	myHash.Enum(&loop, var, filter, HEnumLoop);

	return;
}

void EvalHEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
        qObjHCtx *hCtx;
	if (args->Count() >= 1) {
        	if (hCtx=GetHCtx(ctx, data)) {
			CStr var   = (*args)[0];
			args->SetAt(2, "V");
			hCtx->HEnum(ctx, out, args);
		}
	}
}

void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
        qObjHCtx *hCtx;
        if (args->Count() >= 1) {
        	if (hCtx=GetHCtx(ctx, data)) {
			CStr var   = (*args)[0];
			args->SetAt(2, "K");
			hCtx->HEnum(ctx, out, args);
		}
	}
}

void EvalHEnumTree(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
        qObjHCtx *hCtx;
        if (args->Count() >= 1) {
        	if (hCtx=GetHCtx(ctx, data)) {
			CStr var   = (*args)[0];
			args->SetAt(2, "T");
			hCtx->HEnum(ctx, out, args);
		}
	}
}

CStr GetHomePath() {
	CStr tmpName;
#ifdef WIN32
        tmpName = getenv("HOMEDRIVE");
        if (!tmpName.IsEmpty()) {
                tmpName += getenv("HOMEPATH");
                tmpName.RTrim('/');
		tmpName.RTrim('\\');
        } else {
                tmpName = ".";
        }
#else
        tmpName = getenv("HOME");
#endif
	return tmpName;
}

CStr GetTempPath() {
	CStr tmpName;
#ifdef WIN32
        tmpName = getenv("TEMP");
#else
        tmpName = "/tmp";
#endif
	return tmpName;
}


void LoadHSet(qCtx *ctx) {
    CStr tmpName;

    tmpName = getenv("SMXHOME");

    if (tmpName.IsEmpty()) {
	tmpName = GetHomePath();
    	if (tmpName.IsEmpty())
		tmpName = GetTempPath();
	tmpName = tmpName << DIRSEP << ".smx";
    }
    tmpName = tmpName << DIRSEP << "hset.db";

#ifdef WIN32
    remove(tmpName);
#endif

    qObjHCtx *hCtx = new qObjHCtx(ctx);

    hCtx->SetPath(tmpName, true);

    ctx->MapObj(hCtx, "<hctx>");

    ctx->MapObj("<hctx>", EvalHSet,  "hset");
    ctx->MapObj("<hctx>", EvalHSet,  "hdel");
    ctx->MapObj("<hctx>", EvalHGet,  "hget");
    ctx->MapObj("<hctx>", EvalHExists,  "hexists");
    ctx->MapObj("<hctx>", EvalHFile,  "hdbfile");

    ctx->MapObj("<hctx>", EvalHEnumValues,                      "henumvalues");
    ctx->MapObj("<hctx>", EvalHEnumKeys,                        "henumkeys");
    ctx->MapObj("<hctx>", EvalHEnumTree,                        "henumtree");
}


void LoadPSet(qCtx *ctx) {
    CStr tmpName;

    tmpName = getenv("SMXHOME");

    if (tmpName.IsEmpty()) {
        tmpName = GetHomePath();
        if (tmpName.IsEmpty())
                tmpName = ".";
        tmpName = tmpName << DIRSEP << ".smx";
    }
    tmpName = tmpName << DIRSEP << "pset.db";

    qObjHCtx *pCtx = new qObjHCtx(ctx);

    pCtx->SetPath(tmpName, false);

    ctx->MapObj(pCtx, "<pctx>");

    ctx->MapObj("<pctx>", EvalHSet,  "pset");
    ctx->MapObj("<pctx>", EvalHSet,  "pdel");
    ctx->MapObj("<pctx>", EvalHGet,  "pget");
    ctx->MapObj("<pctx>", EvalHExists,  "pexists");
    ctx->MapObj("<pctx>", EvalHFile,  "pdbfile");

    ctx->MapObj("<pctx>", EvalHEnumValues,                      "penumvalues");
    ctx->MapObj("<pctx>", EvalHEnumKeys,                        "penumkeys");
    ctx->MapObj("<pctx>", EvalHEnumTree,                        "penumtree");
    ctx->MapObj("<pctx>", EvalCounter,                        "counter");

}


#endif  // HAVE_LIBTDB/SQLITE3
