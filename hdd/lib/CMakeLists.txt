find_library( LIBEMU_LIBRARY libemu.so)
if(${LIBEMU_LIBRARY-NOTFOUND})
    message(FATAL_ERROR "Libemu library not found")
endif(${LIBEMU_LIBRARY-NOTFOUND})

include_directories(
        ${SHELLCODE_DIR}/disasm/
        ${CMAKE_CURRENT_SOURCE_DIR}/
#        ${PROJECT_SOURCE_DIR}/include
#        ${PROJECT_SOURCE_DIR}/aura/include
)
set (   libemu_SRC
        emulator_libemu.cpp
        emulator.cpp
)

set (   finddecryptor_SRC
        data.cpp
        finder.cpp
        timer.cpp
        finder-cycle.cpp
        finder-getpc.cpp
        finder-libemu.cpp
        reader.cpp
        reader_pe.cpp
        finddecryptor.cpp
)

add_library( emulator_libemu SHARED ${libemu_SRC} )
set_target_properties( emulator_libemu PROPERTIES COMPILE_FLAGS "-fPIC -DBACKEND_LIBEMU")
target_link_libraries( emulator_libemu ${LIBEMU_LIBRARY} libdasm )

add_library( finddecryptor SHARED ${finddecryptor_SRC} )
set_target_properties( finddecryptor PROPERTIES COMPILE_FLAGS "-fPIC -DBACKEND_LIBEMU")
target_link_libraries( finddecryptor libdasm  emulator_libemu )

#if (PACK_SHELLCODE_PLUGINS)
#    install( TARGETS finddecryptor LIBRARY DESTINATION ${IDS_LIB_INSTALL_TARGET} )
#    install(  TARGETS emulator_libemu LIBRARY DESTINATION ${IDS_LIB_INSTALL_TARGET} )
#endif(PACK_SHELLCODE_PLUGINS)
