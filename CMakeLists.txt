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

## ADD XML library linking
string(CONCAT ROOT_LIBRARIES "${ROOT_LIBRARIES}" " -lXMLIO")
message("ROOT_LIBRARIES: ${ROOT_LIBRARIES}")


# ## Look for NPTOOL
# if (DEFINED ENV{NPTOOL})
#   message("Nptool found! Enabling nptool output...")
#   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{NPTOOL}/NPLib/include")
#   set (NPTOOL_LIBS "-L$ENV{NPTOOL}/NPLib/lib -lNPCore -lNPInitialConditions -lNPMDM -lNPFPDTamu")
#   add_definitions(-DHAVE_NPLIB)
# else()
#   message("Nptool not found! Disabling nptool output...")
#   set (NPTOOL_LIBS "")
# endif()
  

## Set up c++ flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/include/json/*.*)

#----------------------------------------------------------------------------
# Add the shared library & executables, and link it to the necessary libraries
#
find_library(r6utils_location NAMES libr6utils.so)
message(STATUS ${r6utils_location})
#add_library(r6utils SHARED ${sources} ${headers} Dict.cxx r6utils.sh)
add_executable(compass-unpack ${sources}) # ${PROJECT_SOURCE_DIR}/src/compass-unpack.cpp)
target_link_libraries(compass-unpack ${ROOT_LIBRARIES} ${NPTOOL_LIBS})

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
#install(TARGETS r6utils DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
