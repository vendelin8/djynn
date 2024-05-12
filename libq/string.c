
#include "libq.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "error.h"
#include "string.h"
#include "bytes.h"


Q_ERR(__FILE__)


#ifdef __unix__
#define STRING_ENDL "\n"
#elif __linux__
#define STRING_ENDL "\n"
#elif _APPLE_
#define STRING_ENDL "\r"
#elif _WIN32
#define STRING_ENDL "\r\n"
#endif

#define STRING_WHITESPACE " \t\n\r\f\v"


static const char upper_hex[17] = "0123456789ABCDEF";
static const char lower_hex[17] = "0123456789abcdef";

const char *q_blank = "";
const char *q_endl = STRING_ENDL;
const char *q_whitespace = STRING_WHITESPACE;

typedef struct _DjynnHtmlEntity DjynnHtmlEntity;

struct _DjynnHtmlEntity {
	const char *name;
	size_t len;
};

static const DjynnHtmlEntity html_entities[256] = {
	{ "#0",			2 },			// 0
	{ "#1",			2 },			// 1
	{ "#2",			2 },			// 2
	{ "#3",			2 },			// 3
	{ "#4",			2 },			// 4
	{ "#5",			2 },			// 5
	{ "#6",			2 },			// 6
	{ "#7",			2 },			// 7
	{ "#8",			2 },			// 8
	{ 0,				0 },			// 9		<TAB>
	{ 0,				0 },			// 10		<NEW_LINE>
	{ 0,				0 },			// 11		<VTAB>
	{ 0,				0 },			// 12		<FORM_FEED>
	{ 0,				0 },			// 13		<CARRIAGE_RETURN>
	{ "#14",			3 },			// 14
	{ "#15",			3 },			// 15
	{ "#16",			3 },			// 16
	{ "#17",			3 },			// 17
	{ "#18",			3 },			// 18
	{ "#19",			3 },			// 19
	{ "#20",			3 },			// 20
	{ "#21",			3 },			// 21
	{ "#22",			3 },			// 22
	{ "#23",			3 },			// 23
	{ "#24",			3 },			// 24
	{ "#25",			3 },			// 25
	{ "#26",			3 },			// 26
	{ "#27",			3 },			// 27
	{ "#28",			3 },			// 28
	{ "#29",			3 },			// 29
	{ "#30",			3 },			// 30
	{ "#31",			3 },			// 31
	{ 0,				0 },			// 32		<SPACE>
	{ 0,				0 },			// 33		!
	{ "quot",		4 },			// 34		"		&quot;
	{ 0,				0 },			// 35		#
	{ 0,				0 },			// 36		$
	{ 0,				0 },			// 37		%
	{ "amp",			3 },			// 38		&		&amp;
	{ "apos",		4 },			// 39		'		&apos;
	{ 0,				0 },			// 40		(
	{ 0,				0 },			// 41		)
	{ 0,				0 },			// 42		*
	{ 0,				0 },			// 43		+
	{ 0,				0 },			// 44		,
	{ 0,				0 },			// 45		-
	{ 0,				0 },			// 46		.
	{ 0,				0 },			// 47		/
	{ 0,				0 },			// 48		0
	{ 0,				0 },			// 49		1
	{ 0,				0 },			// 50		2
	{ 0,				0 },			// 51		3
	{ 0,				0 },			// 52		4
	{ 0,				0 },			// 53		5
	{ 0,				0 },			// 54		6
	{ 0,				0 },			// 55		7
	{ 0,				0 },			// 56		8
	{ 0,				0 },			// 57		9
	{ 0,				0 },			// 58		:
	{ 0,				0 },			// 59		;
	{ "lt",			2 },			// 60		<		&lt;
	{ 0,				0 },			// 61		=
	{ "gt",			2 },			// 62		>		&gt;
	{ 0,				0 },			// 63		?
	{ 0,				0 },			// 64		@
	{ 0,				0 },			// 65		A
	{ 0,				0 },			// 66		B
	{ 0,				0 },			// 67		C
	{ 0,				0 },			// 68		D
	{ 0,				0 },			// 69		E
	{ 0,				0 },			// 70		F
	{ 0,				0 },			// 71		G
	{ 0,				0 },			// 72		H
	{ 0,				0 },			// 73		I
	{ 0,				0 },			// 74		J
	{ 0,				0 },			// 75		K
	{ 0,				0 },			// 76		L
	{ 0,				0 },			// 77		M
	{ 0,				0 },			// 78		N
	{ 0,				0 },			// 79		O
	{ 0,				0 },			// 80		P
	{ 0,				0 },			// 81		Q
	{ 0,				0 },			// 82		R
	{ 0,				0 },			// 83		S
	{ 0,				0 },			// 84		T
	{ 0,				0 },			// 85		U
	{ 0,				0 },			// 86		V
	{ 0,				0 },			// 87		W
	{ 0,				0 },			// 88		X
	{ 0,				0 },			// 89		Y
	{ 0,				0 },			// 90		Z
	{ 0,				0 },			// 91		[
	{ 0,				0 },			// 92		'\'
	{ 0,				0 },			// 93		]
	{ 0,				0 },			// 94		^
	{ 0,				0 },			// 95		_
	{ 0,				0 },			// 96		`
	{ 0,				0 },			// 97		a
	{ 0,				0 },			// 98		b
	{ 0,				0 },			// 99		c
	{ 0,				0 },			// 100	d
	{ 0,				0 },			// 101	e
	{ 0,				0 },			// 102	f
	{ 0,				0 },			// 103	g
	{ 0,				0 },			// 104	h
	{ 0,				0 },			// 105	i
	{ 0,				0 },			// 106	j
	{ 0,				0 },			// 107	k
	{ 0,				0 },			// 108	l
	{ 0,				0 },			// 109	m
	{ 0,				0 },			// 110	n
	{ 0,				0 },			// 111	o
	{ 0,				0 },			// 112	p
	{ 0,				0 },			// 113	q
	{ 0,				0 },			// 114	r
	{ 0,				0 },			// 115	s
	{ 0,				0 },			// 116	t
	{ 0,				0 },			// 117	u
	{ 0,				0 },			// 118	v
	{ 0,				0 },			// 119	w
	{ 0,				0 },			// 120	x
	{ 0,				0 },			// 121	y
	{ 0,				0 },			// 122	z
	{ 0,				0 },			// 123	{
	{ 0,				0 },			// 124	|
	{ 0,				0 },			// 125	}
	{ 0,				0 },			// 126	~
	{ "#127",		4 },			// 127
	{ "#128",		4 },			// 128
	{ "#129",		4 },			// 129
	{ "#130",		4 },			// 130
	{ "#131",		4 },			// 131
	{ "#132",		4 },			// 132
	{ "#133",		4 },			// 133
	{ "#134",		4 },			// 134
	{ "#135",		4 },			// 135
	{ "#136",		4 },			// 136
	{ "#137",		4 },			// 137
	{ "#138",		4 },			// 138
	{ "#139",		4 },			// 139
	{ "#140",		4 },			// 140
	{ "#141",		4 },			// 141
	{ "#142",		4 },			// 142
	{ "#143",		4 },			// 143
	{ "#144",		4 },			// 144
	{ "#145",		4 },			// 145
	{ "#146",		4 },			// 146
	{ "#147",		4 },			// 147
	{ "#148",		4 },			// 148
	{ "#149",		4 },			// 149
	{ "#150",		4 },			// 150
	{ "#151",		4 },			// 151
	{ "#152",		4 },			// 152
	{ "#153",		4 },			// 153
	{ "#154",		4 },			// 154
	{ "#155",		4 },			// 155
	{ "#156",		4 },			// 156
	{ "#157",		4 },			// 157
	{ "#158",		4 },			// 158
	{ "#159",		4 },			// 159
	{ "nbsp",		4 },			// 160	 		&nbsp;
	{ "iexcl",		5 },			// 161	¡ 	inverted exclamation mark 	&iexcl; 	&#161;
	{ "cent",		4 },			// 162	¢ 	cent 	&cent; 	&#162;
	{ "pound",		5 },			// 163	£ 	pound 	&pound; 	&#163;
	{ "curren",		6 },			// 164	¤ 	currency 	&curren; 	&#164;
	{ "yen",			3 },			// 165	¥ 	yen 	&yen; 	&#165;
	{ "brvbar",		6 },			// 166	¦ 	broken vertical bar 	&brvbar; 	&#166;
	{ "sect",		4 },			// 167	§ 	section 	&sect; 	&#167;
	{ "uml",			3 },			// 168	¨ 	spacing diaeresis 	&uml; 	&#168;
	{ "copy",		4 },			// 169	© 	copyright 	&copy; 	&#169;
	{ "ordf",		4 },			// 170	ª 	feminine ordinal indicator 	&ordf; 	&#170;
	{ "laquo",		5 },			// 171	« 	angle quotation mark (left) 	&laquo; 	&#171;
	{ "not",			3 },			// 172	¬ 	negation 	&not; 	&#172;
	{ "shy",			3 },			// 173	­ 	soft hyphen 	&shy; 	&#173;
	{ "reg",			3 },			// 174	® 	registered trademark 	&reg; 	&#174;
	{ "macr",		4 },			// 175	¯ 	spacing macron 	&macr; 	&#175;
	{ "deg",			3 },			// 176	° 	degree 	&deg; 	&#176;
	{ "plusmn",		6 },			// 177	± 	plus-or-minus  	&plusmn; 	&#177;
	{ "sup2",		4 },			// 178	² 	superscript 2 	&sup2; 	&#178;
	{ "sup3",		4 },			// 179	³ 	superscript 3 	&sup3; 	&#179;
	{ "acute",		5 },			// 180	´ 	spacing acute 	&acute; 	&#180;
	{ "micro",		5 },			// 181	µ 	micro 	&micro; 	&#181;
	{ "para",		4 },			// 182	¶ 	paragraph 	&para; 	&#182;
	{ "middot",		6 },			// 183	· 	middle dot 	&middot; 	&#183;
	{ "cedil",		5 },			// 184	¸ 	spacing cedilla 	&cedil; 	&#184;
	{ "sup1",		4 },			// 185	¹ 	superscript 1 	&sup1; 	&#185;
	{ "ordm",		4 },			// 186	º 	masculine ordinal indicator 	&ordm; 	&#186;
	{ "raquo",		5 },			// 187	» 	angle quotation mark (right) 	&raquo; 	&#187;
	{ "frac14",		6 },			// 188	¼ 	fraction 1/4 	&frac14; 	&#188;
	{ "frac12",		6 },			// 189	½ 	fraction 1/2 	&frac12; 	&#189;
	{ "frac34",		6 },			// 190	¾ 	fraction 3/4 	&frac34; 	&#190;
	{ "iquest",		6 },			// 191	¿ 	inverted question mark 	&iquest; 	&#191;
	{ "Agrave",		6 },			// 192	À 	capital a, grave accent 	&Agrave; 	&#192;
	{ "Aacute",		6 },			// 193	Á 	capital a, acute accent 	&Aacute; 	&#193;
	{ "Acirc",		5 },			// 194	Â 	capital a, circumflex accent 	&Acirc; 	&#194;
	{ "Atilde",		6 },			// 195	Ã 	capital a, tilde 	&Atilde; 	&#195;
	{ "Auml",		4 },			// 196	Ä 	capital a, umlaut mark 	&Auml; 	&#196;
	{ "Aring",		5 },			// 197	Å 	capital a, ring 	&Aring; 	&#197;
	{ "AElig",		5 },			// 198	Æ 	capital ae 	&AElig; 	&#198;
	{ "Ccedil",		6 },			// 199	Ç 	capital c, cedilla 	&Ccedil; 	&#199;
	{ "Egrave",		6 },			// 200	È 	capital e, grave accent 	&Egrave; 	&#200;
	{ "Eacute",		6 },			// 201	É 	capital e, acute accent 	&Eacute; 	&#201;
	{ "Ecirc",		5 },			// 202	Ê 	capital e, circumflex accent 	&Ecirc; 	&#202;
	{ "Euml",		4 },			// 203	Ë 	capital e, umlaut mark 	&Euml; 	&#203;
	{ "Igrave",		6 },			// 204	Ì 	capital i, grave accent 	&Igrave; 	&#204;
	{ "Iacute",		6 },			// 205	Í 	capital i, acute accent 	&Iacute; 	&#205;
	{ "Icirc",		5 },			// 206	Î 	capital i, circumflex accent 	&Icirc; 	&#206;
	{ "Iuml",		4 },			// 207	Ï 	capital i, umlaut mark 	&Iuml; 	&#207;
	{ "ETH",			3 },			// 208	Ð 	capital eth, Icelandic 	&ETH; 	&#208;
	{ "Ntilde",		6 },			// 209	Ñ 	capital n, tilde 	&Ntilde; 	&#209;
	{ "Ograve",		6 },			// 210	Ò 	capital o, grave accent 	&Ograve; 	&#210;
	{ "Oacute",		6 },			// 211	Ó 	capital o, acute accent 	&Oacute; 	&#211;
	{ "Ocirc",		5 },			// 212	Ô 	capital o, circumflex accent 	&Ocirc; 	&#212;
	{ "Otilde",		6 },			// 213	Õ 	capital o, tilde 	&Otilde; 	&#213;
	{ "Ouml",		4 },			// 214	Ö 	capital o, umlaut mark 	&Ouml; 	&#214;
	{ "times",		5 },			// 215	× 	multiplication 	&times; 	&#215;
	{ "Oslash",		6 },			// 216	Ø 	capital o, slash 	&Oslash; 	&#216;
	{ "Ugrave",		6 },			// 217	Ù 	capital u, grave accent 	&Ugrave; 	&#217;
	{ "Uacute",		6 },			// 218	Ú 	capital u, acute accent 	&Uacute; 	&#218;
	{ "Ucirc",		5 },			// 219	Û 	capital u, circumflex accent 	&Ucirc; 	&#219;
	{ "Uuml",		4 },			// 220	Ü 	capital u, umlaut mark 	&Uuml; 	&#220;
	{ "Yacute",		6 },			// 221	Ý 	capital y, acute accent 	&Yacute; 	&#221;
	{ "THORN",		5 },			// 222	Þ 	capital THORN, Icelandic 	&THORN; 	&#222;
	{ "szlig",		5 },			// 223	ß 	small sharp s, German 	&szlig; 	&#223;
	{ "agrave",		6 },			// 224	à 	small a, grave accent 	&agrave; 	&#224;
	{ "aacute",		6 },			// 225	á 	small a, acute accent 	&aacute; 	&#225;
	{ "acirc",		5 },			// 226	â 	small a, circumflex accent 	&acirc; 	&#226;
	{ "atilde",		6 },			// 227	ã 	small a, tilde 	&atilde; 	&#227;
	{ "auml",		4 },			// 228	ä 	small a, umlaut mark 	&auml; 	&#228;
	{ "aring",		5 },			// 229	å 	small a, ring 	&aring; 	&#229;
	{ "aelig",		5 },			// 230	æ 	small ae 	&aelig; 	&#230;
	{ "ccedil",		6 },			// 231	ç 	small c, cedilla 	&ccedil; 	&#231;
	{ "egrave",		6 },			// 232	è 	small e, grave accent 	&egrave; 	&#232;
	{ "eacute",		6 },			// 233	é 	small e, acute accent 	&eacute; 	&#233;
	{ "ecirc",		5 },			// 234	ê 	small e, circumflex accent 	&ecirc; 	&#234;
	{ "euml",		4 },			// 235	ë 	small e, umlaut mark 	&euml; 	&#235;
	{ "igrave",		6 },			// 236	ì 	small i, grave accent 	&igrave; 	&#236;
	{ "iacute",		6 },			// 237	í 	small i, acute accent 	&iacute; 	&#237;
	{ "icirc",		5 },			// 238	î 	small i, circumflex accent 	&icirc; 	&#238;
	{ "iuml",		4 },			// 239	ï 	small i, umlaut mark 	&iuml; 	&#239;
	{ "eth",			3 },			// 240	ð 	small eth, Icelandic 	&eth; 	&#240;
	{ "ntilde",		6 },			// 241	ñ 	small n, tilde 	&ntilde; 	&#241;
	{ "ograve",		6 },			// 242	ò 	small o, grave accent 	&ograve; 	&#242;
	{ "oacute",		6 },			// 243	ó 	small o, acute accent 	&oacute; 	&#243;
	{ "ocirc",		5 },			// 244	ô 	small o, circumflex accent 	&ocirc; 	&#244;
	{ "otilde",		6 },			// 245	õ 	small o, tilde 	&otilde; 	&#245;
	{ "ouml",		4 },			// 246	ö 	small o, umlaut mark 	&ouml; 	&#246;
	{ "divide",		6 },			// 247	÷ 	division 	&divide; 	&#247;
	{ "oslash",		6 },			// 248	ø 	small o, slash 	&oslash; 	&#248;
	{ "ugrave",		6 },			// 249	ù 	small u, grave accent 	&ugrave; 	&#249;
	{ "uacute",		6 },			// 250	ú 	small u, acute accent 	&uacute; 	&#250;
	{ "ucirc",		5 },			// 251	û 	small u, circumflex accent 	&ucirc; 	&#251;
	{ "uuml",		4 },			// 252	ü 	small u, umlaut mark 	&uuml; 	&#252;
	{ "yacute",		6 },			// 253	ý 	small y, acute accent 	&yacute; 	&#253;
	{ "thorn",		5 },			// 254	þ 	small thorn, Icelandic 	&thorn; 	&#254;
	{ "yuml",		4 },			// 255	ÿ 	small y, umlaut mark 	&yuml; 	&#255;
};

