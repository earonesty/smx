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

