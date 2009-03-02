/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "regx.h"
#ifdef unix
#include "unix.h"
#endif //ifdef unix
/*

>HISTORY
>-------
>	There is not much to tell.  This was found in on a German
>	BBS.  It was written in the old "C" convention and contained
>	a single comment describing that it was a regular expression
>	parser and that was it.  

EA. I left the comments, added a class wrapper, and fixed a 
subexpression bug which was probably added by someone other than
the original author.

EA. Also added a global "case-instensitive match" flag to the class.

TODO:
	const vars should be used correctly and not coerced
	code frgments below marked ***TODO*** need to be to-done

*/

/*
 * The "internal use only" fields in regx are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * m_start	char that must begin a match; '\0' if none obvious
 * m_anchor	is the match anchored (at beginning-of-line only)?
 * m_must	string (pointer into program) that match must include, or NULL
 * m_must_len	length of m_must string
 *
 * m_start and m_anchor permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  m_must permits fast rejection
 * of lines that cannot possibly match.  The m_must tests are costly enough
 * that regcomp() supplies a m_must only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  m_must_len is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is

 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

// definition	number	opnd?	meaning
#define	END		0	//	no		End of program. 
#define	BOL		1	//	no		Match "" at beginning of line. 
#define	EOL		2	//	no		Match "" at end of line. 
#define	ANY		3	//	no		Match any one character. 
#define	ANYOF	4	//	str		Match any character in this string. 
#define	ANYBUT	5	//	str		Match any character not in this string. 
#define	BRANCH	6	//	node	Match this alternative, or the next... 
#define	BACK	7	//	no		Match "", "next" ptr points backward. 
#define	EXACTLY	8	//	str		Match this string. 
#define	NOTHING	9	//	no		Match empty string. 
#define	STAR	10	//	node	Match this (simple) thing 0 or more times. 
#define	PLUS	11	//	node	Match this (simple) thing 1 or more times. 
#define	WORDA	12	//	no		Match "" at wordchar, where prev is nonword 
#define	WORDZ	13	//	no		Match "" at nonwordchar, where prev is word 
#define	STARX	14	//	node	Match this (simple) thing 0 or more times - minimum number!
#define	PLUSX	15	//	node	Match this (simple) thing 1 or more times - minimum number!

#define	OPEN	20	//	no		Mark this point in input as start of #n. 
#define	CLOSE	(OPEN + MAX_SUBX + 1)	// no	Analogous to OPEN. 

// used
#define	MAGIC	((char) (CLOSE + MAX_SUBX + 1))

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)		(*(p))
#define	NEXT(p)		(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)
#define	UCHARAT(p)	((int)*(unsigned char *)(p))

 // optimization flags
#define	HASWIDTH	01	// known never to match null string.
#define	SIMPLE		02	// simple enough to be STAR/PLUS operand.
#define	SPSTART		04	// starts with * or +.
#define	WORST		0	// worst case

#define FAIL(m) { SetError(m); return(0); }
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')

int CRegX::s_code_grow = 2048;

CRegX::CRegX() 
{
	m_code		= NULL;		// free this
	m_code_size	= 0;

	m_parse		= NULL;
	m_subx_count= -1;
	m_code_ptr	= NULL;

	m_startp[0] = NULL;
	m_endp[0]	= NULL;

	m_start		= '\0';
	m_anchor	= '\0';
	m_must		= NULL;
	m_must_len	= 0;

	m_sensitive = 1;

	m_errmsg = NULL;		// free this
}

CRegX::~CRegX()
{
	if (m_code) free(m_code);
	if (m_errmsg) free(m_errmsg);
}

int CRegX::Compile(const char *exp) 
{
	char *scan, *longest;
	int len, flags;

	if (exp == NULL)
		FAIL("NULL argument");

	/* First pass: determine size, legality. */
#ifdef notdef
	if (exp[0] == '.' && exp[1] == '*') exp += 2;  /* aid grep */
#endif

	if (m_code == NULL) {
		// guess max code size in advance
		if (!ParseGrowBuf(strlen(exp)*3+8)) 
			return 0;
	}

	m_parse = (char *) exp; // ***TODO*** fix coerce
	m_subx_count = 1;
	m_code_ptr = m_code;

	ParseC(MAGIC);
	
	if (Parse(m_parse, 0, &flags) == NULL)
		return(0);

