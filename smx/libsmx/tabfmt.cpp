#ifdef _SMX_IMPLEMENTATION
#include "qsmx.h"
#else
#include "stdafx.h"
#endif
#include "tabfmt.h"
#include <assert.h>

CTabFmt::CTabFmt()
{
	m_cFmt  = 0;
	m_cTot  = 0;
	Lev     = -1;

	m_nCols = 0;
	m_nAlloc = 0;
	m_nRows = 0;
	m_nWid  = 0;
	memset(m_cntx,0,sizeof(m_cntx));

	m_nBuf  = 0;
	m_cbBuf = 2048;
	m_buf   = (char *) malloc(m_cbBuf);

	m_pStream = 0;
}

void CTabFmt::Init(OUTSTREAM *pStream)
{
	m_pStream = pStream;
}

void parseqstr(const char *&p, char *&ep, int &len) 
{
	ep = (char *) ++p;
	while (*ep && *ep != '\'')
		++ep;
	if (*ep == '\'' && ep[1] == '\'') {
		char *o = ++ep;
		do {
			++ep;
			while (*ep && *ep != '\'')
				*o++=*ep++;
		} while (*ep == '\'' && ep[1] == '\'');
		*o = *ep;
		len = o-p;
	} else
		len = ep-p;
}

void CTabFmt::ColDef(ColFmt &col, const char *szFmt)
{
	const char *p = szFmt;
	char *ep;
	
	col.dec = -1;

	while (*p) {
		switch(*p) {
		case 'r':case 'l':case 'c':case 'd':
			if (p[1]=='=') ++p;
			col.pad = *p;
			col.wid = strtol(++p, &ep, 0);
			p = ep;
			break;
		case '.':
			if (p[1]=='=') ++p;
			col.dec = strtol(++p, &ep, 0);
			p = ep;
			break;
		case '$':
			col.pre = '$';
			++p;
			break;
		case '%':
			col.pos = '%';
			++p;
			break;
		case 'p':
			if (p[1]=='=') ++p;
			col.spc = strtol(++p, &ep, 0);
			p = ep;
			break;
		case 'f':
			if (p[1]=='=') ++p;
			col.rgb = strtol(++p, &ep, 16);
			p = ep;
			break;
		case 'z':
			if (p[1]=='=') ++p;
			col.zer = *++p;
			break;
		case 'n':
			if (p[1]=='=') ++p;
			col.neg = strtol(++p, &ep, 16);
			p = ep;
			break;
		case 's':
			col.sum = 's';
			++p;
			break;
		case 'h':
			col.hdr = 'h';
			++p;
			break;
		case 'u':
			col.att |= ATT_U;
			++p;
			break;
		case 'b':
			col.att |= ATT_B;
			++p;
			break;
		case 'i':
			col.att |= ATT_I;
			++p;
			break;
		case '!':
			col.att |= ATT_NULL;
			++p;
			break;
		case ',': case ';': 
			if (p[1]=='=') p+=2;

			if (*p=='0')
				col.tho = 0;
			else
				col.tho = *p;

			++p;
			break;
		case 't':
			if (p[1]=='=') ++p;
			if (isdigit(*(p+1))) {
				col.trz = strtol(++p, &ep, 0) + 1;
			} else
				col.trz = 1;
			++p;
			break;
		case '\'': {
			int len;
			parseqstr(p, ep, len);
			col.tdx = (char *) memcpy(malloc(len+1), p, len);
			col.tdn = len;
			col.tdx[len] = 0;

			p = ep;
			if (*p == '\'')
				++p;

			break;
				   }
		default:
			++p;
		}
	}
}

