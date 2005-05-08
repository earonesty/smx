#ifndef _TABPRE_H_
#define _TABPRE_H_

class CTabPre : public CTabFmt
{
	int m_pRgb;

public:
	CTabPre(OUTSTREAM *pStream) {Init(pStream); m_pRgb = 0;}

	void ColV(ColFmt &col, const char **data, double value, int rgb, char **bufp);
	void ColN(ColFmt &col, int rgb, char **bufp);
	void RowB(char **bufp);
	void RowE(char **bufp);
};

#endif //#ifndef _TABPRE_H_
