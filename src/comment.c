

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libq/array.h>
#include <libq/string.h>
#include <libq/gtk/menu.h>
#include "djynn.h"
#include "comment.h"


static void menu_activate(QMenuItem *menuitem) {
	if(menuitem!=NULL) {
		gboolean check = FALSE;
		if(menuitem->type==Q_MENU_CHECKBOX || menuitem->type==Q_MENU_RADIO)
			check = q_menu_is_checked(menuitem->id);
		djynn_comment_action(menuitem->id,check);
	}
}

enum {
	TOGGLE_COMMENT = 0x3001,
	TOGGLE_BLOCK,
	INSERT_DOXYGEN_COMMENT,
	STRIP_COMMENTS,
};

static QMenuItem comment_menu_items[] = {
/*   type          id                      label & tooltip                        icon  submenu   activate */
	{ Q_MENU_SEPARATOR },
	{ Q_MENU_LABEL, TOGGLE_COMMENT,         N_("Toggle Comment(s)"),NULL,          NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, TOGGLE_BLOCK,           N_("Toggle Block Comment(s)"),NULL,    NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, INSERT_DOXYGEN_COMMENT, N_("Insert Doxygen Comment(s)"),NULL,  NULL, NULL,     menu_activate },
	{ Q_MENU_LABEL, STRIP_COMMENTS,         N_("Strip Comments"),NULL,             NULL, NULL,     menu_activate },
	{ 0,0 }
};

static DjynnKeybind comment_keybindings[] = {
	{ TOGGLE_COMMENT,         "comment",         -1, NULL },
	{ TOGGLE_BLOCK,           "block_comment",   -1, NULL },
	{ INSERT_DOXYGEN_COMMENT, "doxygen_comment", -1, NULL },
	{ STRIP_COMMENTS,         "strip_comments",  -1, NULL },
	{ DJYNN_KEYBIND_END }
};


enum {
	LINE_COMMENT,
	BLOCK_COMMENT,
	DOXYGEN_COMMENT
};

#define DOXYGEN_LINE_INDENT 3
#define MIN_DOXYGEN_LINE_LEN 40
#define MAX_DOXYGEN_LINE_LEN 80


typedef struct _DjynnCommentStyle DjynnCommentStyle;

struct _DjynnCommentStyle {
	char *line;
	char *open;
	char *close;
	char *dox_line;
	char *dox_line_back;
	char *dox_open;
	char *dox_block;
	char *dox_close;

	struct {
		int line;
		int open;
		int close;
		int dox_line;
		int dox_line_back;
		int dox_open;
		int dox_block;
		int dox_close;
	} len;

	int lines;
	int blocks;
	int nested_blocks;
	int dox;
	int dox_blocks;
};

enum {
	STYLE_ASM,
	STYLE_C,
	STYLE_CAML,
	STYLE_F77,
	STYLE_FORTRAN,
	STYLE_HASKELL,
	STYLE_FREEBASIC,
	STYLE_LATEX,
	STYLE_LUA,
	STYLE_NSIS,
	STYLE_PASCAL,
	STYLE_PERL,
	STYLE_VHDL,
	STYLE_XML,
};

static DjynnCommentStyle comment_styles[] = {
	{ ";",	"",		"",		"",		"",		"",		"",		"",		{1,0,0,0,0,0,0,0},	1,0,0,0,0 }, // STYLE_ASM
	{ "//",	"/*",		"*/",		"//!",	"//!<",	"/**",	" *",		" */",	{2,2,2,3,4,3,2,3},	1,1,0,1,1 }, // STYLE_C
	{ "",		"(*",		"*)",		"",		"",		"",		"",		"",		{0,2,2,0,0,0,0,0},	0,1,0,0,0 }, // STYLE_CAML
	{ "c",	"",		"",		"",		"",		"",		"",		"",		{1,0,0,0,0,0,0,0},	1,0,0,0,0 }, // STYLE_F77
	{ "!",	"",		"",		"",		"",		"",		"",		"",		{1,0,0,0,0,0,0,0},	1,0,0,0,0 }, // STYLE_FORTRAN
	{ "--",	"{-",		"-}",		"",		"",		"",		"",		"",		{2,2,2,0,0,0,0,0},	1,1,1,0,0 }, // STYLE_HASKELL
	{ "'",	"",		"",		"",		"",		"",		"",		"",		{1,0,0,0,0,0,0,0},	1,0,0,0,0 }, // STYLE_FREEBASIC
	{ "%",	"",		"",		"",		"",		"",		"",		"",		{1,0,0,0,0,0,0,0},	1,0,0,0,0 }, // STYLE_LATEX
	{ "--",	"--[[",	"--]]",	"--!",	"--!<",	"--!",	"--!",	"--!",	{2,4,4,3,4,3,3,3},	1,1,0,1,0 }, // STYLE_LUA
	{ ";",	"/*",		"*/",		"",		"",		"",		"",		"",		{1,2,2,0,0,0,0,0},	1,1,0,0,0 }, // STYLE_NSIS
	{ "",		"{",		"}",		"",		"",		"",		"",		"",		{0,1,1,0,0,0,0,0},	0,1,0,0,0 }, // STYLE_PASCAL
	{ "#",	"",		"",		"#!",		"#!<",	"##",		"#",		"#",		{1,0,0,2,3,2,1,1},	1,0,0,1,0 }, // STYLE_PERL
	{ "--",	"",		"",		"--!",	"--!<",	"--!",	"--!",	"--!",	{2,0,0,3,4,3,3,3},	1,0,0,1,0 }, // STYLE_VHDL
	{ "",		"<!--",	"-->",	"",		"",		"",		"",		"",		{0,4,3,0,0,0,0,0},	0,1,0,0,0 }, // STYLE_XML
};


