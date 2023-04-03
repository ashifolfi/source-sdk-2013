# video_base.cmake

set(QUICKTIME_WINDOWS 0)

set(USE_GL 0)
set(USE_SDL 0)

if (${IS_POSIX} AND NOT ${DEDICATED})
	set(USE_GL 1)
	set(USE_SDL 1)
endif()


add_compile_definitions(
	$<$<AND:${QUICKTIME_WINDOWS},${IS_WINDOWS}>:BINK_VIDEO>
	$<${IS_LINUX}:BINK_VIDEO>
	AVI_VIDEO
	WMV_VIDEO

	"$<${USE_GL}:GL_GLEXT_PROTOTYPES;DX_TO_GL_ABSTRACTION>"
	$<${USE_SDL}:USE_SDL>
)