/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


// fbuf.h


// File Buffer class helps parse files as-you-read them
//      scanning/spanning functions like strchr,strpbrk ... on a file
//      as-far-as-you-need look-ahead pointers
//      automatic buffer allocation/recycle/deallocate

class CFBuf {
protected:
    static int s_alloc_extra;
    static int s_block_size;

// buffer
	char*	m_a1;       // alloc base
    char*   m_a2;       // alloc end
	int     m_ax;       // alloc amount

// pointers
	char*	m_p1;       // scan base
	char*	m_p2;       // scan lookahead
    char*	m_rp;       // read pointer

// file
	FILE*	m_file;

// allocate and read count chars offset from base char *, return actual count
    int AllocR(int count, char * const base); 
    
public:
	~CFBuf ();
	CFBuf ();

    void Attach(FILE *f)    {m_file = f;}

// true if the file & any read-ahead is empty 
	int Eof() const      {return feof(m_file) && (m_rp - m_p1 <= 0);}

// seek "n" or seek for delimiters, saves pointer2, returns length
    int Seek(char delim);                 // seek for a char
    int Seek(char *delims, char &stop);   // seek for a list of delimiters
    int Seek(int &n);                   // seek # of chars
    int Seek(int *seek_f(char c));      // seek using seek_f() for "is-a-delim"

// spanning, save pointer2 / return pointer2-pointer1
    int Span(char set, char &stop);     
    int Span(char *set, char &stop); 
    int Span(int *seek_f(char c), 
        char &stop);        // like span but uses seek_f() for in-set

// span whitespace
    int SpanWS(char &stop);    // span using isspace()

// look-ahead, return char or null
    char Peek(int off = 1); 

// the following operations may cause a recycle & invalidate previously returned pointers
    char *Read(int &n);     // read, pointer1 += n, return pointer, assign n
    void Skip(int n);       // like read, but discard results

};
