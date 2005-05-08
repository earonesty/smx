/* COPYRIGHT 1998 Prime Data Corp.
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "fbuf.h"

int CFBuf::s_block_size = 1024;
int CFBuf::s_alloc_extra = 256;

CFBuf::CFBuf() {
       m_p1 = m_p2 = m_a1 = m_rp = (char *) malloc(s_block_size);
       m_a2 = m_a1 + s_block_size;
       m_ax = s_block_size;
       m_file = NULL;
}


CFBuf::~CFBuf() {
    free(m_a1);
}

int CFBuf::AllocR(int count, char * const base) {
    assert(m_file != NULL);
    if (count > (m_rp - base)) {
        if (count > (m_a2 - base)) {
            // calculate alloc size
            m_ax += count - (m_a2 - base) + s_alloc_extra;
            m_ax = s_block_size * (1 + (m_ax / s_block_size));
            m_a2 = m_a1 + m_ax;
            
            // reallocate & calculate offset
            char *old_a1 = m_a1;
            m_a1 = (char *) realloc(m_a1, m_ax);

            // no mem
            assert(m_a1 != NULL);

            long  offset = m_a1 - old_a1;

            // offset all pointers
            m_a2 += offset; 
            m_p1 += offset; 
            m_p2 += offset; 
            m_rp += offset; 

            // assert pointer arith (must have flat addr)
            assert(m_a2 == (m_a1 + m_ax));
        }
        m_rp += fread(m_rp, 1, count - (m_rp - base), m_file);
        count = m_rp - base;
        base[count] = '\0'; // terminate after read
    }
    return count;
}

void CFBuf::Skip(int n) {
    Read(n);
}

char *CFBuf::Read(int &n) {
    n = AllocR(n, m_p1);

    if (n >= 0) {
        // save pointer (return value)
        char *r_p1 = m_p1;

        // move up
        m_p2 = m_p1 += n;

        if ((m_p1 - m_a1) > (m_a2 - m_p1)) {
                // shift down & calculate offset
                long offset = m_a1 - r_p1;
                memcpy(m_a1, r_p1, m_rp - r_p1);

                // offset all non-alloc pointers (inc. return value)
                m_p1 += offset;
                m_p2 += offset;
                m_rp += offset;
                r_p1 += offset;
        }

        // return saved pointer
        return r_p1;
    } else {
        return NULL;
    }
}

char CFBuf::Peek(int off) {
    int n = AllocR(off, m_p2);
    if (n >= off)
        return m_p2[off - 1];
    else
        return '\0';
}

int CFBuf::Seek(char delim) {
    do {
        while (m_p2 < m_rp) {
            if (*m_p2 == delim) {
                ++m_p2;
                goto Seek_C;
            }
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));

Seek_C:
    return m_p2 - m_p1;
}

int CFBuf::Seek(int *seek_f(char c)) {
    do {
        while (m_p2 < m_rp) {
            if (seek_f(*m_p2)) {
                ++m_p2;
                goto Seek_C;
            }
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));

Seek_C:
    return m_p2 - m_p1;
}

int CFBuf::Seek(char *delims, char &stop) {
    char *bdelim;
    stop = '\0';
    do {
        while (m_p2 < m_rp) {
            bdelim = delims;
            while (*bdelim) {
                if (*bdelim == *m_p2)  {
                    stop = *bdelim;
                    ++m_p2;
                    goto Seek_Cs;
                }
                ++bdelim;
            }
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));

Seek_Cs:
    return m_p2 - m_p1;
}

int CFBuf::Seek(int &n) {
    n = AllocR(n, m_p2);

    if (n >= 0) {
        m_p2 += n;
    }

    return m_p2 - m_p1;
}


int CFBuf::Span(char set, char &stop) {
    stop = '\0';
    do {
        while (m_p2 < m_rp) {
            if (*m_p2 != set) {
                stop = *m_p2;
                goto Span_C;
            }
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));

Span_C:
    return m_p2 - m_p1;
}

int CFBuf::Span(int *seek_f(char c), char &stop) {
    stop = '\0';
    do {
        while (m_p2 < m_rp) {
            if (!seek_f(*m_p2)) {
                stop = *m_p2;
                goto Span_C;
            }
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));

Span_C:
    return m_p2 - m_p1;
}

int CFBuf::SpanWS(char &stop) {
    stop = '\0';
    do {
        while (m_p2 < m_rp) {
            if (!isspace(*m_p2) || (*m_p2 == '\n')) {
                stop = *m_p2;
                goto Span_C;
            }
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));
Span_C:
    return m_p2 - m_p1;
}

int CFBuf::Span(char *set, char &stop) {
    char *bset;
    stop = '\0';
    do {
        while (m_p2 < m_rp) {
            bset = set;
            while (*bset) {
                if (*bset == *m_p2)  {
                    goto Span_Ok;
                }
                ++bset;
            }
            stop = *m_p2;
            goto Span_Cs;
Span_Ok:
            ++m_p2;
        }
    } while (AllocR(s_alloc_extra, m_p2));

Span_Cs:
    return m_p2 - m_p1;
}