typedef struct _DjynnFileTypeStyle DjynnFileTypeStyle;

struct _DjynnFileTypeStyle {
	char *name;
	int style;
};

static DjynnFileTypeStyle filetype_styles[] = {
	{ "ActionScript",	STYLE_C				},	//		//
	{ "Ada",				STYLE_VHDL			},	//		--
	{ "ASM",				STYLE_ASM			},	//		;
	{ "C",				STYLE_C				},	//		/* */
	{ "C#",				STYLE_C				},	//		//
	{ "C++",				STYLE_C				},	//		//
	{ "CAML",			STYLE_CAML			},	//		(* *)
	{ "CMake",			STYLE_PERL			},	//		#
	{ "Conf",			STYLE_PERL			},	//		#
	{ "CSS",				STYLE_C				},	//		/* */
	{ "D",				STYLE_C				},	//		//
	{ "Diff",			STYLE_PERL			},	//		#
	{ "Docbook",		STYLE_XML			},	//		<!-- -->
	{ "F77",				STYLE_F77			},	//		c
	{ "Ferite",			STYLE_C				},	//		/* */
	{ "Fortran",		STYLE_FORTRAN		},	//		!
	{ "FreeBasic",		STYLE_FREEBASIC	},	//		'
	{ "GLSL",			STYLE_C				},	//		/* */
	{ "Haskell",		STYLE_HASKELL		},	//		--
	{ "Haxe",			STYLE_C				},	//		//
	{ "HTML",			STYLE_XML			},	//		<!-- -->
	{ "Java",			STYLE_C				},	//		/* */
	{ "Javascript",	STYLE_C				},	//		//
	{ "LaTeX",			STYLE_LATEX			},	//		%
	{ "Lua",				STYLE_LUA			},	//		--
	{ "Make",			STYLE_PERL			},	//		#
	{ "Matlab",			STYLE_LATEX			},	//		%
	{ "NSIS",			STYLE_NSIS			},	//		;
	{ "Pascal",			STYLE_PASCAL		},	//		{ }
	{ "Perl",			STYLE_PERL			},	//		#
	{ "PHP",				STYLE_C				},	//		//
	{ "Po",				STYLE_PERL			},	//		#
	{ "Python",			STYLE_PERL			},	//		#
	{ "R",				STYLE_PERL			},	//		#
	{ "Ruby",			STYLE_PERL			},	//		#
	{ "Sh",				STYLE_PERL			},	//		#
	{ "SQL",				STYLE_C				},	//		/* */
	{ "Tcl",				STYLE_PERL			},	//		#
	{ "Vala",			STYLE_C				},	//		//
	{ "Verilog",		STYLE_C				},	//		/* */
	{ "VHDL",			STYLE_VHDL			},	//		--
	{ "XML",				STYLE_XML			},	//		<!-- -->
	{ "YAML",			STYLE_PERL			},	//		#
{NULL,0}};


static QArray styles = NULL;
static char *eol;
static int eol_len = 0;

static void skip_string(char **s) {
	char *s1 = *s,c = *s1;
	for(s1++; *s1!='\0' && *s1!=c; ++s1) {
		if(*s1=='\\') {
			++s1;
			if(eol_len==2 && *s1==*eol && s1[1]==eol[1]) ++s1;
		}
	}
	*s = s1+1;
}

static void copy_string(char **d,char **s) {
	char *d1 = *d,*s1 = *s,c = *s1;
	for(*d1++=*s1++; *s1!='\0' && *s1!=c; *d1++=*s1++) {
		if(*s1=='\\') {
			*d1++ = *s1++;
			if(eol_len==2 && *s1==*eol && s1[1]==eol[1]) *d1++ = *s1++;
		}
	}
	*d1++ = *s1++,*d = d1,*s = s1;
}

