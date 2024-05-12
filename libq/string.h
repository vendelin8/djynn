#ifndef _LIBQ_STRING_H_
#define _LIBQ_STRING_H_

/**
 * @file libq/string.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2012-12-23
 */ 

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "libq.h"
#include "type.h"


#define _DEF_TO_STR(z) #z
#define DEF_TO_STR(x) _DEF_TO_STR(x)


/** Languages defined for comment-handling.
 * @see size_t stripComments(int,long,long) */
enum {
	LANG_BASH,
	LANG_C,
	LANG_CPP,
	LANG_CFG,
	LANG_HASKELL,
	LANG_HTML,
	LANG_INI,
	LANG_JAVA,
	LANG_JAVASCRIPT,
	LANG_PERL,
	LANG_PHP,
	LANG_PROPERTIES,
	LANG_SHELL,
	LANG_SQL,
	LANG_XML,
	LANG_LANGS,
};

/** Escape flags.
 * @see void escape(long o,long l,const char *,int) */
enum {
	ESCAPE_QUOTE		= 0x00000001,	//!< Escape/Unescape quotes.
	ESCAPE_SL_EOL		= 0x00000002,	//!< Escape '\n' so that it becomes '\\' and '\n' instead of "\\n".
	ESCAPE_HEX			= 0x00000004,	//!< Escape/Unescape hexadecimal characters, so that '~' becomes "\\x7F".
	ESCAPE_UNICODE		= 0x00000008,	//!< Escape/Unescape unicode characters.
	ESCAPE				= 0x00000009,	//!< Default for escape(). Escape quotes, breaks, tabs and unicode.
	UNESCAPE				= 0x0000000f,	//!< Default for unescape(). Unescape all escape sequences.
};

/** Tokenizing flags
 * @see long matchToken(const char *,long,long,int) */
enum {
	TOKEN_LTRIM			= 0x00000001,	//!< Trim tokens on left side.
	TOKEN_RTRIM			= 0x00000002,	//!< Trim tokens on right side.
	TOKEN_TRIM			= 0x00000004,	//!< Trim tokens on either side.
	TOKEN_ESCAPE		= 0x00000008,	//!< Include escape sequences in tokens.
	TOKEN_DELIM_CH		= 0x00000000,	//!< Each char in the delimiter matches.
	TOKEN_DELIM_STR	= 0x00000010,	//!< The entire string is used as delimiter.
};

/** Match flags
 * @see long matchValue(int,long,long,int) */
enum {
	MATCH_COMMENTS		= 0x00000001,	//!< Include comments in match.
};

/** Encode HTML flags
 * @see void encodeHTML(int) */
enum {
	HTML_QUOTE			= 0x00000001,	//!<
	HTML_AMP				= 0x00000002,	//!<
	HTML_LTGT			= 0x00000004,	//!<
	HTML_NAMED			= 0x000000ff,	//!<
	HTML_CODES			= 0x00000100,	//!<
	HTML_UNICODE		= 0x00000200,	//!<
	HTML_ALL				= 0xffffffff,	//!<
};

extern const char *q_blank;
extern const char *q_endl;
extern const char *q_whitespace;

/** Cyclic Redundancy Check
 * Reads in a string s as a command-line argument, and prints out
 * its 32 bit Cyclic Redundancy Check (CRC32 or Ethernet / AAL5 or ITU-TSS).
 * 
 * Uses direct table lookup.
 * @param str String to calcualte hash from.
 * @param cins Case insensitive, if true calculates hash based on lower case of the string.
 * @return Hashvalue. */
uint32_t q_crc32(const char *str,int cins);

/** Cyclic Redundancy Check
 * Reads in a string s as a command-line argument, and prints out
 * its 32 bit Cyclic Redundancy Check (CRC32 or Ethernet / AAL5 or ITU-TSS).
 * 
 * Uses direct table lookup.
 * @param str String to calcualte hash from.
 * @param o Offset position.
 * @param l Length of string, if zero or negative length is calculated from the entire string length and backward.
 * @param cins Case insensitive, if true calculates hash based on lower case of the string.
 * @return Hashvalue. */
uint32_t q_crc32n(const char *str,long o,long l,int cins);

