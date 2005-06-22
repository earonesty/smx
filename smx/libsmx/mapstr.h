/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _MAPSTR_H
#define _MAPSTR_H

#include "map.h"
#include "str.h"

hash_val_t HashCaseCharPtr(const void *key);
int EqualCaseCharPtr (const void *x, const void *y);

// string mapping class
template<class DATA> class CMapStr : public CMap<const char *, DATA>
{
        static void MapFree(hnode_t *node, void *context)
        {
            if (node->hash_data) delete (DATA *) node->hash_data;
            if (node->hash_key) delete[] (char *)node->hash_key;
            delete node;
        }
public:
	void *CopyKey(const char * k) {
		char *k2 = new char[strlen(k)+1];
		strcpy(k2, k);
		return (void *)k2;
	}
        CMapStr() : CMap<const char *, DATA>(HashCaseCharPtr, EqualCaseCharPtr) {
                hash_set_allocator(myH, MapAlloc, MapFree, this);
        }
};

#endif // #ifndef _MAP_H
