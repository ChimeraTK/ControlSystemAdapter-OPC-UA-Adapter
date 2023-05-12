cmake_minimum_required(VERSION 2.8.0) 

include_directories(${CMAKE_SOURCE_DIR}/tests/include)

aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src/ testSources)

foreach( testSourceFile ${testSources})
	get_filename_component(executableName ${testSourceFile} NAME_WE)
	add_executable(${executableName} ${testSourceFile} ${testAuxSources})
	target_link_libraries(${executableName} ChimeraTK-ControlSystemAdapter-OPCUAAdapter)
	target_link_libraries(${executableName} open62541::open62541)
	target_link_libraries(${executableName} pthread)
	target_link_libraries(${executableName}
		${Boost_FILESYSTEM_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
		${Boost_CHRONO_LIBRARY}
	)

	add_test(${executableName} ${executableName})
endforeach(testSourceFile)

add_custom_target(test-verbose COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
