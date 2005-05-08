/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
#ifdef _MT
	#define TLS __declspec(thread)
#else
	#define TLS 
#endif

void setarg(int *argc, char ** argv);
void getarg(int *argc, char ** *argv);
int getargc();
char *optcmp(const char *name, const char *p);
int argdex(int index);
char *getarg(int index, const char *def = NULL);
char *getopt(const char **names, const char *def = NULL);
char *getopt(const char *name, const char *def = NULL, const char *neg = NULL);
char *getopts(const char *name, ...);
char *getopts(va_list list);
int getoptbool(const char *name, int def = 0);
