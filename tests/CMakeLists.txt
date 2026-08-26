cmake_minimum_required(VERSION 3.16)

include_directories(${CMAKE_SOURCE_DIR}/tests/include)

aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src/ testSources)
remove( testSources ${CMAKE_SOURCE_DIR}/tests/src//LocalHttpServer.cpp ${CMAKE_SOURCE_DIR}/tests/src//test_influx_client.cpp )

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

target_link_libraries(test_xml_handler ${LIBXML2_LIBRARIES})

add_executable(test_influx_client ${CMAKE_SOURCE_DIR}/tests/src/test_influx_client.cpp ${CMAKE_SOURCE_DIR}/tests/src/LocalHttpServer.cpp)
target_link_libraries(test_influx_client ChimeraTK-ControlSystemAdapter-OPCUAAdapter)
target_link_libraries(test_influx_client open62541::open62541)
target_link_libraries(test_influx_client pthread)
target_link_libraries(test_influx_client
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_CHRONO_LIBRARY}
)
add_test(test_influx_client test_influx_client)
set_tests_properties(test_influx_client PROPERTIES
	ENVIRONMENT "TSAN_OPTIONS=$ENV{TSAN_OPTIONS}:suppressions=${CMAKE_SOURCE_DIR}/tests/tsan_test_influx_client.supp"
)
add_custom_target(test-verbose COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
