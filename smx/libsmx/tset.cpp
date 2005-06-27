// Copyright (C) 1999 by Prime Data Corp. All Rights Reserved.
// For information contact erik@primedata.org

#include "stdafx.h"

#include "qctx.h"
#include "qobj-ctx.h"
#include "tset.h"

/////////////////// %tset/%tget //////////////////

void qObjTCtx::Eval(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (!myStr.IsEmpty()) 
		out->PutS(myStr); 
	else if (args->Count() > 1) {
		ctx->MapObj(this, (QOBJMETH) &qObjTCtx::TEnum, "tenum");
		qCtxBased *tmp = new qCtxBased;
		try {
			tmp->SetParent(ctx);
			tmp->SetBase(myCtx);
			tmp->qCtx::Parse(args->GetAt(1), out); 
		} catch(qCtxEx ex) {
			tmp->Free();
			throw ex;
		} catch (qCtxExAbort ex) {
			tmp->Free();
			throw ex;
		} catch(...) {
		}
		tmp->Free();
	}
}

#define T_KEYS   1
#define T_VALUES 2

int qObjTCtx::TEnum(qCtx *ctx, qStr *out, qArgAry *args)
{
// requite body argument
	if (args->Count() < 1)
		return 1;

// get context
	CStr var   = (*args)[0];

	qObjTSRef ref = GetRef();
	qObjTCtx *found = TFind(var, ref);
	if (!found)
		return 2;

// get map
	CLock lock = found->myCtx->Enter();
	qObjMap *map = found->myCtx->GetMap();
	if ( !map || (map->Count() <= 0 ) )
		return 3;

// read filter
	int filter = 0;

	if (args->Count() > 2) {
		CStr tmp = (*args)[2];
		strlwr(tmp.GetBuffer());
		if (strchr(tmp, 'v'))
			filter |= T_VALUES;
		if (strchr(tmp, 'k'))
			filter |= T_KEYS; 
	} else 
		filter = T_KEYS | T_VALUES;

	qCtxTmp tmpCtx(ctx);
	qObjTCtx *obj;
	MAPPOS pos;
	const char *key;
	int i;

// create new topen macro
	tmpCtx.MapObj(this, (QOBJMETH) &qObjTCtx::TOpen, "topen");

// loop through objects in my map
	for (i = 0, pos = map->First(); map->Next(&pos, &key, (qObj **) &obj) ; ++i) {
		tmpCtx.MapObj(obj, (QOBJMETH) &qObjTCtx::TEnum, "tenum");
		tmpCtx.MapObj(&i,  "num");
		tmpCtx.MapObj(key, "name");
		if (obj->GetStr()) {
			if (filter & T_VALUES) {
				tmpCtx.MapObj(CStr(""),		 "key");
				tmpCtx.MapObj(obj->GetStr(), "value");
			}
		} 
		if (obj->GetCtx() && obj->GetCtx()->GetMap() && obj->GetCtx()->GetMap()->Count() > 0) {
			if (filter & T_KEYS) {
				tmpCtx.MapObj(key,			 "key");
				tmpCtx.MapObj(CStr(""),		 "value");
			}
		}
		tmpCtx.Parse(args->GetAt(1), out); 
	}
	return 0;
}

bool qObjTCtx::TGet(const char *path, CStr &val) 
{
	qStrBuf tmp;
	{
		qObjTSRef ref = GetRef();
		qObj *found = TFind(path, ref);

		if (!found)
			return false;

		found->Eval(myCtx, &tmp, 0);
	}
	val = tmp;
	return true;
}

bool qObjTCtx::TExists(const char *path) 
{
	qObjTSRef ref = GetRef();
	qObj *found = TFind(path, ref);
	if (!found)
		return false;
	else
		return true;
}

bool qObjTCtx::TDel(CStr &path) 
{
	char *last = path ? (char *) path.GetBuffer() : "";
	last = strrchr(path, '/');
	if (last) 
		*last++ = '\0';

	qObjTSRef ref = GetRef();
	qObjTCtx *found;
	if (last) {
		found = (qObjTCtx *) TFind(path, ref);
		last[-1] = '/';
		if (!found)
			return false;
	} else {
		last = path.GetBuffer();
		found = this;
	}
	found->myCtx->DelObj(last);

	return true;
}