static const char *escape_sequences = "'\"?\\\a\b\f\n\r\t\v";
static const char *escape_chars = "0      abtnvfr";

// Comment types
static const char *comments[] = {
	// Line Comments:
	"//",								// 0 - C/C++, Java, JavaScript, PHP
	";","#","!",					// 1 - Bash, Perl, CFG, INI, Properties
	"--",								// 4 - SQL
	// Block Comments:
	"/""*","*""/",					// 5 - C/C++, Java, JavaScript, PHP
	"{-","-}",						// 7 - Haskell
	"<!--","-->",					// 9 - HTML
};

// String-length for comment types
static const int comments_len[] = {
	// Line Comments:
	2,									// 0 - C/C++, Java, JavaScript
	1,1,1,							// 1 - Bash, Perl, CFG, INI, Properties
	2,									// 4 - SQL
	// Block Comments:
	2,2,								// 5 - C/C++, Java, JavaScript, PHP
	2,2,								// 7 - Haskell
	4,3,								// 9 - HTML
};


#define C_LN		0
#define CFG_LN		1
#define PL_LN		2
#define SQL_LN		4
#define C_BL		5
#define H_BL		7
#define HTML_BL	9

/* [0] Index i comments-array for line-comments
 * [1] Range in comments-array (number of line-comment types), or zero if no line-comment type exist in language
 * [2] Index in comments-array for block-comments
 * [3] Range in comments-array (number of block-comment types), or zero if no block-comment type exist in language */
