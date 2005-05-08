/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#include <stdlib.h>
#include <string.h>
#include "base64.h"

static char encode_map[64]= {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', 
	'4', '5', '6', '7', '8', '9', '+', '/'
};

char *mime64(char *str, int &len)
{
	int rlen;
	char *encode_buf;

	if (!str) return str;
	if (len == 0) len = strlen(str);

	rlen  = 4 * (((len - len % 3) / 3) + (len % 3 ? 1 : 0)); // encode
	rlen += 2 * (1 + len / 48); // crlf
	rlen += 1; // \0
	
	encode_buf = (char *) malloc(rlen+2);	

	char *pi = str;
	char *po = encode_buf;

	while (len > 48) {
		base64(pi, 48, po);
		pi += 48;
		po += 64;
		*po++ = '\r'; *po++ = '\n';
		len -= 48;
	}
	if (len > 0) {
		base64(pi, len, po);
		po += 4 * (((len - len % 3) / 3) + (len % 3 ? 1 : 0));
		*po++ = '\r'; *po++ = '\n';
	}
	*po = '\0';
	len = (po - encode_buf);
	return encode_buf;
}

char *base64(char *str, int len, char *buf)
{
	unsigned char *pm = (unsigned char *) encode_map;
	unsigned char *po = (unsigned char *) buf;
	unsigned char *pi = (unsigned char *) str;
	unsigned char *pe = (unsigned char *) str + len;
	unsigned char  c1, c2;
	
	while (pi < pe)

	{
		c1 = *pi >> 2;					// 6 bits
		c2 = (*pi << 4) & 060;			// 2 bits + 0000
		*po++=pm[c1];

		if (++pi >= pe) {*po++=pm[c2]; *po++='='; *po++='=';break;}
				
		c1 = c2 | ((*pi >> 4) & 017);	// 4 bits
		c2 = (*pi << 2) & 074;          // 4 bits + 00
		
		*po++=pm[c1];
		if (++pi >= pe) {*po++=pm[c2]; *po++='=';break;}

		*po++= pm[c2 | (*pi >> 6) & 003];	// 2 bits
		*po++= pm[*pi & 077];				// 6 bits
		++pi;
	}
	*po = '\0';
	return buf;
}

static int decode_init = 0;
static int decode_map[255];

char *base64_decode(char *str, int len, char *buf)
{
	int i;
	if (!decode_init) {
		memset(decode_map, 0, 255 * sizeof(int));
		for (i = 0; i < 64; ++i) {
			decode_map[encode_map[i]]=i;
		}
		decode_init = 1;
	}

	int v = 0;
	int j = 0;
	int bi = 0;
	for (i = 0; str[i]; ++i)
	{
		v += (decode_map[str[i]] << ((3-j) * 6) );
		++j;
		if (j == 4)
		{
			if (v & 0xFF0000) buf[bi++] = (v & 0xFF0000)>>16;
			if (v & 0xFF00) buf[bi++] = (v & 0xFF00)>>8;
			if (v & 0xFF) buf[bi++] = (v & 0xFF);
			v=0;
			j=0;
		}
	}
	buf[bi] = '\0';
	return buf;
}




/*
		c1 = *pi & 0x3F;				// 6 bits
		c2 = *pi >> 6;					// 2 bits
		*po++=c1;

		if (!*++pi) {*po++=c2; *po++='='; *po++='=';break;}
				
		c1 = c2 | (*pi << 2) & 0x3C;	// 4 bits
		c2 = *pi >> 4;					// 4 bits
		
		if (!*++pi) {*po++=c2; *po++='=';break;}

		*po++=c1;

		*po++= c2 | (*pi << 4) & 0x30;	// 2 bits
		*po++= *pi >> 2;				// 6 bits

  */



