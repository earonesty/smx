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