static int find_line_comment(DjynnCommentStyle *c,gchar *text) {
	int i = 0,n = 0;
	gchar *p1 = text,p0 = '\n';
	// If first line starts with line comment return true.
	while(*p1!='\0') { // Search for line comments at start of line.
		if((*p1=='"' || *p1=='\'') && i==0) {
			p0 = *p1;
			skip_string(&p1);
		}
		if(p0=='\n' && *p1==*c->line && !strncmp(p1,c->line,c->len.line)) return 1; 
		else if(c->lines && *p1==*c->line && !strncmp(p1,c->line,c->len.line)) p0 = *p1,p1 += c->len.line,i = 1;
		else if(c->lines && i==1 && *p1==*eol && (eol_len==1 || p1[1]==eol[1])) p0 = '\n',p1 += eol_len,i = 0;
		else if(c->blocks && *p1==*c->open && !strncmp(p1,c->open,c->len.open)) p0 = *p1,p1 += c->len.open,i = 2,++n;
		else if(c->blocks && *p1==*c->close && !strncmp(p1,c->close,c->len.close)) {
			p0 = *p1,p1 += c->len.close,n--;
			if(!c->nested_blocks || n==0) i = 0;
		} else if(*p1==*eol && (eol_len==1 || p1[1]==eol[1])) p0 = '\n',p1 += eol_len;
		else p0 = *p1++;
	}
	return 0;
}

static int find_block_comment(DjynnCommentStyle *c,gchar *text) {
	int i = 0;
	gchar *p1 = text;
	while(*p1!='\0') { // Search for block comments inside selected text.
		if((*p1=='"' || *p1=='\'') && i==0) skip_string(&p1);
		if((*p1==*c->open && !strncmp(p1,c->open,c->len.open)) ||
			(*p1==*c->close && !strncmp(p1,c->close,c->len.close))) return 1;
		else if(c->lines && *p1==*c->line && !strncmp(p1,c->line,c->len.line)) p1 += c->len.line,i = 1;
		else if(c->lines && i==1 && *p1==*eol && (eol_len==1 || p1[1]==eol[1])) p1 += eol_len,i = 0;
		else ++p1;
	}
	return 0;
}

static void toggle_block_comment(ScintillaObject *sci,DjynnCommentStyle *c,gint start,gint end) {
	gchar *text = sci_get_contents_range(sci,start,end);
	int i = 0,len = strlen(text);
	gchar buf[len+129];
	gchar *p1 = text,*p2 = buf;
	if(!c->nested_blocks && find_block_comment(c,p1)) { // Remove block comments.
		while(*p1!='\0') {
			if((*p1=='"' || *p1=='\'') && i==0) copy_string(&p2,&p1);
			else if(*p1==*c->open && !strncmp(p1,c->open,c->len.open)) p1 += c->len.open,i = 2;
			else if(*p1==*c->close && !strncmp(p1,c->close,c->len.close)) p1 += c->len.close,i = 0;
			else if(c->lines && *p1==*c->line && !strncmp(p1,c->line,c->len.line)) {
				memcpy(p2,c->line,c->len.line);
				p2 += c->len.line,p1 += c->len.line,i = 1;
			} else if(c->lines && i==1 && *p1==*eol && (eol_len==1 || p1[1]==eol[1])) {
				if(eol_len==2) *p2++ = *p1++;
				*p2++ = *p1++,i = 0;
			} else *p2++ = *p1++;
		}
		*p2 = '\0';
	} else if(c->nested_blocks && !strncmp(p1,c->open,c->len.open)) { // Remove nested block comment.
		int n = 0;
		p1 += c->len.open;
		while(*p1!='\0') {
			if((*p1=='"' || *p1=='\'') && i==0) skip_string(&p1);
			else if(*p1==*c->open && !strncmp(p1,c->open,c->len.open)) {
				memcpy(p2,c->open,c->len.open);
				p2 += c->len.open,p1 += c->len.open,i = 2,++n;
			} else if(*p1==*c->close && !strncmp(p1,c->close,c->len.close)) {
				if(--n==0) {
					strcpy(p2,p1+c->len.close);
					p2 += strlen(p2);
					break;
				} else {
					memcpy(p2,c->close,c->len.close);
					p2 += c->len.close,p1 += c->len.close,i = 0;
				}
			} else if(c->lines && *p1==*c->line && !strncmp(p1,c->line,c->len.line)) {
				memcpy(p2,c->line,c->len.line);
				p2 += c->len.line,p1 += c->len.line,i = 1;
			} else if(c->lines && i==1 && *p1==*eol && (eol_len==1 || p1[1]==eol[1])) {
				if(eol_len==2) *p2++ = *p1++;
				*p2++ = *p1++,i = 0;
			} else *p2++ = *p1++;
		}
		*p2 = '\0';
	} else { // Insert block comment.
		sprintf(p2,"%s%s%s",c->open,p1,c->close);
	}
	sci_replace_sel(sci,buf);
	g_free(text);
}