/** Cyclic Redundancy Check
 * Reads in a string s as a command-line argument, and prints out
 * its 64 bit Cyclic Redundancy Check (CRC64 or Ethernet / AAL5 or ITU-TSS).
 * 
 * Uses direct table lookup.
 * @param str String to calcualte hash from.
 * @param cins Case insensitive, if true calculates hash based on lower case of the string.
 * @return Hashvalue. */
uint64_t q_crc64(const char *str,int cins);

/** Cyclic Redundancy Check
 * Reads in a string s as a command-line argument, and prints out
 * its 64 bit Cyclic Redundancy Check (CRC64 or Ethernet / AAL5 or ITU-TSS).
 * 
 * Uses direct table lookup.
 * @param str String to calcualte hash from.
 * @param o Offset position.
 * @param l Length of string, if zero or negative length is calculated from the entire string length and backward.
 * @param cins Case insensitive, if true calculates hash based on lower case of the string.
 * @return Hashvalue. */
uint64_t q_crc64n(const char *str,long o,long l,int cins);

#if __SIZEOF_LONG__ == 8
#define q_crc   q_crc64
#define q_crcn  q_crc64n
#else
#define q_crc   q_crc32
#define q_crcn  q_crc32n
#endif

/** Turns a char containing a hexadecimal value into a decimal integer value. */
int q_x(char c);

/** Turns an integer with the value 0x0-0xf into a corresponding hexadecimal char '0'-'f' (lower case). */
char q_tox(int c);

/** Turns an integer with the value 0x0-0xf into a corresponding hexadecimal char '0'-'f' (upper case). */
char q_toX(int c);

/** Convert hexadecimal string to decimal integer value.
 * @param str String to convert.
 * @return Unsigned integer. */
uint64_t q_xtoi(const char *str);

/** Convert decimal integer value into a hexadecimal string (lower case).
 * @param str Buffer to write hexadecimal string to.
 * @param i Unsigned decimal integer to convert to hexadecimal.
 * @return Address of str. */
char *q_itox(char *str,uint64_t i);

/** Convert decimal integer value into a hexadecimal string (upper case).
 * @param str Buffer to write hexadecimal string to.
 * @param i Unsigned decimal integer to convert to hexadecimal.
 * @return Address of str. */
char *q_itoX(char *str,uint64_t i);

/** Convert integer to string.
 * @param str Buffer to write to (must be at least 22 chars in length).
 * @param n Integer to convert.
 * @param len Minimum length in chars, if shorter string is written it's padded with pad. If n is negative minus is placed left-most. For no padding set len to zero.
 * @param pad Char to pad with.
 * @return Address of str. */
char *q_itostr(char *str,int64_t n,int len,char pad);

/** Convert double to string.
 * Writes at least dmin decimals, up to dmax decimals with a rounded last decimal. Removes trailing zeros.
 * @param str Buffer to write to (no check is made for length, it needs to be large enough).
 * @param d Double value.
 * @param dmin Minimum number of decimals.
 * @param dmax Maximum number of decimals (at most 15, but 12 or less gives correct precision).
 * @return Address of str. */
char *q_dtostr(char *str,double d,int dmin,int dmax);

/** Macro that returns lower case of a char, if it is an upper case latin letter (A-Z). */
#define q_lower(c) (((c)>='A' && (c)<='Z')? (c)+32 : (c))

/** Converts a string to lower case. No buffer is used.
 * @param str String to convert.
 * @return Address of str. */
char *q_tolower(char *str);

/** Macro that returns upper case of a char, if it is a lower case latin letter (a-z). */
#define q_upper(c) (((c)>='a' && (c)<='z')? (c)-32 : (c))

/** Converts a string to upper case. No buffer is used.
 * @param str String to convert.
 * @return Address of str. */
char *q_toupper(char *str);

/** Returns the character for certain escape sequences without the slash, eg. 'n' for '\n'.
 * If it's not an escape sequence character, returns c. */
#define q_esc(c) ((c)=='\0'? '0' : (c)=='\t'? 't' : (c)=='\n'? 'n' : (c)=='\r'? 'r' : (c)=='\f'? 'f' : (c)=='\v'? 'v' : (c))