static const int comments_lang[LANG_LANGS][4] = {
	/* Bash */					{ PL_LN,		1,		0,0 			}, // # line comment
	/* C */						{ C_LN,		1,		C_BL,		2	}, // C comments
	/* C++ */					{ C_LN,		1,		C_BL,		2	}, // C comments
	/* CFG */					{ CFG_LN,	2,		0,0			}, // ; and # line comments
	/* Haskell */				{ SQL_LN,	1,		H_BL,		2 	}, // -- line comments, {- -} block comments
	/* HTML */					{ 0,0,				HTML_BL,	2 	}, // HTML block comments
	/* INI */					{ CFG_LN,	2,		0,0			}, // ; line comments
	/* Java */					{ C_LN,		1,		C_BL,		2	}, // C comments
	/* JavaScript */			{ C_LN,		1,		C_BL,		2	}, // C comments
	/* Perl */					{ PL_LN,		1,		0,0			}, // # line comment, POD-documentation excluded
	/* PHP */					{ C_LN,		2,		C_BL,		2	}, // C-comments
	/* Properties (Java) */	{ PL_LN,		2,		0,0			}, // # and ! line comments
	/* Shell */					{ PL_LN,		1,		0,0			}, // # line comments
	/* SQL */					{ SQL_LN,	1,		0,0			}, // -- line comments
	/* XML */					{ 0,0,				HTML_BL,	2	}, // HTML block comments
};

static uint32_t *crc32_table = NULL;
static uint64_t *crc64_table = NULL;
/*
static const uint32_t crc32_table[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

static const uint64_t crc64_table[256] = {
	0x0000000000000000ULL, 0x7ad870c830358979ULL, 0xf5b0e190606b12f2ULL, 0x8f689158505e9b8bULL, 0xc038e5739841b68fULL, 0xbae095bba8743ff6ULL,
	0x358804e3f82aa47dULL, 0x4f50742bc81f2d04ULL, 0xab28ecb46814fe75ULL, 0xd1f09c7c5821770cULL, 0x5e980d24087fec87ULL, 0x24407dec384a65feULL,
	0x6b1009c7f05548faULL, 0x11c8790fc060c183ULL, 0x9ea0e857903e5a08ULL, 0xe478989fa00bd371ULL, 0x7d08ff3b88be6f81ULL, 0x07d08ff3b88be6f8ULL,
	0x88b81eabe8d57d73ULL, 0xf2606e63d8e0f40aULL, 0xbd301a4810ffd90eULL, 0xc7e86a8020ca5077ULL, 0x4880fbd87094cbfcULL, 0x32588b1040a14285ULL,
	0xd620138fe0aa91f4ULL, 0xacf86347d09f188dULL, 0x2390f21f80c18306ULL, 0x594882d7b0f40a7fULL, 0x1618f6fc78eb277bULL, 0x6cc0863448deae02ULL,
	0xe3a8176c18803589ULL, 0x997067a428b5bcf0ULL, 0xfa11fe77117cdf02ULL, 0x80c98ebf2149567bULL, 0x0fa11fe77117cdf0ULL, 0x75796f2f41224489ULL,
	0x3a291b04893d698dULL, 0x40f16bccb908e0f4ULL, 0xcf99fa94e9567b7fULL, 0xb5418a5cd963f206ULL, 0x513912c379682177ULL, 0x2be1620b495da80eULL,
	0xa489f35319033385ULL, 0xde51839b2936bafcULL, 0x9101f7b0e12997f8ULL, 0xebd98778d11c1e81ULL, 0x64b116208142850aULL, 0x1e6966e8b1770c73ULL,
	0x8719014c99c2b083ULL, 0xfdc17184a9f739faULL, 0x72a9e0dcf9a9a271ULL, 0x08719014c99c2b08ULL, 0x4721e43f0183060cULL, 0x3df994f731b68f75ULL,
	0xb29105af61e814feULL, 0xc849756751dd9d87ULL, 0x2c31edf8f1d64ef6ULL, 0x56e99d30c1e3c78fULL, 0xd9810c6891bd5c04ULL, 0xa3597ca0a188d57dULL,
	0xec09088b6997f879ULL, 0x96d1784359a27100ULL, 0x19b9e91b09fcea8bULL, 0x636199d339c963f2ULL, 0xdf7adabd7a6e2d6fULL, 0xa5a2aa754a5ba416ULL,
	0x2aca3b2d1a053f9dULL, 0x50124be52a30b6e4ULL, 0x1f423fcee22f9be0ULL, 0x659a4f06d21a1299ULL, 0xeaf2de5e82448912ULL, 0x902aae96b271006bULL,
	0x74523609127ad31aULL, 0x0e8a46c1224f5a63ULL, 0x81e2d7997211c1e8ULL, 0xfb3aa75142244891ULL, 0xb46ad37a8a3b6595ULL, 0xceb2a3b2ba0eececULL,
	0x41da32eaea507767ULL, 0x3b024222da65fe1eULL, 0xa2722586f2d042eeULL, 0xd8aa554ec2e5cb97ULL, 0x57c2c41692bb501cULL, 0x2d1ab4dea28ed965ULL,
	0x624ac0f56a91f461ULL, 0x1892b03d5aa47d18ULL, 0x97fa21650afae693ULL, 0xed2251ad3acf6feaULL, 0x095ac9329ac4bc9bULL, 0x7382b9faaaf135e2ULL,
	0xfcea28a2faafae69ULL, 0x8632586aca9a2710ULL, 0xc9622c4102850a14ULL, 0xb3ba5c8932b0836dULL, 0x3cd2cdd162ee18e6ULL, 0x460abd1952db919fULL,
	0x256b24ca6b12f26dULL, 0x5fb354025b277b14ULL, 0xd0dbc55a0b79e09fULL, 0xaa03b5923b4c69e6ULL, 0xe553c1b9f35344e2ULL, 0x9f8bb171c366cd9bULL,
	0x10e3202993385610ULL, 0x6a3b50e1a30ddf69ULL, 0x8e43c87e03060c18ULL, 0xf49bb8b633338561ULL, 0x7bf329ee636d1eeaULL, 0x012b592653589793ULL,
	0x4e7b2d0d9b47ba97ULL, 0x34a35dc5ab7233eeULL, 0xbbcbcc9dfb2ca865ULL, 0xc113bc55cb19211cULL, 0x5863dbf1e3ac9decULL, 0x22bbab39d3991495ULL,
	0xadd33a6183c78f1eULL, 0xd70b4aa9b3f20667ULL, 0x985b3e827bed2b63ULL, 0xe2834e4a4bd8a21aULL, 0x6debdf121b863991ULL, 0x1733afda2bb3b0e8ULL,
	0xf34b37458bb86399ULL, 0x8993478dbb8deae0ULL, 0x06fbd6d5ebd3716bULL, 0x7c23a61ddbe6f812ULL, 0x3373d23613f9d516ULL, 0x49aba2fe23cc5c6fULL,
	0xc6c333a67392c7e4ULL, 0xbc1b436e43a74e9dULL, 0x95ac9329ac4bc9b5ULL, 0xef74e3e19c7e40ccULL, 0x601c72b9cc20db47ULL, 0x1ac40271fc15523eULL,
	0x5594765a340a7f3aULL, 0x2f4c0692043ff643ULL, 0xa02497ca54616dc8ULL, 0xdafce7026454e4b1ULL, 0x3e847f9dc45f37c0ULL, 0x445c0f55f46abeb9ULL,
	0xcb349e0da4342532ULL, 0xb1eceec59401ac4bULL, 0xfebc9aee5c1e814fULL, 0x8464ea266c2b0836ULL, 0x0b0c7b7e3c7593bdULL, 0x71d40bb60c401ac4ULL,
	0xe8a46c1224f5a634ULL, 0x927c1cda14c02f4dULL, 0x1d148d82449eb4c6ULL, 0x67ccfd4a74ab3dbfULL, 0x289c8961bcb410bbULL, 0x5244f9a98c8199c2ULL,
	0xdd2c68f1dcdf0249ULL, 0xa7f41839ecea8b30ULL, 0x438c80a64ce15841ULL, 0x3954f06e7cd4d138ULL, 0xb63c61362c8a4ab3ULL, 0xcce411fe1cbfc3caULL,
	0x83b465d5d4a0eeceULL, 0xf96c151de49567b7ULL, 0x76048445b4cbfc3cULL, 0x0cdcf48d84fe7545ULL, 0x6fbd6d5ebd3716b7ULL, 0x15651d968d029fceULL,
	0x9a0d8ccedd5c0445ULL, 0xe0d5fc06ed698d3cULL, 0xaf85882d2576a038ULL, 0xd55df8e515432941ULL, 0x5a3569bd451db2caULL, 0x20ed197575283bb3ULL,
	0xc49581ead523e8c2ULL, 0xbe4df122e51661bbULL, 0x3125607ab548fa30ULL, 0x4bfd10b2857d7349ULL, 0x04ad64994d625e4dULL, 0x7e7514517d57d734ULL,
	0xf11d85092d094cbfULL, 0x8bc5f5c11d3cc5c6ULL, 0x12b5926535897936ULL, 0x686de2ad05bcf04fULL, 0xe70573f555e26bc4ULL, 0x9ddd033d65d7e2bdULL,
	0xd28d7716adc8cfb9ULL, 0xa85507de9dfd46c0ULL, 0x273d9686cda3dd4bULL, 0x5de5e64efd965432ULL, 0xb99d7ed15d9d8743ULL, 0xc3450e196da80e3aULL,
	0x4c2d9f413df695b1ULL, 0x36f5ef890dc31cc8ULL, 0x79a59ba2c5dc31ccULL, 0x037deb6af5e9b8b5ULL, 0x8c157a32a5b7233eULL, 0xf6cd0afa9582aa47ULL,
	0x4ad64994d625e4daULL, 0x300e395ce6106da3ULL, 0xbf66a804b64ef628ULL, 0xc5bed8cc867b7f51ULL, 0x8aeeace74e645255ULL, 0xf036dc2f7e51db2cULL,
	0x7f5e4d772e0f40a7ULL, 0x05863dbf1e3ac9deULL, 0xe1fea520be311aafULL, 0x9b26d5e88e0493d6ULL, 0x144e44b0de5a085dULL, 0x6e963478ee6f8124ULL,
	0x21c640532670ac20ULL, 0x5b1e309b16452559ULL, 0xd476a1c3461bbed2ULL, 0xaeaed10b762e37abULL, 0x37deb6af5e9b8b5bULL, 0x4d06c6676eae0222ULL,
	0xc26e573f3ef099a9ULL, 0xb8b627f70ec510d0ULL, 0xf7e653dcc6da3dd4ULL, 0x8d3e2314f6efb4adULL, 0x0256b24ca6b12f26ULL, 0x788ec2849684a65fULL,
	0x9cf65a1b368f752eULL, 0xe62e2ad306bafc57ULL, 0x6946bb8b56e467dcULL, 0x139ecb4366d1eea5ULL, 0x5ccebf68aecec3a1ULL, 0x2616cfa09efb4ad8ULL,
	0xa97e5ef8cea5d153ULL, 0xd3a62e30fe90582aULL, 0xb0c7b7e3c7593bd8ULL, 0xca1fc72bf76cb2a1ULL, 0x45775673a732292aULL, 0x3faf26bb9707a053ULL,
	0x70ff52905f188d57ULL, 0x0a2722586f2d042eULL, 0x854fb3003f739fa5ULL, 0xff97c3c80f4616dcULL, 0x1bef5b57af4dc5adULL, 0x61372b9f9f784cd4ULL,
	0xee5fbac7cf26d75fULL, 0x9487ca0fff135e26ULL, 0xdbd7be24370c7322ULL, 0xa10fceec0739fa5bULL, 0x2e675fb4576761d0ULL, 0x54bf2f7c6752e8a9ULL,
	0xcdcf48d84fe75459ULL, 0xb71738107fd2dd20ULL, 0x387fa9482f8c46abULL, 0x42a7d9801fb9cfd2ULL, 0x0df7adabd7a6e2d6ULL, 0x772fdd63e7936bafULL,
	0xf8474c3bb7cdf024ULL, 0x829f3cf387f8795dULL, 0x66e7a46c27f3aa2cULL, 0x1c3fd4a417c62355ULL, 0x935745fc4798b8deULL, 0xe98f353477ad31a7ULL,
	0xa6df411fbfb21ca3ULL, 0xdc0731d78f8795daULL, 0x536fa08fdfd90e51ULL, 0x29b7d047efec8728ULL
};
*/

