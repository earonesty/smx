/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#include "qctx.h"

#include "util.h"

#ifdef WIN32
	#include <atlconv.h>
	#include "qwebm.h"
#endif

#include "qctx-comp.h"

#include "qfopen.h"

#include "pstime.h"

#include "psxext.h"
#include "psximpl.h"

#include "qobj-cache.h"
#include "qstr-pipe.h"

qObjCache::~qObjCache()
{
	myCtx->Free();
}

qObjTimed *qObjCache::MapObj(qObj *obj, const char *name, qStr *fp, int size)
{
	CLock lock = myCtx->Enter();
	
	qObjTimed *t = new qObjTimed(obj, this, name, size);

	myCtx->MapObj(t, name);

	t->SetTime(fp->GetLastModified());

	return t;
}

qStr *qObjCache::OpenFileFQ(qCtx *ctx, qObjTimed **found, const char *p)
{
 *found = NULL;
	if (p && *p) {
		CStr orig_path = p;
		CStr path = p; 
		path.Trim();
		fix_path(path.GetBuffer());

		qObjTimed *timed;

		CLock lock = myCtx->Enter();

		myCtx->Find((qObj**)&timed, path);

		if (!timed)
			lock.Leave();
		else
			timed->AddRef();

#ifdef WIN32
		if (!strnicmp(path, "http", 4) 
			&& (path[4] == ':' || (path[4] == 's' && path[5] == ':'))
			) {
			return OpenURL(ctx, path);
		} 
		else 
#endif
		{
			FILE *fp = safe_fopen(ctx, path, "rb");
			qStrFileI *fi = NULL;

			try {
				if (fp && timed) {
					time_t is  = GetFileModified(fp);
					time_t was = timed->GetTime();
					if (is != was) {
						timed->Free();
						timed = NULL;
					}
				}
				if (timed) {
					*found = timed;
				}

				if (fp) {
					fi = new qStrFileI(fp,true);
				}


				return fi;
			} catch (...) {
				assert(0);
				fclose(fp);
				return NULL;
			}
		}
	}
	return NULL;
}

CStr qStrCache::OpenFile(qCtx *ctx, CStr &path)
{
	qObjTimed *found;
	qStrRef fin = qObjCache::OpenFile(ctx, &found, path);
	if (fin) {
		CStr rStr;
		if (!found) {
			qStrBuf   out;
			out.qStr::PutS(fin);
			qObjStr *ostr = new qObjStr((CStr)out);
			CLock lock = myCtx->Enter();
			fix_path(path.GetBuffer());
			MapObj(ostr, path, (qStr *) fin, out.Length());
			rStr =  ostr->GetStr();
		} else {
			rStr = ((qObjStr *)found->GetObj())->GetStr();
			found->Free();
		}
		return rStr;
	} else
		return CStr();
}

CStr qStrCache::OpenFileFQ(qCtx *ctx, const char *p)
{
	qObjTSRef ref = GetRef();

	qObjTimed *found;
	CStr path = p;
	fix_path(path.GetBuffer());
	qStrRef fin = qObjCache::OpenFileFQ(ctx, &found, path);
	if (fin) {
		CStr rStr;
		if (!found || !found->GetObj()) {
			qStrBuf   out;
			out.qStr::PutS(fin);
			CLock lock = myCtx->Enter();
			qObjStr *ostr = new qObjStr((CStr)out);
			rStr = ostr->GetStr();
			MapObj(ostr, path, fin, out.Length());
		} else {
			rStr = ((qObjStr *)found->GetObj())->GetStr();
			found->Free();
		}
		return rStr;
	} else
		return 0;

}

qStr *qObjCache::OpenFile(qCtx *ctx, qObjTimed **found, CStr &path)
{
	*found = NULL;
	if (!path.IsEmpty()) {
		ResolvePath(ctx, path);
		return OpenFileFQ(ctx, found, path);
	}
	return NULL;

}

/*
void qStrCache::EvalInclude(qCtx *ctx, qStr *out, qArgAry *args)
{
	qObjTSRef ref = GetRef();
	CStr buf = OpenFile(ctx, (*args)[0]);
	out->PutS(buf);
}
*/

class qObjModule : public qObjClass
{
#ifdef WIN32 
	HMODULE myHMod;
#else
//	!!!!! TODO: SUPPORT SHARED OBJECTS ON YOUR PLATFORM (JUST THE BASICS) !!!!!
#endif

public:
// override virtuals
	qObjModule(HMODULE hM) {
#ifdef WIN32
		myHMod = hM;
#endif
	}
   ~qObjModule() {
#ifdef WIN32 
		PPSXEXTLIB psx = (PPSXEXTLIB) GetProcAddress(myHMod, "PSXLibrary");
		if(psx)
			psx->pTermLib();
		FreeLibrary(myHMod);
#else
//	!!!!! TODO: FREE YOUR SHARED OBJECTS !!!!!
#endif
	}
};