/** Returns the escape sequence for certain characters without the slash, eg. '\n' for 'n'.
 * If it's not an escape sequence character, returns c. */
#define q_unesc(c) ((c)=='0'? '\0' : (c)=='t'? '\t' : (c)=='n'? '\n' : (c)=='r'? '\r' : (c)=='f'? '\f' : (c)=='v'? '\v' : (c))

/** Case insensitive string comparison.
 * @param str1 String 1.
 * @param str2 String 2.
 * @return Zero on equal, else non-zero. */
int q_stricmp(const char *str1,const char *str2);

/** Case insensitive string comparison.
 * @param str1 String 1.
 * @param str2 String 2.
 * @param n Number of chars to compare.
 * @return Zero on equal, else non-zero. */
int q_strnicmp(const char *str1,const char *str2,size_t n);

/** Search string for substring, case insensitive.
 * @param str1 Haystack.
 * @param str2 Needle.
 * @return If found, address of location, else NULL. */
char *q_stristr(char *str1,const char *str2);

/** Remove white space from string, separating words with one space. No buffer is used.
 * @param str String */
char *q_strwhsp(char *str);

/** Count number of char c in string.
 * @param str Haystack.
 * @param c Needle
 * @return Number of times c is found in str. */
int q_strnchr(const char *str,char c);

/** Get an allocated substring.
 * @param str String in which to extract substring.
 * @param o Offset, if <0 from end of string.
 * @param l Length, if 0 full length, if <0 from end of string.
 * @return An allocated string, remember to use free(). */
char *q_substr(const char *str,long o,long l);

/** Reapeat char c for a string of length l at offset o
 * @param str String in which to write repeated chars
 * @param c Char to repeat
 * @param o Offset, if <0 from end of string.
 * @param l Length, if 0 full length, if <0 to end of string.
 * @return Address at o+l (end of repeated chars) */
char *q_repeat(char *str,char c,long o,long l);

int q_tokens(char *str,const char *delim,int cins);
char **q_split(char **list,char *str,const char *delim,int cins);

void q_reverse(char *str,long o,long l);

/** Trim a string from white space.
 * @param str String to trim.
 * @param s If set should contain a string of chars to trim, if NULL is the static variable q_whitespace.
 * @see void trim(const char *,int) */
size_t q_trim(char *str,const char *s);

/** Move o and skip space or chars in s, to the left.
 * @param o Offset, if negative from end of string.
 * @param s Chars to skip through, default String::whitespace.
 * @return Position of first space (or chars in s) left of o.  */
long q_pos_left(const char *str,long o,const char *s);

/** Move o and skip space or chars in s, to the right.
 * @param o Offset, if negative from end of string.
 * @param s Chars to skip through, default String::whitespace.
 * @return Position of first character not space (or char in s) right of o.  */
long q_pos_right(const char *str,long o,const char *s);

void q_print_utf8(char *d,const char *s,size_t o,size_t l);

/** Free string using free(), and set to NULL. */
void q_strfree(char **str);
/** Same as strdup, but only applied if str!=s; also free str unless NULL. */
void q_strpdup(char **str,const char *s);
/** Same as q_strpdup but use a formatted string.. */
void q_strpdupf(char **str,const char *fmt, ...);

#define q_islower(c) ((c)>='a' && (c)<='z')
#define q_isupper(c) ((c)>='A' && (c)<='Z')
#define q_isalpha(c) (q_islower(c) || q_isupper(c))
#define q_islatin(c) (((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z'))
#define q_isdigit(c) ((c)>='0' && (c)<='9')
int q_isnumeric(const char *str);
#define q_ishex(c) (((c)>='0' && (c)<='9') || ((c)>='a' && (c)<='f') || ((c)>='A' && (c)<='F'))
#define q_isword(c) (q_isalpha(c) || q_isdigit(c) || (c)=='_')
#define q_isspace(c) ((c)==' ' || (c)=='\n' || (c)=='\t' || (c)=='\r' || (c)=='\f' || (c)=='\v')
#define q_isescspace(c) ((c)=='\t' || (c)=='\n' || (c)=='\r' || (c)=='\f' || (c)=='\v')
#define q_isurl(c) (q_isdigit(c) || q_isupper(c) || q_islower(c) || (c)=='-' || (c)=='.' || (c)=='_')
int q_ishtmlent(unsigned char c);
#define q_isbreak(c) ((c)=='\n' || (c)=='\r')
#define q_isprint(c) ((c)>='\x20' && (c)<='\x7e')
#define q_ispunct(c) (q_isprint(c) && !q_islower(c) && !q_isupper(c) && (c)!=' ' && !q_isdigit(c))
#define q_isgraph(c) ((c)>='\x21' && (c)<='\x7e')
#define q_iscntrl(c) ((c)<='\x1F' || (c)=='\x7f')
#define q_isquote(c) ((c)=='\'' || (c)=='"')
#define q_isutf8(c) ((c)&0x80)

