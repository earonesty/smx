#ifndef _GDFX_H_
#include "gdfx.h"
#endif _GDFX_H_

gdFontPtr fontLoadText(FILE *fp, int allow_x);
gdFontPtr fontLoadBinary(FILE *fp);
gdFontPtr fontLoad(const char *fpath, int allow_x);
gdFontPtr fontLoad(FILE *fp, const char *fpath, int allow_x);