// put expression back
	m_parse = (char *) exp; // ***TODO*** fix coerce

	// grope for simple optimizations to RegExec.

	m_start = '\0';	
	m_anchor = 0;
	m_must = NULL;
	m_must_len = 0;
	scan = m_code+1;			/* First BRANCH. */

	if (OP(ParseNext(scan)) == END) {		/* Only one top-level choice. */
		scan = OPERAND(scan);

		/* Starting-point info. */
		if (OP(scan) == EXACTLY)
			m_start = *OPERAND(scan);
		else if (OP(scan) == BOL)
			m_anchor++;

		/*
		 * If there's something expensive in the r.e., find the
		 * longest literal string that must appear and make it the
		 * m_must.  Resolve ties in favor of later strings, since
		 * the m_start check works with the beginning of the r.e.
		 * and avoiding duplication strengthens checking.  Not a
		 * strong reason, but sufficient in the absence of others.
		 */

		if (flags & SPSTART) {
			longest = NULL;
			len = 0;
			for (; scan != NULL; scan = ParseNext(scan))
				if (OP(scan) == EXACTLY && ((int)(strlen(OPERAND(scan))) >= len)) {
					longest = OPERAND(scan);
					len = strlen(OPERAND(scan));
				}
			m_must = longest;
			m_must_len = len;
		}
	}

	return 1;
}

/*
 - Parse - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a bit forced, but the need to tie the tails of the branches to what
 * follows makes this easier
 */

char * CRegX::Parse(char *&parse, int paren, int *flagp)
{
	char *ret;
	char *br;
	char *ender;
	int parno;
	int flags;

	*flagp = HASWIDTH;	// default..set false if we find otherwise

	// Make an OPEN node, if parenthesized.
	if (paren) {
		if (m_subx_count >= MAX_SUBX)
			FAIL("too many ()");
		parno = m_subx_count;
		m_subx_count++;
		ret = ParseNode(OPEN+parno);
	} else
		ret = NULL;

	// Pick up the branches, linking them together.
	br = ParseBranch(parse, &flags);
	if (br == NULL)
		return(NULL);

	if (ret != NULL)
		ParseTail(ret, br);	// OPEN -> first.
	else
		ret = br;
	
	if (!(flags & HASWIDTH))
		*flagp &= ~HASWIDTH;

	*flagp |= flags & SPSTART; // complex R.E. flag

	while (*parse == '|' || *parse == '\n') {
		parse++;
		br = ParseBranch(parse, &flags);
		if (br == NULL)
			return(NULL);
		ParseTail(ret, br);	/* BRANCH -> BRANCH. */
		if (!(flags&HASWIDTH))
			*flagp &= ~HASWIDTH;
		*flagp |= flags & SPSTART;
	}

	// Make a closing node, and hook it on the end.
	ender = ParseNode((paren) ? CLOSE+parno : END);	
	ParseTail(ret, ender);

	// Hook the tails of the branches to the closing node.
	for (br = ret; br != NULL; br = ParseNext(br))
		ParseOpTail(br, ender);

	// Check for proper termination.
	if (paren && *parse++ != ')') {
		FAIL("unmatched ()");
	} else if (!paren && *parse != '\0') {
		if (*parse == ')') {
			FAIL("unmatched ()");
		} else
			FAIL("junk on end");	// "Can't happen".
		// NOTREACHED
	}

	return(ret);
}

/*
 - ParseBranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */

char * CRegX::ParseBranch(char *&parse, int *flagp)
{
	register char *ret;
	register char *chain;
	register char *latest;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	ret = ParseNode(BRANCH);
	chain = NULL;
	while (*parse != '\0' && *parse != ')' &&
	       *parse != '\n' && *parse != '|') {
		latest = ParsePiece(parse, &flags);
		if (latest == NULL)
			return(NULL);
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)	/* First piece. */
			*flagp |= flags&SPSTART;
		else
			ParseTail(chain, latest);
		chain = latest;
	}
	if (chain == NULL)	/* Loop ran zero times. */
		(void) ParseNode(NOTHING);

	return(ret);
}