char *q_stralloc(char **str,size_t len,size_t *cap,long l);
long q_strmove(char **str,size_t *len,size_t *cap,long o,long l);

#define q_s_resize              q_string_resize
#define q_s_move                q_string_move

#define q_s_add                 q_string_append
#define q_s_addn                q_string_appendn
#define q_s_addc                q_string_append_char
#define q_s_addi                q_string_append_int
#define q_s_addf                q_string_append_float
#define q_s_adds                q_string_append_string
#define q_s_addh                q_string_append_hex
#define q_s_addbase             q_string_append_base
#define q_string_add            q_string_append
#define q_string_addn           q_string_appendn
#define q_string_add_char       q_string_append_char
#define q_string_add_chars      q_string_append_chars
#define q_string_add_int        q_string_append_int
#define q_string_add_float      q_string_append_float
#define q_string_add_string     q_string_append_string
#define q_string_add_hex        q_string_append_hex
#define q_string_add_base       q_string_append_base
#define q_string_addf           q_string_appendf
#define q_string_vaddf          q_string_vappendf

#define q_s_upper               q_string_upper
#define q_s_lower               q_string_lower
#define q_s_chars               q_string_chars
#define q_s_char                q_string_char
#define q_s_length              q_string_length
#define q_s_capacity            q_string_capacity

#define q_string_resize(str,l)	q_stralloc(&(str)->ptr,(str)->len,&(str)->cap,l)
#define q_string_move(str,o,l)	q_strmove(&(str)->ptr,&(str)->len,&(str)->cap,o,l)

#define q_string_new            q_s_new
#define q_string_free           q_s_free
#define q_string_clear          q_s_clear
#define q_string_dup            q_s_dup
#define q_string_insert         q_s_ins
#define q_string_insertn        q_s_insn
#define q_string_insert_char    q_s_insc
#define q_string_insert_chars   q_s_inscn
#define q_string_insert_int     q_s_insi
#define q_string_insert_float   q_s_insf
#define q_string_insert_string  q_s_inss
#define q_string_insert_hex     q_s_insh
#define q_string_insert_base    q_s_insbase

#define q_string_print          q_s_print
#define q_string_escape         q_s_escape
#define q_string_unescape       q_s_unescape

typedef struct _QString _QString;
typedef _QString *QString;

struct _QString {
	size_t len;
	size_t cap;
	char *ptr;
};

QString q_string_new();
void q_string_free(QString str);
QString q_string_clear(QString str);
QString q_string_dup(const QString str);

QString q_string_insert(QString str,long n,const char *s);
QString q_string_insertn(QString str,long n,const char *s,long o,long l);
QString q_string_insert_char(QString str,long n,char c);
QString q_string_insert_chars(QString str,long n,char c,int l);
QString q_string_insert_int(QString str,long n,int64_t i);
QString q_string_insert_float(QString str,long n,double f,int p);
QString q_string_insert_string(QString str,long n,const QString s);
QString q_string_insert_hex(QString str,long n,uint64_t i,int upper);
QString q_string_insert_base(QString str,long n,uint64_t i,int base);
QString q_string_insertf(QString str,long n,const char *f, ...);
QString q_string_vinsertf(QString str,long n,const char *f,va_list list);

