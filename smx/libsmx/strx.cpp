/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#include <stdio.h>
#include <stdarg.h>

#include "qlib.h"
#include "str.h"
#include "map.h"

char *CStr::s_empty = "";

CStr &operator <<(CStr &left, float right)
{
	return operator <<(left, (double) right);
}
CStr &operator <<(CStr &left, double right)
{
	int l = left.Length();
	int r = strlen(_gcvt(right, 10, left.Grow(l + 32) + l));
	left.Grow(l + r);
	return left;
}
CStr operator <<(CStr left, int right)
{
	int l = left.Length();
	int r = strlen(_itoa(right, left.Grow(l + 32) + l, 10));
	left.Grow(l + r);
	return left;
}
CStr &operator <<(CStr &left, long right)
{
	int l = left.Length();
	int r = strlen(_itoa(right, left.Grow(l + 32) + l, 10));
	left.Grow(l + r);
	return left;
}

CStr operator <<(const char *left, const CStr &right)
{
	return CStr(left) << right;
}

CStr CStr::Null = 0;
CStr CStr::Empty = CStr("", 0);

// string conversion helpers
void strcln(const char *p, char *t, char *e)
{
	while (*p && t < e) {
		if (!(*p == '$' || *p == ','))
			*t++ = *p++;
		++p;
	}
}

double strtodx(const char *str)
{
	const char *base, *p; char *endp;
	p = base = str;
	while (*p) {
		if (*p == '$' || *p == ',') {
			char buf[256];
			int len = MIN(128,p - base);
			char *t = buf + len;
			char *e = buf + 256;
			memcpy(buf, base, len);	
			++p;
			strcln(p, t, e);
			return strtod(buf, &endp);
		}
		++p;
	}
	return strtod(base, &endp);
}

int strlenx(const char *b)
{
	const char *p = b;
	while (*p)
		if (*p == '<')
			return p - b;
		else
			++p;
	return p - b;
}

double strtolx(const char *str)
{
	const char *base, *p; char *endp;
	p = base = str;
	while (*p) {
		if (*p == '$' || *p == ',') {
			char buf[128];
			int len = MIN(64,p - base);
			char *t = buf + len;
			char *e = buf + 128;
			memcpy(buf, base, len);	
			++p;
			strcln(p, t, e);
			return strtol(buf, &endp, 0);
		}
		++p;
	}
	return strtol(base, &endp, 0);
}
