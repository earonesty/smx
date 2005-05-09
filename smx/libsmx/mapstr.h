/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _MAPSTR_H
#define _MAPSTR_H

#ifndef _MAP_H
	#include "map.h"
#endif

#ifndef _STR_H
	#include "str.h"
#endif

class HashCaseCharPtr {
	public:
		unsigned int operator()(char const *str) const {
			std::string s = str;
			strlwr(&(s[0]));
			return STD::hash<char const *>()(s.c_str());
		}
};

class EqualCaseCharPtr {
	public:
		bool operator()(char const *x, char const *y) const {
		return !stricmp(x, y);
		}
};


// simple mapping class
template<class DATA> class CMapStr : public CMap<CStr, const char *, DATA, HashCaseCharPtr, EqualCaseCharPtr>
{
};


#endif // #ifndef _MAP_H