/*
 - ParsePiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
char *CRegX::ParsePiece(char *&parse, int *flagp)
{
	register char *ret;
	register char op;
	register char *next;
	int flags;

	if (!ParseGrowBuf(255)) // ensure large piece can be added
		return NULL;

	ret = ParseAtom(parse, &flags);
	if (ret == NULL)
		return(NULL);

	op = *parse;
	if (!ISMULT(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != '?')
		FAIL("*+ operand could be empty");
	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

	if (op == '*' && (flags&SIMPLE)) {
		if (parse[1] == '?') {
			++parse;
			ParseInsert(STARX, ret);
		} else
			ParseInsert(STAR, ret);
	} else if (op == '*') {
		// Emit x* as (x&|), where & means "self".
		ParseInsert(BRANCH, ret);				// Either x
		ParseOpTail(ret, ParseNode(BACK));		// and loop
		ParseOpTail(ret, ret);				// back
		ParseTail(ret, ParseNode(BRANCH));		// or
		ParseTail(ret, ParseNode(NOTHING));		// null.
	} else if (op == '+' && (flags&SIMPLE)) {
		if (parse[1] == '?') {
			++parse;
			ParseInsert(PLUSX, ret);
		} else
			ParseInsert(PLUS, ret);
	} else if (op == '+') {
		// Emit x+ as x(&|), where & means "self".
		next = ParseNode(BRANCH);				/* Either */
		ParseTail(ret, next);
		ParseTail(ParseNode(BACK), ret);		/* loop back */
		ParseTail(next, ParseNode(BRANCH));		/* or */
		ParseTail(ret, ParseNode(NOTHING));		/* null. */
	} else if (op == '?') {
		/* Emit x? as (x|) */
		ParseInsert(BRANCH, ret);			/* Either x */
		ParseTail(ret, ParseNode(BRANCH));		/* or */
		next = ParseNode(NOTHING);		/* null. */
		ParseTail(ret, next);
		ParseOpTail(ret, next);
	}
	parse++;
	if (ISMULT(*parse))
		FAIL("nested *?+");

	return(ret);
}