#define CRC32(crc,c)		(crc32_table[((crc)^(c))&0xff]^((crc)>>8))
#define CRC32_POLY		UINT32_C(0xedb88320)
#define CRC32_INIT		UINT32_C(0xffffffff)
#define CRC32_FINAL		UINT32_C(0xffffffff)

#define CRC64(crc,c)		(crc64_table[((crc)^(c))&0xff]^((crc)>>8))
#define CRC64_POLY		UINT64_C(0xc96c5795d7870f42)
#define CRC64_INIT		UINT64_C(0xffffffffffffffff)
#define CRC64_FINAL		UINT64_C(0xffffffffffffffff)


static void crc_free() {
	if(crc32_table!=NULL) { q_free(crc32_table);crc32_table = NULL; }
	if(crc64_table!=NULL) { q_free(crc64_table);crc64_table = NULL; }
}


static void crc32_init() {
	int n,i;
	uint32_t crc;
	crc32_table = (uint32_t *)q_malloc(0x400);
	if(crc32_table==NULL) {
		q_err(ERR_MALLOC,NULL);
		exit(1);
	}

	for(n=0; n<0x100; ++n) {
		for(i=0,crc=n; i<8; ++i)
			crc = (crc&1)? ((crc>>1)^CRC32_POLY) : (crc>>1);
		crc32_table[n] = crc;
	}

#if __BYTE_ORDER == __BIG_ENDIAN
	for(n=0; n<0x100; ++n) crc32_table[n] = bswap32(crc32_table[n]);
#endif
/*
printf("crc32_init [\n");
	for(n=0; n<256; ++n) {
printf(" %08" PRIX32 ",",crc32_table[n]);
if((n%8)==7 || n==255) fputc('\n',stdout);
	}
printf("]\n");
*/
	if(crc64_table==NULL) atexit(crc_free);
}

uint32_t q_crc32(const char *str,int cins) {
	register uint8_t *ustr = (uint8_t *)str;
	register uint32_t crc = CRC32_INIT,c;
//printf("crc32[%s]\n",str);
	if(crc32_table==NULL) crc32_init();
	if(cins) while(*ustr!='\0') c = *ustr++,c = q_lower(c),crc = CRC32(crc,c);
	else while(*ustr!='\0') c = *ustr++,crc = CRC32(crc,c);
	return crc^CRC32_FINAL;
}

uint32_t q_crc32n(const char *str,long o,long l,int cins) {
	register uint8_t *ustr = (uint8_t *)str;
	register uint32_t crc = CRC32_INIT,c;
//printf("crc32[%s]\n",str);
	if(o<0 || l<=0) {
		long n = strlen(str);
		if(o<0) o = n+o;
		if(l<=0) l = n-o+l;
		if(o<0 || l<=0 || o+l>n) return crc;
	}
	if(crc32_table==NULL) crc32_init();
	if(o>0) ustr += o;
	if(cins) while(l--) c = *ustr++,c = q_lower(c),crc = CRC32(crc,c);
	else while(l--) c = *ustr++,crc = CRC32(crc,c);
	return crc^CRC32_FINAL;
}

static void crc64_init() {
	static const uint64_t poly64 = CRC64_POLY;
	int n,i;
	uint64_t crc;
	crc64_table = (uint64_t *)q_malloc(0x800);
	if(crc64_table==NULL) {
		q_err(ERR_MALLOC,NULL);
		exit(1);
	}
	for(n=0; n<0x100; ++n) {
		for(i=0,crc=n; i<8; ++i) {
			if(crc&1) crc = (crc>>1)^poly64;
			else crc >>= 1;
		}
		crc64_table[n] = crc;
	}

#if __BYTE_ORDER == __BIG_ENDIAN
	for(n=0; n<0x100; ++n) crc64_table[n] = bswap64(crc64_table[n]);
#endif
/*
printf("crc64_init [\n");
	for(n=0; n<256; ++n) {
printf(" %016" PRIX64 ",",crc64_table[n]);
if((n%6)==5 || n==255) fputc('\n',stdout);
	}
printf("]\n");
*/
	if(crc32_table==NULL) atexit(crc_free);
}

uint64_t q_crc64(const char *str,int cins) {
	register uint8_t *ustr = (uint8_t *)str;
	register uint64_t crc = CRC64_INIT,c;
//printf("crc64\n");
	if(crc64_table==NULL) crc64_init();
	if(cins) while(*ustr!='\0') c = *ustr++,c = q_lower(c),crc = CRC64(crc,c);
	else while(*ustr!='\0') c = *ustr++,crc = CRC64(crc,c);
	return crc^CRC64_FINAL;
}

uint64_t q_crc64n(const char *str,long o,long l,int cins) {
	register uint8_t *ustr = (uint8_t *)str;
	register uint64_t crc = CRC64_INIT,c;
//printf("crc64n\n");
	if(o<0 || l<=0) {
		long n = strlen(str);
		if(o<0) o = n+o;
		if(l<=0) l = n-o+l;
		if(o<0 || l<=0 || o+l>n) return crc;
	}
	if(crc64_table==NULL) crc64_init();
	if(o>0) ustr += o;
	if(cins) while(l--) c = *ustr++,c = q_lower(c),crc = CRC64(crc,c);
	else while(l--) c = *ustr++,crc = CRC64(crc,c);
	return crc^CRC64_FINAL;
}

int q_x(char c) { return c>='0' && c<='9'? c-'0' : (c>='a' && c<='f'? c-'a'+10 : (c>='A' && c<='F'? c-'A'+10 : 0x20)); }

char q_tox(int c) { return c>=0 && c<=9? c+'0' : (c>=0xa && c<=0xf? c-10+'a' : '0'); }

char q_toX(int c) { return c>=0 && c<=9? c+'0' : (c>=0xa && c<=0xf? c-10+'a' : '0'); }

uint64_t q_xtoi(const char *str) {
	int i;
	char c;
	uint64_t n = 0;
	if(str!=NULL)
		for(c=str[i=0]; q_ishex(c) && i<16; c=str[++i]) n = (n<<4)|q_x(c);
	return n;
}

char *q_itox(char *str,uint64_t i) {
	uint64_t n;
	int u = 0;
	for(n=i; n; n>>=4) ++u;
	for(str+=u,*str='\0'; i; i>>=4) *--str = lower_hex[i&0xf];//(u=(i&0xf))<10? '0'+u : 'a'+(u-10);
	return str;
}

char *q_itoX(char *str,uint64_t i) {
	uint64_t n;
	int u = 0;
	for(n=i; n; n>>=4) ++u;
	for(str+=u,*str='\0'; i; i>>=4) *--str = upper_hex[i&0xf];//(u=(i&0xf))<10? '0'+u : 'a'+(u-10);
	return str;
}

char *q_itostr(char *str,int64_t n,int len,char pad) {
	char *p = &str[21],c = 0;
	*p-- = '\0';
	if(n<0) c = '-',n = -n,--len;
	for(; n; n/=10,--len) *p-- = '0'+(n%10);
	if(len>0) for(; len>0; --len) *p-- = pad;
	if(c) *p-- = c;
	return ++p;
}

#define DMAX 10

char *q_dtostr(char *str,double d,int dmin,int dmax) {
	char *p = str;
	int n;
	if(dmin<0) dmin = 0;
	if(dmax<0 && dmin>0) dmax = dmin;
	else if(dmax<=0 || dmax>DMAX) dmax = DMAX;
	else if(dmax<dmin) dmax = dmin;
	sprintf(str,"%.*f",dmax,d);
	while(*p!='\0' && *p++!='.');
	if(*p!='\0' && dmax>dmin && p[dmax-1]=='0') {
		for(n=dmax-1; n>=dmin && p[n]=='0'; --n);
		if(n<dmax-1) p[n+1] = '\0';
	}
/*	char s[22],*p = str,*t;
	int l;
	int64_t n;
	if(d<0) *p++ = '-',d = -d;
	n = d;
	t = q_itostr(s,n,0,0);
	l = strlen(t);
	strcpy(p,t);
	if(dmin>0 && dmax>0) {
		long double ld = (long double)d;
		ld -= (long double)n;
		p += l;
		n = (int64_t)(ld*1000000000000.0L);
		if(dmin<0) dmin = 0;
		if(dmax<0) dmax = 0;
		else if(dmax>12) dmax = 12;
		t = q_itostr(s,n,12,'0');
		if(dmax>0 && dmax<12 && t[dmax]>='5') ++t[dmax-1];
		t[dmax] = '\0';
		for(l=dmax-1; l>=dmin && t[l]=='0'; --l) t[l] = '\0';
		if(l>0) {
			*p++ = '.';
			strcpy(p,t);
		}
	}*/
//	if((double)((int64_t)d)==d) sprintf(str,"%.*f",min,d);
//	else sprintf(str,max<=4? "%.*g" : "%.*f",max,f);
	return str;
}