#define q_string_append(str,s)           q_string_insert((str),(str)->len,s)
#define q_string_appendn(str,s,o,l)      q_string_insertn((str),(str)->len,s,o,l)
#define q_string_append_char(str,c)      q_string_insert_char((str),(str)->len,c)
#define q_string_append_chars(str,c,l)   q_string_insert_chars((str),(str)->len,c,l)
#define q_string_append_int(str,i)       q_string_insert_int((str),(str)->len,i)
#define q_string_append_float(str,f,p)   q_string_insert_float((str),(str)->len,f,p)
#define q_string_append_string(str,s)    q_string_insert_string((str),(str)->len,s)
#define q_string_append_hex(str,i,upper) q_string_insert_hex((str),(str)->len,i,upper)
#define q_string_append_base(str,i,base) q_string_insert_base((str),(str)->len,i,base)
#define q_string_appendf(str, ...)       q_string_insertf((str),(str)->len,__VA_ARGS__)
#define q_string_vappendf(str,f,list)    q_string_vinsertf((str),(str)->len,f,list)

QString q_string_includef(QString str,const char *format, ...);
QString q_string_include(QString str,const char *fn);
QString q_string_finclude(QString str,FILE *fp);

void q_string_print(const QString str);

/** @name Find, Match & Skip
 * @{ */

/** Find a string.
 * @param str String to search.
 * @param s String to find.
 * @param o Offset position.
 * @param l Length in string to search.
 * @param sl Length of s, if zero or negative length is calculated from the entire length and backward.
 * @return Position in string where string is, or -1 if not found. */
long q_string_find(const QString str,const char *s,long o,long l,long sl);

/** Find a char.
 * @param str String to search.
 * @param c Char to find.
 * @param o Offset position.
 * @param l Length in string to search.
 * @return Position in string where char is, or -1 if not found. */
long q_string_find_char(const QString str,char c,long o,long l);

/** Find any char from s in string.
 * @param str String to search.
 * @param s Any chars in string s will be matched.
 * @param o Offset position.
 * @param l Length in string to search.
 * @return Position in string where char is, or -1 if not found. */
long q_string_find_chars(const QString str,const char *s,long o,long l);

/** @see long q_string_find(const QString,const char *,long,long,long) */
#define q_string_find_string(str,s,o,l) q_string_find((str),(s)->ptr,o,l,(s)->len)

/** Match nested tags.
 * Returns the position at the beginning of the last ending tag. For the string
 * "abc [b]def [b]ghi[/b] jkl[/b] mno." will return 25, the position at the last "[/b]".
 * 
 * For example, you may wish to match the last tag for a nested table in a HTML-document, from position p,
 * and so you call <tt>matchNestedTags("<table","</table>",p,0," \n\t>")</tt>.
 * 
 * Another example can be matching nested curly-braces, then you call <tt>matchNestedTags("{","}",p)</tt>.
 * @param tag1 Opening tag, does not have to be complete, such as "[b", but make sure c1 is set.
 * @param tag2 Closing tag, does not have to be complete, such as "[/b", but make sure c2 is set.
 * @param o Offset, if negative offset is counted from ending of string.
 * @param l Length to search in string, if zero or negative length is calculated from the entire string length and backward.
 * @param c1 String of chars to accept completing of tag1, e.g. for tag1="[b" and c1=" \n\t]" then "[b ar]" will match, but "[bar]" will not match.
 * @param c1 String of chars to accept completing of tag2, e.g. for tag2="[/b" and c2=" \n\t]" then "[/b ar]" will match, "but "[/bar]" will not match.
 * @return Zero based index of beginning of last nested tag, or -1 on fail. */
long q_string_match_tags(const QString str,const char *tag1,const char *tag2,long o,long l,const char *c1,const char *c2);

/** Match a quoted string.
 * Returns the position after the ending quote sign of the string. Escaped quotes are included
 * in the match so that "abc\"def"@ will return 11, or the position at '@'. If the char at o
 * is not a quot-sign - " or ', o is returned. Ending quote-sign has to be the same as the
 * starting quote-sign.
 * @param o Offset, if negative offset is counted from ending of string.
 * @param l Length to search in string, if zero or negative length is calculated from the entire string length and backward.
 * @return Zero based index after ending quote sign, or -1 on fail. */
long q_string_match_quotes(const QString str,long o,long l);

