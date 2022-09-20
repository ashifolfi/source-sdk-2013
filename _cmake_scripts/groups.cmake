# groups.cmake

if (${BUILD_GROUP} STREQUAL "game")
	include("${SRCDIR}/game/client/client_hl2mp.cmake")
	include("${SRCDIR}/mathlib/mathlib.cmake")
	include("${SRCDIR}/raytrace/raytrace.cmake")
	include("${SRCDIR}/game/server/server_hl2mp.cmake")
	include("${SRCDIR}/tier1/tier1.cmake")
	include("${SRCDIR}/vgui2/vgui_controls/vgui_controls.cmake")
elseif (${BUILD_GROUP} STREQUAL "everything")

	if (${IS_WINDOWS})
		include("${SRCDIR}/utils/captioncompiler/captioncompiler.cmake")
	endif()

	include("${SRCDIR}/game/client/client_hl2mp.cmake")
	
	if (${IS_WINDOWS})
		include("${SRCDIR}/fgdlib/fgdlib.cmake")
	endif()

	include("${SRCDIR}/materialsystem/stdshaders/game_shader_dx9_hl2mp.cmake")

	if (${IS_WINDOWS})
		include("${SRCDIR}/utils/glview/glview.cmake")
	endif()

	include("${SRCDIR}/mathlib/mathlib.cmake")
	include("${SRCDIR}/raytrace/raytrace.cmake")
	include("${SRCDIR}/game/server/server_hl2mp.cmake")
	include("${SRCDIR}/tier1/tier1.cmake")
	include("${SRCDIR}/vgui2/vgui_controls/vgui_controls.cmake")
elseif (${BUILD_GROUP} STREQUAL "shaders")
	include("${SRCDIR}/materialsystem/stdshaders/game_shader_dx9_hl2mp.cmake")
	include("${SRCDIR}/mathlib/mathlib.cmake")
	include("${SRCDIR}/tier1/tier1.cmake")
endif()