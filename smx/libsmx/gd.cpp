#include "unix.h"

#ifdef HAVE_LIBGD

#include <gd.h>
#include <math.h>

#include "crit.h"

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "util.h"

typedef struct
{
	int r;
	int g;
	int b;
} imgClr;

#define MAXVERT 20
#define MAXIMG 10000
#define MAXDASH 100
#define MAXIMGTEXT 1000

enum imgType
{
	imgGif,
	imgJpeg,
	imgPng
};

class imgOpts
{
public:
	imgOpts();
	imgOpts(imgOpts *def);
	bool Parse(char *str);

	CStr font;
	int pcnt;
	gdPoint pts[MAXVERT];
	double angle;
	double ptsize;
	imgClr fg;
	imgClr bg;
	bool fill;
	int dash;
	int thick;
	imgType type;
};

typedef struct
{
	gdImagePtr im;
	imgOpts	opts;
} imgData;

char * ParseColor(char *p, imgClr *clr)
{
	char *e;
	char *b = p;

	if (!strnicmp(p, "white", 5)) {
		clr->r=255;
		clr->g=255;
		clr->b=255;
		p+=5;
	}
	if (!strnicmp(p, "black", 5)) {
		clr->r=0;
		clr->g=0;
		clr->b=0;
		p+=5;
	}
	if (!strnicmp(p, "red", 3)) {
		clr->r=255;
		clr->g=0;
		clr->b=0;
		p+=3;
	}
        if (!strnicmp(p, "blue", 4)) {
                clr->r=0;
                clr->g=0;
                clr->b=255;
                p+=4;
        }
	if (*p=='#' && isxdigit(p[1])) {
		unsigned c = strtol(p, &e, 16);
		clr->r=(0xFF0000&c)>>2;
		clr->g=(0x00FF00&c)>>1;
		clr->b=(0x0000FF&c);
		p = e;
	}

	return p;
}

#define deg2rad(x)  (0.0174532925*(x))
#define rad2deg(x)  (57.2957795*(x))

imgOpts::imgOpts() 
{
// inherited attributes
	fg.r = fg.g = fg.b = 0;
	bg.r = bg.g = bg.b = 255;
	ptsize=16;
	dash=0;
	thick=1;
	type=imgGif;

// noninherited
	angle=0.0;
    	pcnt=0;
	fill=false;
}

imgOpts::imgOpts(imgOpts *def) 
{
// inherited attributes
        fg = def->fg;
        bg = def->bg;
        ptsize=def->ptsize;
        dash=def->dash;
        thick=def->thick;
	type=def->type;

// noninherited
        angle=0.0;
        pcnt=0;
        fill=false;
}

bool imgOpts::Parse(char *str)
{
// font:arial 5,6 
	char *p = str;
	char *e;

	if (!p) return false;

	bool ret = false;

    while (*p) {
	while (isspace(*p)) ++p;

	if (!strnicmp(p, "font:", 5)) {
		p+=5;
		while (isspace(*p)) ++p;
		e = p;
		while (*e && !isspace(*e)) ++e;
		*e = '\0';
		font = p;
		p = e +1;
		ret = true;
        } else if (!strnicmp(p, "dashed", 6)) {
                p+=6;
                dash=4;
		ret = true;
        } else if (!strnicmp(p, "dotted", 6)) {
                p+=6;
                dash=1;
		ret = true;
        } else if (!strnicmp(p, "jpeg", 4)) {
                p+=4;
                type=imgJpeg;
		ret = true;
        } else if (!strnicmp(p, "png", 3)) {
                p+=3;
                type=imgPng;
		ret = true;
        } else if (!strnicmp(p, "gif", 3)) {
                p+=3;
                type=imgGif;
		ret = true;
        } else if (!strnicmp(p, "dash:", 5)) {
                p+=5;
                dash=strtol(p,&e,10);
		if (dash > MAXDASH) 
			dash=0;
                p=e;
		ret = true;
        } else if (!strnicmp(p, "thick:", 6)) {
                p+=6;
                thick=strtol(p,&e,10);
		if (thick > MAXIMG) 
			thick=1;
                p=e;
		ret = true;
	} else if (!strnicmp(p, "fg:", 3) || !strnicmp(p, "color:", 6)) {
		while (*p != ':') ++p;
		while (isspace(*p)) ++p;
		e = p;
		while (*e && !isspace(*e)) ++e;
                ParseColor(p, &fg);
		p = e+1;
		ret = true;
	} else if (!strnicmp(p, "bg:", 3) || !strnicmp(p, "fill:", 5)) {
                while (*p != ':') ++p;
		while (isspace(*p)) ++p;
		e = p;
		while (*e && !isspace(*e)) ++e;
                ParseColor(p, &bg);
		p = e+1;
		fill=true;
		ret = true;
        } else if (isdigit(*p) || (*p=='-' && isdigit(p[1]))) {
		double d = strtod(p, &e);
		while(isspace(*e)) ++e;
		if (!strnicmp(e, "pt", 2)||!strnicmp(e, "point", 5)) {
			ptsize=d; p = e;
			while(*p && !isspace(*p)) ++p;
			ret = true;
		} else if (!strnicmp(e, "deg", 3)) {
			angle=d; p = e;	
			while(*p && !isspace(*p)) ++p;
			ret = true;
		} else if (!strnicmp(e, "rad", 3)) {
			angle=rad2deg(d); p = e;	
			while(*p && !isspace(*p)) ++p;
			ret = true;
		} else {
			int l = strtol(p, &e, 10);
			while(isspace(*e)) ++e;
			p = e;
			if (*p == 'x' || *p == ',' || *p == '.') {
				if (pcnt < MAXVERT)
					pts[pcnt].x=l;
				++p;
				while(isspace(*p)) ++p;
				if (pcnt < MAXVERT)
					pts[pcnt].y=strtol(p, &e, 0);
				p = e;
				if (pcnt < MAXVERT)
					++pcnt;
				ret = true;
			}
		}
        } else {
		e = ParseColor(p, &fg);
		if (e > p) {
			p = e; 
			ret = true;
		} else { 
			while(*p && !isspace(*p))
			++p;
		}
	}
    }

    return ret;
}

