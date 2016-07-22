cmake_minimum_required(VERSION 2.8.0)

project(ControlSystem-OPCUA_Adapter)

##########################################################################################################
## Includes, Link dirs und Quellcode
##########################################################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/open62541/)
include_directories(${CMAKE_SOURCE_DIR}/include/ControlSystemAdapter/)
link_directories(${CMAKE_SOURCE_DIR}/lib)

set(objectSources  ${CMAKE_SOURCE_DIR}/src/ua_mapped_class.cpp
                  ${CMAKE_SOURCE_DIR}/src/ua_proxies.cpp
                  ${CMAKE_SOURCE_DIR}/src/ipc_managed_object.cpp
                  ${CMAKE_SOURCE_DIR}/src/ipc_manager.cpp
                  ${CMAKE_SOURCE_DIR}/src/ipc_task.cpp
)

# Create the target directory for model initializers, open62541 and ControlSystemAdapter libraries/headers
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src_generated")
include_directories(${PROJECT_BINARY_DIR}/src_generated/)

set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/ua_mapped_class.cpp APPEND PROPERTY OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/lib/libopen62541.so)
set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/ControlSystem_OPCUA_Sample_Adapter.cpp APPEND PROPERTY OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/lib/libopen62541.so)

##########################################################################################################
## Optionen
##########################################################################################################

## Project specific options
option(ENABLE_BUILDMODEL      "Create model from XML description"      ON)

## Code quality control options
option(ENABLE_COVERAGE        "Enable coverage stats for server build" ON)
option(BUILD_STATIC_ANALYSIS  "Instead of building a binary, perform a static code analysis using clangs analyzer." OFF)
option(ENABLE_LINTING         "Enables running the cppcheck static analyzer prior to compilation." OFF)
option(ENABLE_UNIT_TESTS      "Compile and run unit tests." OFF)

if(ENABLE_BUILDMODEL)
  set(MTCA_MODEL_XML "templatemodel.xml" CACHE STRING "Namespace definition XML file for MTCA Model")
  
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
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_STATIC_ANALYSIS)
  set(CMAKE_C_COMPILER   ccc-analyzer)
  set(CMAKE_CXX_COMPILER ccc-analyzer)
  add_definitions(-o ${PROJECT_BINARY_DIR}/static-analysis})
endif()

##########################################################################################################
## Custom Commands (z.b. model-compiler, autobuild-dependencies)
##########################################################################################################

add_custom_command(OUTPUT   ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist_FullNS0.txt
                            ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist.txt
                            ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/generate_open62541CCode.py
                            ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/open62541_MacroHelper.py
                            ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_builtin_types.py
                            ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_constants.py
                            ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_namespace.py
                            ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_node_types.py
                            ${PROJECT_SOURCE_DIR}/lib/libopen62541.so
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
                                                ${PROJECT_SOURCE_DIR}/model/${MTCA_MODEL_XML}
                                                ${PROJECT_BINARY_DIR}/src_generated/mtca_namespaceinit_generated
                   DEPENDS ${PROJECT_SOURCE_DIR}/model/Opc.Ua.NodeSet2.xml
                           ${PROJECT_SOURCE_DIR}/model/${MTCA_MODEL_XML}
                           ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist_FullNS0.txt
                           ${PROJECT_SOURCE_DIR}/model/NodeID_Blacklist.txt
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/generate_open62541CCode.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/open62541_MacroHelper.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_builtin_types.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_constants.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_namespace.py
                           ${PROJECT_SOURCE_DIR}/tools/pyUANamespace/ua_node_types.py)

                           
##########################################################################################################
##    Build-Abschnitt
##########################################################################################################
## Config File
configure_file (
  "${PROJECT_SOURCE_DIR}/include/csa_config.h.in"
  "${PROJECT_BINARY_DIR}/src_generated/csa_config.h"
  )
 
## Object Files
add_library(mtca_objects OBJECT ${objectSources})

## Binary: ControlSystem-OPCUA_Sample_Adapter
add_executable(ControlSystem-OPCUA_Sample_Adapter ${CMAKE_SOURCE_DIR}/src/ControlSystem_OPCUA_Sample_Adapter.cpp $<TARGET_OBJECTS:mtca_objects> )
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter open62541)
target_link_libraries(ControlSystem-OPCUA_Sample_Adapter pthread)
if(ENABLE_LINTING)
add_custom_command( TARGET ControlSystem-OPCUA_Sample_Adapter
                    PRE_BUILD
                    COMMAND /usr/bin/cppcheck --std=c++11 --inline-suppr --enable=all ${objectSources}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Running CppCheck on all CPP Sources" VERBATIM)
endif()
## Ends binary ControlSystem-OPCUA_Sample_Adapter


install(TARGETS ControlSystem-OPCUA_Sample_Adapter RUNTIME DESTINATION bin)
  