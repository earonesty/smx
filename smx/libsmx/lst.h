/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef _LST_H
#define _LST_H

// template for very simple singly linked list

template<class TYPE>
class CLst {
// buffer
	const CLst<TYPE> *m_next;
	TYPE	          m_elem;

public:
	CLst()                                 { m_next = 0; }
	CLst(const TYPE &e)				       { m_elem = e; m_next = 0; }
	CLst(const TYPE &e, const CLst *next)  { m_elem = e; m_next = next; }

// inline
	operator TYPE &() const                { return m_elem; }
	TYPE &Data() const					   { return (TYPE &)m_elem; }

	CLst<TYPE> *&Next() const              { return (CLst<TYPE> *&) m_next; }
	CLst<TYPE> *Link(const TYPE &e) const  { return new CLst(e, this); }
	CLst<TYPE> *New(const TYPE &e) const   { return new CLst(e); }
	
	CLst<TYPE> *Delete()	               { const CLst<TYPE> *tmp = m_next; if (m_next) {*this = *m_next; delete (CLst<TYPE>*) m_next; return this; } else delete this; return 0; }
	CLst<TYPE> *DeleteTop()				   { const CLst<TYPE> *tmp = m_next; delete this; return (CLst<TYPE> *) tmp; }
};

#endif// #ifndef _LST_H
