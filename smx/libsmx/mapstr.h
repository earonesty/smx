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
