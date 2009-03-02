/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "opt.h"

#ifdef unix
#include "unix.h"
#endif

static char **pargv;
static int *pargc; 

void setarg(int *argc, char ** argv)
{
	pargc = argc;
	pargv = argv;
}

int getargc()
{
	return *pargc;
}

char *delarg(int index)
{
	int i;
	char *s = pargv[index];
	for (i = index; i < *pargc; ++i)
		pargv[i] = pargv[i+1];
	--(*pargc);
	return s;
}

int argdex(int index)
{
	int i = 0;
	while (i < *pargc) {
		if (*pargv[i] != '/' && *pargv[i] != '-') {
			if (!index)
				return i;
			--index;
		}
		++i;
	}
	return i;
}

char *getarg(int index, const char *def)
{
 	int i = argdex(index);
	if (i < *pargc) {
		return delarg(i);
	}
	return (char *) def;
}

char *getopts(va_list list) {
	char *opt = NULL;
	const char *name = *((char **) list - 1);
	while (name && !(opt =getopt(name)))
		name = va_arg(list, const char *);
	return opt;
}

char *getopts(const char *name, ...) {
	va_list list;
	va_start( list, name );
	char *opt = getopts(list);
	va_end( list ); 
	return opt;
}

char *optcmp(const char *name, const char *p){
	int match;

	if (!p) return NULL;
	while(*p && *name && (tolower(*p) == *name)) {++p; ++name;}

	if (*name == '*') {
		++name;
		while(*p && *name && tolower(*p) == *name) {++p; ++name;}
		match = 1;

	}
	else
		match = (*name == '\0');
	while (isspace(*p))  ++p;
	match &= (*p == ':') || (*p == '=') || (*p == '\0');

	if (match)
		return (char*) ((*p == '\0') ? p : p + 1);
	else
		return NULL;
}

char *getopt(const char *name, const char *def, const char *neg){
	char *p, *f; char **pv;
	for (pv = pargv + 1; (p = *pv); ++pv){
		if (*p == '/' || *p == '-') {
			++p;
			if ((f = optcmp(name, p))) {
				while (*pv) {pv[0] = pv[1]; ++pv;} --*pargc; // delarg
				return f;
			} else if (  (!strnicmp(p,"no",2) && *(p+=2))
			   || (!strnicmp(p,"!",1) && *(++p))  ) {
				if (optcmp(name, p)) {
					while (*pv) {pv[0] = pv[1]; ++pv;} --*pargc; // delarg
					return (char *) neg;
				}
			}
		}
	}
	return (char *) def;
}

int getoptbool(const char *name, int def) {
	char *p = getopt(name, "-", NULL);
	if (p) {
		if (*p == 'f' || *p == 'F' || *p == '0')
			return 0;
		else if (*p == '-')
			return def;
		else
			return 1;
	}
	else
		return 0;
}
