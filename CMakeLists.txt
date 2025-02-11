cmake_minimum_required(VERSION 3.16)

##########################################################################################################
## Project specification
##########################################################################################################
project(ChimeraTK-ControlSystemAdapter-OPCUAAdapter)
SET(${PROJECT_NAME}_MAJOR_VERSION 04)
SET(${PROJECT_NAME}_MINOR_VERSION 00)
SET(${PROJECT_NAME}_PATCH_VERSION 03)
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

##########################################################################################################
## Hard package dependencies
##########################################################################################################

find_package(Boost COMPONENTS system filesystem thread chrono unit_test_framework REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(PythonInterp REQUIRED)
find_package(open62541 1.4 REQUIRED)

find_package(ChimeraTK-ControlSystemAdapter 02.05 REQUIRED)

##########################################################################################################
## Set the build type to Release if none is specified
## Force it into Release if "None" is specified (needed to overrule dkpg_buildpackage)
##########################################################################################################

include(cmake/set_default_build_to_release.cmake)

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

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/examples/)
include_directories(SYSTEM ${LIBXML2_INCLUDE_DIR})

link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${LIBXML2_LIBRARIES})

set(objectSources  ${CMAKE_SOURCE_DIR}/src/ua_mapped_class.cpp
        ${CMAKE_SOURCE_DIR}/src/ua_map_types.cpp
        ${CMAKE_SOURCE_DIR}/src/csa_opcua_adapter.cpp
        ${CMAKE_SOURCE_DIR}/src/node_historizing.cpp
        ${CMAKE_SOURCE_DIR}/src/void_type.cpp
        ${CMAKE_SOURCE_DIR}/src/xml_file_handler.cpp
        ${CMAKE_SOURCE_DIR}/src/csa_processvariable.cpp
        ${CMAKE_SOURCE_DIR}/src/csa_additionalvariable.cpp
        ${CMAKE_SOURCE_DIR}/src/ua_adapter.cpp
        ${CMAKE_SOURCE_DIR}/src/csa_opcua_application.cpp
        #${CMAKE_SOURCE_DIR}/src/csa_namespace.c -> see comment in the Object building section
        )

# Create the target directory for model initializers, open62541 and ControlSystemAdapter libraries/headers
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src_generated")
include_directories(${PROJECT_BINARY_DIR}/src_generated/)

##########################################################################################################
## Options
##########################################################################################################

## Code quality control options
option(BUILD_STATIC_ANALYSIS  "Instead of building a binary, perform a static code analysis using clangs analyzer." OFF)
option(ENABLE_LINTING         "Enables running the cppcheck static analyzer prior to compilation." OFF)
option(ENABLE_UNIT_TESTS 			"Compile and run unit tests." ON)
option(ENABLE_DOCUMENTATION		"Generate Doxygen documentation" OFF)
option(ENABLE_RUNTIME_ANALYSIS "Build Runtime analyses" OFF)
option(INSTALL_SAMPLE_ADAPTER "Install an example application called ControlSystem-OPCUA_Sample_Adapter" OFF)
# TODO: As soon if the test passes enable it per default. 
#       If removing the option add info in csa_opcua_application.cpp doxygen docu.
option(CHECK_CTK_STYLE        "Check ChimeraTK code style and license information." OFF)

include(cmake/enable_code_coverage_report.cmake)

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
set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS} -g -O1 -fsanitize=thread  -fno-inline")
set(CMAKE_C_FLAGS_TSAN "${CMAKE_C_FLAGS} -g -O1 -fsanitize=thread  -fno-inline")

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
    configure_file(cmake/make_valgrind.sh.in ${CMAKE_CURRENT_BINARY_DIR}/make_valgrind.sh @ONLY)
    add_custom_target(valgrind
            ./make_valgrind.sh 
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            # Bad solution to provide all dependencies
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_csa_opcua_adapter
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_processvariable
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_additionalvariable
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_opcua_uaadapter
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/test_xml_handler
            COMMENT "Generating valgrind runtime test" VERBATIM
            )
