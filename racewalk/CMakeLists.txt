include_directories(
	${SHELLCODE_DIR}/racewalk/
	${SHELLCODE_DIR}/disasm/
#	${PROJECT_SOURCE_DIR}/aura/include
#	${PROJECT_SOURCE_DIR}/include	
)
set (	Racewalk_SRC 
	racewalk.h
	racewalk.cpp
)
set (	SVM_SRC
	svm.h
	svm.cpp
)
add_library ( Racewalk SHARED ${Racewalk_SRC} )
set_target_properties( Racewalk PROPERTIES COMPOLE_FLAGS "-Wno-all" )
target_link_libraries( Racewalk RaceDisasLib )

add_library( Race_SVM SHARED ${SVM_SRC} )
set_target_properties( Race_SVM PROPERTIES COMPILE_FLAGS "-Wno-all" )
target_link_libraries( Race_SVM Racewalk )

#add_executable( racewalk racewalkrun.cpp)
#target_link_libraries( racewalk Racewalk Race_SVM RaceDisasLib pcap )

#add_executable( racewalkDetect racewalkDetect.cpp)
#target_link_libraries( racewalkDetect Racewalk Race_SVM RaceDisasLib pcap )

#add_library( racewalkPlugin SHARED racewalkPlugin.cpp )
#set_target_properties( racewalkPlugin PROPERTIES COMPILE_FLAGS "-Wno-all" )
#target_link_libraries( racewalkPlugin Racewalk Race_SVM RaceDisasLib)

#if( PACK_SHELLCODE_PLUGINS )
#    install( TARGETS racewalkPlugin LIBRARY
#            DESTINATION ${IDS_PLUGIN_INSTALL_TARGET})
#    install( TARGETS Race_SVM LIBRARY
#            DESTINATION ${IDS_LIB_INSTALL_TARGET})
#    install( TARGETS Racewalk LIBRARY
#            DESTINATION ${IDS_LIB_INSTALL_TARGET})
 #   if(PACK_STANDALONE_SHELLCODE_BINARIES)
 #       install ( TARGETS racewalk  RUNTIME
#            DESTINATION ${IDS_NOSUID_EXECUTABLES_INSTALL_TARGET} )
#        install ( TARGETS racewalkDetect  RUNTIME
#            DESTINATION ${IDS_NOSUID_EXECUTABLES_INSTALL_TARGET} )
#    endif(PACK_STANDALONE_SHELLCODE_BINARIES)
#endif( PACK_SHELLCODE_PLUGINS )

#if(WANT_TESTS)
#        add_custom_target( racewalkTest  ALL )

#        configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/racewalkTest.ids
#                	${CMAKE_CURRENT_BINARY_DIR}/racewalkTest.ids
#			COPYONLY )
#        ids_compile (	racewalkTest
#			${CMAKE_CURRENT_BINARY_DIR}/racewalkTest.ids
#			-I${CMAKE_CURRENT_SOURCE_DIR} 
#                        -I${PROJECT_SOURCE_DIR}/include
#			-I${PROJECT_SOURCE_DIR}/aura/include )
#
#        configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/racewalk.conf
#			${CMAKE_CURRENT_BINARY_DIR}/racewalk.conf
#			 @ONLY
#			)
#	ids_sign_conf ( racewalkTest ${CMAKE_CURRENT_BINARY_DIR}/racewalk.conf )
#endif(WANT_TESTS)