typedef struct _DjynnFunctionData DjynnFunctionData;

struct _DjynnFunctionData {
	int indent;
	char *name;
	char *ret_type;
	int nparams;
	char **params;
};

static void extract_function(ScintillaObject *sci,gint line,DjynnFunctionData *fd) {
	gchar *text;
	char *p1;
	text = sci_get_line(sci,line);
	for(fd->indent=0,p1=text; *p1=='\t'; ++p1,++fd->indent);
	if(strchr(text,'(')!=NULL) {
		int i,lines = sci_get_line_count(sci);
		char f[1025],*p2,*p3;
		strcpy(f,text);
		for(i=1; strchr(f,')')==NULL && i<=20 && line+i<lines; ++i) {
			g_free(text);
			text = sci_get_line(sci,line+i);
			strcat(f,text);
		}
		if(i<=20) {
			q_strwhsp(f);
//debug_output("function: %s\n",f);
			p2 = strchr(f,'('),*p2++ = '\0';
			if((p1=strrchr(f,'*'))!=NULL || (p1=strrchr(f,' '))!=NULL) ++p1;
			else p1 = f;
			fd->name = strdup(p1);
			if(p1!=f) {
				*p1 = '\0';
				q_strwhsp(f);
				fd->ret_type = strdup(f);
			} else fd->ret_type = NULL;
			p1 = p2,p2 = strchr(p1,')'),*p2 = '\0';
			if(*p1=='\0') fd->nparams = 0;
			else {
				fd->nparams = q_strnchr(p1,',')+1;
				fd->params = (char **)malloc(sizeof(char *)*(fd->nparams*2));
				for(i=0; p1!=NULL && *p1!='\0'; i+=2,p1=p2) {
					if((p2=strchr(p1,','))!=NULL) *p2++ = '\0';
					if((p3=strrchr(p1,'*'))!=NULL || (p3=strrchr(p1,' '))!=NULL) ++p3;
					if(p3!=NULL) {
						fd->params[i+1] = strdup(p3);
						*p3 = '\0';
						q_strwhsp(p1);
						fd->params[i] = strdup(p1);
					} else {
						fd->params[i] = strdup("");
						fd->params[i+1] = strdup(p1);
					}
				}
			}
//debug_output("name=%s, ret_type=%s, nparams=%d\n",fd->name,fd->ret_type,fd->nparams);
//for(i=0; i<fd->nparams; ++i)
//debug_output("param[%d]=[%s] %s\n",i,fd->params[i*2],fd->params[i*2+1]);
		}
	}
	g_free(text);
}

static void delete_function(DjynnFunctionData *fd) {
	if(fd->name!=NULL) free(fd->name);
	if(fd->ret_type!=NULL) free(fd->ret_type);
	if(fd->nparams>0) {
		int i;
		for(i=0; i<fd->nparams*2; ++i) free(fd->params[i]);
		free(fd->params);
	}
	free(fd);
}

