#ifndef _LIBQ_ERROR_H_
#define _LIBQ_ERROR_H_

/**
 * @file libq/error.h  
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2014-05-22
 */ 

#include "libq.h"


#ifdef Q_DEBUG
#define Q_ERR(f) \
static const char *__file__ = f;
#define __line__ __LINE__
#define q_dbg_printf(...) fprintf(stderr,__VA_ARGS__)
#define q_dbg_putc(c) fputc(c,stderr)
#else
#define Q_ERR(f)
#define __file__ NULL
#define __line__ -1
#define q_dbg_printf(...)
#define q_dbg_putc(c)
#endif


enum {
	ERR_OPEN = (__LIBQ_ERROR__<<16),
	ERR_MALLOC,
	ERR_NULL_POINTER,
};

enum {
	ERR_WARNING,
	ERR_ERROR,
	ERR_CRITICAL
};

typedef struct _QError _QError;
typedef const _QError * const QError;

struct _QError {
	const char *file;
	int line;
	int status;
	int id;
	const char *message;
};

const char *q_err_log(const char *fn,const char *tmf,int clean);
void q_err_handler(void (*cb)(QError));

void q_err_r(int st,int id,const char *msg,const char *f,int ln);

#ifdef Q_DEBUG
#define q_warning(id,msg) q_err_r(ERR_WARNING,(id),(msg),__file__,__line__)
#define q_warning_line(id,msg,ln) q_err_r(ERR_WARNING,(id),(msg),__file__,(ln))
#define q_err(id,msg) q_err_r(ERR_ERROR,(id),(msg),__file__,__line__)
#define q_err_line(id,msg,ln) q_err_r(ERR_ERROR,(id),(msg),__file__,(ln))
#define q_critical(id,msg) q_err_r(ERR_CRITICAL,(id),(msg),__file__,__line__)
#define q_critical_line(id,msg,ln) q_err_r(ERR_CRITICAL,(id),(msg),__file__,(ln))
#else
#define q_warning(id,msg) q_err_r(ERR_WARNING,(id),NULL,__file__,__line__)
#define q_warning_line(id,msg,ln) q_err_r(ERR_WARNING,(id),NULL,__file__,(ln))
#define q_err(id,msg) q_err_r(ERR_ERROR,(id),NULL,__file__,__line__)
#define q_err_line(id,msg,ln) q_err_r(ERR_ERROR,(id),NULL,__file__,(ln))
#define q_critical(id,msg) q_err_r(ERR_CRITICAL,(id),NULL,__file__,__line__)
#define q_critical_line(id,msg,ln) q_err_r(ERR_CRITICAL,(id),NULL,__file__,(ln))
#endif

void q_err_msg(QError e,const char *msg);

#define q_error_message q_err_msg

#endif /* _LIBQ_ERROR_H_ */