endif()

include(cmake/enable_doxygen_documentation.cmake)

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
target_link_libraries(mtca_objects PRIVATE open62541::open62541)
target_link_libraries(mtca_objects PUBLIC ChimeraTK::ChimeraTK-ControlSystemAdapter)

# ChimeraTK::ChimeraTK-ControlSystemAdapter exports the flag c++-17 with INTERFACE_COMPILE_OPTIONS. 
# This causes a problem with clang c compilation.
add_library(mtca_c_objetcs OBJECT ${CMAKE_SOURCE_DIR}/src/csa_namespace.c)
target_link_libraries(mtca_c_objetcs PRIVATE open62541::open62541)

## Early declaration due to the requirement to run the dependencies update prior to building
## Create shared lib
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:mtca_objects> $<TARGET_OBJECTS:mtca_c_objetcs>)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION}
        SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} PUBLIC ChimeraTK::ChimeraTK-ControlSystemAdapter)
target_link_libraries(${PROJECT_NAME} PRIVATE open62541::open62541)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBXML2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)
target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_CHRONO_LIBRARY}
        )

# do not remove runtime path of the library when installing
set_property(TARGET ${PROJECT_NAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
##########################################################################################################
## Install all Libraries
##########################################################################################################

# this defines architecture-dependent ${CMAKE_INSTALL_LIBDIR}
include(GNUInstallDirs)
install( TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}")    

##########################################################################################################
##  Tests and executables
##########################################################################################################

link_directories(${CMAKE_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/opcuaAdapter_mapping.xml DESTINATION ${CMAKE_BINARY_DIR}/)

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

if(ENABLE_LINTING)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/static_analysis")
    add_custom_command( TARGET ControlSystem-OPCUA_Sample_Adapter
            PRE_BUILD
            COMMAND /usr/bin/cppcheck --std=c++11 --inline-suppr --force --template=gcc --enable=all ${objectSources} 2> ${CMAKE_BINARY_DIR}/static_analysis/cppcheck.log
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running CppCheck on all CPP Sources" VERBATIM)
endif()

if(CHECK_CTK_STYLE)
  include(cmake/enable_code_style_check.cmake)
endif(CHECK_CTK_STYLE)

if(INSTALL_SAMPLE_ADAPTER)
  install(TARGETS ControlSystem-OPCUA_Sample_Adapter RUNTIME DESTINATION bin)
  file(COPY ${PROJECT_SOURCE_DIR}/opcuaAdapter_mapping.xml DESTINATION ${CMAKE_BINARY_DIR}/)  
endif(INSTALL_SAMPLE_ADAPTER)

##########################################################################################################
##  Install map file generator
##########################################################################################################


include(pythonBindingHelperMacros)

#
# We build for python3 by default
set(PYTHON3 TRUE CACHE BOOL "Builds modules for python3 if true and python2 if false")
initialize()

find_package(PythonLibs 3 REQUIRED)
get_python_module_install_path(PYTHONLIBS_VERSION_STRING python_module_path)

install(DIRECTORY tools/mapfileGenerator/ctk_opcua_generator_tools
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${python_module_path})
install(FILES tools/mapfileGenerator/chimeratk-opc-ua-mapfile-generator
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                     GROUP_READ GROUP_EXECUTE
                     WORLD_READ WORLD_EXECUTE)

##########################################################################################################
## Create the config files by filling the correct variables into the template (*.cmake.in).
## All variables @VARIABLE@ are replaced with the current values, but they have to be set first....
## Only the reference header is needed. The other variables are for dependencies of this library,
## but there are none.
##########################################################################################################


# we support our cmake EXPORTS as imported targets
set(PROVIDES_EXPORTED_TARGETS 1)
# we need the public dependencies so create_cmake_config_files can find them as implicit dependencies
list(APPEND ${PROJECT_NAME}_PUBLIC_DEPENDENCIES "ChimeraTK-ControlSystemAdapter")
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()
