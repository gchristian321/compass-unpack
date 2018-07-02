# $Id: CMakeLists.txt 68058 2013-03-13 14:47:43Z gcosmo $

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(compass-unpack CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

## Find ROOT
EXECUTE_PROCESS(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND root-config --libs --glibs OUTPUT_VARIABLE ROOT_LIBRARIES OUTPUT_STRIP_TRAILING_WHITESPACE)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")

## Set up c++ flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
# set (NPTOOL_LIBS "-L$ENV{NPTOOL}/NPLib/lib -lNPInitialConditions -lNPTiara")

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/include/json/*.*)
#list(REMOVE_ITEM sources "${PROJECT_SOURCE_DIR}/src/compass-unpack.cpp")

# add_custom_command(OUTPUT Linkdef.h COMMAND ${PROJECT_SOURCE_DIR}/scripts/MakeLinkdef.sh Linkdef.h "${PROJECT_SOURCE_DIR}/include/*.hpp" "${PROJECT_SOURCE_DIR}/include/json/*.*" DEPENDS ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/include/json/*.*)
# add_custom_command(OUTPUT Dict.cxx COMMAND rootcint -f Dict.cxx -c "${PROJECT_SOURCE_DIR}/include/*.hxx" Linkdef.h DEPENDS Linkdef.h)
# add_custom_command(OUTPUT r6utils.sh COMMAND ${PROJECT_SOURCE_DIR}/scripts/gen_r6utils.sh ${PROJECT_SOURCE_DIR}/scripts/r6utils.sh ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/scripts/r6utils.C)



#----------------------------------------------------------------------------
# Add the shared library & executables, and link it to the necessary libraries
#
# add_library(r6utils SHARED ${sources} ${headers} Dict.cxx r6utils.sh)
#target_link_libraries(r6utils ${ROOT_LIBRARIES})
add_executable(compass-unpack ${sources}) # ${PROJECT_SOURCE_DIR}/src/compass-unpack.cpp)
target_link_libraries(compass-unpack ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
# install(TARGETS r6utils DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
