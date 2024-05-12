
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "error.h"

#ifdef WIN32
#define strerror_r(errno,buf,len) strerror_s(buf,len,errno)
#endif

Q_ERR(__FILE__)


static const char *err_file = NULL;
static const char *err_time = NULL;
static void (*err_handler)(QError) = NULL;

static const char *err_status[] = { "Warning","Error","Critical Error" };

static void err_message(QError e,const char *msg,void (*cb)(const char *)) {
	int n = 32;
	if(e->file!=NULL) n += strlen(e->file);
	if(msg!=NULL && *msg!='\0') n += strlen(msg);
	else if(e->message!=NULL && *e->message!='\0') n += strlen(e->message);
	{
		char buf[n];
		n = 0;
		if(e->file!=NULL) n += sprintf(&buf[n],"%s",e->file);
		if(e->line>=0) n += sprintf(&buf[n],e->file!=NULL? ":%d" : "%d",e->line);
		if(e->status>=0) n += sprintf(&buf[n],e->file!=NULL || e->line>=0? ": %s[%x]" : "%s[%x]",err_status[e->status],e->id);
		if((msg!=NULL && *msg!='\0') || (e->message!=NULL && *e->message!='\0'))
			n += sprintf(&buf[n],e->file!=NULL || e->line>=0 || e->status>=0? ": %s" : "%s",msg!=NULL? msg : e->message);
		cb(buf);
	}
}

static void err_print(const char *msg) {
	char buf[33] = "";
	if(err_time!=NULL) {
		time_t t;
		struct tm *ti;
		time(&t);
		ti = localtime(&t);
		strftime(buf,32,err_time,ti);
	}
	fprintf(stderr,"%s%s\n",buf,msg);
	fflush(stderr);
}

const char *q_err_log(const char *fn,const char *tmf,int clean) {
	if(fn!=NULL && err_file==NULL) {
		int n;
		err_file = fn;
		n = open(err_file,O_WRONLY|(clean? O_TRUNC : O_APPEND)|O_CREAT,0644);
		if(n==-1) q_err(ERR_OPEN,"Could not open error log file.");
		else {
			close(STDERR_FILENO);
			dup2(n,STDERR_FILENO);
			close(n);
		}
	}
	err_time = tmf;
	return err_file;
}

void q_err_handler(void (*cb)(QError)) {
	err_handler = cb;
}

void q_err_r(int st,int id,const char *msg,const char *f,int ln) {
	struct _QError e = { f,ln,st,id,msg };
	char buf[257];

	if(msg==NULL && errno!=0) {
		strerror_r(errno,buf,256);
		e.message = buf;
	}

	if(err_handler==NULL) q_err_msg((const QError)&e,NULL);
	else err_handler((const QError)&e);
}

void q_err_msg(QError e,const char *msg) { err_message(e,msg,err_print); }