/*
 - ParseAtom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
char *CRegX::ParseAtom(char *&parse, int *flagp)
{
	register char *ret;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	switch (*parse++) {
	/* ***TODO***: these chars only have meaning at beg/end of pat? */
	case '^':
		ret = ParseNode(BOL);
		break;
	case '$':
		ret = ParseNode(EOL);
		break;
	case '.':
		ret = ParseNode(ANY);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case '[': {
			// ***TOTO*** optimize ANYOF/ANYBUT so 
			// 1. c++ >=OP1 && c++ <=OP2
			//    is used instead of strchr for
			// 2. overlapping ranges are compressed

			register int iclass;
			register int classend;

			if (*parse == '^') {	/* Complement of range. */
				ret = ParseNode(ANYBUT);
				parse++;
			} else
				ret = ParseNode(ANYOF);
			if (*parse == ']' || *parse == '-')
				ParseC(*parse++);
			while (*parse != '\0' && *parse != ']') {
				if (*parse == '-') {
					parse++;
					if (*parse == ']' || *parse == '\0')
						ParseC('-');
					else {
						iclass = UCHARAT(parse-2)+1;
						classend = UCHARAT(parse);
						if (iclass > classend+1)
							FAIL("invalid [] range");
						if (m_sensitive) {
							for (; iclass <= classend; iclass++)
								ParseC(iclass);
						} else {
							for (; iclass <= classend; iclass++) {
								if (isalpha(iclass)){
									ParseC(tolower(iclass));
									ParseC(toupper(iclass));
								} else {
									ParseC(iclass);
								}
							}
						}
						parse++;
					}
				} else
					ParseC(*parse++);
			}
			ParseC('\0');
			if (*parse != ']')
				FAIL("unmatched []");
			parse++;
			*flagp |= HASWIDTH|SIMPLE;
		}
		break;
	case '(':
		ret = Parse(parse,1, &flags);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case '\0':
	case '|':
	case '\n':
	case ')':
		FAIL("internal urp");	/* Supposed to be caught earlier. */
		break;
	case '?':
	case '+':
	case '*':
		FAIL("?+* follows nothing");
		break;
	case '\\':
		switch (*parse++) {
		case '\0':
			FAIL("trailing \\");
			break;
		case '<':
			ret = ParseNode(WORDA);
			break;
		case '>':
			ret = ParseNode(WORDZ);
			break;
		/* ***TODO***: add support for \1, \2, ... */
		default:
			// handle general quoted chars in exact-match routine
			goto de_fault;
		}
		break;
	de_fault:
	default:
		/*
		 * Encode a string of characters to be matched exactly.
		 *
		 * This is a bit tricky due to quoted chars and due to
		 * '*', '+', and '?' taking the SINGLE char previous
		 * as their operand.
		 *
		 * On entry, the char at parse[-1] is going to go
		 * into the string, no matter what it is.  (It could be
		 * following a \ if we are entered from the '\' case.)
		 * 
		 * Basic idea is to pick up a good char in  ch  and
		 * examine the next char.  If it's *+? then we twiddle.
		 * If it's \ then we frozzle.  If it's other magic char
		 * we push  ch  and terminate the string.  If none of the
		 * above, we push  ch  on the string and go around again.
		 *
		 *  regprev  is used to remember where "the current char"
		 * starts in the string, if due to a *+? we need to back
		 * up and put the current char in a separate, 1-char, string.
		 * When  regprev  is NULL,  ch  is the only char in the
		 * string; this is used in *+? handling, and in setting
		 * flags |= SIMPLE at the end.
		 */
		{
			char *regprev;
			register char ch;

			parse--;			/* Look at cur char */
			ret = ParseNode(EXACTLY);
			for ( regprev = 0 ; ; ) {
				ch = *parse++;	/* Get current char */
				switch (*parse) {	/* look at next one */

				default:
					ParseC(ch);	/* Add cur to string */
					break;

				case '.': case '[': case '(':
				case ')': case '|': case '\n':
				case '$': case '^':
				case '\0':
				// ***TODO***, $ and ^ should not always be magic 
				magic:
					ParseC(ch);	// dump cur char 
					goto done;	// and we are done 

				case '?': case '+': case '*':
					if (!regprev) 	// If just ch in str, 
						goto magic;	// use it 
					// End mult-char string one early 
					parse = regprev; // Back up parse 
					goto done;

				case '\\':
					ParseC(ch);	// Cur char OK 
					switch (parse[1]){ // Look after '\'
					case '\0':
					case '<':
					case '>':
					// ***TODO***: Someday handle \1, \2, ... 
						goto done; // Not quoted 
					default:
						// Backup point is \, scan							 * point is after it. 
						regprev = parse;
						parse++; 
						continue;	// NOT break; 
					}
				}
				regprev = parse;	// Set backup point 
			}
		done:
			ParseC('\0');
			*flagp |= HASWIDTH;
			if (!regprev)		// One char? 
				*flagp |= SIMPLE;
		}

		break;
	}

	return(ret);
}

int CRegX::ParseGrowBuf(int n)
{
	if ((m_code_ptr - m_code + n) > m_code_size )
	{
		m_code = (char *) realloc(m_code, m_code_size += s_code_grow);
		if (m_code == NULL) FAIL("out of memory");
	}
	return 1;
}

// ParseNode - emit a node
char * CRegX::ParseNode(char op)
{
	register char *ret;
	register char *ptr;

	ret = m_code_ptr;

	ptr = ret;
	*ptr++ = op;
	*ptr++ = '\0';		/* Null "next" pointer. */
	*ptr++ = '\0';
	m_code_ptr = ptr;

	return(ret);
}

// ParseC - emit (if appropriate) a byte of code
void CRegX::ParseC(char b)
{
	*m_code_ptr++ = b;
}

// ParseInsert - insert an operator in front of already-emitted operand
//		this means relocating the operand.
void CRegX::ParseInsert(char op, char *opnd)
{
	register char *src;
	register char *dst;
	register char *place;

	src = m_code_ptr;
	m_code_ptr += 3;
	dst = m_code_ptr;
	while (src > opnd)
		*--dst = *--src;

	place = opnd;		/* Op node, where operand used to be. */
	*place++ = op;
	*place++ = '\0';
	*place++ = '\0';
}

/*
 - ParseTail - set the next-pointer at the end of a node chain
 */
void CRegX::ParseTail(char *p, char *val)
{
	register char *scan;
	register char *temp;
	register int offset;

	/* Find last node. */
	scan = p;
	for (;;) {
		temp = ParseNext(scan);
		if (temp == NULL)
			break;
		scan = temp;
	}

	if (OP(scan) == BACK)
		offset = scan - val;
	else
		offset = val - scan;
	*(scan+1) = (offset>>8)&0377;
	*(scan+2) = offset&0377;
}

