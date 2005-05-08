/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
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
		storType s;
		int i;
	} * p;

	int N;
	int Max;

public:
	enum {
		minGrow = 4
	};

	compType compMAX;

	qPriQ(compType mComp, int mElem = 0) {
		compMAX = mComp;
		p = 0; Max = N = 0; 
		grow(mElem);
	}

	qPriQ(qPriQ<compType, storType> &Q) {
		compMAX = Q.compMAX;
		p = 0; Max = N = 0; 
		operator =(Q);
	}

   ~qPriQ() {
	   clear();
	}

	void destruct() {
		delete this;
	}

	bool empty() { 
		return (N <= 0); 
	}
	
	int count() { 
		return (N); 
	}

	void clear() {
		if (p) delete[] p;
		p = 0; Max = N = 0; 
	}
	
	qPriQ<compType, storType> &operator =(qPriQ<compType, storType> &from) {
		clear();
		grow(from.N+1);
		int i; for (i = 1; i <= from.N; ++i) {
			p[i] = from.p[i];
		}
		Max = N = from.N;
		compMAX = from.compMAX;
		return *this;
	}

	void     grow(int max);
	void     insert(compType v, storType x);
	storType remove(int i=1);
	storType peek();
	storType replace(compType v, storType x);
	void     upheap(int k);
	void     downheap(int k);
	bool     getnext(int i, compType *v, storType *x) {
		if (i <= N) {
			*v = p[i].c;
			*x = p[i].s;
			return true;
		} else
			return false;
	}

};

template <class compType, compType *compMAX, class storType>
void heapsort(compType a[], int N);

template <class compType, class storType>
void qPriQ<compType, storType>::upheap(int k)
{
    posType v;
    v = p[k]; 
	p[0].c = compMAX;
    while (p[k/2].c <= v.c){
		p[k] = p[k/2];
		k = k/2; 
	}
	p[k] = v;
}

template <class compType, class storType>
void qPriQ<compType, storType>::grow(int newMax)
{
	if (p) {
		posType *tp = new posType[newMax+1];
//		storType *ts = new storType[newMax+1];
		int i; for (i = 1; i <= N; ++i) {
			tp[i] = p[i];
		}
		delete[] p;
		p = tp;
	} else {
		p = new posType[newMax+1]; 
	}
	Max = newMax;
}

template <class compType, class storType>
void qPriQ<compType, storType>::insert(compType c, storType x)
{
	assert(c < compMAX);

	if (N >= Max)
		grow(Max < minGrow ? minGrow : Max * 2);

	p[++N].c = c;  
	  p[N].s = x;  

	upheap(N); 
}
 
template <class compType, class storType>
void qPriQ<compType, storType>::downheap(int k)
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
 
template <class compType, class storType>
storType qPriQ<compType, storType>::peek()
{
    return p[1].s;
}

template <class compType, class storType>
storType qPriQ<compType, storType>::remove(int i)
{
    posType v = p[i];
    p[i] = p[N--];
    downheap(i);
    return v.s;
}

template <class compType, class storType>
storType qPriQ<compType, storType>::replace(compType v, storType x)
{
    a[0].c = v;
	a[0].s = x;
    downheap(0);
    return x;
}
 
template <class compType, class storType>
void heapsort(compType a[], int N)
{
    int i; qPriQ<compType,storType> heap(N);    
	
	for (i = 1; i <= N; i++) 
		heap.insert(a[i]);    

    for (i = N; i >= 1; i--) 
		a[i] = heap.remove();    
}

#endif // _QPRIQ_H