static void toggle_comment(int st) {
	GeanyDocument *doc = document_get_current();

//debug_output("toggle_comment(%p)\n",doc);

	if(doc==NULL) return;
	else {
		ScintillaObject *sci = doc->editor->sci;
		DjynnCommentStyle *c = (DjynnCommentStyle *)q_array_get_pointer(styles,doc->file_type->name);
		if(c==NULL) {
//			msgwin_status_add(_("Djynn does not support comments for this file type."));
			ui_set_statusbar(FALSE,_("Djynn does not support comments for this file type."));
		} else if(st==DOXYGEN_COMMENT && !c->dox) {
//			msgwin_status_add(_("Djynn does not support comments for this file type."));
			ui_set_statusbar(FALSE,_("Doxygen does not support this language."));
		} else {
			gint line = sci_get_current_line(sci);
			gboolean sel = sci_has_selection(sci);
			gint pos,start,end,start_line,end_line,end_col;
			gchar *text,*buf,*p1,*p2;
			int i = scintilla_send_message(sci,SCI_GETEOLMODE,0,0);
			switch(i) {
				case SC_EOL_CRLF:eol = "\r\n",eol_len = 2;break;
				case SC_EOL_CR:eol = "\r",eol_len = 1;break;
				default:eol = "\n",eol_len = 1;break;
			}
			pos = sci_get_current_position(sci);
			start = sci_get_selection_start(sci);
			end = sci_get_selection_end(sci);
//debug_output("toggle_comment(file_type=%s,st=%d)\n",doc->file_type->name,st);
			if(sel) { // If there is a selection
				if((st==BLOCK_COMMENT && c->blocks) || (st==LINE_COMMENT && !c->lines)) {
//debug_output("toggle_block_comment()\n");
					toggle_block_comment(sci,c,start,end);
				} else {
					start_line = sci_get_line_from_position(sci,start);
					end_line = sci_get_line_from_position(sci,end);
//debug_output("toggle_comment(start_line=%d,end_line=%d)\n",start_line,end_line);
					if(end_line>start_line) { // If many lines are selected.
						QArray arr = q_array_new(0,0);
						QType val;
						start = sci_get_position_from_line(sci,start_line);
						end_col = sci_get_col_from_position(sci,end);
						if(end_col>0) ++end_line;
						end = sci_get_position_from_line(sci,end_line);
						text = sci_get_contents_range(sci,start,end);
//debug_output("toggle_comment(text:\n%s)\n",text);
						q_array_split(arr,text,eol,ARR_SPLIT_EMPTY_ITEMS);
//debug_output("toggle_comment(lines=%d)\n",(int)q_array_size(arr));
						if(st==LINE_COMMENT || st==BLOCK_COMMENT) {
//debug_output("LINE_COMMENT || BLOCK_COMMENT\n");
							if(find_line_comment(c,text)) { // Remove line comments.
								for(; q_array_each(arr,&val); ) {
									p1 = (char *)val.s;
									if(*p1==*c->line && !strncmp(p1,c->line,c->len.line))
										q_array_replace(arr,&p1[c->len.line]);
								}
							} else { // Insert line comments.
								for(; q_array_each(arr,&val); ) {
									p1 = (char *)val.s;
									if(*p1=='\0') continue;
									p2 = (char *)malloc(sizeof(char)*(c->len.line+strlen(p1)+1));
									strcpy(p2,c->line);
									strcat(p2,p1);
									q_array_replace(arr,p2);
									free(p2);
								}
							}
						} else if(st==DOXYGEN_COMMENT) { // Insert doxygen comments for many lines.
//debug_output("DOXYGEN_COMMENT\n");
							QType v1,v2;
							q_array_min(arr,&v1);
							q_array_max(arr,&v2);
							q_array_reset(arr);
							if(q_array_size(arr)>=3 && // Insert a doxygen group comment when first and last selected line is empty.
									*v1.s=='\0' && *v2.s=='\0') {
								int t;
								for(q_array_min(arr,&v1); v1.t==ARR_STRING && (v2=q_array_next_value(arr)).t==ARR_STRING; q_array_next(arr,&v1))
									if(*v1.s=='\0' && *v2.s!='\0') break;
								if(v1.t==ARR_STRING && v2.t==ARR_STRING) {
									QArrayIter iter1/*,iter2 = NULL*/;
									iter1 = q_array_get_iter(arr);
									p1 = v2.s;
									q_array_reset(arr);
									for(q_array_max(arr,&v1); v1.t==ARR_STRING && (v2=q_array_previous_value(arr)).t==ARR_STRING; q_array_previous(arr,&v1))
										if(*v1.s=='\0' && *v2.s!='\0') break;
									if(v1.t==ARR_STRING && v2.t==ARR_STRING) {
//debug_output("n1=%p\tn2=%p\n",iter1,iter2);
										for(t=0; *p1=='\t'; ++p1,++t);
										;
//debug_output("p1=%s\tt=%d\n",p1,t);
										{
											char t1[t+1]; // Indentation
											char buf[1025];

											for(i=0; i<t; ++i) t1[i] = '\t';
											t1[i] = '\0';

											p1 = buf;
											p1 += sprintf(p1,"%s%s @}",t1,c->dox_open);
											if(c->dox_blocks) p1 += sprintf(p1,"%s",c->dox_close);
											strcpy(p1,eol);
											q_array_replace(arr,buf);

											p1 = buf;
											p1 += sprintf(p1,"%s%s%s @name %s%s%s @{",eol,t1,c->dox_open,eol,t1,c->dox_block);
											if(c->dox_blocks) p1 += sprintf(p1,"%s",c->dox_close);
											q_array_set_iter(arr,iter1);
											q_array_replace(arr,buf);
										}
									}
								}
							} else { // Insert back comment after each line.
								int tab = sci_get_tab_width(sci),len,ilen,max = 0,line_max = 0,ind = -1;
								char ind_str[257] = "";
								for(; q_array_each(arr,&val); ) {
									p1 = val.s;
									if(*p1=='\0' || (ind>0 && strncmp(p1,ind_str,ind)!=0) || strstr(p1,c->dox_line)!=NULL) continue;
									len = strlen(p1);
									for(p2=p1; *p2!='\0'; ++p2) if(*p2=='\t') len += tab-1;
									ilen = len+DOXYGEN_LINE_INDENT;
									if(ilen<=MAX_DOXYGEN_LINE_LEN && ilen>max) max = ilen;
									if(len>line_max) line_max = len;
									if(ind<0) {
										for(p2=p1,ind=0; *p2!='\0'; ++p2)
											if(*p2==' ' || *p2=='\t') ++ind;
											else break;
										if(ind>0) {
											if(ind>256) ind = 256;
											strncpy(ind_str,p1,ind);
											ind_str[ind] = '\0';
										}
									}
								}
//debug_output("max: %d\nline_max: %d\nind: %d\nind_str: \"%s\"\n",max,line_max,ind,ind_str);
								if(max<MIN_DOXYGEN_LINE_LEN) max = MIN_DOXYGEN_LINE_LEN;
								if(max>MAX_DOXYGEN_LINE_LEN) max = MAX_DOXYGEN_LINE_LEN;
								buf = (char *)g_malloc(sizeof(char)*(line_max+max+17));
								for(; q_array_each(arr,&val); ) {
									p1 = val.s;
									if(*p1=='\0' || (ind>0 && strncmp(p1,ind_str,ind)!=0) || strstr(p1,c->dox_line)!=NULL) continue;

									len = strlen(p1);
									memcpy(buf,p1,len);
									for(p2=p1,ilen=len; *p2!='\0'; ++p2) if(*p2=='\t') ilen += tab-1;

									p2 = buf+len;
									if(ilen+DOXYGEN_LINE_INDENT>MAX_DOXYGEN_LINE_LEN) { // If line too long, place comment on new line.
										*p2++ = *eol;
										if(eol_len==2) *p2++ = eol[1];
										ilen = 0;
									}
									p2 = q_repeat(p2,' ',0,max-ilen);
									p2 += sprintf(p2,"%s ",c->dox_line_back);
									if(!c->lines) p2 += sprintf(p2,"%s",c->dox_close);
//debug_output("line: %s\n",buf);
									q_array_replace(arr,buf);
								}
								g_free(buf);
							}
						}
//q_array_print(arr);
						buf = q_array_join(arr,eol,0);
//debug_output("buf=%s\n",buf);
						sci_set_selection_start(sci,start);
						sci_set_selection_end(sci,end);
						sci_replace_sel(sci,buf);
						free(buf);
						g_free(text);
						q_array_free(arr);
					} else { // If selection is within one line.
						if(st==LINE_COMMENT) { // Selecting text within one line becomes a block comment.
							toggle_block_comment(sci,c,start,end);
						} else if(st==DOXYGEN_COMMENT) {
						}
					}
				}
			} else { // No selected text.
				if(st==BLOCK_COMMENT) {
					if(c->blocks) {
						char buf[32];
						sprintf(buf,"%s  %s",c->open,c->close);
						sci_insert_text(sci,pos,buf);
					}
				} else {
					if(st==LINE_COMMENT) { // Toggle comment for one line without selected text.
						start_line = sci_get_line_from_position(sci,start);
						text = sci_get_line(sci,start_line);
						start = sci_get_position_from_line(sci,start_line);
						if(*text==*c->line && !strncmp(text,c->line,c->len.line)) {
							sci_set_selection_start(sci,start);
							sci_set_selection_end(sci,start+c->len.line);
							sci_replace_sel(sci,"");
						} else {
							sci_insert_text(sci,start,c->line);
						}
						g_free(text);
					} else if(st==DOXYGEN_COMMENT) {
						char str[2049];
						DjynnFunctionData *fd = (DjynnFunctionData *)malloc(sizeof(DjynnFunctionData));
						memset(fd,0,sizeof(DjynnFunctionData));
						extract_function(sci,line,fd);
						{
							char t[fd->indent+1];
							for(i=0,p1=t; i<fd->indent; ++i) *p1++ = '\t';
							*p1 = '\0';
							p1 = str;
							p1 += sprintf(p1,"%s%s%s",t,c->dox_open,eol);
							for(i=0; i<fd->nparams; ++i)
								p1 += sprintf(p1,"%s%s @param %s %s",t,c->dox_block,fd->params[i*2+1],eol);
							if(fd->ret_type!=NULL &&
									(strstr(fd->ret_type,"void")==NULL || strstr(fd->ret_type,"void*")!=NULL))
								p1 += sprintf(p1,"%s%s @return %s",t,c->dox_block,eol);
							if(c->dox_blocks) p1 += sprintf(p1,"%s%s%s",t,c->dox_close,eol);
							pos = sci_get_position_from_line(sci,line);
							sci_insert_text(sci,pos,str);
						}
						delete_function(fd);
					}
				}
			}
		}
	}
}