char *q_tolower(char *str) {
	char c = *str,*s = str;
	while(c!='\0') *s = q_lower(c),c = *++s;
	return str;
}

char *q_toupper(char *str) {
	char c = *str,*s = str;
	while(c!='\0') *s = q_upper(c),c = *++s;
	return str;
}

int q_stricmp(const char *str1,const char *str2) {
	char c1 = *str1,c2 = *str2;
	for(; c1!='\0' && c2!='\0'; c1=*++str1,c2=*++str2)
		if(q_lower(c1)!=q_lower(c2))
			return q_lower(c1)<q_lower(c2)? -1 : 1;
	return (c1=='\0' && c2=='\0')? 0 : (c1=='\0'? -1 : 1);
}

int q_strnicmp(const char *str1,const char *str2,size_t n) {
	char c1 = *str1,c2 = *str2;
	for(; n>0 && c1!='\0' && c2!='\0'; c1=*++str1,c2=*++str2,--n)
		if(q_lower(c1)!=q_lower(c2))
			return q_lower(c1)<q_lower(c2)? -1 : 1;
	return (c1=='\0' && c2=='\0')? 0 : (c1=='\0'? -1 : 1);
}

char *q_stristr(char *str1,const char *str2) {
	int l = strlen(str2),c1,c2 = *str2;
	for(c2=q_lower(c2); (c1=*str1)!='\0'; ++str1)
		if(q_lower(c1)==c2 && q_strnicmp(str1,str2,l)==0) return str1;
	return NULL;
}

char *q_strwhsp(char *str) {
	char *s1 = str,*s2 = str,c1,c2;
	for(c1=';',c2=*s2; c2!='\0'; c1=*s1++=*s2++,c2=*s2)
		if(q_isspace(c2)) {
			for(c2=*++s2; c2!='\0' && q_isspace(c2); c2=*++s2);
			if(q_isword(c1) && q_isword(c2)) *s1++ = ' ';
		}
	*s1 = '\0';
	return str;
}

int q_strnchr(const char *str,char c) {
	int n = 0;
	while(*str!='\0') if(*str++==c) ++n;
	return n;
}

char *q_substr(const char *str,long o,long l) {
	char *s = NULL;
	if(str && *str) {
		long n = strlen(str);
		if(o<0) o = n+o;
		if(l<=0) l = n-o+l;
		s = (char *)q_malloc(l+1);
		if(s==NULL) q_err(ERR_MALLOC,NULL);
		else {
			if(l>0) memcpy(s,str+o,l);
			s[l] = '\0';
		}
	}
	return s;
}

char *q_repeat(char *str,char c,long o,long l) {
	char *s = str;
	if(str) {
		long n = *str!='\0'? strlen(str) : 0;
		if(o<0) o = n+o;
		if(l<=0) l = n-o+l;
		s += o;
		while(l) *s++ = c,--l;
		if(o==n) *s = '\0';
	}
	return s;
}

int q_tokens(char *str,const char *delim,int cins) {
	int l = strlen(delim),n = 0;
	if(cins) for(; str && *str; n++) {
		if((str=q_stristr(str,delim))) str += l;
	} else for(; str && *str; n++) {
		if((str=strstr(str,delim))) str += l;
	}
	return n;
}

char **q_split(char **list,char *str,const char *delim,int cins) {
	size_t l = strlen(delim),n = 0;
	if(cins) for(; str && *str; ++n) {
		list[n] = str;
		if((str=q_stristr(str,delim))) *str = '\0',str += l;
	} else for(; str && *str; ++n) {
		list[n] = str;
		if((str=strstr(str,delim))) *str = '\0',str += l;
	}
	return list;
}

void q_reverse(char *str,long o,long l) {
	if(str && *str) {
		long n = strlen(str);
		if(o<0) o = n+o;
		if(l<=0) l = n-o+l;
		if(o>=0 && l>0 && o+l<=n) {
			char *p1 = &str[o],*p2 = &str[o+l-1],c;
			while(p1<p2) c = *p1,*p1++ = *p2,*p2-- = c;
		}
	}
}

size_t q_trim(char *str,const char *s) {
	if(!str || !*str) return 0;
	size_t l = 0;
	char *p = str;
	if(!s) s = q_whitespace;
	while(*p && strchr(s,*p)) ++p;
	if(p!=str) while(*p) *str++ = *p++,++l;
	else while(*str) ++str,++l;
	if(l) {
		while(strchr(s,*--str)) --l;
		*++str = '\0';
	} else *str = '\0';
	return l;
}

long q_pos_left(const char *str,long o,const char *s) {
	if(str!=NULL && *str!='\0') {
		long l = strlen(str);
		if(o<0) o = (long)l+o;
		if(s==NULL) s = q_whitespace;
		if(o>=0 && o<(long)l) 
			while(strchr(s,str[o-1]) && o>0) --o;
		return o;
	}
	return -1;
}

long q_pos_right(const char *str,long o,const char *s) {
	if(str!=NULL && *str!='\0') {
		long l = strlen(str);
		if(o<0) o = (long)l+o;
		if(s==NULL) s = q_whitespace;
		if(o>=0 && o<(long)l) 
			while(strchr(s,str[o]) && o<(long)l) ++o;
		return o;
	}
	return -1;
}

void q_print_utf8(char *d,const char *s,size_t o,size_t l) {
	char c;
	size_t i,n;
	if(o) for(i=0; *s!='\0' && i<o; i++) {
		if(*s&0x80) {
			for(c=(*s<<1),n=1; c&0x80 && n<8; c<<=1,n++);
			while(n--) s++;
		} else s++;
	}
	if(l) for(i=0; *s!='\0' && i<l; i++) {
		if(*s&0x80) {
			for(c=(*s<<1),n=1; c&0x80 && n<8; c<<=1,n++);
			while(n--) *d++ = *s++;
		} else *d++ = *s++;
	}
	*d = '\0';
}

void q_strfree(char **str) {
	if(str!=NULL) {
		if(*str!=NULL) q_free(*str);
		*str = NULL;
	}
}

void q_strpdup(char **str,const char *s) {
	if(str!=NULL && s!=NULL) {
		if(*str!=NULL) {
			if(**str==*s && strcmp(*str,s)==0) return;
			q_free(*str);
		}
		*str = q_strdup(s);
	}
}

void q_strpdupf(char **str,const char *fmt, ...) {
	if(str!=NULL && fmt!=NULL) {
		va_list list,copy;
		int l;
		va_start(list,fmt);
		va_copy(copy,list);
		l = vsnprintf(NULL,0,fmt,copy);
		if(l>0) {
			char *s = (char *)q_malloc(l+1);
			vsprintf(s,fmt,list);
			if(*str!=NULL) {
				if(**str==*s && strcmp(*str,s)==0)
					goto q_strdupf_end;
				q_free(*str);
			}
			*str = s;
			if(0) {
q_strdupf_end:
				q_free(s);
			}
		}
		va_end(copy);
		va_end(list);
	}
}

int q_isnumeric(const char * str) {
	char * p;
	if(str==NULL || *str=='\0' || q_isalpha(*str)) return 0;
	strtod(str,&p);
	return *p=='\0';
}

int q_ishtmlent(unsigned char c) {
	return html_entities[c].name!=0;
}


char *q_stralloc(char **str,size_t len,size_t *cap,long l) {
	size_t c = *cap;
	if(str==NULL) return NULL;
	if(l>=0 && len+l+l>=c) {
//fprintf(stdout,"q_stralloc(len=%d,cap=%d,l=%d)\n",(int)len,(int)c,(int)l);
//fflush(stdout);
		if(c==0) c = 1;
		if(l>0 && len+l>=c) c = len+l+1;
		if(c<1024) c <<= 1;
		else c += 1024;
		if(*str==NULL) *str = (char *)q_malloc(c+1);
		else *str = (char *)q_realloc(*str,c+1);
		if(*str==NULL) q_err(ERR_MALLOC,NULL);
		else (*str)[c] = '\0';
		*cap = c;
	}
	return *str;
}

long q_strmove(char **str,size_t *len,size_t *cap,long o,long l) {
	if(str==NULL) return 0;
	if(o<0) o = (long)*len+o;
	if(o>=0 && o<(long)*len && l!=0) {
		char *s;
		long n;
		if(l>0) {
			if(*str==NULL || *len+l+2>=*cap) {
				q_stralloc(str,*len,cap,l);
				if(*str==NULL) {
					q_err(ERR_MALLOC,NULL);
					return 0;
				}
			}
			for(s=*str,n=(long)*len; n>=o; --n) s[n+l] = s[n];
			*len += l;
		} else if(*str!=NULL && -l>=(long)*len) {
			if(o+l<0) o = 0;
			for(s=*str,n=(o+l>=0? o : -l); n<=(long)*len; ++n) s[n] = s[n+l];
			*len -= l;
		}
	}
	return o;
}

QString q_string_new() {
	QString str = (QString)q_malloc(sizeof(_QString));
	if(str==NULL) q_err(ERR_MALLOC,NULL);
	else {
		str->len = 0;
		str->cap = 0;
		str->ptr = NULL;
	}
	return str;
}

