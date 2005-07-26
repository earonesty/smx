/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include <stdio.h>
#include <stdarg.h>

#include "qlib.h"
#include "str.h"
#include "map.h"

char *CStr::s_empty = "";
CStr CStr::Null = 0;
CStr CStr::Empty = CStr("", 0);

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
