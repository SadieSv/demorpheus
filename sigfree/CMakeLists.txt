include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${SHELLCODE_DIR}/disasm
		${SHELLCODE_DIR}/
)
set (	SigFree_SRC
	eifg.h
	eifg.cpp
	state.h
	state.cpp
	analyzer.h
	analyzer.cpp
	${SHELLCODE_DIR}/macros.h
)

add_library( SigFree SHARED ${SigFree_SRC} )
set_target_properties( SigFree PROPERTIES COMPILE_FLAGS "-Wno-sign-compare" )
target_link_libraries( SigFree libdasm )

