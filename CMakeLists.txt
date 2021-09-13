cmake_minimum_required(VERSION 2.8.0)

##########################################################################################################
## Project specification
##########################################################################################################
project(ChimeraTK-ControlSystemAdapter-OPCUAAdapter)

SET(${PROJECT_NAME}_MAJOR_VERSION 02)
SET(${PROJECT_NAME}_MINOR_VERSION 00)
SET(${PROJECT_NAME}_PATCH_VERSION 02)
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

##########################################################################################################
## Hard package dependencies
##########################################################################################################

find_package(Boost COMPONENTS system filesystem thread chrono unit_test_framework REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(PythonInterp REQUIRED)

#Do not put any of the following packages as required. We will install it if it is not found.
find_package(ChimeraTK-ControlSystemAdapter 01.00)
find_package(ChimeraTK-DeviceAccess 01.00)

find_package(open62541 1.2 REQUIRED)
IF (${open62541_FOUND})
    MESSAGE(STATUS "Found open62541 in Version: ${open62541_VERSION}. Include DIR ${open62541_LIBRARIES}")
ELSE (${open62541_FOUND})
    MESSAGE(STATUS "Could not locate open62541.")
ENDIF (${open62541_FOUND})

#Install the DeviceAccess if it is not pre-installed
if(NOT ChimeraTK-DeviceAccess_FOUND)
                ExternalProject_Add(external-ChimeraTK-DeviceAccess
                        GIT_REPOSITORY "https://github.com/ChimeraTK/DeviceAccess.git"

                        GIT_TAG "master"
                        PREFIX "${CMAKE_BINARY_DIR}/ChimeraTK-DeviceAccess_src"
                        CMAKE_ARGS
                        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess"
                        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
                        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
                        )

                set(ChimeraTK-DeviceAccess_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess/include)
                set(ChimeraTK-DeviceAccess_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess/lib)
                set(ChimeraTK-DeviceAccess_LIBRARIES ChimeraTK-DeviceAccess)
        endif()

#Install the ControlSystemAdapter if it is not pre-installed
if(NOT ChimeraTK-ControlSystemAdapter_FOUND)
        message("Installing ChimeraTK-ControlSystemAdapter as external dependency inside the build directory.")
        message("You will not be able to make a proper installation of the OPCUA_Adapter. Only use this for testing and development.\n")
        ExternalProject_Add(external-ChimeraTK-ControlSystemAdapter
                GIT_REPOSITORY "https://github.com/ChimeraTK/ControlSystemAdapter.git"
                GIT_TAG "master"
                PREFIX "${CMAKE_BINARY_DIR}/ChimeraTK-ControlSystemAdapter_src"
                DEPENDS external-ChimeraTK-DeviceAccess
                CMAKE_ARGS
                        "-DCMAKE_MODULE_PATH=${CMAKE_BINARY_DIR}/ChimeraTK/DeviceAccess/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules/"
                        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ChimeraTK/ControlSystemAdapter"
                        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
                        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        )
        set(USE_ControlSystemAdapter TRUE)
        set(ChimeraTK-ControlSystemAdapter_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/ControlSystemAdapter/include)
        set(ChimeraTK-ControlSystemAdapter_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/ChimeraTK/ControlSystemAdapter/lib)
        set(ChimeraTK-ControlSystemAdapter_LIBRARIES ChimeraTK-ControlSystemAdapter)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-ControlSystemAdapter_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${ChimeraTK-ControlSystemAdapter_LINKER_FLAGS}")

##########################################################################################################
## Set the build type to Release if none is specified
## Force it into Release if "None" is specified (needed to overrule dkpg_buildpackage)
##########################################################################################################

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING
                "Choose the type of build, options are: Debug Release RelWithDebInfo tsan asan"
                FORCE
        )
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

        file(COPY ${PROJECT_SOURCE_DIR}/opcuaAdapter_mapping.xml DESTINATION ${CMAKE_BINARY_DIR}/)
endif()

##########################################################################################################
## Compiler specific stuff
##########################################################################################################