void q_string_free(QString str) {
	if(str==NULL) return;
	if(str->ptr!=NULL) q_free(str->ptr);
	q_free(str);
}

QString q_string_clear(QString str) {
	if(str==NULL) str = q_string_new();
	if(str->ptr!=NULL) *str->ptr = '\0',str->len = 0;
	return str;
}

QString q_string_dup(const QString str) {
	if(str==NULL) return NULL;
	else {
		QString str2 = (QString)q_malloc(sizeof(_QString));
		if(str2==NULL) q_err(ERR_MALLOC,NULL);
		else {
			str2->len = str->len;
			str2->cap = str->cap;
			str2->ptr = (char *)q_malloc(str2->cap);
			if(str2->ptr==NULL) {
				q_err(ERR_MALLOC,NULL);
				str2->len = str2->cap = 0;
			} else memcpy(str2->ptr,str->ptr,str2->len+1);
		}
		return str2;
	}
}

QString q_string_insert(QString str,long n,const char *s) {
	if(str==NULL) str = q_string_new();
	if(s==NULL || *s=='\0') return str;
	else {
		size_t l = strlen(s);
		if(str->len+l+1>=str->cap) {
			q_string_resize(str,l+1);
			if(str->ptr==NULL) {
				str->len = str->cap = 0;
				return str;
			}
		}
		if(n>(long)str->len) n = str->len;
		if(n==(long)str->len) {
			memcpy(&str->ptr[str->len],s,l);
			str->len += l;
			str->ptr[str->len] = '\0';
		} else {
			n = q_string_move(str,n,l);
			memcpy(&str->ptr[n],s,l);
		}
		return str;
	}
}

QString q_string_insertn(QString str,long n,const char *s,long o,long l) {
	if(str==NULL) str = q_string_new();
	if(s==NULL || *s=='\0') return str;
	if(o<0 || l<=0) {
		long sn = strlen(s);
		if(o<0) o = sn+o;
		if(l<=0) l = sn-o+l;
		if(o<0 || l<=0 || o+l>sn) return str;
	}
	if(str->len+l+1>=str->cap) {
		q_string_resize(str,l+1);
		if(str->ptr==NULL) {
			str->len = str->cap = 0;
			return str;
		}
	}
	if(n>(long)str->len) n = str->len;
	if(n==(long)str->len) {
		memcpy(&str->ptr[str->len],s+o,l);
		str->len += l;
		str->ptr[str->len] = '\0';
	} else {
		n = q_string_move(str,n,l);
		memcpy(&str->ptr[n],s+o,l);
	}
	return str;
}

QString q_string_insert_char(QString str,long n,char c) {
	if(str==NULL) str = q_string_new();
	if(c!='\0') {
		if(str->len+2>=str->cap) {
			q_string_resize(str,1);
			if(str->ptr==NULL) {
				str->len = str->cap = 0;
				return str;
			}
		}
		if(n>(long)str->len) n = str->len;
		if(n==(long)str->len) {
			str->ptr[str->len++] = c;
			str->ptr[str->len] = '\0';
		} else {
			n = q_string_move(str,n,1);
			str->ptr[n] = c;
		}
	}
	return str;
}

QString q_string_insert_chars(QString str,long n,char c,int l) {
	if(str==NULL) str = q_string_new();
	if(c!='\0' && l>0) {
		if(str->len+l+1>=str->cap) {
			q_string_resize(str,l+1);
			if(str->ptr==NULL) {
				str->len = str->cap = 0;
				return str;
			}
		}
		if(n>(long)str->len) n = str->len;
		if(n==(long)str->len) {
			while(l--) str->ptr[str->len++] = c;
			str->ptr[str->len] = '\0';
		} else {
			n = q_string_move(str,n,l);
			while(l--) str->ptr[n+l-1] = c;
		}
	}
	return str;
}

QString q_string_insert_int(QString str,long n,int64_t i) {
	if(i>=0 && i<=9) return q_string_insert_char(str,n,'0'+i);
	else {
		char s[22];
		return q_string_insert(str,n,q_itostr(s,i,0,0));
	}
}

QString q_string_insert_float(QString str,long n,double f,int p) {
	char s[42];
	return q_string_insert(str,n,q_dtostr(s,f,1,p));
}

QString q_string_insert_string(QString str,long n,const QString s) {
	if(str==NULL) str = q_string_new();
	if(s==NULL) return str;
	return q_string_insertn(str,n,s->ptr,0,s->len);
}

QString q_string_insert_hex(QString str,long n,uint64_t i,int upper) {
	const char *s = upper? upper_hex : lower_hex;
	if(i<=0xf) return q_string_insert_char(str,n,s[i&0xf]);
	else {
		char h[17],*p = h+16;
		*p-- = '\0';
		for(; i; i>>=4) *p-- = s[i&0xf];
		return q_string_insert(str,n,p+1);
	}
}

QString q_string_insert_base(QString str,long n,uint64_t i,int base) {
	if(i==0) return q_string_insert_char(str,n,'0');
	else {
		char s[22],*p = s+21,b;
		*p-- = '\0';
		for(; i; i/=base) b = i%base,*p-- = b<=9? '0'+b : 'A'+b-10;
		return q_string_insert(str,n,p+1);
	}
}

QString q_string_insertf(QString str,long n,const char *f, ...) {
	va_list list;
	va_start(list,f);
	str = q_string_vinsertf(str,n,f,list);
	va_end(list);
	return str;
}

QString q_string_vinsertf(QString str,long n,const char *f,va_list list) {
	if(str==NULL) str = q_string_new();
	if(f!=NULL && *f!='\0') {
		va_list list_copy;
		int l;
		va_copy(list_copy,list);
		l = vsnprintf(NULL,0,f,list_copy);
		va_end(list_copy);
		if(l==-1) q_err(0,NULL);
		else if(l>0) {
			if(str->len+l>=str->cap) q_string_resize(str,l+1);

			if(n>(long)str->len) n = str->len;
//fprintf(stdout,"q_string_vinsertf(n=%ld,ptr=\"%s\",len=%d,cap=%d,l=%d)\n",n,&str->ptr[n],(int)str->len,(int)str->cap,l);
			if(n==(long)str->len) vsprintf(&str->ptr[n],f,list);
			else {
				char c;
				n = q_string_move(str,n,l);
				c = str->ptr[n+l];
				vsprintf(&str->ptr[n],f,list);
				str->ptr[n+l] = c;
			}
//fprintf(stdout,"q_string_vinsertf(str=\"%s\")\n",str->ptr);
			str->len += l;
		}
	}
	return str;
}

QString q_string_includef(QString str,const char *format, ...) {
	char buf[257];
	va_list args;
   va_start(args,format);
	vsnprintf(buf,256,format,args);
   va_end(args);
	return q_string_include(str,buf);
}

QString q_string_include(QString str,const char *fn) {
	FILE *fp = fopen(fn,"rb");
	if(fp) {
		str = q_string_finclude(str,fp);
		fclose(fp);
	} else q_err(0,NULL);
	return str;
}

QString q_string_finclude(QString str,FILE *fp) {
	static const char s[32] = { 0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if(str==NULL) str = q_string_new();
	if(fp!=NULL) {
		size_t l,n;
		char buf[1026];
		int c,c0;
//debug_output("String::include()\n");
		for(l=0,n=0,c0=0; (c=fgetc(fp)) && c!=EOF;) {
//debug_putc(c);
			if((c<32 && !s[c]) || c==0x7f) continue;
			if((c=='\r' || c=='\n') && !c0) c0 = c;
			else {
				if(c0) {
					if(c0==c || (c!='\n' && c!='\r')) buf[n++] = '\n',++l;
					if(c=='\r') c = '\n';
					c0 = 0;
				}
				buf[n++] = c,++l;
				if(n>=1024) {
					buf[n] = '\0';
//debug_output("q_string_finclude(%s)\n",buf);
					q_string_appendn(str,buf,0,n);
					n = 0;
				}
			}
		}
		if(n>0) {
			buf[n] = '\0';
//debug_output("q_string_finclude(%s)\n",buf);
			q_string_appendn(str,buf,0,n);
		}
	}
	return str;
}

void q_string_print(QString str) {
	if(str==NULL) return;
	printf("%s",str->ptr);
}

long q_string_find(const QString str,const char *s,long o,long l,long sl) {
	if(str!=NULL && str->ptr!=NULL && str->len>0 && s!=NULL && *s!='\0') {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(sl<=0) sl = (long)strlen(s)+sl;
		if(o>=0 && l>0 && sl>0 && o+l<=(long)str->len) {
			size_t i,n;
			const char *p = str->ptr;
			for(i=o,n=o+l; i<n; ++i)
				if(p[i]==*s && strncmp(&p[i],s,sl)==0) return i;
		}
	}
	return -1;
}

long q_string_find_char(const QString str,char c,long o,long l) {
	if(str!=NULL && str->ptr!=NULL && str->len>0 && c!='\0') {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			size_t i,n;
			const char *p = str->ptr;
			for(i=o,n=o+l; i<n; ++i)
				if(p[i]==c) return i;
		}
	}
	return -1;
}
long q_string_find_chars(const QString str,const char *s,long o,long l) {
	if(str!=NULL && str->ptr!=NULL && str->len>0 && s!=NULL && *s!='\0') {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			size_t i,n;
			const char *p = str->ptr;
			for(i=o,n=o+l; i<n; ++i)
				if(strchr(s,p[i])!=NULL) return i;
		}
	}
	return -1;
}

long q_string_match_tags(const QString str,const char *tag1,const char *tag2,long o,long l,const char *c1,const char *c2) {
	if(str!=NULL && str->len>0) {
		long i,n,l1 = strlen(tag1),l2 = strlen(tag2),nest;
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			const char *p = str->ptr;
			for(i=o,n=o+l,nest=0; i<n; ++i)
				if(p[i]==*tag1 && (l1==1 || !strncmp(&p[i],tag1,l1)) &&
						(c1==NULL || strchr(c1,p[i+l1]))) i += l1-1,++nest;
				else if(p[i]==*tag2 && (l2==1 || !strncmp(&p[i],tag2,l2)) &&
						(c2==NULL || strchr(c2,p[i+l2]))) {
					if(--nest==0) return i;
					i += l2-1;
				}
		}
	}
	return -1;
}

