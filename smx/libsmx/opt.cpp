/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
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
				while (*pv) {*pv = *(pv++ + 1);} --*pargc; // delarg
				return f;
			} else if (  (!strnicmp(p,"no",2) && *(p+=2))
			   || (!strnicmp(p,"!",1) && *(++p))  ) {
				if (optcmp(name, p)) {
					while (*pv) {*pv = *(pv++ + 1);} --*pargc; // delarg
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