if(CMAKE_COMPILER_IS_GNUCC OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  add_definitions(-DNDEBUG)
  add_definitions(-fPIC)
  add_definitions(-Wextra)
endif()

##########################################################################################################
## Includes, Link dirs und Quellcode
##########################################################################################################

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

include_directories(${CMAKE_SOURCE_DIR}/include)
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
                   ${CMAKE_SOURCE_DIR}/src/csa_opcua_adapter.cpp
                   ${CMAKE_SOURCE_DIR}/src/xml_file_handler.cpp
                   ${CMAKE_SOURCE_DIR}/src/ua_processvariable.cpp
                   ${CMAKE_SOURCE_DIR}/src/ua_additionalvariable.cpp
                   ${CMAKE_SOURCE_DIR}/src/ua_adapter.cpp
                   ${CMAKE_SOURCE_DIR}/src/csa_opcua_application.cpp

                   #${CMAKE_SOURCE_DIR}/src/open62541.c
                   # Example HelperClass
                   #${CMAKE_SOURCE_DIR}/examples/runtime_value_generator.cpp
)

# Create the target directory for model initializers, open62541 and ControlSystemAdapter libraries/headers
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src_generated")
include_directories(${PROJECT_BINARY_DIR}/src_generated/)

##########################################################################################################
## Optionen
##########################################################################################################

## Project specific options
option(ENABLE_CREATEMODEL      "Create model from XML description"      OFF)
## Code quality control options
option(ENABLE_COVERAGE        "Enable coverage stats and tests for adapter build" OFF)
option(BUILD_STATIC_ANALYSIS  "Instead of building a binary, perform a static code analysis using clangs analyzer." OFF)
option(ENABLE_LINTING         "Enables running the cppcheck static analyzer prior to compilation." OFF)
option(ENABLE_UNIT_TESTS 			"Compile and run unit tests." ON)
option(ENABLE_DOCUMENTATION		"Generate Doxygen documentation" OFF)
option(ENABLE_RUNTIME_ANALYSIS "Build Runtime analyses" OFF)

if(ENABLE_CREATEMODEL)
  set(MODEL_XML_FILE "adapterim.xml" CACHE STRING "Namespace definition XML file for MTCA Model")

  list(APPEND objectSources ${PROJECT_BINARY_DIR}/src_generated/csa_namespaceinit_generated.c)
else()
  include_directories(${CMAKE_SOURCE_DIR}/include/model_prebuilt/)
  list(APPEND objectSources ${CMAKE_SOURCE_DIR}/include/model_prebuilt/csa_namespaceinit_generated.c)
endif()

if(ENABLE_COVERAGE)
  set(CMAKE_BUILD_TYPE DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

  file(COPY ${PROJECT_SOURCE_DIR}/cmake/make_coverage.sh.in DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  add_custom_target(coverage
    ${CMAKE_CURRENT_BINARY_DIR}/make_coverage.sh.in ${CMAKE_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      # Bad soulution to provide all depencys
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_csa_opcua_adapter
      #DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_csa_opcua_application
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_ipc
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_processvariable
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_additionalvariable
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_uaadapter
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_xml_handler
      COMMENT "Generating test coverage documentation" VERBATIM
  )

endif()

##########################################################################################################
## Definition for compiler for static analysis
##########################################################################################################
if(BUILD_STATIC_ANALYSIS)
        set(CMAKE_C_COMPILER   ccc-analyzer)
        set(CMAKE_CXX_COMPILER c++-analyzer)
        add_definitions(-o ${PROJECT_BINARY_DIR}/static-analysis})
endif()

##########################################################################################################
## Definition for the thread and address sanitizer. Must be set before adding the test subdirectories
##########################################################################################################
# We need spearate configuration types becasue flags from other types are conflicting
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;asan;tsan")
set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS} -g -O1 -fsanitize=thread")
set(CMAKE_C_FLAGS_TSAN "${CMAKE_C_FLAGS} -g -O1 -fsanitize=thread")

set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS} -g -O0 -fsanitize=address -fsanitize=undefined -fsanitize=leak")
set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS} -g -O0 -fsanitize=address -fsanitize=undefined -fsanitize=leak")

if(ENABLE_UNIT_TESTS)
        file(GLOB testFiles "${CMAKE_SOURCE_DIR}/tests/uamapping_test*.xml")
        foreach(testFile ${testFiles})
                file(COPY ${testFile} DESTINATION ${PROJECT_BINARY_DIR}/tests)
                message("Copy ${testFile}")
        endforeach(testFile)

        enable_testing()
        add_subdirectory(tests)
endif()