void qObjTCtx::_TGet(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1){
		CStr var   = (*args)[0];
		CStr val;
		if (TGet(var, val)) {
			out->PutS(val);
		}
	}
}

void qObjTCtx::_TExists(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1){
		CStr var   = (*args)[0];
		if (TExists(var)) {
			out->PutC('T');
		}
	}
}

qObjTCtx *qObjTCtx::TSet(char *path, CStr &val) 
{
	qObjTSRef ref = GetRef();
	return TSet(path, val, ref);
}

qObjTCtx *qObjTCtx::TSet(char *path, CStr &val, qObjTSRef &ref) 
{
	// make sure i don't die
	ref = GetRef();

	char *b = path ? path : "";
	char *p = b;
	
	while (*p == '/')
		++p;
	while (isspace(*p)) 
		++p;

	while (*p) {
		if (*p == '/') {
			*p = '\0';
			qObjTCtx *sub;

			{
				CLock lock = myCtx->Enter();
				if (!myCtx->FindL((qObj **)&sub, b) || !sub) {
					sub = new qObjTCtx;
					myCtx->MapObj(sub, b);
				}
				ref = sub->GetRef();
			}

			*p = '/';
			while (isspace(*++p));
			if (*p) {
				sub->TSet(p, val, ref);
			}
			return sub;
		}
		++p;
	}

	myCtx->MapObj(new qObjTCtx(val), path);
	return this;
}

void qObjTCtx::_TSet(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		if (args->Count() >= 2) {
			CStr val   = (*args)[1];
			TSet(var.GetBuffer(), val);
		} else {
			TDel(var);
		}
	}
}

qObjTCtx *qObjTCtx::TFind(const char *path, qObjTSRef &ref) 
{
	const char *b = path ? path : "";
	const char *p = b;

	while (*p == '/')
		++p;
	while (isspace(*p)) 
		++p;

	while (*p) {
		if (*p == '/') {
			break;
		}
		++p;
	}

	ref = GetRef();

	if (p > b) {
		qObjTCtx *sub; 

		{
			CLock lock = myCtx->Enter();
			if (!myCtx->FindL((qObj **)&sub, CStr(b, p-b)) || !sub) {
				return NULL;
			}
			ref = sub->GetRef();
		}

		while (*p == '/')
			++p;
		while (isspace(*p)) 
			++p;

		if (*p) {
			return sub->TFind(p, ref);
		} else {
			return sub;
		}
	} else {
		return this;
	}

	return NULL;
}

void qObjTCtx::TOpen(qCtx *ctx, qStr* out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		qObjTSRef ref = GetRef();
		qObj *found = TFind((*args)[0], ref);
		if (!found)
			return;
		found->Eval(ctx, out, args);
	}
}

void EvalTEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		args->SetAt(2, "V");
		((qObjTCtx*)data)->TEnum(ctx, out, args);
	}
}

void EvalTEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		CStr var   = (*args)[0];
		args->SetAt(2, "K");
		((qObjTCtx*)data)->TEnum(ctx, out, args);
	}
}

void LoadTSet(qCtx *ctx) {
//tset
	qObjTCtx *hCtx = new qObjTCtx;

	ctx->MapObj(hCtx,      "hctx");
	ctx->MapObj(hCtx, (QOBJMETH) &(qObjTCtx::_TSet),   "tset");
	ctx->MapObj(hCtx, (QOBJMETH) &(qObjTCtx::_TGet),   "tget");
	ctx->MapObj(hCtx, (QOBJMETH) &(qObjTCtx::_TExists),"texists");
	ctx->MapObj(hCtx, (QOBJMETH) &(qObjTCtx::TOpen),  "topen");

	ctx->MapObj(hCtx, EvalTEnumValues,  "tenumvalues","01");
	ctx->MapObj(hCtx, EvalTEnumKeys,    "tenumkeys","01");

}
