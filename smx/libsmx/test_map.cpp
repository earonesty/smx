#include "mapstr.h"

int main()
{
	// set 10 map locations, assert they can be looked-up
	CMapStr<CStr> map;
	char tmp[32];
	int i;
	for (i=0;i<10;++i) {
		sprintf(tmp, "%i",i);
		map[tmp] = tmp;
		assert(!strcmp(tmp, (const char *)map[tmp]));
	}
	
	// check iteration
	MAPPOS pos = map.First();

	const char *key = NULL; 
	CStr *val = NULL;

	int ary[10];

	memset(ary, 0, sizeof(ary));

	while(map.Next(&pos, &key, &val)) {
		i = *key-'0';	
		ary[i]=1;
		assert(!strcmp(key, (const char *)*val));
	}

	//assert all 10 slots were found via iteration
	for(i=0;i<10;++i) {
		assert(ary[i]==1);
	}

	printf("OK\n");

	exit(0);
}