long q_string_match_quotes(const QString str,long o,long l) {
	if(str!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		char q = str->ptr[o];
		if(o>=0 && l>0 && o+l<=(long)str->len && q_isquote(q)) {
			long i,n;
			const char *p = str->ptr;
			for(i=o,n=o+l; i<n; ++i)
				if(p[i]=='\\') ++i;
				else if(p[i]==q) return i+1;
		}
	}
	return -1;
}

long q_string_match_token(const QString str,const char *delim,long o,long l,int f) {
	if(str!=NULL && str->len>0 && delim!=NULL && *delim!='\0') {
		long i,j = -1,n,dl = strlen(delim);
		const char *p = str->ptr;
		if(o<0) o = (long)str->len+o;
		if(o>=0 && (f&TOKEN_LTRIM)) o = q_pos_right(p,o,NULL);
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			for(i=o,n=o+l; i<n; ++i)
				if(p[i]=='\\' && (f&TOKEN_ESCAPE)) ++i;
				else if((f&TOKEN_DELIM_STR)? strncmp(delim,&p[i],dl)==0 : strchr(delim,p[i])!=NULL) {
					j = i;
					if((f&TOKEN_DELIM_STR) || !q_isspace(p[i])) break;
				} else if(j>=0) break;
			if(f&TOKEN_RTRIM) j = q_pos_left(p,j,NULL);
		}
		return j;
	}
	return -1;
}

long q_string_match_value(const QString str,int lang,long o,long l,int f) {
	if(str!=NULL && str->len>0) {
		long i,n;
		const char *p = str->ptr;
		char c,q;
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		i = o,q = p[o];
		if(q_isquote(q)) ++i;
		else q = 0;
		if(o>=0 && l>0 && o+l<=(long)str->len && lang>=0 && lang<LANG_LANGS) {
			const int *cl = comments_lang[lang];
			const char **cc = &comments[cl[0]],**cb = cl[3]? &comments[cl[2]] : 0;
			const int *ccl = &comments_len[cl[0]],*cbl = cl[3]? &comments_len[cl[2]] : 0;
			int j;
			for(n=o+l; i<n; ++i)
				if((c=p[i])=='\\') ++i;
				else if(q && c==q) return i+1;
				else if(!q) {
					if(q_isbreak(c)) return i;
					else if(!(f&MATCH_COMMENTS)) {
						// Match line comment
						for(j=0; j<cl[1]; ++j)
							if(c==*cc[j] && (ccl[j]==1 || !strncmp(&p[i],cc[j],ccl[j]))) n = 0;
						// Match block comment
						if(cb && c==*cb[0] && (cbl[0]==1 || !strncmp(&p[i],cb[0],cbl[0]))) n = 0;
						if(n==0) { // Find first white space before comment
							for(; i>o && q_isspace(p[i-1]); --i);
							return i;
						}
					}
				}
		}
	}
	return -1;
}

long q_string_skip_comment(const QString str,int lang,long o,long l) {
	if(str!=NULL && str->len>0) {
		long i;
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len && lang>=0 && lang<LANG_LANGS) {
			const char *p = str->ptr;
			const int *cl = comments_lang[lang];
			const char **cc = &comments[cl[0]],**cb = cl[3]? &comments[cl[2]] : 0;
			const int *ccl = &comments_len[cl[0]],*cbl = cl[3]? &comments_len[cl[2]] : 0;
			int j;
			// Skip line comment
			for(j=0; j<cl[1]; ++j)
				if(p[o]==*cc[j] && (ccl[j]==1 || !strncmp(&p[o],cc[j],ccl[j]))) {
					for(o+=ccl[j]; !q_isbreak(p[o]); ++o);
					return o;
				}
			// Skip block comment
			if(cb && p[o]==*cb[0] && (cbl[0]==1 || !strncmp(&p[o],cb[0],cbl[0]))) {
				i = q_string_find(str,cb[1],o,l-o,cbl[1]);
				if(i==-1) return o+l;
			}
		}
		return o;
	}
	return -1;
}

long q_string_skip_comments(const QString str,int lang,long o,long l) {
	if(str!=NULL && str->len>0) {
		long i,n;
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len && lang>=0 && lang<LANG_LANGS) {
			const char *p = str->ptr;
			const int *cl = comments_lang[lang];
			const char **cc = &comments[cl[0]],**cb = cl[3]? &comments[cl[2]] : 0;
			const int *ccl = &comments_len[cl[0]],*cbl = cl[3]? &comments_len[cl[2]] : 0;
			int j;
			for(i=o,n=o+l; i<n; ++i) {
				// Skip line comment
				for(j=0; j<cl[1]; ++j)
					if(p[i]==*cc[j] && (ccl[j]==1 || !strncmp(&p[i],cc[j],ccl[j]))) {
						for(i+=ccl[j]; !q_isbreak(p[i]); ++i);
						goto skip_comments_loop_end;
					}
				// Skip block comment
				if(cb && p[i]==*cb[0] && (cbl[0]==1 || !strncmp(&p[i],cb[0],cbl[0]))) {
					i = q_string_find(str,cb[1],i,l-(i-o),cbl[1]);
					if(i==-1) return o+l;
					goto skip_comments_loop_end;
				}
				return i;
				skip_comments_loop_end:
				;
			}
		}
		return o;
	}
	return -1;
}

int q_string_equals(const QString str,const char *s,long o,long l) {
	if(str!=NULL && str->len>0 && s!=NULL && *s!='\0') {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)strlen(s)+l;
		if(o>=0 && l>0 && o+l<=(long)str->len)
			return strncmp(&str->ptr[o],s,l)==0;
	}
	return 0;
}

int q_string_compare(const QString str,const char *s,long o,long l) {
	if(str!=NULL && str->len>0 && s!=NULL && *s!='\0') {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)strlen(s)+l;
		if(o>=0 && l>0 && o+l<=(long)str->len)
			return strncmp(&str->ptr[o],s,l);
	}
	return str!=NULL && str->len>0? -1 : 1;
}

size_t q_string_strip_comments(QString str,int lang,long o,long l) {
	long i,m = 0,n;
	if(o<0) o = (long)str->len+o;
	if(l<=0) l = (long)str->len-o+l;
//debug_output("String::stripComments(o=%d,l=%d)\n",(int)o,(int)l);
	if(o>=0 && l>0 && o+l<=(long)str->len && lang>=0 && lang<LANG_LANGS) {
		const int *cl = comments_lang[lang];
		const char **cc = &comments[cl[0]],**cb = cl[3]? &comments[cl[2]] : 0;
		const int *ccl = &comments_len[cl[0]],*cbl = cl[3]? &comments_len[cl[2]] : 0;
		long j,f;
//debug_output("cl[%d,%d,%d,%d]\n",cl[0],cl[1],cl[2],cl[3]);
//for(j=0; j<cl[1]; ++j)
//debug_output("cc[%d]: %s\n",j,cc[j]);
//if(cb)
//debug_output("cb: %s %s\n",cb[0],cb[1]);
		for(i=o,n=o+l,m=0; i+m<n; ++i) {
			if(q_isquote(str->ptr[i+m])) {
				f = q_string_match_quotes(str,i+m,n-(i+m));
				if(f==-1) f = n+1;
				if(m>0) for(; i+m<f; ++i) str->ptr[i] = str->ptr[i+m];
				else i = f;
			}
			// Strip line comment
			for(j=0; j<cl[1]; ++j)
				if(str->ptr[i+m]==*cc[j] && (ccl[j]==1 || !strncmp(&str->ptr[i+m],cc[j],ccl[j]))) {
//debug_output("String::stripComments(line[i=%d,m=%d]: %s)\n",(int)i,(int)m,cc[j]);
					for(m+=ccl[j]; !q_isbreak(str->ptr[i+m]); ++m);
				}
			// Strip block comment
			if(cb && str->ptr[i+m]==*cb[0] && (cbl[0]==1 || !strncmp(&str->ptr[i+m],cb[0],cbl[0]))) {
//debug_output("String::stripComments(block[i=%d,m=%d]: %s)\n",(int)i,(int)m,cb[0]);
				f = q_string_find(str,cb[1],i+m,l-(i+m-o),cbl[1]);
				if(f==-1) break;
				m = f-i+cbl[1];
			}
			if(m>0) str->ptr[i] = str->ptr[i+m];
		}
		if(m>0) {
			for(; i+m<(long)str->len; ++i) str->ptr[i] = str->ptr[i+m];
			str->ptr[i] = '\0';
		}
	}
//debug_output("String::str->ptripComments(m=%d)\n",(int)m);
	return (size_t)m;
}