void qObjCache::EvalModule(qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr path = (*args)[0];
	if (path.IsEmpty())
		return;

	qObjTSRef ref = GetRef();
	qObjClass *module = 0;
	qObjTimed *found = 0;
	qStrRef fin;

	if (! (fin = OpenFile(ctx, &found, path)) ) {
		ctx->Throw(out, 552, "Module file could not be found/opened.");
		return;
	}

	if (found) {
		module = (qObjClass*)found->GetObj();
		module->AddRef();
	}

	if (!module) {
		if (!stricmp(path.GetBuffer() + path.Length() - strlen(SHARED_LIB_EXT), SHARED_LIB_EXT)) {
			PPSXEXTLIB psx;

	#ifdef WIN32
			HMODULE hM = (HMODULE) LoadLibrary(path);

			if (!hM) {
				ctx->ThrowF(out, 550, "Load library failure. %y", GetLastError());
				return;
			}

			psx = (PPSXEXTLIB) GetProcAddress(hM, "PSXLibrary");

			if (!psx) {
				ctx->ThrowF(out, 551, "PSXLibrary not found in module.");
				FreeLibrary(hM);
				return;
			}

	#else
			//!!!!
			void * hM;
	#endif

			if (psx->dwSize != sizeof(*psx) || !psx->pLoadLib) {
				ctx->Throw(out, 552, "PSXLibrary incompatible version.");
	#ifdef WIN32
				FreeLibrary(hM);
	#else
				//!!!!
	#endif
				return;
			}

			module = new qObjModule(hM);
			module->GetCtx()->SetParent(ctx);
			psxExContextImpl extCtx(module->GetCtx(), false);
			psx->pLoadLib(&extCtx);
			module->GetCtx()->SetParent(NULL);

		} else { //! SHARED_LIB_EXT
			qStrNull  null;
			module = new qObjClass();
			// todo: ??move the session context pointer to the context class instead of the environment??
			module->GetCtx()->SetParent(ctx);
			qCtx *saveCtx = module->GetCtx()->GetEnv()->GetSessionCtx();
			module->GetCtx()->GetEnv()->SetSessionCtx(module->GetCtx());

			qCtxEx exception(0);

			try {
				char c = ((qStr *)fin)->GetC();
				((qStr *)fin)->UngetC(c);
				if (c == CMAGIC_V1[0]) {
					module->GetCtx()->ParseMagic(fin,&null);
				} else {
					module->GetCtx()->Parse(fin,&null);
				}
			} catch (qCtxEx ex) {
				exception = ex;
				exception.SetLineNum(((qStr *)fin)->GetLineNum());
			}

			module->GetCtx()->GetEnv()->SetSessionCtx(saveCtx);

			qCtx *resetCtx = module->GetCtx();
			
			while (resetCtx->GetParent() && resetCtx->GetParent() != ctx) {
				resetCtx = resetCtx->GetParent();
			}

			if (resetCtx->GetParent() == ctx) {
				resetCtx->SetParent(NULL);
			} else {
				// assert(0);
			}

			fix_path(path.GetBuffer());

			if (exception.GetID())
				throw exception;
		}

		MapObj(module, path, fin, 1024);
	}

	if (module) {
		CStr namesp = (*args)[1];

		if (!namesp.IsEmpty()) {
			qObjCtxP *ref = new qObjCtxP(module->GetCtx());
			ctx->MapObj(ref, namesp);
		} else {
	 		CStr mname = '<'; mname << path << '>';
			qObjClass *copy = new qObjClass(module);
			copy->Chain(ctx);
			ctx->MapObj(copy, mname);
		}

		if (found) {
			module->Free();
		}
	}

	if (found) {
		found->Free();
	}
}


#ifdef WIN32

#include <crtdbg.h>
#include <stddef.h>
#include <ole2.h>
#include <atlconv.h>

static bool  sInitOLE = false;
static CCrit sCrit;

HRESULT CreateObject(LPSTR pszProgID, IDispatch ** ppdisp);
HRESULT Invoke(LPDISPATCH pdisp, WORD wFlags, LPVARIANT pvRet, EXCEPINFO * pexcepinfo, UINT * pnArgErr, LPSTR pszName, qCtx *ctx, qStr *out, qArgAry *args);

class qObjActiveX : public qObj
{
	IDispatch *myDisp;

public:
// override virtuals
	qObjActiveX(IDispatch * pDisp) {
		myDisp = pDisp;
	}
   ~qObjActiveX() {
		myDisp->Release();
	}

    virtual void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		USES_CONVERSION;

		VARIANT ret;
		VariantInit(&ret);
		UINT nArgErr;
		EXCEPINFO ex;
		
		if (!FAILED(Invoke(myDisp, DISPATCH_METHOD, &ret, &ex, &nArgErr, (*args)[0].GetBuffer(), ctx, out, args))) {
			VARIANT str;
			VariantInit(&str);
			VariantChangeType(&str, &ret, 0, VT_BSTR);
			out->PutS(W2CA(str.bstrVal), SysStringLen(str.bstrVal));
			VariantClear(&str);
		} else {
			ctx->Throw(out, 559, "method failed");
		}


		VariantClear(&ret);
    }
};


