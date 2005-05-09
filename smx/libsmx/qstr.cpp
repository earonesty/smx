/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