// ParseOpTail - ParseTail on operand of first argument; nop if operandless
void CRegX::ParseOpTail(char *p, char *val)
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (p == NULL || OP(p) != BRANCH)
		return;
	ParseTail(OPERAND(p), val);
}

/*
	GetMatchInfo - get match information
*/
int	CRegX::GetMatchInfo(const char *&startp, const char *&endp, int nth) const
{
	if (nth <= m_subx_count && m_startp[nth])
	{
		startp = m_startp[nth];
		endp = m_endp[nth];
		return 1;
	}
	else
		return 0;
}

int CRegX::Match(const char *string)
{
	register char *s;

	// Be paranoid... */
	if (m_code == NULL || string == NULL) {
		FAIL("NULL parameter");
	}

	// Check validity of program
	if (*m_code != MAGIC) {
		FAIL("corrupted program");
	}

	// If there is a "must appear" string, look for it.
	if (m_must != NULL) {
		s = (char *)string;
		while ((s = strchr(s, m_must[0])) != NULL) {
			if (strncmp(s, m_must, m_must_len) == 0)
				break;	// Found it
			s++;
		}
		if (s == NULL)	// Not present
			return(0);
	}

	// Mark beginning of line for ^
	m_input = (char *) string;		// ***TODO*** fix coerce
	m_subx_count = -1;

	// Simplest case:  anchored match need be tried only once
	if (m_anchor)
		return(MatchTry(string));

	// Messy cases:  unanchored match
	s = (char *) string;			// ***TODO*** fix coerce
	if (m_start != '\0')
		// We know what char it must start with
		while ((s = strchr(s, m_start)) != NULL) {
			if (MatchTry(s))
				return(m_startp[0] - m_input + 1);
			s++;
		}
	else
		// We don't -- general case.
		do {
			if (MatchTry(s))
				return(m_startp[0] - m_input + 1);
		} while (*s++ != '\0');

	// Failure.
	return(0);
}


// - MatchTry match at specific point (0 failure, 1 success)

int CRegX::MatchTry(const char *string)
{
	char **sp, **ep;

	sp = m_startp;
	ep = m_endp;
	m_input_ptr = (char *) string;			// ***TODO*** fix coerce

	m_subx_count = 0;
	if (MatchX(m_code + 1)) 
	{
//		m_subx_count = max(m_subx_count, 0);
		m_startp[0] = (char *) string;		// ***TODO*** fix coerce
		m_endp[0] = m_input_ptr;
		return(string - m_input + 1);
	} 
	else
	{
		m_subx_count = -1;
		return(0);
	}
}

/*
 - MatchX - main matching routine (0 failure, 1 success)
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  

 * In practice we make some effort to avoid recursion, in particular by 
 * going through "ordinary" nodes (that don't need to know whether 
 * the rest of the match failed) with a loop
 */

