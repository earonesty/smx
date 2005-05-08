/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#include "stdafx.h"
#include "qstr.h"

#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/io.h>
#endif

int fcmppos(fpos_t a, fpos_t b)
{
  #ifdef WIN32
    return (int)(a-b);
  #endif
  
  #ifdef linux
    return (int) (a.__pos - b.__pos);
  #endif
}

int qStrFileI::GetLineNum() 
{
// painfully derive line number

	fpos_t sav;
	fgetpos(myFile, &sav);
	fseek(myFile, 0, SEEK_SET);
  
	fpos_t p;
	fgetpos(myFile, &p);

	char c;
	int lc = 1;
	while (fcmppos(p,sav) < 0 && (c=fgetc(myFile) ) != EOF) {
		if (c == '\n') 
			++lc;
  	fgetpos(myFile, &p);
	}

	fsetpos(myFile,&sav);

	return lc;

}

int qStrBuf::GetLineNum() 
{
// painfully derive line number

	int i;
	int lc = 1;

	for (i = 0; i < myP; ++i ) {
		if (Data()[myP] == '\n') 
			++lc;

	}
	return lc;

}

int qStrReadBuf::GetLineNum() 
{
// painfully derive line number

	const char *pc;

	int lc = 1;
	for (pc = myB; pc < myP; ++pc ) {
		if (*pc == '\n') 
			++lc;

	}
	return lc;
}
