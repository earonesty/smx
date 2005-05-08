/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
// loads data from the resource fork
#ifndef _RES_H_
#define _RES_H_

long resLoad(long hres, long id, char **pBuf);
long resLoad(long hres, long id, char *buf, int buf_len);
long resInit(char *file);
void resTerm(long handle);

// global resource handle
extern long ghRes;

// resource errors
#define RES_ERRNOTFOUND -1
#define RES_ERRVERSION -2
#define RES_ERRREAD -3

#ifdef RES_INIT
	#undef RES_INIT
#endif

#if defined _WINDOWS || defined WIN32
	#ifdef DLL_NAME
		#define RES_INIT ghRes ? ghRes : (ghRes = resInit(DLL_NAME));
	#else
		#define RES_INIT ghRes ? ghRes : (ghRes = resInit(NULL));
	#endif
#else
	#define RES_INIT ghRes ? ghRes : (ghRes = resInit(argv[0]));
#endif

#if defined _WINDOWS || defined _WIN32
	#ifndef LITTLE_ENDIAN
		#define LITTLE_ENDIAN
	#endif	
#endif

#endif //#ifndef _RES_H_
