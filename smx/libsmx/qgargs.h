enum graphTypes {
	graphUnknown = 0,
	graphLine,
	graphPie,
	graphBar,
	graphStack,
    graphScatter,
	graphLegend
};

#pragma pack( 4 )


// label alignments
#define AL_DEFAULT  -1
#define AL_CENTER	0
#define AL_LEFT	    1
#define AL_RIGHT	2

// label definition struct
struct LABELINFO {
	LPFONT		Font;
	COLORREF	Rgb;
	short		Align;
	BOOL		Vert;
	char *		Format;
};

// caption locations/alignments
#define CAP_CENTER	0
#define CAP_LEFT	1		// main-top-left, x/y align
#define CAP_RIGHT	2		// main-top-right, x/y align
#define CAP_BOTTOM	1		// main-bottom-center, x-left-left
#define CAP_TOP		2		// x-right, y-top-center

struct CAPTION {
	char *		Text;
	short		Align;
	LABELINFO	Info;
};

// graph parameter definition
struct argStruct {
	short graphType;

	GDPIX xSize;
	GDPIX ySize;
	GDPIX xMark;
	GDPIX yMark;
	GDPIX xGrid;
	GDPIX yGrid;
	GDPIX xOffset;
	GDPIX yOffset;

    double xMin;
	double yMin;
    double xMax;
	double yMax;
    double xStep;
	double yStep;

	BOOL  normal;
	BOOL  interpolate;
	BOOL  transpose;
	short factor;

	GDPIX joints;
	short legend;
	GDPIX thick;
	short smooth;
	short dataRows;
	GDPIX pad; 
	GDPIX borderWidth;
	short colorCycle;

	short noX;

	COLORREF xMarkColor;
	COLORREF yMarkColor;
	COLORREF xGridColor;
	COLORREF yGridColor;
	COLORREF borderColor; 
	COLORREF bgColor;
	COLORREF bgColorLegend;

	LABELINFO xLabel;
	LABELINFO yLabel;
	LABELINFO legendLabel;
	
	char colDelim;
	char *inPath;
	char *outPath;
	char *tplPath;

	short tplX;
	short tplY;
	short tplLayer;

	CAPTION cCaption;
	CAPTION xCaption;
	CAPTION yCaption;
};
#pragma pack(  )

// base-type command-line parsers
COLORREF			parseRGB		(const char *opt);
void				parseLabel		(const char *opt, LABELINFO *li);
void				parseCaption	(const char *opt, CAPTION *cap);
