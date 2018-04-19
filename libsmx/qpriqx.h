/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _QPRIQ_H
#define _QPRIQ_H

#include <memory.h>
#include <math.h>

template <class compType, class storType>
class qPriQ
{
private:
	struct posType {
		compType c;
		int i;
	} * p;

	storType *s;
	
	int N;
	int Max;

public:
	enum {
		minGrow = 4
	};

	compType compMAX;

	qPriQ(compType mItem, int mElem = 0) {
		compMAX = mItem;
		s = 0; p = 0; Max = N = 0; 
		grow(mElem);
	}

	qPriQ(qPriQ<compType> &Q) {
		compMAX = Q.compMAX;
		s = 0; p = 0; Max = N = 0; 
		operator =(Q);
	}

   ~qPriQ() {
	   clear();
	}

	bool empty() { 
		return (N <= 0); 
	}

	void clear() {
		if (a) delete a;
		if (s) delete s;
		if (p) delete s;
		a = 0; s = 0; p = 0; Max = N = 0; 
	}
	
	qPriQ<compType> &operator =(qPriQ<compType> &from) {
		clear();
		grow(from.N+1);
		int i; for (i = 1; i <= from.N; ++i) {
			p[i] = from.a[i];
			s[i] = from.s[i];
		}
		Max = N = from.N;
		compMAX = from.compMAX;
		return *this;
	}

	void     grow(int max);
	void     insert(compType v);
	compType remove();
	compType replace(compType v);
	void     upheap(int k);
	void     downheap(int k);
};

template <class compType, class compType *compMAX, class storType>
void heapsort(compType a[], int N);

template <class compType>
void qPriQ<compType>::upheap(int k)
{
    posType v;
	int i;
    v = p[k]; 
	a[0] = compMAX;
    while (p[k/2].c <= v.c){
		p[k] = p[k/2];
		k = k/2; 
	}
	p[k] = v;
}

template <class compType>
void qPriQ<compType>::grow(int newMax)
{
	if (a) {
		posType *tp = (posType *) new posType[newMax+1];
		storType *ts = (storType *) new storType[newMax+1];
		int i; for (i = 1; i <= N; ++i) {
			tp[i] = p[i];
			ts[i] = s[i];
		}
		delete p;
		delete s;
		p = tp;
		s = ts;
	} else {
		p = new posType[newMax+1]; 
	}
	Max = newMax;
}

template <class compType>
void qPriQ<compType>::insert(compType c, storType s)
{
	assert(v.c < compMAX);

	if (N >= Max)
		grow(Max < minGrow ? minGrow : Max * 2);

	p[++N].c = c;  
	  p[N].i = N;  
	  s[N]   = s;
	
	upheap(N); 
}
 
template <class compType>
void qPriQ<compType>::downheap(int k)
{
    int j; posType v;
    v = p[k];
    while (k <= N/2)
    {
        j = k+k;
        if (j<N && p[j].c<p[j+1].c) j++;
        if (v.c >= p[j].c) break;
        p[k] = p[j]; k = j;
    }

    p[k] = v;
}
 
template <class compType>
storType qPriQ<compType>::remove()
{
    posType v = p[1];
    p[1] = p[N--];
    downheap(1);
    return s[v.i];
}
 
template <class compType>
compType qPriQ<compType>::replace(compType v)
{
    a[0] = v;
    downheap(0);
    return a[0];
}
 
template <class compType>
void heapsort(compType a[], int N)
{
    int i; qPriQ heap(N);    
	
	for (i = 1; i <= N; i++) 
		heap.insert(a[i]);    

    for (i = N; i >= 1; i--) 
		a[i] = heap.remove();    
}

#endif // _QPRIQ_H