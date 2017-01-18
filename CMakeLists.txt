# (c) Isil Dillig (idillig@cs.wm.edu), Thomas Dillig (tdillig@cs.wm.edu)
#
# This is the main cmake file coordinating SAIL, Mistral and Compass (and serialization).
# SAIL and Mistral can build on their own--simply use their subdirectory.
#
#
#set(CMAKE_CXX_COMPILER "g++-4.5")
#set(CMAKE_C_COMPILER "gcc-4.5")

#set(CMAKE_CXX_COMPILER "g++")
#set(CMAKE_C_COMPILER "gcc")

cmake_minimum_required(VERSION 2.6)

#We use build type DEBUG so assert is enabled.
#set (CMAKE_BUILD_TYPE Debug)

#These flags have been tuned to give the best performance, however, they may render
# valgrind useless. If you must debug memory corruption using valgrind, remove 
# -ffast-math -mfpmath=sse -msse3


#set(CMAKE_CXX_COMPILER             "clang++")

set(CMAKE_CXX_FLAGS "-std=c++0x -O2 -mtune=native -march=native -ftree-vectorize -funroll-loops -fstrict-aliasing  -fno-stack-protector -ffast-math -mfpmath=sse -msse3 -Wall -fmessage-length=0 -g -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -O2 -mtune=native -march=native -ftree-vectorize -funroll-loops -fstrict-aliasing  -fno-stack-protector -ffast-math -mfpmath=sse -msse3 -Wall -fmessage-length=0 -g ")


#set required include dirs



#list all subdirectories that contain CMakeLists.txt
#add_subdirectory(sail)
#add_subdirectory(dex-parser)
#add_subdirectory(sail-test)



project (compass)

#We need KDE4 to build the compass_ui
find_package(KDE4 REQUIRED)
include (KDE4Defaults)

#set required include dirs
include_directories(${KDE4_INCLUDES} ./util ./compass/access-path ./compass/client-analysis ./compass/client-analysis-manager ./compass/error-traces ./compass/error-diagnosis ./compass/manager ./compass/memory-graph ./compass/summary ./compass/ui ./serialization  ./sail ./sail/sail ./sail/il ./dex-parser ./mistral/ ./mistral/cnode ./mistral/sat-solver ./mistral/term ./mistral/solver ./mistral/numeric-lib)


#list all subdirectories that contain CMakeLists.txt
add_subdirectory(util)
add_subdirectory(sail)
add_subdirectory(dex-parser)
add_subdirectory(sail-test)
add_subdirectory(mistral)
add_subdirectory(serialization)
add_subdirectory(compass)
add_subdirectory(abductor)
#add_subdirectory(pointer-analysis)
add_subdirectory(dex-parser-test)