void EvalImageSX(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (((imgData *)data)->im) out->PutN(gdImageSX(((imgData *)data)->im));
}
void EvalImageSY(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (((imgData *)data)->im) out->PutN(gdImageSY(((imgData *)data)->im));
}

bool CreateImageFromOpts(imgOpts *opts, imgData *pdat)
{
	int i;
	int minx =opts->pts[0].x, maxx=opts->pts[0].x, miny=opts->pts[0].y, maxy=opts->pts[0].y;

	for (i=1;i<opts->pcnt;++i) {
		if (opts->pts[i].x < minx) minx = opts->pts[i].x;
		if (opts->pts[i].x > maxx) maxx = opts->pts[i].x;
		if (opts->pts[i].y < miny) miny = opts->pts[i].y;
		if (opts->pts[i].y > maxy) maxy = opts->pts[i].y;
	}
	if (maxx > minx && maxy > miny) {
		pdat->im = gdImageCreate(maxx-minx,maxy-miny);
		if (pdat->im)
			gdImageColorResolve(pdat->im, opts->bg.r, opts->bg.g, opts->bg.b);
		return pdat->im!=NULL;
	}
}

int StyleResolve(gdImagePtr im, imgOpts *opts)
{
	int fg = gdImageColorResolve(im, opts->fg.r, opts->fg.g, opts->fg.b);
	if (opts->thick > 0) 
		gdImageSetThickness(im, opts->thick);	
	if (opts->dash==0) return fg;
	if (opts->dash>MAXDASH) return fg;
	int *dash=(int *)malloc(opts->dash*2*sizeof(int));
	int i;
	for (i=0;i<opts->dash;++i) 
		dash[i]=fg;
	for (;i<opts->dash*2;++i) 
		dash[i]=gdTransparent;
	gdImageSetStyle(im, dash, opts->dash*2);
	return gdStyled;
}

void EvalImagePoly(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        imgData *pdat = (imgData *) data;

        imgOpts opts(&pdat->opts);

	int i;
	for (i=0;i<(args->Count()-1) && opts.pcnt < MAXVERT;i+=2) {
		opts.pts[opts.pcnt].x=ParseInt((*args)[i]);
		opts.pts[opts.pcnt++].y=ParseInt((*args)[i+1]);
	}
	if (i < args->Count()) 
		opts.Parse((*args)[i]);

	if (!pdat->im)
		if (!CreateImageFromOpts(&opts, pdat))
			return;

	int fg;

	if (opts.pcnt < 2) return;

	if (opts.fill)
		fg = gdImageColorResolve(pdat->im, opts.bg.r, opts.bg.g, opts.bg.b);
	else
		fg = StyleResolve(pdat->im, &opts);
	
	if (opts.pcnt>2) {
		if (opts.fill)
			gdImageFilledPolygon(pdat->im, opts.pts, opts.pcnt, fg);
		else
			gdImagePolygon(pdat->im, opts.pts, opts.pcnt, fg);
	} else {
		gdImageLine(pdat->im, opts.pts[0].x, opts.pts[0].y, opts.pts[1].x, opts.pts[1].y, fg);
	}
}

