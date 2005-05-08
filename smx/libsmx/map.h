/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
#ifndef _MAP_H
#define _MAP_H

#ifndef _QLIB_H
#include "qlib.h"
#endif

#ifndef _INC_STRING
	#include <string.h>
	#define _INC_STRING
#endif

#ifndef _INC_ASSERT
	#include <assert.h>
	#define _INC_ASSERT
#endif

// simple mapping class

#ifndef MAP_GROW
	#define MAP_GROW 8
#endif

typedef void * MAPPOS;

#ifdef __GNUC__
#define STD __gnu_cxx
#else
#define STD std
#endif

template<class KEY, class ARG_KEY, class DATA, class HASH = STD::hash<KEY>, class COMP = STD::equal_to<KEY> > class CMap 
	: public STD::hash_map<KEY, DATA, HASH, COMP>
{

public:
// initialization
	~CMap() {clear();};
	CMap()  {}

// maintenance
	void  Clear() {clear();}

// enumeration
	typedef typename CMap<KEY, ARG_KEY, DATA, HASH, COMP>::iterator MYITER;

	MAPPOS First() {MYITER *iter = new MYITER; *iter = begin(); return (MAPPOS) iter;}

	bool   Next(MAPPOS *pos, KEY *key, DATA *data) {
		DATA *tmp; 
		return Next(pos, key, &tmp) ? (*data = *tmp, true) : false;
	}
	bool   Next(MAPPOS *pos, KEY *key, DATA **data) {
		MYITER *iter = * (MYITER **) pos;
		if (*iter != end()) {
			*key = (*iter)->first;
			*data = &(*iter)->second;
			++(*iter);
			*pos = (MAPPOS) iter;
			return true;
		} else {
			delete iter;
			return false;
		}
	}
	void   Enum(bool (*MapEnumProc) (void *other, KEY &key, DATA &data), void *other=NULL) {
		MYITER iter;
		for( iter = begin(); iter != end(); ++iter  ) { 
			MapEnumProc(other, iter->first, iter->second);
		}
	}

	int  Count() {
		return size();
	}

	const DATA *Nf() {
		return NULL;
	}

// hash
	DATA *Find(ARG_KEY k) {
		MYITER iter = find(k);
		return (iter != end()) ? (&iter->second) : ((DATA *) Nf());
	}

	bool Find(ARG_KEY k, DATA &d) {
		DATA *pd; return Find(k, &pd) ? ((d = *pd), true) : false;
	}

	bool Find(ARG_KEY k, DATA **d) {
                MYITER iter = find(k);
                if (iter != end()) {
			*d = &iter->second;
			return true;
		} else {
			return false;
		}
	}

	bool  Del(ARG_KEY k, DATA &d) {
                MYITER iter = find(k);
		if (iter != end()) {
			d = iter->second;
			erase(iter);
			return true;
		} else {
			return false;
		}
	}

	const DATA &Set(ARG_KEY k, const DATA &d) {
                MYITER iter = find(k);
                if (iter != end()) {
                        return iter->second = d;
                } else {
                        return Add(k) = d;
                }
	}

	DATA &Set(ARG_KEY k, const DATA &d, DATA *old) {
		MYITER iter = find(k);
                if (iter != end()) {
			*old = iter->second;
                        return iter->second = d;
                } else {
                        return (*this)[k]=d;
                }
	}

	bool  Del(ARG_KEY k, DATA *d = 0) {
                MYITER iter = find(k);
                if (iter != end()) {
                        if (d) *d = iter->second;
                        erase(iter);
                        return true;
                } else {
                        return false;
                }
	}

	DATA &Add(ARG_KEY k) {
		return (*this)[k];
	}
	DATA &Add(ARG_KEY k, DATA &old) {
		return Add(k,&old);
	}
	DATA &Add(ARG_KEY k, DATA *old) {
                MYITER iter = find(k);
                if (iter != end()) {
                        *old = iter->second;
                        return iter->second;
                } else {
                        return Add(k);
                }
	}
};

#endif // #ifndef _MAP_H