/** Match a string value.
 * Similar to matchQuotes, except will accept an unquoted string and in such a case instead of returning
 * at end quote, returns at first white space before end of line or first white space before a comment.
 * Type of comment is defined by lang.
 * @param delim Delimiter separating tokens.
 * @param o Offset, if negative offset is counted from ending of string.
 * @param l Length to search in string, if zero or negative length is calculated from the entire string length and backward.
 * @param f Flags defined by the TOKEN_* enums.
 * @return Zero based index after ending quote sign, or -1 on fail. */
long q_string_match_token(const QString str,const char *delim,long o,long l,int f);

/** Match a string value.
 * Similar to matchQuotes, except will accept an unquoted string and in such a case instead of returning
 * at end quote, returns at first white space before end of line or first white space before a comment.
 * Type of comment is defined by lang.
 * @param lang Language by which comments to match.
 * @param o Offset, if negative offset is counted from ending of string.
 * @param l Length to search in string, if zero or negative length is calculated from the entire string length and backward.
 * @return Zero based index after ending quote sign, or -1 on fail. */
long q_string_match_value(const QString str,int lang,long o,long l,int f);

long q_string_skip_comment(const QString str,int lang,long o,long l);
long q_string_skip_comments(const QString str,int lang,long o,long l);
/** @} */

/** @name Equals
 * @{ */

/** Compare string (or substring) with s and return true if equal, or false. If o and l is set,
 * a substring is compared, otherwise the entire string.
 * @param s String to compare.
 * @param o Offset position.
 * @param l Length in string to compare.
 * @return True if equal, otherwise false. */
int q_string_equals(const QString str,const char *s,long o,long l);

/** @see int q_string_equals(const QString,const char *,long,long) */
#define q_string_equals_string(str,s,o,l) q_string_equals((str),(s)->ptr,o,l)
/** @} */


/** @name Compare
 * @{ */

/** Compare string (or substring) with s and return result. If o and l is set,
 * a substring is compared, otherwise the entire string.
 * @see int strncmp (const char *,const char *,size_t)
 * @param s String to compare.
 * @param o Offset position.
 * @param l Length in string to compare.
 * @return Zero if equal, a positive value if string is greater than s, otherwise a negative value. */
int q_string_compare(const QString str,const char *s,long o,long l);

/** @see int q_string_compare(const QString,const char *,long,long) */
#define q_string_compare_string(str,s,o,l) q_string_compare((str),(s)->ptr,o,l)
/** @} */

/** @name Count
 * @{ */

/** Count the number of times s appears in string.
 * @param s String to look for.
 * @return Number of times s is found in string. */
size_t q_string_count(const QString str,const char *s);

/** Count the number of times c appears in string.
 * @param c Char to look for.
 * @return Number of times c is found in string. */
size_t q_string_count_char(const QString str,char c);
/** @} */

/** @name Replace
 * @{ */
//size_t q_string_replace(QString str,const char *s,const char *r);
//size_t q_string_replace(QString str,const char *s, ...);
//size_t q_string_replace(QString str,const char **arr);
/** @} */

/** @name Strip
 * @{ */
size_t q_string_strip_comments(QString str,int lang,long o,long l);
size_t q_string_strip_html(QString str);
/** @} */

/** @name Substring
 * @{ */
//size_t q_string_substr(QString str,String *s,long o,long l) const { if(s) return substr(*s,o,l);return 0; }
//size_t q_string_substr(QString str,String &s,long o,long l) const;
//size_t q_string_substr(QString str,char *s,long o,long l) const;
/** @} */

void q_string_escape(QString str,long o,long l,const char *s,int f);
void q_string_unescape(QString str,long o,long l,int f);

void q_string_encode_html(QString str,long o,long l,int f);
void q_string_decode_html(QString str,long o,long l);
void q_string_encode_url(QString str,long o,long l);
void q_string_decode_url(QString str,long o,long l);

#define q_string_upper(str)        q_toupper((str)->ptr)
#define q_string_lower(str)        q_tolower((str)->ptr)
#define q_string_chars(str)        ((const char * const)((str)->ptr))
#define q_string_char(str,i)       ((i)>=0 && (i)<(str)->len? ((str)->ptr[i]) : '\0')
#define q_string_length(str)       ((int)((str)->len))
#define q_string_capacity(str)     ((int)((str)->cap))

#endif /* _LIBQ_STRING_H_ */

