/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifdef unix

#include "unix.h"
// Windows to UNIX compatibility library.

#include <stdio.h>


#define isslash(c) ((c=='/') || (c=='\\'))
inline char *PathGetDotDot(char *str);
inline char *PathGetDot(char *str);


char *strrev(char *buffer)
{
	size_t len;
	int maxswaps;

	char tmp;
	int i;

	len=strlen(buffer);
	maxswaps=len/2;
	len--;

	for(i=0; i<maxswaps; i++)
	{
		tmp=buffer[i];
		buffer[i]=buffer[len-i];
		buffer[len-i]=tmp;
	}

  return buffer;
}



// populates buf with ascii version of num, base radix.

char *itoa(int num, char *buf, int radix) 
{
	char *b = buf;

	if (num<0) {
		num=-num;    
		*b++='-';
	}
	
	char *p = b;

	if (num==0) {
		*b++='0';
		*b++='\0';
	} else {
		while (num!=0) {
			*p++ = '0' + num%radix;
			num /= radix;
		}

		*p = '\0';

		strrev(b);
	}

	return buf;
}

// populates buf with ascii version of num, base radix.

char *ultoa(unsigned long num, char *buf, int radix) 
{
	char *b = buf;

	if (num<0) {
		num=-num;    
		*b++='-';
	}
	
	char *p = b;

	if (num==0) {
		*b++='0';
		*b++='\0';
	} else {
		while (num!=0) {
      if (num%radix > 9)
  			*p++ = 'a' + num%radix - 10;
      else
  			*p++ = '0' + num%radix;
			num /= radix;
		}

		*p = '\0';

		strrev(b);
	}

	return buf;
}

char *ltoa(long num, char *buf, int radix) 
{
	char *b = buf;

	if (num<0) {
		num=-num;    
		*b++='-';
	}
	
	char *p = b;

	if (num==0) {
		*b++='0';
		*b++='\0';
	} else {
		while (num!=0) {
			*p++ = '0' + num%radix;
			num /= radix;
		}

		*p = '\0';

		strrev(b);
	}

	return buf;
}


/*
#ifdef NEVER
#include <stdio.h>
int main(void)
{
	char string[20];

	itoa(-1,string,10);
	printf("%s\n", string);

	return 0;
}
#endif //ifdef NEVER
*/

char *PathCombine(char *dest, const char *abs, const char *rel)
	{
	if (abs && rel) 
		{	
		int abslen = strlen(abs);

		strcpy(dest, abs);

		while (*abs && ((abs[abslen-1] == '/')||(abs[abslen-1] == '\\')))
			abslen--;	

		while ((*rel == '/')||(*rel == '\\'))
			rel++;

		strncpy(dest, abs, abslen);
		strncpy(dest+abslen, "/", 1);
		strcpy(dest+abslen+1, rel);
		return (dest);
		}
	else
		{
		return (NULL);
		}
	}

/*
char *PathCanonicalize(char *dest, char *src)
	{
//	int srcpos=0;
//	int destpos=0;
	char *dotpos;
	char *prevpos;
	if (src)
		{
		strcpy(dest,src);
		while((dotpos = PathGetDotDot(dest))) // returns pointer to first dot, won't find a .. at the beginning like '../hi'
			{
			prevpos=dotpos-2;
			while ((prevpos>=dest) && (*prevpos != '/') && (*prevpos != '\\'))
				--prevpos;
			if (*(dotpos+2) == '\0')
				prevpos[1] = '\0';
			else
				strcpy(prevpos+1, dotpos+3);
			}
		while((dotpos = PathGetDot(dest)))
			{
			if (*(dotpos+1) == '\0')
				*dotpos  = '\0';
			else
				strcpy(dotpos,dotpos+2);
			}
                while((dotpos = strstr(dest, "//")))
                        {
                        	strcpy(dotpos,dotpos+1);
                        }
		return (dest);
		}
	else
		{
		return (NULL);
		}
	
	}
*/

char *PathCanonicalize(char *dest, char *src)
{
	return realpath(src, dest);
}

inline char *PathGetDotDot(char *str)
	{
	char* match;

	while ((match = strstr(str, "..")))
		{
		if ((match!=str) && isslash(*(match-1)) && ((*(match+2) == '\0') || isslash(*(match+2))))
			return(match);
		str=match+1;
		}
	return (NULL);	
	}

inline char *PathGetDot(char *str)
	{
	char* match;
	while ((match = strchr(str, '.')))
		{
		if ((match!=str) && isslash(*(match-1)) && ((*(match+1) == '\0') || isslash(*(match+1))))
			return(match);
		str=match+1;
		}
	return (NULL);	
	}
	
bool PathMakePretty(char *dummy)
	{
	return (true);
	}

int GetTempPath(int len, char *name)
{
  strncpy(name, "/tmp/psx.XXXXXX", len);
  if (mkstemp(name) == -1)
    return 0;
  else
    return(strlen(name));
}


int SearchPath(const char *path, const char *fname, const char *ext, int fbuf_size, char *fbuf_result, char **file_component)
{
  if (fbuf_result != fname) {
  	strncpy(fbuf_result, fname, fbuf_size);
  }
  *file_component = fbuf_result;  
  return strlen(fbuf_result);
}


#endif

