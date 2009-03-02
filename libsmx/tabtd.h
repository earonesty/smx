/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

class CTabTD : public CTabFmt
{
	char *m_trex;
	int m_trex_n;

	char *m_tdex;
	int m_tdex_n;
public:
	CTabTD(OUTSTREAM *pStream) {
		Init(pStream); 
		m_trex = NULL;
		m_trex_n = 0;
		m_tdex = NULL;
		m_tdex_n = 0;
	}
	~CTabTD() {
		if (m_trex) 
			free(m_trex);
		if (m_tdex) 
			free(m_tdex);
	}

	void SetRowEx(const char *trx, const char *tdx) {
		if (trx && (m_trex_n = strlen(trx)) > 0)
			strcpy(m_trex = (char *) malloc(m_trex_n+1), trx);
		else if (m_trex) {
			free(m_trex);
			m_trex = NULL;
			m_trex_n = 0;
		}
		if (tdx && (m_tdex_n = strlen(tdx)) > 0)
			strcpy(m_tdex = (char *) malloc(m_tdex_n+1), tdx);
		else if (m_tdex) {
			free(m_tdex);
			m_tdex = NULL;
			m_tdex_n = 0;
		}
	}

	void ColB(ColFmt &col, char **bufp);
	void ColV(ColFmt &col, const char **data, double value, int rgb, char **bufp);
	void ColN(ColFmt &col, int rgb, char **bufp);
	void ColE(ColFmt &col, char **bufp);

	void RowB(char **bufp);
	void RowE(char **bufp);
};

