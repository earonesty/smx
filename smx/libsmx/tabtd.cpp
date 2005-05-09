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
#include "tabtd.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

void CTabTD::ColB(ColFmt &col, char **bufp)
{
	char *r = *bufp;

	*r++ = '\n';
	*r++ = ' ';
	*r++ = ' ';
	*r++ = '<';
	*r++ = 'T';
	*r++ = col.hdr ? 'H' : 'D';

	if (col.pad == 'r' || col.pad == 'd') {
		r = r + (sprintf(r, " Align=Right"));
	} else if (col.pad == 'c') {
		r = r + (sprintf(r, " Align=Center"));
	} else if (col.hdr) {
		r = r + (sprintf(r, " Align=Left"));
	} 

	if (m_tdex) {
		*r++ = ' ';
		memcpy(r, m_tdex, m_tdex_n);
		r += m_tdex_n;
	}

	if (col.tdx) {
		*r++ = ' ';
		memcpy(r, col.tdx, col.tdn);
		r += col.tdn;
	}

	*r++ = '>';

	*bufp = r;
}

void CTabTD::ColE(ColFmt &col, char **bufp)
{
	char *r = *bufp;

	*r++ = '<';
	*r++ = '/';
	*r++ = 'T';
	*r++ = col.hdr ? 'H' : 'D';
	*r++ = '>';

	*bufp = r;
}

void CTabTD::ColN(ColFmt &col, int rgb, char **bufp)
{
	if (!(col.att & ATT_NULL)) {
		char *r = *bufp;

		r += sprintf(r, "&nbsp;");

		*bufp = r;
	}
}

void CTabTD::ColV(ColFmt &col, const char **data, double val, int rgb, char **bufp)
{
	const char *p = *data;
	char *r = *bufp;


	if (rgb) {
		r = r + (sprintf(r, "<font color=%06x>", rgb));
	}

	if (col.pre)
		*r++ = col.pre;

	if (col.att & ATT_B)
		r+= sprintf(r, "<b>");
	if (col.att & ATT_U)
		r+= sprintf(r, "<u>");
	if (col.att & ATT_I)
		r+= sprintf(r, "<u>");

	while(*p) {
		*r++ = *p++;
	}

	if (col.att & ATT_I)
		r+= sprintf(r, "</i>");
	if (col.att & ATT_U)
		r+= sprintf(r, "</u>");
	if (col.att & ATT_B)
		r+= sprintf(r, "</b>");

	if (col.pos)
		*r++ = col.pos;

	if (rgb) {
		r = r + (sprintf(r, "</font>"));
	}

	*bufp = r;
	*data = p;
}


void CTabTD::RowB(char **bufp)
{
	strcpy(*bufp, "\n <TR"); *bufp+=5;
	if (m_trex) {
		*(*bufp)++ = ' ';
		strcpy(*bufp, m_trex); *bufp+=m_trex_n;
	}
	*bufp+=sprintf(*bufp, ">"); 
}

void CTabTD::RowE(char **bufp)
{
	*bufp+=sprintf(*bufp, " </TR>"); 
}