void CTabFmt::RowDef(const char *szFmt[], int nFmt)
{
	if (nFmt > m_nAlloc) {
		m_cTot = (ColTot *) realloc(m_cTot, nFmt * sizeof(*m_cTot));
		memset(m_cTot + m_nAlloc, 0, (nFmt - m_nAlloc) * sizeof(*m_cTot));

		free(m_cFmt);
		m_cFmt = (ColFmt *) malloc(nFmt * sizeof(*m_cFmt));
	}
	memset(m_cFmt,0,nFmt * sizeof(*m_cFmt));

	m_nAlloc = max(m_nAlloc, nFmt);
	m_nCols = nFmt;
	m_nWid = 0;

	if (m_cFmt) {
		int i;
		for (i = 0; i < (nFmt-1); ++i) {
			m_cFmt[i].spc = 1;
			m_cFmt[i].neg = 0xFF0000;
			m_cFmt[i].rgb = -1;
			m_cFmt[i].tho = ',';
			ColDef(m_cFmt[i], szFmt[i]);
			m_nWid += m_cFmt[i].wid + m_cFmt[i].spc + !!m_cFmt[i].pre + !!m_cFmt[i].pos;
		}
		m_cFmt[i].neg = 0xFF0000;
		m_cFmt[i].spc = 0;
		m_cFmt[i].rgb = -1;
		m_cFmt[i].tho = ',';
		ColDef(m_cFmt[i], szFmt[i]);
		m_nWid += m_cFmt[i].wid + m_cFmt[i].spc + !!m_cFmt[i].pre + !!m_cFmt[i].pos;
	}
}

const char *CTabFmt::NumFmt(ColFmt &col, const char *szDat, char *szOut, int cbOut, double *rVal)
{
	int i, len;
	double val;
	const char *p;

	if (szDat && ((col.dec > -1) || col.sum))
		val = strtodx(szDat);
	else
		val = 0.0;

	*szOut++ = '\0'; --cbOut;

	if ((col.dec > -1) || col.sum) {
		if (col.dec > -1) {
			if (val == 0.0) {
				if (col.zer == '-') {
					len = _snprintf(szOut, cbOut, "%*s", col.dec+2, "-");
				} else if (col.zer == ' ' || col.zer == '_') {
					len = 0;  *szOut = '\0';
				} else {
					len = _snprintf(szOut, cbOut, "%.*f", col.dec, val);
				}
				if (col.dch && col.dch != '.') {
					char *n;
					if ((n = strchr(szOut, '.'))) {
						*n = col.dch;
					}
				}
				p = szOut;
			} else {
				len = _snprintf(szOut, cbOut, "%.*f", col.dec, val);

				if (col.dec > 0 && col.trz) {
					int t = col.dec - col.trz + 1;
					p = szOut + len - 1;
					while (t-- > 0 && *p == '0' && *p != '.')
						((char&) *p--) = ' ';
				}

				if (fabs(val) >= 1000.0) {
					char *n = szOut + cbOut - 1;
					*n-- = '\0';
					p = szOut + len - 1;

					if (col.dec > 0)
						while ((*n-- = *p--) != '.');
					else if (*p == '.')
						--p;

					if (*n && n[1] == '.' && col.dch)
						n[1] = col.dch;

					if (*p) {
						*n-- = *p--;
						i = 0;
						while (*p) {
							if (col.tho && isdigit(*p)) {
								++i;
								if (!(i % 3)) *n--=col.tho;
							}
							*n-- = *p--;
						}
					}
					p = n + 1;
				} else {
					if (col.dch && col.dch != '.') {
						char *n;
						if ((n = strchr(szOut, '.'))) {
							*n = col.dch;
						}
					}
					p = szOut;
				}
			}
		} else {
			p = szDat;
		}
	} else {
		p = szDat;
	}

	*rVal = val;

	return p;
}

double CTabFmt::Col(ColFmt &col, const char *szDat)
{
	if ((m_nBuf + 1024) > m_cbBuf) {
		m_cbBuf += 1024;
		m_buf = (char *) realloc(m_buf, m_cbBuf);
	}

	char *b = m_buf + m_nBuf;
	char *r = b;

	double val;
	int rgb;

	if (szDat && *szDat) {

		ColB(col, &r);

		HTIncr(szDat, r);

		const char *p;
		p = NumFmt(col, szDat, m_tmp, MAX_TMP, &val);

		if (col.rgb!=-1)
			rgb = col.rgb;
		else if (val < 0)
			rgb = col.neg;
		else
			rgb = 0;

		ColV(col, &p, val, rgb, &r);

		HTIncr(p, r);

		ColE(col, &r);
	} else {

		val = 0.0;
		rgb = col.rgb;

		ColB(col, &r);
		
		ColN(col, rgb, &r);

		ColE(col, &r);
	}

	m_nBuf += (r - b);

	return val;
}

