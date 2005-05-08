/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
char *mime64(char *str, int &len);				// multiline
char *base64(char *str, int len, char *buf);	// single line
char *base64_decode(char *str, int len, char *buf);	// single line
