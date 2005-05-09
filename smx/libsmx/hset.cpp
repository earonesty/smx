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
#include "qctx.h"
#include "qobj-ctx.h"
#include "hset.h"

/////////////////// %hset/%hget //////////////////

#define H_KEYS   1
#define H_VALUES 2


void qObjHCtx::Cleanup(bool aborted) {
        myHash.Close();
#ifdef WIN32
        if (myTemp) {
                remove(myHash.GetPath());
        }
#endif
        if (myParent && !aborted) {
                myParent->DelObj("hset");
                myParent->DelObj("hget");
                myParent->DelObj("hdel");
                myParent->DelObj("hexists");
                myParent->DelObj("henumvalues");
                myParent->DelObj("henumkeys");
                myParent->DelObj("henumtree");
        }
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
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		args->SetAt(2, "V");
		((qObjHCtx*)data)->HEnum(ctx, out, args);
	}
}

void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		args->SetAt(2, "K");
		((qObjHCtx*)data)->HEnum(ctx, out, args);
	}
}

void EvalHEnumTree(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		args->SetAt(2, "T");
		((qObjHCtx*)data)->HEnum(ctx, out, args);
	}
}

void LoadHSet(qCtx *ctx) {
//core
    CStr tmpName;

    tmpName = getenv("SMXHOME");
    if (tmpName.IsEmpty()) {
#ifdef WIN32
	tmpName = getenv("TEMP");
	if (tmpName.IsEmpty()) {
		tmpName = getenv("HOMEDRIVE");
		if (!tmpName.IsEmpty()) {
			tmpName += getenv("HOMEPATH");
			tmpName.RTrim('/'); tmpName.RTrim('\\');
		} else {
			tmpName = ".";
		}
	}
	tmpName = tmpName + "\\.smx";
	CreateDirectory(tmpName,NULL);
	tmpName = tmpName + "\\hset.db";
#else
        tmpName = getenv("HOME");
	if (tmpName.IsEmpty()) 
		tmpName = "/tmp";
        mkdir(tmpName,0750);
        tmpName = tmpName + "/.smx";
        mkdir(tmpName,0750);
        tmpName = tmpName + "/hset.db";
#endif
    } else {
#ifdef WIN32
	tmpName = tmpName + "\\hset.db";
#else
	tmpName = tmpName + "/hset.db";
#endif
    }

#ifdef WIN32
    remove(tmpName);
#endif

        qObjHCtx *hCtx = new qObjHCtx(ctx);

        hCtx->SetPath(tmpName.GetBuffer(), true);

        ctx->MapObj(hCtx,(QOBJMETH) &qObjHCtx::HSet,  "hset");
        ctx->MapObj(hCtx,(QOBJMETH) &qObjHCtx::HSet,  "hdel");
        ctx->MapObj(hCtx,(QOBJMETH) &qObjHCtx::HGet,  "hget");
        ctx->MapObj(hCtx,(QOBJMETH) &qObjHCtx::HExists,  "hexists");
        ctx->MapObj(hCtx,(QOBJMETH) &qObjHCtx::HFile,  "hdbfile");

        ctx->MapObj(hCtx,EvalHEnumValues,                      "henumvalues");
        ctx->MapObj(hCtx,EvalHEnumKeys,                        "henumkeys");
        ctx->MapObj(hCtx,EvalHEnumTree,                        "henumtree");
        ctx->MapObj(hCtx, "<hctx>");
}
