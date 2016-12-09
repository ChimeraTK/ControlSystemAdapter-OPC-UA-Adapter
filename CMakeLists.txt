cmake_minimum_required(VERSION 2.8.0)

project(ControlSystemAdapterOPCUA)

include(ExternalProject)

SET(${PROJECT_NAME}_MAJOR_VERSION 01)
SET(${PROJECT_NAME}_MINOR_VERSION 02)
SET(${PROJECT_NAME}_PATCH_VERSION 00)
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

##########################################################################################################
## Hard package dependencies
##########################################################################################################

find_package(Boost COMPONENTS system filesystem thread chrono unit_test_framework REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(PythonInterp REQUIRED)

#Do not put the ControlSystemAdapter as required. We will install it if it is not found.
find_package(ChimeraTK-ControlSystemAdapter)
find_package(mtca4u-deviceaccess)

if(NOT mtca4u-deviceaccess_FOUND)		
		ExternalProject_Add(external-ChimeraTK-DeviceAccess
			GIT_REPOSITORY "https://github.com/ChimeraTK/DeviceAccess.git"
			GIT_TAG "master"
			CMAKE_ARGS 
			"-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess"
			)
			
		list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess/share/cmake-3.5/Modules/")
		
		set(ChimeraTK-DeviceAccess_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess/include)
		set(ChimeraTK-DeviceAccess_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess/lib)
		set(ChimeraTK-DeviceAccess_LIBRARIES mtca4u-deviceaccess)
	endif()
	
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-DeviceAccess_CXX_FLAGS}")
	
#Install the ControlSystemAdapter if it is not pre-installed
if(NOT ChimeraTK-ControlSystemAdapter_FOUND)
  message("Installing ChimeraTK-ControlSystemAdapter as external dependency inside the build directory.")
	message("You will not be able to make a proper installation of the OPCUA_Adapter. Only use this for testing and development.\n")
  ExternalProject_Add(external-ChimeraTK-ControlSystemAdapter
    GIT_REPOSITORY "https://github.com/ChimeraTK/ControlSystemAdapter.git"
    #When creationg a tag, base it on a tag of the ControlSystemAdapter so it stays stable
    #GIT_TAG "00.02.00"
    GIT_TAG "master"
    DEPENDS external-ChimeraTK-DeviceAccess
    CMAKE_ARGS 
    "-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ChimeraTK/ControlSystemAdapter" 
    )
  #FIXME: Find a better name for this variable
  set(USE_SELFINSTALLED_ControlSystemAdapter TRUE)
  #As the external project is only condifured in the build step, we have to cheat the
  #configuration here which otherwise is provided by find_package.
  #Attention: This is a hack and can become inconsistent with the read config.
  #
  #ChimeraTK-ControlSystemAdapter_CXX_FLAGS currently only provides C++11, which is used anyway
  #ChimeraTK-ControlSystemAdapter_LINKER_FLAGS provides the rpath, not needed for testing
  #
  #We leave out the boost flags as the OPCUA_Adapter uses them itself
  set(ChimeraTK-ControlSystemAdapter_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/ControlSystemAdapter/include)
  set(ChimeraTK-ControlSystemAdapter_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/ControlSystemAdapter/lib)
  set(ChimeraTK-ControlSystemAdapter_LIBRARIES ChimeraTK-ControlSystemAdapter)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-ControlSystemAdapter_CXX_FLAGS}")

##########################################################################################################
## Set the build type to Release if none is specified
## Force it into Release if "None" is specified (needed to overrule dkpg_buildpackage)
##########################################################################################################

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")

##########################################################################################################
## Copy mapping.xml into build folder
##########################################################################################################
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	message("Copy mapping files into build folder.")
	file(GLOB mappingFiles "${CMAKE_SOURCE_DIR}/*_mapping.xml")
	foreach(mappingFile ${mappingFiles})
		file(COPY ${mappingFile} DESTINATION ${CMAKE_BINARY_DIR}/)
		message("Copy ${mappingFile}")
	endforeach(mappingFile)
endif()         

##########################################################################################################
## Compiler specific stuff
##########################################################################################################

