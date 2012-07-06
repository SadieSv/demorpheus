cmake_minimum_required(VERSION 2.6)

#set( SHELLCODE_DIR ../ )

include_directories(
	${SHELLCODE_DIR}/disasm
)

set(	RaceDisas_SRC
	${SHELLCODE_DIR}/disasm/i386-dismod.c
	${SHELLCODE_DIR}/disasm/i386-dismod.h
	${SHELLCODE_DIR}/disasm/dis-asm.h
	${SHELLCODE_DIR}/disasm/bfd.h
	${SHELLCODE_DIR}/disasm/opintl.h
	${SHELLCODE_DIR}/disasm/inst_types.h
)

set(	libdasm_SRC
	${SHELLCODE_DIR}/disasm/libdasm.h
	${SHELLCODE_DIR}/disasm/libdasm.c
	${SHELLCODE_DIR}/disasm/opcode_tables.h
	${SHELLCODE_DIR}/disasm/inst_types.h
)

set(	comDisas_SRC
	${SHELLCODE_DIR}/disasm/inst_types.h
	${SHELLCODE_DIR}/disasm/flow.h
	${SHELLCODE_DIR}/disasm/flow.cpp
	${SHELLCODE_DIR}/disasm/com_disas.h
	${SHELLCODE_DIR}/disasm/com_disas.cpp
)

add_executable( RaceDisas  disrun.cpp ${RaceDisas_SRC} )

add_library( RaceDisasLib SHARED ${RaceDisas_SRC} )
set_target_properties( RaceDisasLib PROPERTIES COMPILE_FLAGS " -Wno-all" )

add_library( libdasm SHARED ${libdasm_SRC} )

add_library( CommonDisas SHARED ${comDisas_SRC})
set_target_properties( CommonDisas PROPERTIES COMPILE_FLAGS  "-D ${DISAS_MODE} -${DEBUG_MODE} DEBUG "  )
target_link_libraries( CommonDisas libdasm )

if( PACK_SHELLCODE_PLUGINS )
    install( TARGETS RaceDisasLib LIBRARY DESTINATION ${IDS_LIB_INSTALL_TARGET})
    if(PACK_STANDALONE_SHELLCODE_BINARIES)
        install( TARGETS RaceDisas RUNTIME DESTINATION ${IDS_NOSUID_EXECUTABLES_INSTALL_TARGET})
        install( TARGETS CommonDisas LIBRARY DESTINATION ${IDS_LIB_INSTALL_TARGET} )
    endif(PACK_STANDALONE_SHELLCODE_BINARIES)
    install( TARGETS libdasm LIBRARY DESTINATION ${IDS_LIB_INSTALL_TARGET} )
endif( PACK_SHELLCODE_PLUGINS )
set( cleanFiles
	RaceDisas
	${SHELLCODE_DIR}/lib/RaceDisasLib
	${SHELLCODE_DIR}/lib/libdasm
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${cleanFiles} )