int CRegX::MatchX(char *prog)
{
	register char *scan;	// cur node
	char *next;				// next node

	scan = prog;
	while (scan != NULL) {
		next = ParseNext(scan);

		switch (OP(scan)) {
		case BOL:
			if (m_input_ptr != m_input)
				return(0);
			break;
		case EOL:
			if (*m_input_ptr != '\0')
				return(0);
			break;
		case WORDA:
			// Must be looking at a letter, digit, or _
			if ((!isalnum(*m_input_ptr)) && *m_input_ptr != '_')
				return(0);
			// Prev must be BOL or nonword
			if (m_input_ptr > m_input &&
			    (isalnum(m_input_ptr[-1]) || m_input_ptr[-1] == '_'))
				return(0);
			break;
		case WORDZ:
			// Must be looking at non letter, digit, or _
			if (isalnum(*m_input_ptr) || *m_input_ptr == '_')
				return(0);
			// We don't care what the previous char was
			break;
		case ANY:
			if (*m_input_ptr == '\0')
				return(0);
			m_input_ptr++;
			break;
		case EXACTLY: {
				register int len;
				register char *opnd;

				opnd = OPERAND(scan);
				// Inline the first character, for speed.
				if (*opnd != *m_input_ptr)
					return(0);
				len = strlen(opnd);
				if (m_sensitive) {
					if (len > 1 && strncmp(opnd, m_input_ptr, len) != 0)
						return(0);
				} else {
					if (len > 1 && strnicmp(opnd, m_input_ptr, len) != 0)
						return(0);
				}
				m_input_ptr += len;
			}
			break;
		case ANYOF:
 			if (*m_input_ptr == '\0' || strchr(OPERAND(scan), *m_input_ptr) == NULL)
				return(0);
			m_input_ptr++;
			break;
		case ANYBUT:
 			if (*m_input_ptr == '\0' || strchr(OPERAND(scan), *m_input_ptr) != NULL)
				return(0);
			m_input_ptr++;
			break;
		case NOTHING:
			break;
		case BACK:
			break;
		case OPEN+1:
		case OPEN+2:
		case OPEN+3:
		case OPEN+4:
		case OPEN+5:
		case OPEN+6:
		case OPEN+7:
		case OPEN+8:
		case OPEN+9: {
				int no = OP(scan) - OPEN;
				char *save = m_input_ptr;
				if (no > m_subx_count) // subx has not been found for this paren
				{
					m_subx_count = no;
					m_startp[no] = NULL;
					m_endp[no] = NULL;
				}

				if (MatchX(next)) {
					// overwrite subx with earliest subexpression
					m_startp[no] = save;
					return(1);
				} else
					return(0);
			}
			break;
		case CLOSE+1:
		case CLOSE+2:
		case CLOSE+3:
		case CLOSE+4:
		case CLOSE+5:
		case CLOSE+6:
		case CLOSE+7:
		case CLOSE+8:
		case CLOSE+9: {
				int no = OP(scan) - CLOSE;
				char *save = m_input_ptr;

				if (MatchX(next)) {
					if (m_endp[no] == NULL) // subx has not been found 
						m_endp[no] = save;  // only save latest subexpression
					return(1);
				} else
					return(0);
			}
			break;
		case BRANCH: {
				register char *save;

				if (!next || (OP(next) != BRANCH))	// No choice.
					next = OPERAND(scan);			// Avoid recursion.
				else {
					do {
						save = m_input_ptr;
						if (MatchX(OPERAND(scan)))
							return(1);
						m_input_ptr = save;
						scan = ParseNext(scan);
					} while (scan != NULL && OP(scan) == BRANCH);
					return(0);
					// NOTREACHED
				}
			}
			break;
		case STAR:
		case PLUS: {
				register char nextch;
				register int no;
				register char *save;
				register int min;

				/*
				 * Lookahead to avoid useless match attempts
				 * when we know what character comes next.
				 */
				nextch = '\0';
				if (next && OP(next) == EXACTLY)
					nextch = *OPERAND(next);
				min = (OP(scan) == STAR) ? 0 : 1;
				save = m_input_ptr;
				no = MatchRepeat(OPERAND(scan));
				while (no >= min) {
					// If it could work, try it.
					if (m_sensitive) {
						if (nextch == '\0' || (*m_input_ptr == nextch))
							if (MatchX(next))
								return(1);
					} else {
						if (nextch == '\0' || (tolower(*m_input_ptr) == tolower(nextch)))
							if (MatchX(next))
								return(1);
					}
					// Couldn't or didn't -- back up.
					no--;
					m_input_ptr = save + no;
				}
				return(0);
			}
			break;
		case STARX:
		case PLUSX: {
				register char nextch;
				register int no;
				register char *save;
				register int max;

				/*
				 * Lookahead to avoid useless match attempts
				 * when we know what character comes next.
				 */
				nextch = '\0';
				if (next && OP(next) == EXACTLY)
					nextch = *OPERAND(next);
				no = (OP(scan) == STARX) ? 0 : 1;
				save = m_input_ptr;
				max = MatchRepeat(OPERAND(scan));
				while (no <= max) {
					m_input_ptr = save + no;
					if (m_sensitive) {
						if (nextch == '\0' || (*m_input_ptr == nextch))
							if (MatchX(next))
								return(1);
					} else {
						if (nextch == '\0' || (tolower(*m_input_ptr) == tolower(nextch)))
							if (MatchX(next))
								return(1);
					}
					// couldn't or didn't -- go forward
					no++;
				}
				return(0);
			}
			break;
		case END:
			return(1);	// Success!
			break;
		default:
			FAIL("invalid opcode");
			break;
		}

		scan = next;
	}

// We get here only if there's trouble -- normally "case END" is
	FAIL("corrupted pointers");
}

