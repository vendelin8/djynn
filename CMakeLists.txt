
# CMakeLists.txt


cmake_minimum_required(VERSION 2.8)
project(djynn)

set(PACKAGE "djynn")
set(PACKAGE_NAME "Djynn")
set(PACKAGE_TITLE "${PACKAGE_NAME} Geany Plugin")
set(PACKAGE_VERSION_MAJOR 1)
set(PACKAGE_VERSION_MINOR 0)
set(PACKAGE_VERSION_BUILD 0)
set(PACKAGE_MAINTAINER "Per Löwgren")
set(PACKAGE_BUGREPORT "per.lowgren@gmail.com")
set(PACKAGE_URL "https://github.com/perlowgren/Djynn")
set(PACKAGE_IS_LIBRARY 1)

option(ENABLE_NLS "Enable building of translations" ON)

set(USE_GTK3 3.24)
set(USE_GIO "gio-2.0 >= 2.32.0")
set(USE_GEANY 1)
set(USE_GRESOURCES 1)
include(libq.cmake)

configure_file(
	"${PROJECT_SOURCE_DIR}/src/_config.h"
	"${PROJECT_BINARY_DIR}/src/config.h"
)

set(djynn_src
	${gresources_src}
	libq/array.c
	libq/base58.c
	libq/base64.c
	libq/decimal.c
	libq/error.c
	libq/json.c
	libq/random.c
#	libq/rle.c
	libq/string.c
	libq/tree.c
	libq/glib/config.c
	libq/gtk/menu.c
	src/comment.c
	src/dialog.c
	src/djynn.c
	src/encode.c
	src/line.c
	src/project.c
	src/session.c
	src/vim.c
	src/workspace.c
)

add_library(${PACKAGE} MODULE ${djynn_src})
set_target_properties(${PACKAGE} PROPERTIES PREFIX "")

install(
	TARGETS
		${PACKAGE}
	DESTINATION
		"${PACKAGE_PLUGINS_DIR}"
)

if(GETTEXT_FOUND AND ENABLE_NLS)
	add_subdirectory(po)
endif()



