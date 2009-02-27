/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "smxext.h"

#include <dlfcn.h>

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* declare thread-safe storage */

#define XS_VERSION

#define MY_CXT_KEY "SmxExt::_guts" XS_VERSION

typedef struct {
	smxExStreamOut *pOut;
	smxExContext *pCtx;
} my_cxt_t;

START_MY_CXT

EXTERN_C void xs_init (pTHX);

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);

EXTERN_C void
xs_init(pTHX)
{
        char *file = __FILE__;
        dXSUB_SYS;

#ifdef unix
	dlopen("libperl.so", RTLD_LAZY|RTLD_GLOBAL);
#endif
        /* DynaLoader is a special case */
        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

/* Outputs stuff to SMX from within Perl */

XS(XS_SmxPerl_output); /* prototype to pass -Wmissing-prototypes */
XS(XS_SmxPerl_output)
{
    dXSARGS;
    if (items != 1)
        Perl_croak(aTHX_ "Usage: SmxExt::output(str)");
    {
	dMY_CXT;
        char *  str = (char *)SvPV_nolen(ST(0));
	MY_CXT.pOut->PutS(str);
    }
    XSRETURN_EMPTY;
}


/* Calls the named perl sub from within SMX */
DECL_SMXUSERFUNC(call_perl_func) {
	dMY_CXT;
	dSP;
	int cnt = call_argv((const char *) pObject, G_SCALAR, (char **) pArgs);
	SPAGAIN;
	while (cnt--) {
		SV * sv = POPs;
		if (sv) pOutput->PutS(SvPV_nolen(sv));
	}
}

/* Exports a perl sub to SMX */

XS(XS_SmxPerl_export); /* prototype to pass -Wmissing-prototypes */
XS(XS_SmxPerl_export)
{
    dXSARGS;
    if (items != 2 && items != 1)
        Perl_croak(aTHX_ "Usage: SmxExt::export(name_of_function)");
    {
        dMY_CXT;
        char * name = SvPV_nolen(ST(0));
	if (name && *name) {
		if (*name == '$') {
			++name;
			SV *val = get_sv(name, 0);
			if (val) {
				MY_CXT.pCtx->MapString(SvPV_nolen(val), name);
			}
		} else {
			if (*name == '&') ++name;
			char * cp_name = (char *) MY_CXT.pCtx->Alloc(strlen(name)+1);
			strcpy(cp_name, name);
			MY_CXT.pCtx->MapFunc(cp_name, (SMXUSERFUNC) &call_perl_func, name);
		}
	}
    }
    XSRETURN_EMPTY;
}

/* Evaluates some perl code */

DECL_SMXUSERFUNC(perl) {
	if (nNumArgs>=1) {
		dMY_CXT;
		MY_CXT.pOut = pOutput;
		MY_CXT.pCtx = pContext;
		SV *val = eval_pv(pArgs[0], TRUE);
	}
}

/* Startup perl */

static PerlInterpreter *my_perl;

STDCALL void LoadLib(smxExContext *pContext) {
	PERL_SYS_INIT3(NULL, NULL, NULL);

	my_perl = perl_alloc();
	perl_construct(my_perl);
	char *embedding[] = { "", "-e", "0" };
	perl_parse(my_perl, xs_init, 3, embedding, NULL);	
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	{ MY_CXT_INIT; }

    	char *file = __FILE__;
   	newXS("main::output", XS_SmxPerl_output, file);
   	newXS("main::export", XS_SmxPerl_export, file);

	pContext->MapFunc(NULL, (SMXUSERFUNC) &perl, "perl");	
};

STDCALL void TermLib() {
	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();
};

SMXLIB_EXPORT SMXEXTLIB SMXLibrary = {
	sizeof(SMXEXTLIB),
	&LoadLib,
	&TermLib,
};
