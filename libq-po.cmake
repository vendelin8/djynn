

message("Configuring language library.")

if(NOT LANGUAGES)
	message(FATAL_ERROR "LANGUAGES not defined")
endif()

find_program(GETTEXT_MSGINIT_EXECUTABLE msginit)
if(NOT GETTEXT_MSGINIT_EXECUTABLE)
	message(FATAL_ERROR "msginit not found")
endif()

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE )
	message(FATAL_ERROR "xgettext not found")
endif()

find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
if(NOT GETTEXT_MSGMERGE_EXECUTABLE )
	message(FATAL_ERROR "msgmerge not found")
endif()

find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTABLE )
	message(FATAL_ERROR "msgcat not found")
endif()

find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
	message(FATAL_ERROR "msgattrib not found")
endif()


add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/po/${PACKAGE}.pot
	COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
		--force-po
		--add-comments=TRANSLATORS
		--directory=${PROJECT_SOURCE_DIR}
		--files-from=${PROJECT_SOURCE_DIR}/po/POTFILES
		--copyright-holder=\"${PACKAGE_MAINTAINER}\"
		--msgid-bugs-address=\"${PACKAGE_BUGREPORT}\"
		--from-code=UTF-8
		--sort-by-file
		--keyword=_ --keyword=Q_:1g --keyword=N_ --keyword=C_:1c,2 --keyword=NC_
		--output=${PROJECT_SOURCE_DIR}/po/${PACKAGE}.pot 
	DEPENDS
		${PROJECT_SOURCE_DIR}/po/POTFILES
	COMMENT "po-update [${PACKAGE}]: Generated source pot file."
)

foreach(LANG ${LANGUAGES})
	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/po/${LANG}.pot
		COMMAND ${GETTEXT_MSGINIT_EXECUTABLE}
			--no-translator 
			--input=${PROJECT_SOURCE_DIR}/po/${PACKAGE}.pot 
			--output-file=${PROJECT_BINARY_DIR}/po/${LANG}.pot
			--locale=${LANG}
		DEPENDS
			${PROJECT_SOURCE_DIR}/po/${PACKAGE}.pot
		COMMENT "po-update [${LANG}]: Initialized pot file."
	)

	add_custom_command(
		OUTPUT ${PROJECT_SOURCE_DIR}/po/${LANG}.po
		COMMAND ${CMAKE_COMMAND} -E touch 
			${PROJECT_SOURCE_DIR}/po/${LANG}.po
		COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} 
			--backup=none
			-U ${PROJECT_SOURCE_DIR}/po/${LANG}.po
			${PROJECT_BINARY_DIR}/po/${LANG}.pot
		DEPENDS
			${PROJECT_BINARY_DIR}/po/${LANG}.pot
		COMMENT "po-update [${LANG}]: Updated po file."
	)

	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/po/locale/${LANG}
		COMMAND ${CMAKE_COMMAND} -E make_directory 
			${PROJECT_BINARY_DIR}/po/locale/${LANG}
		COMMENT "mo-update [${LANG}]: Creating locale directory."
	)

	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/po/locale/${LANG}/${PACKAGE}.mo
		COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
			-v -o
			${PROJECT_BINARY_DIR}/po/locale/${LANG}/${PACKAGE}.mo
			${PROJECT_SOURCE_DIR}/po/${LANG}.po
		DEPENDS 
			${PROJECT_BINARY_DIR}/po/locale/${LANG}
			${PROJECT_SOURCE_DIR}/po/${LANG}.po
		COMMENT "mo-update [${LANG}]: Creating mo file."
	)

	install(
		FILES
			"${PROJECT_BINARY_DIR}/po/locale/${LANG}/${PACKAGE}.mo"
		DESTINATION
			"${ROOT_INSTALL_PREFIX}${PACKAGE_LOCALE_DIR}/${LANG}/LC_MESSAGES"
	)

	set(po-update-SRC
		${po-update-SRC}
		"${PROJECT_SOURCE_DIR}/po/${LANG}.po"
	)

	set(mo-update-SRC
		${mo-update-SRC} 
		"${PROJECT_BINARY_DIR}/po/locale/${LANG}/${PACKAGE}.mo"
	)
endforeach()

add_custom_target(po-update-ALL
	COMMENT "po-update: Done."
	DEPENDS ${po-update-SRC}
)

add_custom_target(mo-update ALL
	COMMENT "mo-update: Done."
	DEPENDS ${mo-update-SRC}
)


