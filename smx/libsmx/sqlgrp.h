/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#define MAX_PREV 128
#define MAX_SUB    8

class CSqlGrp
{

	int				m_nRows;
	int				m_nRowsX[MAX_SUB];

	int				m_nCols;
	const char *	m_szCols[MAX_SUB];
	char			m_szPrev[MAX_SUB][MAX_PREV+1];

	int				m_nGrps;
	const CStr *	m_szGrps;

	const char *	m_szDetail;
	const char *	m_szHead;
	const char *	m_szFoot;

	qCtx * m_pCtx;
	qObj * m_pSqlObj;
public:
	CSqlGrp(qCtx *ctx);

	void SetArgs(CStr *args, int nArgs);
	
	void SqlHead(qCtx *pContext, qStr *pStream);
	void SqlFoot(qCtx *pContext, qStr *pStream);
	void SqlDetail(qCtx *pContext, qStr *pStream);

	long Cnt()		{return m_nRows;}
	long Cnt(int l) {return m_nRows - m_nRowsX[l];}
};

void EvalSqlGrp(const void *data, qCtx *pContext, qStr *pStream, qArgAry *args);