if(CMAKE_COMPILER_IS_GNUCC OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  
  add_definitions(-fPIC)
endif()


##########################################################################################################
## Includes, Link dirs und Quellcode
##########################################################################################################

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/include/open62541/)
include_directories(SYSTEM ${ChimeraTK-ControlSystemAdapter_INCLUDE_DIRS})
include_directories(SYSTEM ${ChimeraTK-DeviceAccess_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/examples/)
include_directories(SYSTEM ${LIBXML2_INCLUDE_DIR})

link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${LIBXML2_LIBRARIES})
link_directories(${ChimeraTK-ControlSystemAdapter_LIBRARY_DIRS})
link_directories(${ChimeraTK-DeviceAccess_LIBRARY_DIRS})
                    
set(objectSources  ${CMAKE_SOURCE_DIR}/src/ua_mapped_class.cpp
                   ${CMAKE_SOURCE_DIR}/src/ua_proxies.cpp
                   ${CMAKE_SOURCE_DIR}/src/ipc_managed_object.cpp
                   ${CMAKE_SOURCE_DIR}/src/ipc_manager.cpp
                   ${CMAKE_SOURCE_DIR}/src/ControlSystemAdapterOPCUA.cpp
                   ${CMAKE_SOURCE_DIR}/src/xml_file_handler.cpp
                   ${CMAKE_SOURCE_DIR}/src/mtca_processvariable.cpp
                   ${CMAKE_SOURCE_DIR}/src/mtca_additionalvariable.cpp
                   ${CMAKE_SOURCE_DIR}/src/mtca_uaadapter.cpp
                   
                   ${CMAKE_SOURCE_DIR}/src/CSA_OPCUA_Application.cpp
                   
                   # Example HelperClass
                   ${CMAKE_SOURCE_DIR}/examples/runtimeValueGenerator.cpp
)

# Create the target directory for model initializers, open62541 and ControlSystemAdapter libraries/headers
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src_generated")
include_directories(${PROJECT_BINARY_DIR}/src_generated/)

##########################################################################################################
## Optionen
##########################################################################################################

## Project specific options
option(ENABLE_BUILDMODEL      "Create model from XML description"      ON)
## Code quality control options
option(ENABLE_COVERAGE        "Enable coverage stats and tests for adapter build" OFF)
option(BUILD_STATIC_ANALYSIS  "Instead of building a binary, perform a static code analysis using clangs analyzer." OFF)
option(ENABLE_LINTING         "Enables running the cppcheck static analyzer prior to compilation." OFF)
option(ENABLE_UNIT_TESTS 			"Compile and run unit tests." OFF)
option(ENABLE_DOCUMENTATION		"Generate Doxygen documentation" OFF)

if(ENABLE_BUILDMODEL)
  set(MODEL_XML_FILE "templatemodel.xml" CACHE STRING "Namespace definition XML file for MTCA Model")
  
  list(APPEND objectSources ${PROJECT_BINARY_DIR}/src_generated/mtca_namespaceinit_generated.c)
else()
  include_directories(${CMAKE_SOURCE_DIR}/include/model_prebuilt/)
  list(APPEND objectSources ${CMAKE_SOURCE_DIR}/include/model_prebuilt/mtca_namespaceinit_generated.c)
  
  include_directories(${PROJECT_SOURCE_DIR}/include/open62541/src/)
  include_directories(${PROJECT_SOURCE_DIR}/include/open62541/deps/)
endif()

if(ENABLE_COVERAGE)
  set(CMAKE_BUILD_TYPE DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
  
	add_custom_target(coverage ALL
										${PROJECT_SOURCE_DIR}/cmake/makeGcov.sh ${PROJECT_SOURCE_DIR}
										WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
										# Bad soulution to provide all depencys
										DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_ControlSystemAdapterOPCUA
										DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_ipc
										DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_processvariable
										DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_additionalvariable
										DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_uaadapter
										DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_xml_handler	
										COMMENT "Generating test coverage documentation" VERBATIM)
endif()

if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_STATIC_ANALYSIS)
  set(CMAKE_C_COMPILER   clang)
  set(CMAKE_CXX_COMPILER clang++)
  add_definitions(-o ${PROJECT_BINARY_DIR}/static-analysis})
endif()

if(ENABLE_DOCUMENTATION)
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
else(DOXYGEN_FOUND)
  message("Doxygen not found, documentation will not be build.")
endif(DOXYGEN_FOUND)
endif()

##########################################################################################################
## Object and shared library compilation required by tests and examples
##########################################################################################################

## Config File
configure_file (
  "${PROJECT_SOURCE_DIR}/include/csa_config.h.in"
  "${PROJECT_BINARY_DIR}/src_generated/csa_config.h"
  )

