/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#include "stdafx.h"

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "sqlgrp.h"

CSqlGrp::CSqlGrp(qCtx *ctx)
{
	m_pCtx = ctx; 
	if (!m_pCtx->Find(&m_pSqlObj, "sql")) 
		m_pSqlObj=0;

	m_nRows = 0;
	m_nCols = 0;

	memset(m_nRowsX,0,sizeof(m_nRowsX));
	memset(m_szPrev,0,sizeof(m_szPrev));
}

void CSqlGrp::SetArgs(CStr *args, int nArgs)
{
	if (nArgs > 0) {
		if (args[0]) {
			char *tok = strtok(args[0].GetBuffer(), ";");
			m_nCols = 0;
			while (tok) {
				m_szCols[m_nCols] = tok;
				++m_nCols;
				tok = strtok(0, ";");
			}
		}
	}

	m_szDetail = ((nArgs > 1) ? (const char *) args[1]   : (const char *) NULL);
	m_szHead   = ((nArgs > 2) ? (const char *) args[2]   : (const char *) NULL);
	m_szFoot   = ((nArgs > 3) ? (const char *) args[3]   : (const char *) NULL);
	m_szGrps   = ((nArgs > 4) ? &args[4]  : NULL);
	m_nGrps    = ((nArgs > 4) ? nArgs - 4 : 0);
}

void CSqlGrp::SqlHead(qCtx *pContext, qStr *pStream)
{
	if (m_szHead)
		pContext->Parse(m_szHead, pStream);
}

void CSqlGrp::SqlFoot(qCtx *pContext, qStr *pStream)
{
	int j, c;
	for (j = (m_nCols-1); j >= 0; --j) {
		if (m_nRows > m_nRowsX[j]) {
			// footers of *all* groups 
			c = j * 2 + 1;
			if (c < m_nGrps)
				pContext->Parse(m_szGrps[c], pStream);
			m_nRowsX[j] = m_nRows;
		}
	}
	if (m_szFoot)
		pContext->Parse(m_szFoot, pStream);
}

void CSqlGrp::SqlDetail(qCtx *pContext, qStr *pStream)
{
	int i, j, c;
	qStrBuf value;
	qObj *col;
	
	if (!pContext->Find(&col, "col"))
		return;
	qArgAry ary;
	
	for (i = 0; i < m_nCols; ++i) {
		value.Clear();
		ary.SetAt(0,m_szCols[i]); col->Eval(pContext, &value, &ary);
		if (!m_szPrev[i] || strcmp(value.SafeP(), m_szPrev[i])) {
		// difference - new grouping at level *i*
			for (j = (m_nCols-1); j >= i; --j) {
				if (m_nRows > m_nRowsX[j]) {
					// footers of all groups after this level
					c = j * 2 + 1;
					if (c < m_nGrps)
						pContext->Parse(m_szGrps[c], pStream);
					m_nRowsX[j] = m_nRows;
				}
			}

			for (j = i; j < m_nCols; ++j) {
				// headers of all groups from this level down
				c = j * 2;

				if (c < m_nGrps)
					pContext->Parse(m_szGrps[c], pStream);

				value.Clear();
				ary.SetAt(0,m_szCols[j]); col->Eval(pContext, &value, &ary);
				strncpy(m_szPrev[j], value, MAX_PREV);
				m_szPrev[j][MAX_PREV] = '\0';
			}
			break;
		}
	}

	++m_nRows;

	if (m_szDetail) 
		pContext->Parse(m_szDetail, pStream);
}


void EvalGrpDetail(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	CSqlGrp *sg = (CSqlGrp *) pObject;
	sg->SqlDetail(pContext, pStream);
}

void EvalGrpHead(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	CSqlGrp *sg = (CSqlGrp *) pObject;
	sg->SqlHead(pContext, pStream);
}

void EvalGrpFoot(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	CSqlGrp *sg = (CSqlGrp *) pObject;
	sg->SqlFoot(pContext, pStream);
}

void EvalSqlGrp(const void *data, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 3) {
		CSqlGrp sg(pContext);

		qCtxTmp pCtx(pContext);

		pCtx.MapObj(&sg, &EvalGrpDetail, "sql-detail");
		pCtx.MapObj(&sg, &EvalGrpHead,   "sql-head");
		pCtx.MapObj(&sg, &EvalGrpFoot,   "sql-foot");
		
		sg.SetArgs(&(*args)[2],args->Count()-2);
		
		qArgAry tmp;
		tmp.Add((*args)[0]);
		tmp.Add((*args)[1]);
		tmp.Add("%sql-detail%");
		tmp.Add("%sql-head%");
		tmp.Add("%sql-foot%");
		pCtx.Eval(pStream, "sql", &tmp);
	}
}
