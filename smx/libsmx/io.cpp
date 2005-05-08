// Copyright (C) 1999 by Prime Data Corp. All Rights Reserved.
// For information contact erik@primedata.org

#include "stdafx.h"

#ifdef WIN32

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"
#include "util.h"

#include <assert.h>

class qCommIO : public qObj
{
	HANDLE myCom;
public:
	qCommIO() {
		myCom = 0;
	}

	bool Open(qCtx *ctx, qStr *out, CStr port, CStr dcb_def) {
		DCB dcb;
		HANDLE tCom;
		tCom = CreateFile( port,
			GENERIC_READ | GENERIC_WRITE,
			0,    // comm devices must be opened w/exclusive-access 
			NULL, // no security attributes 
			OPEN_EXISTING, // comm devices must use OPEN_EXISTING 
			0,    // not overlapped I/O 
			NULL  // hTemplate must be NULL for comm devices 
			);

		if (tCom == INVALID_HANDLE_VALUE) {
			ctx->ThrowF(out, 601, "Invalid comm port. %y", GetLastError());
			return false;
		}

		GetCommState(tCom, &dcb);

		if (!dcb_def.IsEmpty() && 
				(!BuildCommDCB(dcb_def, &dcb) || !SetCommState(tCom, &dcb))
			) {
			ctx->ThrowF(out, 602, "Invalid comm definition parameter(s). %y", GetLastError());
			return false;
		}

		myCom = tCom;
		return true;
	}

	~qCommIO() {
		if (myCom) {
			CloseHandle(myCom);
		}
	}

	int EvalCommWrite(qCtx *ctx, qStr *out, qArgAry *args) {
		CStr str = (*args)[0];
		DWORD dwb, dwtot = str.Length();
		if (!WriteFile(myCom, str, dwtot, &dwb, 0)) {
			ctx->ThrowF(out, 603, "Write failure : %y", GetLastError());
			return false;
		}
		return 0;
	}

	int EvalCommRead(qCtx *ctx, qStr *out, qArgAry *args) {
		int maxbuf = args->GetAt(0).IsEmpty() ? 1 :  ParseInt((*args)[0]);
		int timeout = (int) (1000.0 * ParseDbl((*args)[1]));
		CStr eol = (*args)[2];
		CStr buf(maxbuf);
		char *p = buf.Data();
		const char *b = p;
		DWORD dwb;
		COMMTIMEOUTS to, to2;
		if (timeout > 0) {
			GetCommTimeouts(myCom, &to);
			to2 = to;
			to2.ReadTotalTimeoutMultiplier = 0;
			to2.ReadTotalTimeoutConstant = timeout;
			BOOL rVal = SetCommTimeouts(myCom, &to2);
		}
		while (maxbuf > 0) {
			if (!ReadFile(myCom, p, maxbuf, &dwb, 0)) {
				ctx->ThrowF(out, 604, "Read failure : %y", GetLastError());
				return false;
			}

			if (eol.IsEmpty() || dwb == 0)
				break;

			if (p-b >= eol.Length())
				if (strstr(b, eol))
					break;
				else
					b = p-eol.Length()+1;

			maxbuf-=dwb;
			if (maxbuf > 0)
				Sleep(1);
		}
		out->PutS(buf, p-buf.Data());
		if (timeout > 0) {
			SetCommTimeouts(myCom, &to);
		}
		return 0;
	}
};

void EvalCommOpen(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr port = (*args)[0];
	CStr dcb_def = (*args)[1];

	qCtxTmp tmpCtx(ctx);
	qCommIO io;
	if (io.Open(ctx, out, port, dcb_def)) {
		tmpCtx.MapObj(&io, (QOBJMETH) &qCommIO::EvalCommWrite,  "comm-write");
		tmpCtx.MapObj(&io, (QOBJMETH) &qCommIO::EvalCommRead,   "comm-read");
		tmpCtx.Parse(args->GetAt(2), out);
	}
}

void LoadIO(qCtx *ctx) {
	ctx->MapObj(EvalCommOpen,  "comm-open");
}

#endif //#ifndef unix
