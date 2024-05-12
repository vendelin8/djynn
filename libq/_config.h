#ifndef _LIBQ_CONFIG_H_
#define _LIBQ_CONFIG_H_

/**
 * @file libq/_config.h
 * @author Per Löwgren
 * @date Modified: 2015-09-01
 * @date Created: 2015-07-03
 */

#define PACKAGE                   "@PACKAGE@"
#define PACKAGE_NAME              "@PACKAGE_NAME@"
#define PACKAGE_TITLE             "@PACKAGE_TITLE@"
#define PACKAGE_VERSION_MAJOR     @PACKAGE_VERSION_MAJOR@
#define PACKAGE_VERSION_MINOR     @PACKAGE_VERSION_MINOR@
#define PACKAGE_VERSION_BUILD     @PACKAGE_VERSION_BUILD@
#define PACKAGE_VERSION           "@PACKAGE_VERSION@"
#define PACKAGE_MAINTAINER        "@PACKAGE_MAINTAINER@"
#define PACKAGE_MAINTAINER_EMAIL  "@PACKAGE_MAINTAINER_EMAIL@"
#define PACKAGE_BUGREPORT         "@PACKAGE_BUGREPORT@"
#define PACKAGE_URL               "@PACKAGE_URL@"
#define PACKAGE_STRING            "@PACKAGE_STRING@"
#define PACKAGE_TARNAME           "@PACKAGE_TARNAME@"
#define PACKAGE_TARGET            "@PACKAGE_TARGET@"
#define PACKAGE_YEAR              "@PACKAGE_YEAR@"

#cmakedefine PACKAGE_IS_LIBRARY

#define PACKAGE_INSTALL_PREFIX    "@PACKAGE_INSTALL_PREFIX@"
#define PACKAGE_BINARY_DIR        "@PACKAGE_BINARY_DIR@"
#define PACKAGE_LIBRARY_DIR       "@PACKAGE_LIBRARY_DIR@"
#define PACKAGE_INCLUDE_DIR       "@PACKAGE_INCLUDE_DIR@"
#define PACKAGE_SHARE_DIR         "@PACKAGE_SHARE_DIR@"
#define PACKAGE_DATA_DIR          "@PACKAGE_DATA_DIR@"
#define PACKAGE_ICONS_DIR         "@PACKAGE_ICONS_DIR@"
#define PACKAGE_PIXMAPS_DIR       "@PACKAGE_PIXMAPS_DIR@"
#define PACKAGE_LOCALE_DIR        "@PACKAGE_LOCALE_DIR@"
#define PACKAGE_PLUGINS_DIR       "@PACKAGE_PLUGINS_DIR@"

#define VERSION                   "@PACKAGE_VERSION@"

#cmakedefine HAVE_STDLIB_H
#cmakedefine HAVE_STDDEF_H
#cmakedefine HAVE_STDINT_H
#cmakedefine HAVE_INTTYPES_H
#cmakedefine HAVE_MEMORY_H
#cmakedefine HAVE_STRING_H
#cmakedefine HAVE_STRINGS_H
#cmakedefine HAVE_UNISTD_H
#cmakedefine HAVE_SYS_STAT_H
#cmakedefine HAVE_SYS_TYPES_H

#cmakedefine HAVE_MALLOC
#cmakedefine HAVE_REALLOC
#cmakedefine HAVE_FLOOR
#cmakedefine HAVE_POW
#cmakedefine HAVE_EXP
#cmakedefine HAVE_LOG
#cmakedefine HAVE_SQRT
#cmakedefine HAVE_MEMSET
#cmakedefine HAVE_STRCHR
#cmakedefine HAVE_STRDUP
#cmakedefine HAVE_STRNICMP
#cmakedefine HAVE_STRPBRK
#cmakedefine HAVE_STRSTR
#cmakedefine HAVE_VPRINTF

#cmakedefine USE_SQLITE3
#cmakedefine USE_GTK2
#cmakedefine USE_GLIB
#cmakedefine USE_CAIRO
#cmakedefine USE_GIO
#cmakedefine USE_WEBKIT
#cmakedefine USE_SOURCEVIEW
#cmakedefine USE_JPEG
#cmakedefine USE_TIFF
#cmakedefine USE_GEANY
#cmakedefine USE_SCINTILLA

#if !defined(HAVE_STRNICMP) && defined(HAVE_STRINGS_H)
#define strnicmp strncasecmp
#define HAVE_STRNICMP
#endif

#cmakedefine ENABLE_NLS

#ifdef PACKAGE_IS_LIBRARY
#define GETTEXT_PACKAGE PACKAGE
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#ifdef NDEBUG
#define debug_output(...)
#define debug_putc(c)
#else
#define debug_output(...) fprintf(stderr,__VA_ARGS__)
#define debug_putc(c) fputc(c,stderr)
#endif

#endif /* _LIBQ_CONFIG_H_ */