## Object Files
add_library(mtca_objects OBJECT ${objectSources})
if(USE_SELFINSTALLED_ControlSystemAdapter)
   ADD_DEPENDENCIES(mtca_objects external-ChimeraTK-ControlSystemAdapter) 
   ADD_DEPENDENCIES(mtca_objects external-ChimeraTK-DeviceAccess) 
endif()

## Early declaration due to the requirement to run the dependencies update prior to building
## Create shared lib
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:mtca_objects>)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} open62541)
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-ControlSystemAdapter_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-DeviceAccess_LIBRARIES})
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_CHRONO_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

# install libopen62541.so.0 => TODO this is a temporarly work around!
install(FILES ${PROJECT_SOURCE_DIR}/lib/libopen62541.so.0 DESTINATION lib)

##########################################################################################################
## Custom Commands (z.b. model-compiler, autobuild-dependencies)
##########################################################################################################

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/generate_open62541CCode.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/open62541_MacroHelper.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_builtin_types.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_constants.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_namespace.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_node_types.py
                           ${PROJECT_SOURCE_DIR}/lib/libopen62541.so.0
                           ${PROJECT_SOURCE_DIR}/include/open62541/open62541.h
                   PRE_BUILD
                   COMMAND /bin/bash ${PROJECT_SOURCE_DIR}/tools/buildDependencies.sh ${PROJECT_SOURCE_DIR}
                   DEPENDS ${PROJECT_SOURCE_DIR}/tools/buildDependencies.sh)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/src_generated/mtca_namespaceinit_generated.c
                          ${PROJECT_BINARY_DIR}/src_generated/mtca_namespaceinit_generated.h
                   PRE_BUILD
                   COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/generate_open62541CCode.py
                                                -i ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist_FullNS0.txt
                                                -b ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist.txt
                                                ${PROJECT_SOURCE_DIR}/model/Opc.Ua.NodeSet2.xml
                                                ${PROJECT_SOURCE_DIR}/model/${MODEL_XML_FILE}
                                                ${PROJECT_BINARY_DIR}/src_generated/mtca_namespaceinit_generated
                   DEPENDS ${PROJECT_SOURCE_DIR}/model/Opc.Ua.NodeSet2.xml
                           ${PROJECT_SOURCE_DIR}/model/${MODEL_XML_FILE}
                           ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist_FullNS0.txt
                           ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist.txt
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/generate_open62541CCode.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/open62541_MacroHelper.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_builtin_types.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_constants.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_namespace.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_node_types.py)

                           
##########################################################################################################
##  Tests and executables
##########################################################################################################

link_directories(${CMAKE_BINARY_DIR})

add_executable(ControlSystem-OPCUA_Sample_Adapter ${CMAKE_SOURCE_DIR}/examples/ControlSystem_OPCUA_Sample_Adapter.cpp)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter ${PROJECT_NAME})
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter open62541)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter pthread)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_CHRONO_LIBRARY}
)

if(ENABLE_LINTING)
add_custom_command( TARGET ControlSystem-OPCUA_Sample_Adapter
                    PRE_BUILD
                    COMMAND /usr/bin/cppcheck --std=c++11 --inline-suppr --enable=all ${objectSources}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Running CppCheck on all CPP Sources" VERBATIM)
endif()
## Ends binary ControlSystem-OPCUA_Sample_Adapter

install(TARGETS ControlSystem-OPCUA_Sample_Adapter RUNTIME DESTINATION bin)

##########################################################################################################
## Create the config files by filling the correct variables into the template (*.cmake.in).
## All variables @VARIABLE@ are replaced with the current values, but they have to be set first....
## Only the reference header is needed. The other variables are for dependencies of this library,
## but there are none.
##########################################################################################################

set(${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(${PROJECT_NAME}_LIBRARIES "${ChimeraTK-ControlSystemAdapter_LIBRARIES}")
set(${PROJECT_NAME}_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib ${ChimeraTK-ControlSystemAdapter_LIBRARY_DIRS}")
set(${PROJECT_NAME}_CXX_FLAGS "${ChimeraTK-ControlSystemAdapter_CXX_FLAGS}")
set(${PROJECT_NAME}_LINKER_FLAGS "-Wl,-rpath=${CMAKE_INSTALL_PREFIX}/lib,--enable-new-dtags")
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

