/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "smxext.h"

#include <EXTERN.h>
#include <perl.h>
#include <dlfcn.h>

static PerlInterpreter *my_perl;

EXTERN_C void xs_init (pTHX);

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);

EXTERN_C void
xs_init(pTHX)
{
        char *file = __FILE__;
        dXSUB_SYS;

        /* DynaLoader is a special case */
        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

DECL_SMXUSERFUNC(perl) {
	if (nNumArgs>=1) {
		SV *val = eval_pv(pArgs[0], TRUE);
		pOutput->PutS(SvPV_nolen(val));
	}
}

STDCALL void LoadLib(smxExContext *pContext) {
	dlopen("libperl.so", RTLD_LAZY|RTLD_GLOBAL);
	PERL_SYS_INIT3(NULL, NULL, NULL);
	my_perl = perl_alloc();
	perl_construct(my_perl);
	char *embedding[] = { "", "-e", "0" };
	perl_parse(my_perl, xs_init, 3, embedding, NULL);	
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
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
