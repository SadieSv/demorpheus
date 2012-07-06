cmake_minimum_required(VERSION 2.6)

set( SHELLCODE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

#if libdasm library wanted use LIBDASM define, otherwise use FASTDIS
set( DISAS_MODE LIBDASM )
#set ( DISAS_MODE FASTDIS)

set( DEBUG_MODE D )
#set( DEBUG_MODE U )

add_subdirectory( ${SHELLCODE_DIR}/disasm )
add_subdirectory( ${SHELLCODE_DIR}/racewalk )
add_subdirectory( ${SHELLCODE_DIR}/hdd )
add_subdirectory( ${SHELLCODE_DIR}/sigfree )


include_directories(
		${SHELLCODE_DIR}/
		${SHELLCODE_DIR}/disasm/
		${SHELLCODE_DIR}/sigfree/
		${SHELLCODE_DIR}/racewalk/
		${SHELLCODE_DIR}/hdd/lib/
)

set( New_classifier_src
	classifier.cpp
	graphTopology.cpp
	demorpheus.cpp
)

add_executable( demorpheus demorpheus.cpp classifier.cpp graphTopology.cpp)
set_target_properties( demorpheus PROPERTIES COMPILE_FLAGS " -D ${DISAS_MODE}" )
target_link_libraries( demorpheus
	CommonDisas
	libdasm
	RaceDisasLib
	SigFree
	Racewalk
	Race_SVM
	pcap
	finddecryptor
)
install( TARGETS demorpheus RUNTIME DESTINATION ${SHELLCODE_DIR} )

add_executable( process_log process_log.cpp)