void CTabFmt::Row(const char *szDat[], int nDat)
{
	if (nDat > m_nCols) 
		nDat = m_nCols;
	if (nDat > 0) {
		int i;
		double val;

		m_nBuf = 0;
		m_nRows += 1;

		char *b, *r;

		RowB(&(r = b = (m_buf + m_nBuf)));
		m_nBuf += (r - b);

		for (i = 0; i < nDat; ++i) {
			val = Col(m_cFmt[i], szDat[i]);
			if (m_cFmt[i].sum) {
				m_cTot[i].tot += val;
				m_cTot[i].ssq += val * val;
			}
		}

		RowE(&(r = b = (m_buf + m_nBuf)));
		m_nBuf += (r - b);
		assert((r-m_buf) == m_nBuf);
		Out(m_buf, m_nBuf);
	}
}

void CTabFmt::RowS(const char *szDat[], int nDat)
{
	if (nDat > m_nCols) 
		nDat = m_nCols;
	if (nDat > 0) {
		char *r, *b;
		ColFmt cf;
		int i;

		m_nBuf = 0;

		RowB(&(r = b = (m_buf + m_nBuf)));
		m_nBuf += (r - b);
		
		for (i = 0; i < nDat; ++i) {
			cf = m_cFmt[i];
			cf.sum = 0;
			Col(cf, szDat[i]);
		}

		RowE(&(r = b = (m_buf + m_nBuf)));
		m_nBuf += (r - b);
		assert((r-m_buf) == m_nBuf);
		Out(m_buf, m_nBuf);
	}
}

void CTabFmt::RowH(const char *szDat[], int nDat)
{
	--nDat;
	if (nDat > m_nCols) 
		nDat = m_nCols;
	if (nDat > 0) {
		char *r, *b;
		int i;
		ColFmt cf, hf;

		memset(&hf, 0, sizeof(hf));
		hf.pad = 'c';
		hf.rgb = -1;
		hf.tho = ',';
		ColDef(hf, szDat[0]);

		m_nBuf = 0;
		
		RowB(&(r = b = (m_buf + m_nBuf)));
		m_nBuf += (r - b);

		for (i = 0; i < nDat; ++i) {
			cf = m_cFmt[i];

			if (hf.pad) cf.pad = hf.pad;
			if (hf.rgb!=-1) cf.rgb = hf.rgb;

			cf.wid += cf.spc; cf.spc = 0; cf.sum = 0; cf.dec = -1;
			cf.hdr = 'h';

			Col(cf, szDat[i+1]);
		}

		RowE(&(r = b = (m_buf + m_nBuf)));
		m_nBuf += (r - b);
		assert((r-m_buf) == m_nBuf);
		Out(m_buf, m_nBuf);
	}
}

void CTabFmt::CalcSub(int nLev)
{
	int i;
	if (nLev >= 0 && nLev < MAX_SUB) {
		for (i = 0; i < m_nAlloc; ++i) {
			m_cTot[i].ssqx[nLev] = m_cTot[i].ssq;
			m_cTot[i].totx[nLev] = m_cTot[i].tot;
			m_cntx[nLev] = m_nRows;
		}
	}
}

#ifdef _SMX_IMPLEMENTATION
void CTabFmt::Out(const char *szOut, int nLen)
{
	m_pStream->Append(szOut, nLen);
}

void CTabFmt::Out(const char *szOut)
{
	m_pStream->AppendString(szOut);
}
#else
void CTabFmt::Out(const char *szOut, int nLen)
{
	if (szOut) m_pStream->PutS(szOut, nLen);
}

void CTabFmt::Out(const char *szOut)
{
	if (szOut) m_pStream->PutS(szOut);
}
#endif
