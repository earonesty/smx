/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


/*
 * regx.h
 *
 */

#ifndef _CCRegX_h
#define MAX_SUBX  35

class CRegX
{
protected:
// compile
	char *	m_parse;				// regex string pointer

	char *	m_code;					// opcode parse
	int		m_code_size;
	char *	m_code_ptr;				// code-emit pointer

// debug
	char *  m_errmsg;				// most recent error message

// 
	bool	m_sensitive;			// case sensitivite flag

// optimize
	char	m_start;
	char	m_anchor;
	char *	m_must;
	int		m_must_len;

// match
	char *		m_input;				// match input base
	char *		m_input_ptr;			// match input pointer (shouldn't be member)

	int			m_subx_count;			// supexpression count
	char *		m_startp[MAX_SUBX];		// subexpression start
	char *		m_endp[MAX_SUBX];		// subexpression end


static int		s_code_grow;		// grow code buffer

// parse
	char *		Parse(char *&parse, int paren, int *flagp);
	void		ParseC(char b);
	char *		ParseBranch(char *&parse, int *flagp);
	char *		ParsePiece(char *&parse, int *flagp);
	char *		ParseAtom(char *&parse, int *flagp);

	void		ParseTail(char *p, char *val);
	void		ParseOpTail(char *p, char *val);

	char *		ParseNode(char op);
	void		ParseInsert(char op, char *opnd);
	int			ParseGrowBuf(int n);

	char *		ParseNext(char *p) const;

// match
	int			MatchX(char *prog);
	int			MatchTry(const char *string);
	int			MatchRepeat(char *p);

// debug
	void		SetError(const char *msg);
	char *		DumpProp(char *op) const;

public:
				CRegX();
				CRegX(const char* t);
				~CRegX();

	bool        GetCase() {
				return m_sensitive;
	}

	void        SetCase(bool IsSensitive) {
				m_sensitive = IsSensitive;
	}

	int			Compile(const char *exp);
	int			Match(const char *string);
	int         GetMatchCount() {
		return m_subx_count;
	}
	int			GetMatchInfo(const char *&startp, const char *&endp, int nth = 0) const;
	char *		GetInput() {return m_input;}
	char *		GetRegX() {return m_parse;}
	const char *GetError() const {return m_errmsg;}
	void		Dump(FILE *f = stdout) const;

};
#endif
