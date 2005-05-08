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

	CLst<TYPE> *&Next() const               { return (CLst<TYPE> *&) m_next; }
	CLst<TYPE> *Link(const TYPE &e) const  { return new CLst(e, this); }
	
	CLst<TYPE> *Delete(CLst<TYPE> &)	   { const CLst<TYPE> *tmp = m_next; if (m_next) {*this = *m_next; delete m_next; return this; } else delete this; return 0; }
	CLst<TYPE> *DeleteTop()				   { const CLst<TYPE> *tmp = m_next; delete this; return (CLst<TYPE> *) tmp; }
};

#endif// #ifndef _LST_H