void EvalImageRect(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
        imgData *pdat = (imgData *) data;

        imgOpts opts(&pdat->opts);

        int i;
        for (i=0;i<(args->Count()-1) && opts.pcnt < MAXVERT;i+=2) {
                opts.pts[opts.pcnt].x=ParseInt((*args)[i]);
                opts.pts[opts.pcnt++].y=ParseInt((*args)[i+1]);
        }
	if (i < args->Count()) 
		opts.Parse((*args)[i]);

	if (opts.pcnt == 2 && args->Count() == 3) {
		// interpret opts as "size" 
		opts.pts[1].x=opts.pts[0].x+opts.pts[1].x;
		opts.pts[1].y=opts.pts[0].y+opts.pts[1].y;
	}

	if (!opts.pcnt == 2) {
		return;
	}

        if (!pdat->im)
                if (!CreateImageFromOpts(&opts, pdat))
                        return;

	int fg;

	if (opts.fill)
		fg = gdImageColorResolve(pdat->im, opts.bg.r, opts.bg.g, opts.bg.b);
	else
		fg = StyleResolve(pdat->im, &opts);
	
        if (opts.fill) {
                gdImageFilledRectangle(pdat->im, opts.pts[0].x, opts.pts[0].y, opts.pts[1].x, opts.pts[1].y, fg);
        } else {
                gdImageRectangle(pdat->im, opts.pts[0].x, opts.pts[0].y, opts.pts[1].x, opts.pts[1].y, fg);
        }
}

void EvalImageText(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("text", 1, 2);

	imgData *pdat = (imgData *) data;

        CStr text = (*args)[0];
        CStr optstr = (*args)[1];

        imgOpts opts(&pdat->opts);

	opts.Parse(optstr);

	if (opts.font.IsEmpty()) 
		opts.font="sans";

	if (text.Count() > MAXIMGTEXT)
		text.Grow(MAXIMGTEXT);

	if (text.Count() <= 0)
		return;

	int x=3, y=3;
	int brect[8] = {0,0,0,0,0,0,0,0};

	char *err = gdImageStringFT(NULL,brect,0,opts.font,opts.ptsize,deg2rad(opts.angle),0,0,text);

	if (err) {
		ctx->ThrowF(out, 301, "error: %s");
		return;
	}


	int ox, oy, sx, sy;

	//printf("%d,%d %d,%d %d,%d %d,%d\n",brect[0],brect[1],brect[2],brect[3],brect[4],brect[5],brect[6],brect[7]);

	// brect is LL[01], LR[23], UR[45], UL[67]
	double angle = opts.angle - 360 * trunc(opts.angle / 360.0);
	if (angle < 0) angle += 360;

	// which corner is the offset corner?
	if (angle < 90) {
		// x:ul, y:ur
		ox = brect[6];
		oy = brect[5];
		sx = brect[2]-brect[6];
		sy = brect[1]-brect[5];
	} else if (angle >= 90 && angle < 180) {
		// x:ur, y:lr
		ox = brect[4];
		oy = brect[3];
		sx = brect[0]-brect[4];
		sy = brect[7]-brect[3];
	} else if (angle >= 180 && angle < 270) {
		// x:lr, y:ll
		ox = brect[2];
		oy = brect[1];
		sx = brect[6]-brect[2];
		sy = brect[5]-brect[1];
	} else {
		// x:ll, y:ul
		ox = brect[0];
		oy = brect[7];
		sx = brect[4]-brect[0];
		sy = brect[3]-brect[7];
	}

	//printf("size: %d,%d offset: %d,%d\n",sx, sy, ox, oy);

	if (!pdat->im && sx>0 && sy>0 &&sx<MAXIMG &&sy<MAXIMG) {
		pdat->im = gdImageCreate(sx+6,sy+6);
		if (pdat->im) {
			gdImageColorResolve(pdat->im, opts.bg.r, opts.bg.g, opts.bg.b);
		}
	}

	if (opts.pcnt > 0) {
		x = opts.pts[0].x-ox;
		y = opts.pts[0].y-oy;
	} else {
		x = 3 - ox;
		y = 3 - oy;
	}

	if (!pdat->im)
		return;

	int fg = gdImageColorResolve(pdat->im, opts.fg.r, opts.fg.g, opts.fg.b);

	gdImageStringFT(pdat->im, brect, fg, opts.font, opts.ptsize, deg2rad(opts.angle), x, y, text);

	ctx->MapObj((double)ox, "ox");
	ctx->MapObj((double)oy, "oy");
	ctx->MapObj((double)sx, "sx");
	ctx->MapObj((double)sy, "sy");
}

