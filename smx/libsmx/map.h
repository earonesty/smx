/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

#include "hash.h"

typedef void * MAPPOS;

template<class KEY, class ARG_KEY, class DATA, hash_fun_t hashfun = NULL, hash_comp_t compfun = NULL> class CMap 
{
	hash_t * myH;

	static hnode_t *MapAlloc(void *context)
	{
	    hnode_t *node = new(hnode_t);
	    node->hash_data=NULL;
	    node->hash_key=NULL;
	    return node;
	}

	static void MapFree(hnode_t *node, void *context)
	{
	    if (node->hash_data) delete (DATA *) node->hash_data;
	    if (node->hash_key) delete (KEY *) node->hash_key;
	    delete node;
	}

public:
// initialization
	~CMap() {
		hash_destroy(myH);
	};
	CMap()  {
		myH = hash_create(HASHCOUNT_T_MAX, 0, 0);
		hash_set_allocator(myH, &MapAlloc, &MapFree, this);
	}

// maintenance
	void  Clear() {
		hash_free_nodes(myH);
	}

// enumeration
	MAPPOS First() {
		hscan_t *iter = new hscan_t; 
		hash_scan_begin(iter, myH);
		return (MAPPOS) iter;
	}

	bool   Next(MAPPOS *pos, KEY *key, DATA *data) {
		hnode_t *node = hash_scan_next(*(hscan_t **)pos);
		if (node) {
			key = (KEY *) hnode_getkey(node);
			data = (DATA *) hnode_get(node);
			return true;
		} else {
			return false;
		}
	}
	bool   Next(MAPPOS *pos, KEY *key, DATA **data) {
		hnode_t *node = hash_scan_next(*(hscan_t **)pos);
		if (node) {
			key = hnode_getkey(node);
			data = &((DATA *)node->data);
			return true;
		} else {
			return false;
		}
	}
	void   Enum(bool (*MapEnumProc) (void *other, KEY &key, DATA &data), void *other=NULL) {
		hscan_t hs;
		hnode_t *node;
		hash_scan_begin(&hs, myH);
		while ((node = hash_scan_next(&hs))) {
			MapEnumProc(other, * (KEY *) node->key, * (DATA *) node->data);
		}
	}

	int  Count() {
		return hash_count(myH);
	}

	const DATA *Nf() {
		return NULL;
	}

// hash
	DATA *Find(ARG_KEY k) {
		hnode_t *node = hash_lookup(myH, k);
		return node ? ((DATA *)hnode_get(node)) : ((DATA *) Nf());
	}

	bool Find(ARG_KEY k, DATA &d) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) d = *((DATA *)hnode_get(node));
		return node ? true : false;
	}

	bool Find(ARG_KEY k, DATA **d) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) d = (DATA **) &(node->hash_data);
		return node ? true : false;
	}

	bool  Del(ARG_KEY k, DATA &d) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) {
			d = *(DATA *)hnode_get(node);
			hash_delete(myH, node);
			return true;
		} else {
			return false;
		}
	}

	const DATA &Set(ARG_KEY k, const DATA &d) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) {
			return (* (DATA *) hnode_get(node)) = d;
		} else {
			return Add(k) = d;
		}
	}

	DATA &Set(ARG_KEY k, const DATA &d, DATA *old) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) {
			*old = (* (DATA *) hnode_get(node));
			return (* (DATA *) hnode_get(node)) = d;
		} else {
			return Add(k) = d;
		}
	}

	bool  Del(ARG_KEY k, DATA *d = 0) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) {
			if (d) *d = * (DATA *) hnode_get(node);
			hash_delete(myH, node);
			return true;
		} else {
			return false;
		}
	}

	DATA &operator [](ARG_KEY k) {
		hnode_t *node = hash_lookup(myH, k);
		if (node) {
			return * (DATA *) hnode_get(node);
		} else {
		    KEY *key = new KEY;
		    *key = k;
		    hnode_t *node = myH->hash_allocnode(myH->hash_context);
		    hnode_init(node, new DATA);
		    hash_insert(myH, node, key);
		    return * (DATA *) hnode_get(node);
		}
	}

	DATA &Add(ARG_KEY k) {
		return (*this)[k];
	}
	DATA &Add(ARG_KEY k, DATA &old) {
		return Add(k,&old);
	}
	DATA &Add(ARG_KEY k, DATA *old) {
                hnode_t *node = hash_lookup(myH, k);
                if (node) {
                        *old = *(DATA *) hnode_get(node);
                        return (DATA *) hnode_get(node);
                } else {
                        return Add(k);
                }
	}
};

#endif // #ifndef _MAP_H
