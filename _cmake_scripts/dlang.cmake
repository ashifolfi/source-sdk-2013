# dlang.cmake

set( LIBD_PHOBOS_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}phobos2-ldc-shared${CMAKE_SHARED_LIBRARY_SUFFIX}" )
set( LIBD_RUNTIME_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}druntime-ldc-shared${CMAKE_SHARED_LIBRARY_SUFFIX}" )

function( setup_dlang )
	# some people might not want to use D
	if ( DEFINED NO_DLANG )
		return()
	endif ()

	# setup D
	set( LDC2_RUNTIME_ROOT "$ENV{LDC2_RUNTIME_ROOT}" CACHE PATH "The path to the ldc2 runtime library root (contains `lib32` and `lib64` folders)" )
	if ( LDC2_RUNTIME_ROOT STREQUAL "" )
		message( FATAL_ERROR "`LDC2_RUNTIME_ROOT` was not provided, aborting." )
	endif ()
	if ( NOT WIN32 )
		message( AUTHOR_WARNING "Using D on ${CMAKE_SYSTEM_NAME} is not tested! Here be dragons." )
	endif ()

	set( DLIB_DIR "${LDC2_RUNTIME_ROOT}/lib32" )
	configure_file( "${DLIB_DIR}/${LIBD_PHOBOS_NAME}" "${GAMEDIR}/bin/${LIBD_PHOBOS_NAME}" COPYONLY )
	configure_file( "${DLIB_DIR}/${LIBD_RUNTIME_NAME}" "${GAMEDIR}/bin/${LIBD_RUNTIME_NAME}" COPYONLY )
endfunction()

function( add_dlang_project )
	cmake_parse_arguments( DLP "" "NAMED;IN" "" ${ARGN} )

	# some people might not want to use D
	if ( DEFINED NO_DLANG )
		return()
	endif ()

	# add D target
	if ( NOT DEFINED "LDC2_RUNTIME_ROOT" )
		setup_dlang()
	endif ()
	add_custom_target( ${DLP_NAMED}
		COMMAND
			dub build --quiet --arch=x86
		DEPENDS
			"${GAMEDIR}/bin/${LIBD_PHOBOS_NAME}"
			"${GAMEDIR}/bin/${LIBD_RUNTIME_NAME}"
		WORKING_DIRECTORY
			${DLP_IN}
		USES_TERMINAL
	)
endfunction()