// MatchRepeat - repeatedly match something simple, report how many
int CRegX::MatchRepeat(char *p)
{
	register int count = 0;
	register char *scan;
	register char *opnd;

	scan = m_input_ptr;
	opnd = OPERAND(p);
	switch (OP(p)) {
	case ANY:
		count = strlen(scan);
		scan += count;
		break;
	case EXACTLY:
		if (m_sensitive) {
			while (*opnd == *scan) {
				count++;
				scan++;
			}
		} else {
			while (tolower(*opnd) == tolower(*scan)) {
				count++;
				scan++;
			}
		}
		break;
	case ANYOF:
		while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
			count++;
			scan++;
		}
		break;
	case ANYBUT:
		while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
			count++;
			scan++;
		}
		break;
	default:		// this function was called inappropriately.
		count = 0;	// best compromise.
		FAIL("MatchRepeat - internal foulup");
		break;
	}
	m_input_ptr = scan;

	return(count);
}

// ParseNext - dig the "next" pointer out of a node
char *CRegX::ParseNext(char *p) const
{
	register int offset;

	offset = NEXT(p);
	if (offset == 0)
		return(NULL);

	if (OP(p) == BACK)
		return(p-offset);
	else
		return(p+offset);
}

void CRegX::SetError(const char *msg)
{
	m_errmsg = (char *) realloc(m_errmsg, strlen(msg)+1);
	strcpy(m_errmsg, msg);
}

// Dump - dump a regexp
void CRegX::Dump(FILE *f) const
{
	char *s;
	char op = !END;	// not-END op.
	char *next;

	s = m_code + 1;
	while (op != END) {	/* While that wasn't END last time... */
		op = OP(s);
		fprintf(f, "%2d%s", s-m_code, DumpProp(s));	/* Where, what. */
		next = ParseNext(s);
		if (next == NULL)		/* Next ptr. */
			fprintf(f, "(0)");
		else 
			fprintf(f, "(%d)", (s-m_code)+(next-s));
		s += 3;
		if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
			/* Literal string, where present. */
			while (*s != '\0') {
				fputc(*s, f);
				s++;
			}
			s++;
		}
		fputc('\n', f);
	}

	/* Header fields of interest. */
	if (m_start != '\0')
		fprintf(f, "start `%c' ", m_start);
	if (m_anchor)
		fprintf(f, "anchored ");
	if (m_must != NULL)
		fprintf(f, "must have \"%s\"", m_must);
	fputc('\n', f);
}

// DumpProp - printable representation of opcode
char *CRegX::DumpProp(char *op) const
{
	register char *p;
	static char buf[50];

	(void) strcpy(buf, ":");

	switch (OP(op)) {
	case BOL:
		p = "BOL";
		break;
	case EOL:
		p = "EOL";
		break;
	case ANY:
		p = "ANY";
		break;
	case ANYOF:
		p = "ANYOF";
		break;
	case ANYBUT:
		p = "ANYBUT";
		break;
	case BRANCH:
		p = "BRANCH";
		break;
	case EXACTLY:
		p = "EXACTLY";
		break;
	case NOTHING:
		p = "NOTHING";
		break;
	case BACK:
		p = "BACK";
		break;
	case END:
		p = "END";
		break;
	case OPEN+1:
	case OPEN+2:
	case OPEN+3:
	case OPEN+4:
	case OPEN+5:
	case OPEN+6:
	case OPEN+7:
	case OPEN+8:
	case OPEN+9:
		sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
		p = NULL;
		break;
	case CLOSE+1:
	case CLOSE+2:
	case CLOSE+3:
	case CLOSE+4:
	case CLOSE+5:
	case CLOSE+6:
	case CLOSE+7:
	case CLOSE+8:
	case CLOSE+9:
		sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
		p = NULL;
		break;
	case STAR:
		p = "STAR";
		break;
	case PLUS:
		p = "PLUS";
		break;
	case WORDA:
		p = "WORDA";
		break;
	case WORDZ:
		p = "WORDZ";
		break;
	default:
		p = NULL;
		((CRegX *)this)->SetError("corrupted opcode"); // ***TODO*** don't foolishly override const
		break;
	}
	if (p != NULL)
		(void) strcat(buf, p);
	return(buf);
}