bool ParseImageFname(char **fname, char **ext) 
{
        if (*ext = strrchr(*fname, '.')) {
                ++*ext;
        } else if (*ext = strchr(*fname, ':')) {
        	char *tmp;
                **ext = '\0';
                tmp = *fname;
                *fname = *ext+1;
                *ext = tmp;
        }
	return *ext;
}

void EvalCreateImage(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	VALID_ARGC("create-image", 1, 3);

        CStr body = (*args)[0];
        CStr input_file = (*args)[1];
        CStr output_file = (*args)[2];

	if (body.IsEmpty()) {
		return;
	}

	char *input = input_file;
	imgData pdat;
	pdat.im = NULL;
	char *ext = NULL;

	if (input && ParseImageFname(&input, &ext)) {
	    FILE *f = fopen(input, "r");
	    if (f) {
		if (!stricmp(ext,".png")) 
			pdat.im = gdImageCreateFromPng(f);
		else if (!stricmp(ext,".gif")) 
			pdat.im = gdImageCreateFromGif(f);
		else if (!stricmp(ext,".jpg")||!stricmp(ext,".jpeg")) 
			pdat.im = gdImageCreateFromJpeg(f);
		fclose(f);
	    }
	}

	if (input && !pdat.im) {
		pdat.opts.Parse(input);
		if (pdat.opts.pcnt > 0 && pdat.opts.pts[0].x > 0 && pdat.opts.pts[0].y > 0) {
			pdat.im=gdImageCreate(pdat.opts.pts[0].x, pdat.opts.pts[0].y);
			if (pdat.im) {
				gdImageColorResolve(pdat.im, pdat.opts.bg.r, pdat.opts.bg.g, pdat.opts.bg.b);
			}
		}
	} 

	qCtxTmp tmp(ctx);
	tmp.MapObj((void *)&pdat, EvalImagePoly, "line");
	tmp.MapObj((void *)&pdat, EvalImageRect, "rect");
	tmp.MapObj((void *)&pdat, EvalImagePoly, "poly");
//	tmp.MapObj((void *)&pdat, EvalImageArc,  "arc");
	tmp.MapObj((void *)&pdat, EvalImageText, "text");
	tmp.MapObj((void *)&pdat, EvalImageSX,   "width");
	tmp.MapObj((void *)&pdat, EvalImageSY,   "height");
	tmp.Parse(body);

#define FMEM ((FILE *) -1)

	if (pdat.im) {
            char *fname = output_file;
	    int s;
	    void *p;

	    if (!fname || !*fname) {
		if (pdat.opts.type==imgPng)	
        		p = gdImagePngPtr(pdat.im, &s);
		else if (pdat.opts.type==imgGif)	
        		p = gdImageGifPtr(pdat.im, &s);
		else if (pdat.opts.type==imgJpeg)	
        		p = gdImageJpegPtr(pdat.im, &s, -1);
		out->PutS((char *)p, s);
		gdFree(p);
	    } else if (ParseImageFname(&fname, &ext)) {
                FILE *f = strcmp(fname,"-") ? fopen(fname, "w") : FMEM;
		if (f) {
                if (!stricmp(ext,"png")) {
			if (f != FMEM) 
                        	gdImagePng(pdat.im, f);
			else 
		                p = gdImagePngPtr(pdat.im, &s);
                } else if (!stricmp(ext,"gif")) {
			if (f != FMEM) 
                        	gdImageGif(pdat.im, f);
			else 
		                p = gdImageGifPtr(pdat.im, &s);
                } else if (!stricmp(ext,"jpg")||!stricmp(ext,"jpeg")) {
			if (f != FMEM) 
                        	gdImageJpeg(pdat.im, f, -1);
			else 
		                p = gdImageJpegPtr(pdat.im, &s, -1);
                }
		if (f == FMEM) {
                	out->PutS((char *)p, s);
                	gdFree(p);
		} else
			fclose(f);
		}
            }

	    gdImageDestroy(pdat.im);
	}
}

#endif

void LoadImage(qCtx *ctx) {
//sql
#ifdef HAVE_LIBGD
        ctx->MapObj(EvalCreateImage,    "create-image");
	gdFTUseFontConfig(1);
#endif
}

