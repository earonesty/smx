/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#define ATT_U 1
#define ATT_B 2
#define ATT_I 4
#define ATT_NULL 32


#ifdef _SMX_IMPLEMENTATION
#define OUTSTREAM ICharStream
#else
#include "qstr.h"
#define OUTSTREAM qStr
#endif

#ifndef MAX_SUB
#define MAX_SUB 35
#endif

#ifndef MAX_TMP
#define MAX_TMP 512
#endif

#include <math.h>

struct ColFmt
{
	char  hdr;		// td or th?
	char  pre;		// $
	char  pad;		// pad type (r, l, c, d)
	int   wid;		// pad width
	int   spc;		// num space chars
	char  sum;		// sum, avg, dev operator
	int   dec;		// num decimals for numeric
	char  dch;		// decimal character replacement (if any)
	int   trz;		// trim trailing zero to this many...(trz-1)
	char  tho;		// thousands separator (if any)
	int   rgb;		// font color (def 0)
	int   neg;		// neg color  (def 0xFF0000)
	char  zer;		// zero handling
	int   att;		// font attribute
	char  pos;		// post-char %
	char *tdx;		// extra cell info 
	char  tdn;		// extra cell info length
};

struct ColTot
{
	double ssq;
	double tot;

	double ssqx[MAX_SUB];
	double totx[MAX_SUB];
};

class CTabFmt
{
private:
// syntax: r|l|c|d## [.##] [p##] [s|a|d] [,|;] f###### z[-|0|_]

	ColFmt  *m_cFmt;
	ColTot  *m_cTot;

	int      m_cntx[MAX_SUB];

	int      m_nRows;
	int      m_nCols;
	int      m_nAlloc;
	int      m_nWid;
	char    *m_buf;
	int      m_nBuf;
	int      m_cbBuf;
	char     m_tmp[MAX_TMP];

	OUTSTREAM *m_pStream;

protected:
// override this to implement non-abstract table formatter class
	virtual void ColB(ColFmt &col, char **bufp) {}

	virtual void ColV(ColFmt &col, const char **data, double value, int rgb, char **bufp) = 0;
	virtual void ColN(ColFmt &col, int rgb, char **bufp) = 0;
	virtual void ColE(ColFmt &col, char **bufp) {}

	virtual void RowB(char **bufp) = 0;
	virtual void RowE(char **bufp) = 0;

public:
// numeric format
	static const char *NumFmt(ColFmt &col, const char *szDat, char *szOut, int cbOut, double *pVal);

// construct/destruct
	 CTabFmt();
	virtual ~CTabFmt() {free(m_cFmt); free(m_cTot); free(m_buf);}


	void Init(OUTSTREAM *m_pStream);

// row & column quick-definition parser
	void ColDef(ColFmt &col, const char *szFmt);
	void RowDef(const char *szFmt[], int nFmt);

// calculates subtotals for a given level
	void CalcSub(int nLev);

// output row headers/subtotals
	void RowH(const char *szFmt[], int nFmt);
	void Row(const char *szFmt[], int nFmt);
	void RowS(const char *szFmt[], int nFmt);

// output column
	double Col(ColFmt &col, const char *szDat);

// current number of columns
	int    Cols()	  {return m_nCols;}

	void Out(const char *szOut, int nLen);
	void Out(const char *szOut);

#define VALIDI(i) (i>=0&&i<m_nAlloc)
#define VALIDL(i) (l>=0&&i<MAX_SUB)

	void SetSum(int i, double val) {
		if (VALIDI(i)) {
			m_cTot[i].tot = val;
		}
	}

	void SetSum(int l, int i, double val) {
		if (VALIDI(i)&&VALIDL(l)) {
			m_cTot[i].totx[l] = m_cTot[i].tot - val;
		}
	}

	double Sum(int i) {return VALIDI(i) ? m_cTot[i].tot : 0;}
	long   Cnt()      {return m_nRows;}

	double Avg(int i) {return VALIDI(i) ? m_cTot[i].tot / m_nRows : 0;}
	double Dev(int i) {
		return VALIDI(i) ? 
		(sqrt((m_nRows * m_cTot[i].ssq - m_cTot[i].tot * m_cTot[i].tot) \
			/ (m_nRows * (m_nRows-1)))
		) : 0;
	}

	double Sum(int l, int i) {return (VALIDI(i)&&VALIDL(l)) ? (m_cTot[i].tot - m_cTot[i].totx[l]) : 0;}
	double Ssq(int l, int i) {return (VALIDI(i)&&VALIDL(l)) ? (m_cTot[i].ssq - m_cTot[i].ssqx[l]) : 0;}
	long   Cnt(int l)        {return (           VALIDL(l)) ? (m_nRows - m_cntx[l]) : 0;}

	double Avg(int l, int i) {return (VALIDI(i)&&VALIDL(l)) ? Sum(l, i) / m_nRows : 0;}
	double Dev(int l, int i) {
		return (VALIDI(i)&&VALIDL(l)) ? 
		(sqrt((Cnt(l) * Ssq(l,i) - Sum(l,i) * Sum(l,i))
			/ (Cnt(l) * (Cnt(l)-1))) 
		) : 0;
	}

	int Lev;


	long   Wid()        {return m_nWid;}
	char * Tmp()        {return m_tmp;}

	static inline int CTabFmt::HTIncr(const char *&p, char *&r)
	{
		if (*p == '<') {while (*p && *p != '>') *r++ = *p++; if (*p == '>') *r++ = *p++; return 1;}
		else return 0;
	}


	static inline int CTabFmt::HTIncr(const char *&p)
	{

		if (*p == '<') {while (*p && *p != '>') *p++; if (*p == '>') *p++; return 1;}
		else return 0;
	}

	static inline int CTabFmt::HTDecr(const char *&p)
	{
		if (*p == '>') {while (*p && *p != '<') *p--; if (*p == '<') *p--; return 1;}
		else return 0;
	}
};