void qObjCache::EvalActiveX(qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr name = (*args)[0];

	if (name.IsEmpty())
		return;

	if (!sInitOLE) {
		CLock lock = sCrit.Enter();
		if (!sInitOLE) {
			if(OleInitialize(NULL) != 0)
				return;
			sInitOLE = true;
		}
	}

	qObjTSRef ref = GetRef();

	qObj *module = 0;

	IDispatch *pDisp;

	if (FAILED(CreateObject(name.GetBuffer(), &pDisp))) {
		ctx->Throw(out, 552, "object could not be found/created");
		return;
	}

	CStr namesp = (*args)[1];

	if (!namesp.IsEmpty()) {
		module = new qObjActiveX(pDisp);
		ctx->MapObj(module, namesp);
	} else {
		/*
		CStr mname = '<'; mname << name << '>';
		module->Chain(ctx);
		ctx->MapObj(module, mname);
		*/
	}
}


HRESULT CreateObject(LPSTR pszProgID, IDispatch ** ppdisp)
{
	USES_CONVERSION;

    CLSID clsid;                  // CLSID of ActiveX object.
    HRESULT hr;
    LPUNKNOWN punk = NULL;        // IUnknown of ActiveX object.
    LPDISPATCH pdisp = NULL;      // IDispatch of ActiveX object.

    *ppdisp = NULL;

    // Retrieve CLSID from the ProgID that the user specified.
    hr = CLSIDFromProgID(A2CW(pszProgID), &clsid);
    if (FAILED(hr))
        goto error;

    // Create an instance of the ActiveX object.
    hr = CoCreateInstance(clsid, NULL, CLSCTX_SERVER, 
                            IID_IUnknown, (void **)&punk);
    if (FAILED(hr))
        goto error;

      // Ask the ActiveX object for the IDispatch interface.
    hr = punk->QueryInterface(IID_IDispatch, (void **)&pdisp);
    if (FAILED(hr))
        goto error;

    *ppdisp = pdisp;
    punk->Release();
    return NOERROR;
    
error:
    if (punk) punk->Release();
    if (pdisp) pdisp->Release();
    return hr;
}

HRESULT FAR
Invoke(LPDISPATCH pdisp, WORD wFlags, LPVARIANT pvRet, EXCEPINFO * pexcepinfo, UINT * pnArgErr, LPSTR pszName, qCtx *ctx, qStr *out, qArgAry *args)
{
	USES_CONVERSION;

    DISPID dispid;
    HRESULT hr;
    VARIANTARG* pvarg = NULL;
	ULONG i;

    if (pdisp == NULL)
        return ResultFromScode(E_INVALIDARG);

	OLECHAR * pwName = (OLECHAR *) A2CW(pszName);
    // Get DISPID of property/method.
    hr = pdisp->GetIDsOfNames(IID_NULL, &pwName, 1,
        LOCALE_SYSTEM_DEFAULT, &dispid);
    if(FAILED(hr))
        return hr;

    DISPPARAMS dispparams;
    memset(&dispparams, 0, sizeof dispparams);

    // Determine number of arguments.
    dispparams.cArgs = args->Count() - 1;

    // Property puts have a named argument that represents the value
    // being assigned to the property.
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    if (wFlags & DISPATCH_PROPERTYPUT)
    {
        if (dispparams.cArgs == 0)
            return ResultFromScode(E_INVALIDARG);
        dispparams.cNamedArgs = 1;
        dispparams.rgdispidNamedArgs = &dispidNamed;
    }
    if (dispparams.cArgs != 0)
    {
        // Allocate memory for all VARIANTARG parameters.
        pvarg = new VARIANTARG[dispparams.cArgs];
        if(pvarg == NULL)
            return ResultFromScode(E_OUTOFMEMORY);
        dispparams.rgvarg = pvarg;
        memset(pvarg, 0, sizeof(VARIANTARG) * dispparams.cArgs);

        pvarg += dispparams.cArgs - 1;   // Params go in opposite order.

		CStr arg;
        for(i = 1; i <= dispparams.cArgs; ++i)
        {
			arg = args->GetAt(i);
            V_BSTR(pvarg) = SysAllocStringLen(A2CW(arg), arg.Length());
			if (!V_BSTR(pvarg)) {
				ctx->Throw(out, 552, "error allocating bstr memory");
				goto cleanup;
			}
			pvarg->vt = VT_BSTR;

            --pvarg;
        } //for
    } //if

    // Initialize return variant, in case caller forgot. Caller can pass
    // Null if no return value is expected.
    if (pvRet)
        VariantInit(pvRet);
    // Make the call.
    hr = pdisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, wFlags,
        &dispparams, pvRet, pexcepinfo, pnArgErr);

cleanup:
    // Clean up any arguments that need it.
    if (dispparams.cArgs != 0)
    {
        VARIANTARG * pvarg = dispparams.rgvarg;
        UINT cArgs = dispparams.cArgs;
        while (cArgs--)
        {
            VariantClear(pvarg);
            ++pvarg;
        }
    }
    delete dispparams.rgvarg;
    return hr;
}

#endif



