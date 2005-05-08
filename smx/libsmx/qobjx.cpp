#include "stdafx.h"
#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

void qStrNLTrim::PutS(const char *s)
{
	char *p;
	while (p = strchr(s, '\n')) {
		if (myNL)
			myStr->PutS(s, p - s);
		else
			myStr->PutS(s, p - s + 1);
		s = p + 1;
		while(isspace(*s)) ++s;
		myNL = *s == 0;
	}
	if (myNL) {
		while(isspace(*s)) ++s;
	}
	if (*s != 0)
		myNL = false;
	myStr->PutS(s);
}

void qStrNLTrim::PutS(const char *s, int n)
{
	char *p;
	char *e = s + n;
	while (p = (char *) memchr(s, '\n', n)) {
		if (myNL)
			myStr->PutS(s, p - s);
		else
			myStr->PutS(s, p - s + 1);
		s = p + 1;
		while(s < e && isspace(*p)) ++s;
		n = e - s;
		myNL = (n == 0);
	}
	if (myNL) {
		while(s < e && isspace(*s)) ++s;
	}
	n = e - s;
	if (n > 0)
		myNL = false;
	myStr->PutS(s, n);
	
}

qArg qArg::Null;