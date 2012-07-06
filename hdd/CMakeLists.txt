add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHELLCODE_DIR}/disasm/
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/
#        ${PROJECT_SOURCE_DIR}/include
#        ${PROJECT_SOURCE_DIR}/aura/include
)
set (FIND_DECRYPTOR_SRC
        FindDecryptorPlugin.cpp
)

#add_library ( FindDecryptorPlugin SHARED ${FIND_DECRYPTOR_SRC} )
#set_target_properties ( FindDecryptorPlugin PROPERTIES COMPILE_FLAGS "-fPIC" )
#target_link_libraries ( FindDecryptorPlugin finddecryptor )

#if (PACK_SHELLCODE_PLUGINS)
#    install( TARGETS FindDecryptorPlugin LIBRARY DESTINATION ${IDS_PLUGIN_INSTALL_TARGET} )
#endif(PACK_SHELLCODE_PLUGINS)

#if(WANT_TESTS)
#    add_custom_target( findDecryptorTest  ALL )
    
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/findDecryptor.ids
#            ${CMAKE_CURRENT_BINARY_DIR}/findDecryptor.ids COPYONLY
#    )
#    ids_compile (findDecryptorTest 
#            ${CMAKE_CURRENT_BINARY_DIR}/findDecryptor.ids 
#            -I${CMAKE_CURRENT_SOURCE_DIR} 
#            -I${PROJECT_SOURCE_DIR}/include
#            -I${PROJECT_SOURCE_DIR}/aura/include
#    )
#    configure_file(
#            ${CMAKE_CURRENT_SOURCE_DIR}/findDecryptor.conf
#            ${CMAKE_CURRENT_BINARY_DIR}/findDecryptor.conf @ONLY)
#    ids_sign_conf ( findDecryptorTest ${CMAKE_CURRENT_BINARY_DIR}/findDecryptor.conf )
#endif(WANT_TESTS)

