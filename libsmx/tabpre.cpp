/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef _SMX_IMPLEMENTATION
#include "qsmx.h"
#else
#include "stdafx.h"
#endif
#include "tabfmt.h"
#include "tabpre.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

void CTabPre::ColN(ColFmt &col, int rgb, char **bufp)
{
	char *r = *bufp;
	int i;

	if (col.pos)
		*r++ = ' ';
	if (col.pre)
		*r++ = ' ';
	for(i = 0; i < col.wid; ++i)
		*r++ = ' ';
	for(i = 0; i < col.spc; ++i)
		*r++ = ' ';

	*bufp = r;
}

void CTabPre::ColV(ColFmt &col, const char **data, double val, int rgb, char **bufp)
{
	const char *p = *data;
	char *r = *bufp;

	int i;
	int len;

	if (m_pRgb != rgb) {
		if (m_pRgb)
			r = r + (sprintf(r, "</font>"));
		if (rgb)
			r = r + (sprintf(r, "<font color=%06x>", rgb));
	}

	if (col.pad == 'l') {
		if (col.pre)
			*r++ = col.pre;
		if (col.att & ATT_U)
			r+= sprintf(r, "<u>");
		for(i = 0; *p && i < col.wid; ++i) {
			if (HTIncr(p, r)) {
				--i;
			} else {
				*r++ = *p++;
			}
		}
		while (*p && *p != '<') ++p; HTIncr(p, r);

		if (col.att & ATT_U)
			r+= sprintf(r, "</u>");

		for(; i < col.wid; ++i)
			*r++ = ' ';
	} else if (col.pad == 'c') {
		len = min((int) strlenx(p), col.wid);
		for(i = 0; i < ((col.wid - len) / 2); ++i)
			*r++ = ' ';
		if (col.pre)
			*r++ = col.pre;
		if (col.att & ATT_U)
			r+= sprintf(r, "<u>");
		while(*p && i++ < col.wid) {
			if (HTIncr(p, r)) {
				--i;
			} else {
				*r++ = *p++;
			}
		}

		while (*p && *p != '<') ++p; HTIncr(p, r);

		if (col.att & ATT_U)
			r+= sprintf(r, "</u>");

		for(; i < col.wid; ++i)
			*r++ = ' ';
	} else if (col.pad == 'r') {
		len = min((int) strlenx(p), col.wid);
		if (col.pre)
			*r++ = col.pre;
		for(i = 0; i < (col.wid - len); ++i)
			*r++ = ' ';
		if (col.att & ATT_U)
			r+= sprintf(r, "<u>");

		while(i++ < col.wid && *p) {
			if (HTIncr(p, r)) {
				--i;
			} else {
				*r++ = *p++;
			}
		}

		while (*p && *p != '<') ++p; HTIncr(p, r);
		
		if (col.att & ATT_U)
			r+= sprintf(r, "</u>");
	}
	
	while (*p && *p != '<') ++p; HTIncr(p, r);

	if (col.pos)
		*r++ = col.pos;

	for(i = 0; i < col.spc; ++i)
		*r++ = ' ';

	m_pRgb = rgb;

	*bufp = r;
	*data = p;
}

void CTabPre::RowB(char **bufp)
{
}

void CTabPre::RowE(char **bufp)
{
	if (m_pRgb) 
		*bufp+=sprintf(*bufp, "</font>"); 
	m_pRgb = 0;
	*(*bufp)++ = '\n';
}
