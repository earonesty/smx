#define LPFONT void *
#ifndef DLL
	#define DLL extern "C" __declspec( dllimport ) 
#endif
#ifndef GDPIX
	#define GDPIX short
#endif
#ifndef SEEK_SET
	#include <stdio.h>
#endif
#ifndef COLORREF
	#define COLORREF  long
#endif
#ifndef BOOL
	#define BOOL      int
#endif

#include "qgargs.h"

DLL void qGifCaption(const char *opt, CAPTION *li);
DLL void qGifLabel(const char *opt, LABELINFO *li);
DLL COLORREF qGifRGB(const char *opt);
DLL int qGifDraw(argStruct *args);
DLL int qGifDrawC(argStruct *args, FILE *in, FILE *out, FILE *tpl);
DLL int qGif(int argc, char **argv);
DLL int qGifArgs(int argc, char **argv, argStruct *args);