void q_string_escape(QString str,long o,long l,const char *s,int f) {
	if(str!=NULL && str->ptr!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			int i,j,k,m,n;
			uint32_t c,u,u1;
			for(i=o,m=0,n=o+l; i<n; ++i) // Count number of characters to escape
				if((c=(uint32_t)str->ptr[i]) && strchr(escape_sequences,c) &&
							(!q_isquote(c) || (f&ESCAPE_QUOTE))) ++m; // Escape sequence
				else if(s && c && !(c&0x80) && strchr(s,c)) m += (f&ESCAPE_HEX)? 3 : 1; // Escape selected chars in s
				else if((c&0xc0)==0xc0 && (f&ESCAPE_UNICODE)) { // UTF8
					j = (c&0x20)? ((c&0x10)? ((c&0x08)? ((c&0x04)? 6 : 5) : 4) : 3) : 2;
//fprintf(stderr,"q_string_escape(unicode: c=%02x, j=%d)\n",c,j);
					if(j<6 || !(c&0x02)) {
						for(k=1,u=((c<<(j+1))&0xff)>>(j+1); k<j; ++k)
							u = (u<<6)|(str->ptr[++i]&0x3f);
						/*if(u>0xff) */m += ((u&0xffff0000)? 10 : 6)-j;
//fprintf(stderr,"q_string_escape(unicode: u=%x, n=%d)\n",u,(int)(((u&0xffff0000)? 10 : 6)-j));
					}
				}
//fprintf(stderr,"q_string_escape(m=%d)\n",m);
			if(m>0) { // Only escape strings containing sequences
				if(str->len+m>=str->cap) {
					q_string_resize(str,m);
					if(str->ptr==NULL) {
						str->len = str->cap = 0;
						return;
					}
				}
				if(o+l<(long)str->len) q_string_move(str,o,m);
				else str->len += m;
//fprintf(stderr,"q_string_escape(");
				for(i=o+l,n=m; i>=o && n>0; --i)
					// Escape sequence
					if((c=(uint32_t)str->ptr[i]) && strchr(escape_sequences,c) &&
							(!q_isquote(c) || (f&ESCAPE_QUOTE))) {
//fprintf(stderr,"(escape: %c)",c);
						str->ptr[i+n] = c<=0x0d && (c!='\n' || !(f&ESCAPE_SL_EOL))? escape_chars[c] : c;
						str->ptr[i+ --n] = '\\';
					} else if(s && c && !(c&0x80) && strchr(s,c)) { // Escape selected chars in s
//fprintf(stderr,"(escape: %c)",c);
						if(f&ESCAPE_HEX) {
							j = c&0x0f,str->ptr[i+n] = (char)(j<=9? '0'+j : 'A'+j-10);
							j = (c>>4)&0x0f,str->ptr[i+n-1] = (char)(j<=9? '0'+j : 'A'+j-10);
							str->ptr[i+n-2] = 'x',str->ptr[i+n-3] = '\\',n -= 3;
						} else str->ptr[i+n] = (char)c,str->ptr[i+n-1] = '\\',--n;
					} else if((c&0x80) && (f&ESCAPE_UNICODE)) { // Escape UTF8
//fprintf(stderr,"(unicode: %02x)",c);
						if((c&0xc0)==0xc0) {
							j = (c&0x20)? ((c&0x10)? ((c&0x08)? ((c&0x04)? 6 : 5) : 4) : 3) : 2;
							if(j<6 || !(c&0x02)) {
								for(k=1,u=((c<<(j+1))&0xff)>>(j+1); k<j; ++k)
									u = (u<<6)|(str->ptr[i+k]&0x3f);
//								if(u>0xff) {
									for(k=2,n+=j,j=((u&0xffff0000)? 10 : 6),u1=u; k<j; ++k)
										c = (u1&0xf),u1 >>= 4,str->ptr[i+n-1] = (char)(c<=9? '0'+c : 'A'+c-10),--n;
									str->ptr[i+n-1] = (u&0xffff0000)? 'U' : 'u',str->ptr[i+n-2] = '\\',n -= 2;
//								} else str->ptr[i+n] = (char)u;
							}
						}
					} else {
//fputc(c,stderr);
						str->ptr[i+n] = (char)c; // Ordinary char
					}
//fprintf(stderr,")\n");
			}
		}
	}
}

void q_string_unescape(QString str,long o,long l,int f) {
	if(str!=NULL && str->ptr!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			long i;
			for(i=o; i<=o+l; ++i) if(str->ptr[i]=='\\') break; // Skip string until first escape sequence.
			if(i<(long)str->len) {
				int j,m,n;
				uint32_t c,u;
				const char *p;
				char x1,x2;
				for(n=0; i+n<=(long)str->len; ++i)
					if(i<o+l) {
						if((c=str->ptr[i+n])=='\\') {
							++n,c = str->ptr[i+n];
							if(c!=' ' && (p=strchr(escape_chars,c)) &&
								(!q_isquote(c) || (f&ESCAPE_QUOTE))) str->ptr[i] = (char)(p-escape_chars);
							else if(c=='x' && (f&ESCAPE_HEX)) x1 = str->ptr[i+n+1],x2 = str->ptr[i+n+2],str->ptr[i] = (q_x(x1)<<4)|q_x(x2),n += 2;
							else if((c=='u' || c=='U') && (f&ESCAPE_UNICODE)) {
								for(j=0,m=(c=='u'? 4 : 8),u=0; j<m; ++j) x1 = str->ptr[i+n+1],u = (u<<4)|q_x(x1),++n;
								if(u>0x7f) {
									m = u>0x7ff? (u>0xffff? (u>0x1fffff? (u>0x3ffffff? 6 : 5) : 4) : 3) : 2,n -= m-1;
									c = (0xff<<(8-m))&0xff,str->ptr[i] = (char)(c|(u>>((m-1)*6)));
									for(j=1; j<m; ++j) str->ptr[++i] = (char)(0x80|((u>>((m-j-1)*6))&0x3f));
								} else str->ptr[i] = (char)u;
							} else str->ptr[i] = (char)c;
						} else str->ptr[i] = (char)c;
					}
				str->len -= n;
			}
		}
	}
}

static const DjynnHtmlEntity *select_entity(unsigned char c,int f) {
	if(((c=='\'' || c=='"') && !(f&HTML_QUOTE)) ||
		((c=='&') && !(f&HTML_AMP)) ||
		((c=='<' || c=='>') && !(f&HTML_LTGT))) return NULL;
	else {
		const DjynnHtmlEntity *e = &html_entities[c];
		if(e->len>0)
			if(*e->name=='#' && !(f&HTML_CODES)) return NULL;
		return e;
	}
}

void q_string_encode_html(QString str,long o,long l,int f) {
	if(str!=NULL && str->ptr!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			size_t i,n = 0;
			const DjynnHtmlEntity *e;
			for(i=o; i<o+l; ++i) {
				e = select_entity(str->ptr[i],f);
				if(e && e->len>0) n += e->len+1;
			}
			if(n) {
				if(str->len+n>=str->cap) q_string_resize(str,n);
				char *p0 = &str->ptr[o],*p1 = &str->ptr[o+l-1],*p2 = &str->ptr[o+l+n];
				if(o+l==str->len) *p2-- = '\0';
				else --p2;
				str->len += n;
				while(p1>=p0) {
					e = select_entity(*p1,f);
					if(e && e->len) {
						*p2 = ';',p2 -= e->len;
						memcpy(p2,e->name,e->len);
						--p2,*p2-- = '&',--p1;
					} else *p2-- = *p1--;
				}
			}
		}
	}
}

void q_string_decode_html(QString str,long o,long l) {
	if(str!=NULL && str->ptr!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			size_t i,n = 0;
			const DjynnHtmlEntity *e;
			char *p1 = &str->ptr[o],*p2 = p1,*p3 = &str->ptr[o+l];
			while(p1<=p3) {
				if(*p1=='&') {
					++p1;
					if(*p1=='#') {
						++p1,++n;
						if(*p1=='x' || *p1=='X') i = q_xtoi(++p1),++n;
						else i = atoi(p1);
						if(i>255) *p2++ = '?';
						else *p2++ = i;
						while(*p1!=';' && q_ishex(*p1)) ++p1,++n;
						++p1,++n;
					} else {
						for(i=0; i<255; i++) {
							e = &html_entities[i];
							if(e->len && *p1==*e->name && !strncmp(p1,e->name,e->len)) {
								*p2++ = i,p1 += e->len,n += e->len;
								break;
							}
						}
						if(*p1==';') ++p1,++n;
					}
				} else *p2++ = *p1++;
			}
			str->len -= n;
		}
	}
}

void q_string_encode_url(QString str,long o,long l) {
	if(str!=NULL && str->ptr!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			size_t i,n = 0;
			for(i=o; i<o+l; ++i) if(!q_isurl(str->ptr[i]) && str->ptr[i]!=' ') n += 2;
			if(n>0) {
				unsigned char c;
				char *p0 = &str->ptr[o],*p1 = &str->ptr[o+l-1],*p2 = &str->ptr[o+l+n];
				if(str->len+n>=str->cap) q_string_resize(str,n);
				if(o+l==str->len) *p2-- = '\0';
				else --p2;
				str->len += n;
				while(p1>=p0) {
					if(*p1==' ') *p2-- = '+',p1--;
					else if(!q_isurl(*p1)) {
						c = *p1, c &= 0xf,*p2-- = (c>9? 'A'+c-10 : '0'+c);
						c = *p1,c >>= 4,*p2-- = (c>9? 'A'+c-10 : '0'+c);
						*p2-- = '%',p1--;
					} else *p2-- = *p1--;
				}
			}
		}
	}
}

void q_string_decode_url(QString str,long o,long l) {
	if(str!=NULL && str->ptr!=NULL && str->len>0) {
		if(o<0) o = (long)str->len+o;
		if(l<=0) l = (long)str->len-o+l;
		if(o>=0 && l>0 && o+l<=(long)str->len) {
			size_t n = 0;
			int c1,c2;
			char *p1 = &str->ptr[o],*p2 = p1,*p3 = &str->ptr[o+l];
			while(p1<=p3) {
				if(*p1=='+') *p2++ = ' ',p1++;
				else if(*p1=='%') {
					c1 = p1[1],c1 = c1>='a' && c1<='f'? (c1+10-'a') : (c1>='A' && c1<='F'? (c1+10-'A') : (c1>='0' && c1<='9'? c1-'0' : -1));
					c2 = p1[2],c2 = c2>='a' && c2<='f'? (c2+10-'a') : (c2>='A' && c2<='F'? (c2+10-'A') : (c2>='0' && c2<='9'? c2-'0' : -1));
					if(c1==-1) *p2++ = *p1++;
					else if(c2==-1) p1 += 2,*p2++ = c1,n++;
					else p1 += 3,*p2++ = (c1<<4)|c2,n += 2;
				} else *p2++ = *p1++;
			}
			str->len -= n;
		}
	}
}