static void strip_comments() {
	GeanyDocument *doc = document_get_current();

//	DjynnCommentStyle *c = (DjynnCommentStyle *)q_array_get_pointer(styles,doc->file_type->name);
//	if(c!=NULL) {

	/* strip_comments uses the q_string_strip_comments function,
	 * which hasn't implemented all file type. */
	GeanyFiletype *filetype = doc->file_type;
	int langs[GEANY_MAX_BUILT_IN_FILETYPES];
	int i;
	for(i=0; i<GEANY_MAX_BUILT_IN_FILETYPES; ++i) langs[i] = -1;
	langs[GEANY_FILETYPES_PHP]        = LANG_PHP;
//	langs[GEANY_FILETYPES_BASIC]      = LANG_BASIC;
//	langs[GEANY_FILETYPES_MATLAB]     = LANG_MATLAB;
//	langs[GEANY_FILETYPES_RUBY]       = LANG_RUBY;
//	langs[GEANY_FILETYPES_LUA]        = LANG_LUA;
//	langs[GEANY_FILETYPES_FERITE]     = LANG_FERITE;
//	langs[GEANY_FILETYPES_YAML]       = LANG_YAML;
	langs[GEANY_FILETYPES_C]          = LANG_C;
//	langs[GEANY_FILETYPES_NSIS]       = LANG_NSIS;
//	langs[GEANY_FILETYPES_GLSL]       = LANG_GLSL;
//	langs[GEANY_FILETYPES_PO]         = LANG_PO;
//	langs[GEANY_FILETYPES_MAKE]       = LANG_MAKE;
//	langs[GEANY_FILETYPES_TCL]        = LANG_TCL;
	langs[GEANY_FILETYPES_XML]        = LANG_XML;
//	langs[GEANY_FILETYPES_CSS]        = LANG_CSS;
//	langs[GEANY_FILETYPES_REST]       = LANG_REST;
	langs[GEANY_FILETYPES_HASKELL]    = LANG_HASKELL;
	langs[GEANY_FILETYPES_JAVA]       = LANG_JAVA;
//	langs[GEANY_FILETYPES_CAML]       = LANG_CAML;
//	langs[GEANY_FILETYPES_AS]         = LANG_AS;
//	langs[GEANY_FILETYPES_R]          = LANG_R;
//	langs[GEANY_FILETYPES_DIFF]       = LANG_DIFF;
	langs[GEANY_FILETYPES_HTML]       = LANG_XML;
//	langs[GEANY_FILETYPES_PYTHON]     = LANG_PYTHON;
//	langs[GEANY_FILETYPES_CS]         = LANG_CS;
	langs[GEANY_FILETYPES_PERL]       = LANG_PERL;
//	langs[GEANY_FILETYPES_VALA]       = LANG_VALA;
//	langs[GEANY_FILETYPES_PASCAL]     = LANG_PASCAL;
//	langs[GEANY_FILETYPES_LATEX]      = LANG_LATEX;
//	langs[GEANY_FILETYPES_ASM]        = LANG_ASM;
	langs[GEANY_FILETYPES_CONF]       = LANG_CFG;
//	langs[GEANY_FILETYPES_HAXE]       = LANG_HAXE;
	langs[GEANY_FILETYPES_CPP]        = LANG_CPP;
	langs[GEANY_FILETYPES_SH]         = LANG_BASH;
//	langs[GEANY_FILETYPES_FORTRAN]    = LANG_FORTRAN;
	langs[GEANY_FILETYPES_SQL]        = LANG_SQL;
//	langs[GEANY_FILETYPES_F77]        = LANG_F77;
//	langs[GEANY_FILETYPES_DOCBOOK]    = LANG_DOCBOOK;
//	langs[GEANY_FILETYPES_D]          = LANG_D;
	langs[GEANY_FILETYPES_JS]         = LANG_JAVASCRIPT;
//	langs[GEANY_FILETYPES_VHDL]       = LANG_VHDL;
//	langs[GEANY_FILETYPES_ADA]        = LANG_ADA;
//	langs[GEANY_FILETYPES_CMAKE]      = LANG_CMAKE;
//	langs[GEANY_FILETYPES_MARKDOWN]   = LANG_MARKDOWN;
//	langs[GEANY_FILETYPES_TXT2TAGS]   = LANG_TXT2TAGS;
//	langs[GEANY_FILETYPES_ABC]        = LANG_ABC;
//	langs[GEANY_FILETYPES_VERILOG]    = LANG_VERILOG;
//	langs[GEANY_FILETYPES_FORTH]      = LANG_FORTH;
//	langs[GEANY_FILETYPES_LISP]       = LANG_LISP;
//	langs[GEANY_FILETYPES_ERLANG]     = LANG_ERLANG;
//	langs[GEANY_FILETYPES_COBOL]      = LANG_COBOL;
//	langs[GEANY_FILETYPES_OBJECTIVEC] = LANG_OBJECTIVEC;
//	langs[GEANY_FILETYPES_ASCIIDOC]   = LANG_ASCIIDOC;
//	langs[GEANY_FILETYPES_ABAQUS]     = LANG_ABAQUS;
//	langs[GEANY_FILETYPES_BATCH]      = LANG_BATCH;
//	langs[GEANY_FILETYPES_POWERSHELL] = LANG_POWERSHELL;
//	langs[GEANY_FILETYPES_RUST]       = LANG_RUST;
	if(langs[filetype->id]==-1) {
		ui_set_statusbar(FALSE,_("Djynn does not support strip comments for this file type."));
	} else {
		DjynnTextSelection selection;
		djynn_get_text_selection(&selection,TRUE,FALSE);
		if(selection.text==NULL) return;
		if(selection.length>0) {
			QString string = q_string_new();
			q_string_insert(string,0,selection.text);
			q_string_strip_comments(string,langs[filetype->id],0,0);

			if(selection.all) sci_set_text(selection.scintilla,(const gchar *)q_string_chars(string));
			else sci_replace_sel(selection.scintilla,(const gchar *)q_string_chars(string));

			q_string_free(string);
		}
		g_free(selection.text);
	}
}

