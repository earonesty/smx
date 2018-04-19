/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "smxext.h"

#ifdef unix
	#include <dlfcn.h>
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#define NEED_eval_pv
#define NEED_sv_2pv_flags
#include "ppport.h"

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
        const char *file = __FILE__;
        dXSUB_SYS;

#ifdef unix
	dlopen("libperl.so", RTLD_LAZY|RTLD_GLOBAL);
#endif
        /* DynaLoader is a special case */
        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, (char *) file);
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
	
	char **argv = (char **) MY_CXT.pCtx->Alloc(sizeof(char *)*(nNumArgs+1));
	memcpy(argv, pArgs, sizeof(char *)*(nNumArgs));
	argv[nNumArgs]=NULL;
	
	int cnt = call_argv((const char *) pObject, G_SCALAR, argv);
	SPAGAIN;
	while (cnt--) {
		SV * sv = POPs;
		if (sv) pOutput->PutS(SvPV_nolen(sv));
	}
}

DECL_SMXUSERFUNC(call_get_sv) {
        SV * sv = get_sv((const char *) pObject, 0);
        if (sv) pOutput->PutS(SvPV_nolen(sv));
}

/* Exports a perl sub to SMX */

XS(XS_SmxPerl_export); /* prototype to pass -Wmissing-prototypes */
XS(XS_SmxPerl_export)
{
    dXSARGS;
    if (items != 2 && items != 1)
        Perl_croak(aTHX_ "Usage: SmxExt::export(name_of_function[,alias])");
    {
        dMY_CXT;
        char * name = SvPV_nolen(ST(0));
	if (name && *name) {
                char * cp_name = (char *) MY_CXT.pCtx->Alloc(strlen(name)+1);
		char * cp_alias = cp_name;

                strcpy(cp_name, name);
		if (items == 2) {
        		char * alias = SvPV_nolen(ST(1));
			if (alias && *alias) {
 				cp_alias = (char *) MY_CXT.pCtx->Alloc(strlen(alias)+1);
				strcpy(cp_alias, alias);
			}
		}

		if (*cp_name == '$') {
			++cp_name;
			if (*cp_alias == '$') ++cp_alias;
                        MY_CXT.pCtx->MapFunc(cp_name, (SMXUSERFUNC) &call_get_sv, cp_alias);
		} else {
			if (*cp_name == '&') ++cp_name;
			if (*cp_alias == '&') ++cp_alias;
			MY_CXT.pCtx->MapFunc(cp_name, (SMXUSERFUNC) &call_perl_func, cp_alias);
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

extern char **environ;

STDCALL void LoadLib(smxExContext *pContext) {
	int argc = 0;
	char *argv[1] = {NULL};
	PERL_SYS_INIT3(&argc, (char ***) &argv, &environ);

	my_perl = perl_alloc();
	perl_construct(my_perl);
	const char *embedding[] = { "", "-e", "0" };
	perl_parse(my_perl, xs_init, 3, (char **) embedding, NULL);	
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	{ MY_CXT_INIT; }

    	const char *file = __FILE__;
   	newXS("main::output", XS_SmxPerl_output, (char *) file);
   	newXS("main::export", XS_SmxPerl_export, (char *) file);

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
