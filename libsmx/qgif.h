/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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
