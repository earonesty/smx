/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