if(ENABLE_RUNTIME_ANALYSIS)
        file(COPY ${PROJECT_SOURCE_DIR}/cmake/make_valgrind.sh.in DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        add_custom_target(valgrind ALL
                ${CMAKE_CURRENT_BINARY_DIR}/make_valgrind.sh.in ${CMAKE_SOURCE_DIR} ${PROJECT_BINARY_DIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                        # Bad soulution to provide all depencys
                        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_csa_opcua_adapter
                        #DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_csa_opcua_application
                        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_ipc
                        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_processvariable
                        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_additionalvariable
                        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_uaadapter
                        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_xml_handler
      COMMENT "Generating valgrind runtime test" VERBATIM
     )
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
if(USE_ControlSystemAdapter)
        add_dependencies(mtca_objects external-ChimeraTK-ControlSystemAdapter)
        add_dependencies(mtca_objects external-ChimeraTK-DeviceAccess)
        add_dependencies(mtca_objects open62541::open62541)
endif()

## Early declaration due to the requirement to run the dependencies update prior to building
## Create shared lib
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:mtca_objects>)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION}
                                                 SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-ControlSystemAdapter_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-DeviceAccess_LIBRARIES})
target_link_libraries(${PROJECT_NAME} open62541::open62541)
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_CHRONO_LIBRARY}
)
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})
#set_source_files_properties(${CMAKE_SOURCE_DIR}/src/open62541.c PROPERTIES COMPILE_FLAGS -w)

# do not remove runtime path of the library when installing
set_property(TARGET ${PROJECT_NAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

##########################################################################################################
## Install all Libraries
##########################################################################################################

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib )

##########################################################################################################
## Custom Commands (z.b. model-compiler, autobuild-dependencies)
##########################################################################################################

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/src_generated/csa_namespaceinit_generated.c
                          ${PROJECT_BINARY_DIR}/src_generated/csa_namespaceinit_generated.h
                   PRE_BUILD
                   COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/generate_open62541CCode.py
                                                -i ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist_FullNS0.txt
                                                -b ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist.txt
                                                ${PROJECT_SOURCE_DIR}/model/Opc.Ua.NodeSet2.xml
                                                ${PROJECT_SOURCE_DIR}/model/${MODEL_XML_FILE}
                                                ${PROJECT_BINARY_DIR}/src_generated/csa_namespaceinit_generated
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

add_executable(ControlSystem-OPCUA_Sample_Adapter ${CMAKE_SOURCE_DIR}/examples/csa_opcua_adapter_example.cpp ${CMAKE_SOURCE_DIR}/examples/runtime_value_generator.cpp)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter ${PROJECT_NAME})
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter pthread)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter open62541::open62541)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_CHRONO_LIBRARY}
)

add_executable(ControlSystem-OPCUA_Sample_Application ${CMAKE_SOURCE_DIR}/examples/csa_opcua_adapter_example2.cpp)
target_link_libraries(ControlSystem-OPCUA_Sample_Application ${PROJECT_NAME})
target_link_libraries(ControlSystem-OPCUA_Sample_Application pthread)
target_link_libraries(ControlSystem-OPCUA_Sample_Application open62541::open62541)
target_link_libraries(ControlSystem-OPCUA_Sample_Application
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_CHRONO_LIBRARY}
)

file(COPY ${PROJECT_SOURCE_DIR}/opcuaAdapter_mapping.xml DESTINATION ${CMAKE_BINARY_DIR}/)

if(ENABLE_LINTING)
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/static_analysis")
add_custom_command( TARGET ControlSystem-OPCUA_Sample_Adapter
                    PRE_BUILD
                    COMMAND /usr/bin/cppcheck --std=c++11 --inline-suppr --force --template=gcc --enable=all ${objectSources} 2> ${CMAKE_BINARY_DIR}/static_analysis/cppcheck.log
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
set(${PROJECT_NAME}_LIBRARIES "${ChimeraTK-ControlSystemAdapter_LIBRARIES} ${open62541_LIBRARIES}")
set(${PROJECT_NAME}_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib ${ChimeraTK-ControlSystemAdapter_LIBRARY_DIRS}")
set(${PROJECT_NAME}_CXX_FLAGS "${ChimeraTK-ControlSystemAdapter_CXX_FLAGS}")
set(${PROJECT_NAME}_LINKER_FLAGS "${ChimeraTK-ControlSystemAdapter_LINKER_FLAGS} -Wl,-rpath=${CMAKE_INSTALL_PREFIX}/lib,--enable-new-dtags")
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()