void djynn_comment_init(GeanyData *data) {
	djynn_comment_create();
	if(!djynn_cfg->comment)
		djynn_comment_disable();
}

void djynn_comment_create() {
	if(djynn_widget->comment_menu!=NULL && comment_menu_items[0].widget==NULL) {
		q_menu_create(GTK_MENU_SHELL(djynn_widget->comment_menu),comment_menu_items,Q_MENU_APPEND,0);
	}
	if(styles==NULL) {
		int i;
		DjynnFileTypeStyle *fts;
		styles = q_array_new(0,0);
		for(i=0; 1; ++i) {
			fts = &filetype_styles[i];
			if(fts->name==NULL) break;
			q_array_put_pointer(styles,fts->name,(void *)&comment_styles[fts->style]);
		}
	}
	djynn_comment_update_menu(TRUE);
	djynn_keybind(comment_keybindings);
}


void djynn_comment_cleanup() {
	gint i;
	for(i=0; comment_menu_items[i].type!=0; ++i)
		if(comment_menu_items[i].widget!=NULL) {
			gtk_widget_destroy(comment_menu_items[i].widget);
			comment_menu_items[i].widget = NULL;
		}
	if(styles!=NULL) {
		q_array_free(styles);
		styles = NULL;
	}
	djynn_comment_update_menu(FALSE);
}

