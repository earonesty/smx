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
