#include "str.h"

int foo(const CStr &n)
{
	assert(n.Length() > 0);
	return 1;
}

int main()
{

	foo("hello");
	foo("world");

	// format a bunch
	CStr x;
	CStr y;
	int i;
	for (i=0;i<10;++i) {
		x.Format("%i",i);
		y=x;
	}
	
	assert(x[0]='9');

	// should still point to same location even after array/mod
	assert((const char *)x == (const char *) y);

	x << "8";
	x += "7";
	
	// should not point to same location after append
	assert((const char *)x != (const char *) y);

	assert(x[0]='9');
	assert(x[1]='8');
	assert(x[2]='7');

	assert(x.Length() == 3);

	const char * z = x + 1;

	assert(z[0]=='8');
	assert(z[1]=='7');

	CStr w("abc");
	assert(w.Length() == 3);

	printf("OK\n");

	exit(0);
}
