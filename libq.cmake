
include(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
include(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

if(UNIX AND NOT APPLE)
	set(LINUX 1)
endif()

if(LINUX)
	set(PACKAGE_TARGET "Linux")
elseif(MINGW)
	set(PACKAGE_TARGET "MingW")
endif()

set(LIBQ_LIBRARIES m)
string(TIMESTAMP YEAR "%Y")

set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_BUILD}")
set(PACKAGE_STRING "${PACKAGE_TITLE} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE}")
set(PACKAGE_YEAR ${YEAR})

if(WIN32)
	set(CYAN "")
	set(BLUE "")
	set(RED "")
	set(GREEN "")
	set(BROWN "")
	set(YELLOW "")
	set(NONE "")
else()
	string(ASCII 27 _escape)
	set(CYAN "${_escape}[0;36m")
	set(BLUE "${_escape}[0;34m")
	set(RED "${_escape}[1;31m")
	set(GREEN "${_escape}[0;32m")
	set(BROWN "${_escape}[0;33m")
	set(YELLOW "${_escape}[1;33m")
	set(NONE "${_escape}[0m")
endif()

message(STATUS "${BROWN}CMAKE_ROOT: ${CMAKE_ROOT}${NONE}")
message(STATUS "${BROWN}CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}${NONE}")

find_package(PkgConfig REQUIRED)
find_package(Gettext REQUIRED)

CHECK_INCLUDE_FILE("stdlib.h"    HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("stddef.h"    HAVE_STDDEF_H)
CHECK_INCLUDE_FILE("stdint.h"    HAVE_STDINT_H)
CHECK_INCLUDE_FILE("inttypes.h"  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("memory.h"    HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("string.h"    HAVE_STRING_H)
CHECK_INCLUDE_FILE("strings.h"   HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("unistd.h"    HAVE_UNISTD_H)
CHECK_INCLUDE_FILE("sys/stat.h"  HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)

CHECK_FUNCTION_EXISTS(malloc   HAVE_MALLOC)
CHECK_FUNCTION_EXISTS(realloc  HAVE_REALLOC)
CHECK_FUNCTION_EXISTS(floor    HAVE_FLOOR)
CHECK_FUNCTION_EXISTS(pow      HAVE_POW)
CHECK_FUNCTION_EXISTS(exp      HAVE_EXP)
CHECK_FUNCTION_EXISTS(log      HAVE_LOG)
CHECK_FUNCTION_EXISTS(sqrt     HAVE_SQRT)
CHECK_FUNCTION_EXISTS(memset   HAVE_MEMSET)
CHECK_FUNCTION_EXISTS(strchr   HAVE_STRCHR)
CHECK_FUNCTION_EXISTS(strdup   HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(strnicmp HAVE_STRNICMP)
CHECK_FUNCTION_EXISTS(strpbrk  HAVE_STRPBRK)
CHECK_FUNCTION_EXISTS(strstr   HAVE_STRSTR)
CHECK_FUNCTION_EXISTS(vprintf  HAVE_VPRINTF)

if(USE_THREADS)
	find_package(Threads REQUIRED)
	if(CMAKE_USE_PTHREADS_INIT)
		set(USE_PTHREADS 1)
	else()
		if(CMAKE_USE_WIN32_THREADS_INIT)
			set(USE_WIN32_THREADS 1)
		endif()
	endif()
endif()

if(USE_SQLITE3)
	CHECK_INCLUDE_FILE("sqlite3.h" HAVE_SQLITE3_H)
	if(NOT HAVE_SQLITE3_H)
		message(FATAL_ERROR "${RED}Sqlite required!${NONE}")
	endif()
endif()

if(USE_GTK2)
	find_package(GTK2 ${USE_GTK2} REQUIRED)
#	if(MINGW)
#		PKG_CHECK_MODULES(GTK2 ${USE_GTK2})
#	else()
#		find_package(GTK2 ${USE_GTK2} REQUIRED)
#	endif()
	if(NOT GTK2_FOUND)
		message(FATAL_ERROR "${RED}GTK+ (${USE_GTK2}) required!${NONE}")
	endif()
	include_directories(${GTK2_INCLUDE_DIRS})
	link_directories(${GTK2_LIBRARY_DIRS})
	set(LIBQ_LIBRARIES
		${LIBQ_LIBRARIES}
		${GTK2_LIBRARIES}
		${PANGOCAIRO_LIBRARIES}
	)
	set(USE_GLIB 1)
	set(USE_CAIRO 1)
endif()

if(USE_GIO)
	PKG_CHECK_MODULES(GIO ${USE_GIO})
	if(NOT GIO_FOUND)
		message(FATAL_ERROR "${RED}GIO (${USE_GIO}) required!${NONE}")
	endif()
	include_directories(${GIO_INCLUDE_DIRS})
	link_directories(${GIO_LIBRARY_DIRS})
	set(LIBQ_LIBRARIES
		${LIBQ_LIBRARIES}
		${GIO_LIBRARIES}
	)
endif()

if(USE_WEBKIT)
	PKG_CHECK_MODULES(WEBKIT webkit-1.0)
	if(NOT WEBKIT_FOUND)
		message(FATAL_ERROR "${RED}LibWebKitGTK required!${NONE}")
	endif()
	include_directories(${WEBKIT_INCLUDE_DIRS})
	link_directories(${WEBKIT_LIBRARY_DIRS})
	add_definitions(${WEBKIT_CFLAGS_OTHER})
endif()

if(USE_SOURCEVIEW)
	PKG_CHECK_MODULES(SOURCEVIEW gtksourceview-2.0)
	if(NOT SOURCEVIEW_FOUND)
		message(FATAL_ERROR "${RED}GTKSourceView 2.0 required!${NONE}")
	endif()
	set(HAVE_SOURCEVIEW 1)
	include_directories(${SOURCEVIEW_INCLUDE_DIRS})
	link_directories(${SOURCEVIEW_LIBRARY_DIRS})
	add_definitions(${SOURCEVIEW_CFLAGS_OTHER})
endif()

if(USE_JPEG)
	find_package(JPEG)
	if(NOT JPEG_FOUND)
		message(FATAL_ERROR "${RED}JPEG required!${NONE}")
	endif()
	set(HAVE_JPEG_H 1)
#	if(MINGW)
#		set(LIBQ_LIBRARIES
#			${LIBQ_LIBRARIES}
#			${JPEG_LIBRARIES}
#			jbig z
#		)
#	endif()
endif()

if(USE_TIFF)
	find_package(TIFF)
	if(NOT TIFF_FOUND)
		message(FATAL_ERROR "${RED}TIFF required!${NONE}")
	endif()
	set(HAVE_TIFF_H 1)
	if(MINGW)
		set(LIBQ_LIBRARIES
			${LIBQ_LIBRARIES}
			${TIFF_LIBRARIES}
			${JPEG_LIBRARIES}
		)
	endif()
endif()

if(USE_GEANY)
	PKG_CHECK_MODULES(GEANY geany)
	if(NOT GEANY_FOUND)
		message(FATAL_ERROR "${RED}Geany required!${NONE}")
	endif()
	include_directories(${GEANY_INCLUDE_DIRS})
	link_directories(${GEANY_LIBRARY_DIRS})
	add_definitions(${GEANY_CFLAGS_OTHER})
	set(PACKAGE_PLUGINS_DIR "${GEANY_LIBDIR}/geany")
endif()

if(USE_SCINTILLA)
	PKG_CHECK_MODULES(SCINTILLA scintilla)
	if(NOT SCINTILLA_FOUND)
		message(FATAL_ERROR "${RED}Scintilla required!${NONE}")
	endif()
	set(HAVE_SCINTILLA 1)
	include_directories(${SCINTILLA_INCLUDE_DIRS})
	link_directories(${SCINTILLA_LIBRARY_DIRS})
	add_definitions(${SCINTILLA_CFLAGS_OTHER})
endif()


include_directories(
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_BINARY_DIR}/src"
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_BINARY_DIR}/include"
)


if(CMAKE_C_COMPILER_ID STREQUAL GNU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")
	if(WIN32)
		set(CMAKE_C_FLAGS_RELEASE "-O2")
		set(CMAKE_C_FLAGS_DEBUG  "-Werror -O0 -gdwarf-2 -g")
	else()
		set(CMAKE_C_FLAGS_RELEASE "-O2")
		set(CMAKE_C_FLAGS_DEBUG  "-Werror -O0 -g")
	endif()
endif()

if(UNIX)
	if(APPLE)
		message(STATUS "Platform: ${BROWN}Apple${NONE}")
	else()
		message(STATUS "Platform: ${BROWN}Unix or Linux${NONE}")
	endif()
	link_libraries(${CMAKE_THREAD_LIBS} ${CMAKE_DL_LIBS})
elseif(WIN32)
	message(STATUS "Platform: ${BROWN}Windows${NONE}")
	if(MSVC)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -subsystem:windows")
	elseif(CMAKE_C_COMPILER_ID STREQUAL GNU)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
	endif()
	if(MINGW)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
		set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
		set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -s")
	endif()
else()
	message(STATUS "Platform: ${BROWN}Unknown OS${NONE}")
endif()

if(USE_GRESOURCES)
	message(
		"Configuring GResources:\n"
		"${GREEN}${gresources_files}${NONE}")

	find_program(GLIB_COMPILE_RESOURCES_EXECUTABLE glib-compile-resources)
	if(NOT GLIB_COMPILE_RESOURCES_EXECUTABLE)
		message(FATAL_ERROR "glib-compile-resources not found")
	endif()

	add_custom_command(
		OUTPUT "${PROJECT_BINARY_DIR}/src/res.c"
		COMMAND ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
			--target=${PROJECT_BINARY_DIR}/src/res.c
			--sourcedir=${PROJECT_SOURCE_DIR}/res
			--generate-source
			--c-name=seshat
			${PROJECT_SOURCE_DIR}/res/res.xml
		DEPENDS
			${PROJECT_SOURCE_DIR}/res/res.xml
			${gresources_files}
		COMMENT "Compile GResource C file"
	)

	set(gresources_src
		"${PROJECT_BINARY_DIR}/src/res.c"
	)

	add_custom_target(res-update-ALL
		COMMENT "GResourses update: Done."
		DEPENDS ${gresources_src}
	)
endif()



# When generating a package, a root directory may be prefixed
# to install directory, e.g. for a Debian .deb-package
if(PACKAGE_INSTALL_PREFIX)
	set(PACKAGE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE_INSTALL_PREFIX}")
else()
	set(PACKAGE_INSTALL_PREFIX "")
endif()

# Package directories are only set when empty, and so setting such a
# variable prior to including libq-dirs.cmake will remain
if(UNIX)
	if(APPLE)
	else()
		if(NOT PACKAGE_BINARY_DIR)
			set(PACKAGE_BINARY_DIR  "${CMAKE_INSTALL_PREFIX}/bin")
		endif()
		if(NOT PACKAGE_LIBRARY_DIR)
			set(PACKAGE_LIBRARY_DIR "${CMAKE_INSTALL_PREFIX}/lib")
		endif()
		if(NOT PACKAGE_INCLUDE_DIR)
			set(PACKAGE_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
		endif()
		if(NOT PACKAGE_SHARE_DIR)
			set(PACKAGE_SHARE_DIR   "${CMAKE_INSTALL_PREFIX}/share")
		endif()
		if(NOT PACKAGE_DATA_DIR)
			set(PACKAGE_DATA_DIR    "${PACKAGE_SHARE_DIR}/${PACKAGE}")
		endif()
		if(NOT PACKAGE_ICONS_DIR)
			set(PACKAGE_ICONS_DIR   "${PACKAGE_SHARE_DIR}/icons/hicolor")
		endif()
		if(NOT PACKAGE_PIXMAPS_DIR)
			set(PACKAGE_PIXMAPS_DIR "${PACKAGE_SHARE_DIR}/pixmaps")
		endif()
		if(NOT PACKAGE_LOCALE_DIR)
			set(PACKAGE_LOCALE_DIR  "${PACKAGE_SHARE_DIR}/locale")
		endif()
		if(NOT PACKAGE_PLUGINS_DIR)
			set(PACKAGE_PLUGINS_DIR "${PACKAGE_LIBRARY_DIR}/${PACKAGE}")
		endif()
	endif()
elseif(WIN32)
		if(NOT PACKAGE_BINARY_DIR)
			set(PACKAGE_BINARY_DIR  "bin")
		endif()
		if(NOT PACKAGE_LIBRARY_DIR)
			set(PACKAGE_LIBRARY_DIR "lib")
		endif()
		if(NOT PACKAGE_INCLUDE_DIR)
			set(PACKAGE_INCLUDE_DIR "include")
		endif()
		if(NOT PACKAGE_SHARE_DIR)
			set(PACKAGE_SHARE_DIR   "share")
		endif()
		if(NOT PACKAGE_DATA_DIR)
			set(PACKAGE_DATA_DIR    "data")
		endif()
		if(NOT PACKAGE_ICONS_DIR)
			set(PACKAGE_ICONS_DIR   "icons")
		endif()
		if(NOT PACKAGE_PIXMAPS_DIR)
			set(PACKAGE_PIXMAPS_DIR "pixmaps")
		endif()
		if(NOT PACKAGE_LOCALE_DIR)
			set(PACKAGE_LOCALE_DIR  "locale")
		endif()
		if(NOT PACKAGE_PLUGINS_DIR)
			set(PACKAGE_PLUGINS_DIR "plugins")
		endif()
else()
endif()

message(STATUS "${BROWN}PACKAGE_INSTALL_PREFIX: ${PACKAGE_INSTALL_PREFIX}${NONE}")
message(STATUS "${BROWN}PACKAGE_BINARY_DIR:     ${PACKAGE_BINARY_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_LIBRARY_DIR:    ${PACKAGE_LIBRARY_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_INCLUDE_DIR:    ${PACKAGE_INCLUDE_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_SHARE_DIR:      ${PACKAGE_SHARE_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_DATA_DIR:       ${PACKAGE_DATA_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_ICONS_DIR:      ${PACKAGE_ICONS_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_PIXMAPS_DIR:    ${PACKAGE_PIXMAPS_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_LOCALE_DIR:     ${PACKAGE_LOCALE_DIR}${NONE}")
message(STATUS "${BROWN}PACKAGE_PLUGINS_DIR:    ${PACKAGE_PLUGINS_DIR}${NONE}")

configure_file(
	"${PROJECT_SOURCE_DIR}/libq/_config.h"
	"${PROJECT_BINARY_DIR}/src/libq/config.h"
)

# These directories should be used when installing a package
set(PACKAGE_BINARY_DIR  "${PACKAGE_INSTALL_PREFIX}${PACKAGE_BINARY_DIR}")
set(PACKAGE_LIBRARY_DIR "${PACKAGE_INSTALL_PREFIX}${PACKAGE_LIBRARY_DIR}")
set(PACKAGE_INCLUDE_DIR "${PACKAGE_INSTALL_PREFIX}${PACKAGE_INCLUDE_DIR}")
set(PACKAGE_SHARE_DIR   "${PACKAGE_INSTALL_PREFIX}${PACKAGE_SHARE_DIR}")
set(PACKAGE_DATA_DIR    "${PACKAGE_INSTALL_PREFIX}${PACKAGE_DATA_DIR}")
set(PACKAGE_ICONS_DIR   "${PACKAGE_INSTALL_PREFIX}${PACKAGE_ICONS_DIR}")
set(PACKAGE_PIXMAPS_DIR "${PACKAGE_INSTALL_PREFIX}${PACKAGE_PIXMAPS_DIR}")
set(PACKAGE_LOCALE_DIR  "${PACKAGE_INSTALL_PREFIX}${PACKAGE_LOCALE_DIR}")
set(PACKAGE_PLUGINS_DIR "${PACKAGE_INSTALL_PREFIX}${PACKAGE_PLUGINS_DIR}")