void djynn_comment_action(gint id,gboolean check) {
	if(!djynn_cfg->comment) return;
	switch(id) {
		case TOGGLE_COMMENT:toggle_comment(LINE_COMMENT);break;
		case TOGGLE_BLOCK:toggle_comment(BLOCK_COMMENT);break;
		case INSERT_DOXYGEN_COMMENT:toggle_comment(DOXYGEN_COMMENT);break;
		case STRIP_COMMENTS:strip_comments();break;
	}
}

gint djynn_comment_keybindings() {
	return djynn_count_keybindings(comment_keybindings);
}

void djynn_comment_enable() {
	gint i;
	for(i=0; comment_menu_items[i].type!=0; ++i)
		if(comment_menu_items[i].widget!=NULL)
			gtk_widget_show(comment_menu_items[i].widget);
	djynn_comment_update_menu(TRUE);
}

void djynn_comment_disable() {
	gint i;
	for(i=0; comment_menu_items[i].type!=0; ++i)
		if(comment_menu_items[i].widget!=NULL)
			gtk_widget_hide(comment_menu_items[i].widget);
	djynn_comment_update_menu(FALSE);
}

void djynn_comment_update_menu(gboolean show) {
	if(show && djynn_cfg->replace_geany_comments) {
		gtk_widget_hide(djynn_widget->geany.comment_line);
		gtk_widget_hide(djynn_widget->geany.uncomment_line);
		gtk_widget_hide(djynn_widget->geany.toggle_line_commentation);
		if(comment_menu_items[0].widget!=NULL)
			gtk_widget_hide(comment_menu_items[0].widget);
	} else {
		gtk_widget_show(djynn_widget->geany.comment_line);
		gtk_widget_show(djynn_widget->geany.uncomment_line);
		gtk_widget_show(djynn_widget->geany.toggle_line_commentation);
		if(comment_menu_items[0].widget!=NULL) {
			if(djynn_cfg->comment) gtk_widget_show(comment_menu_items[0].widget);
			else gtk_widget_hide(comment_menu_items[0].widget);
		}
	}
}
