# (c) Isil Dillig (isil@cs.stanford.edu), Thomas Dillig (tdillig@cs.stanford.edu)




set (CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++0x -O2 -mtune=native -march=native -ftree-vectorize -funroll-loops -fstrict-aliasing  -fno-stack-protector -ffast-math -mfpmath=sse -msse3 -Wall -fmessage-length=0 -g ")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -O2 -mtune=native -march=native -ftree-vectorize -funroll-loops -fstrict-aliasing  -fno-stack-protector -ffast-math -mfpmath=sse -msse3 -Wall -fmessage-length=0 -g ")


file(GLOB MY_SRC "*.cpp" "access-path/*.cpp" "client-analysis-manager/*.cpp" "client-analysis/*.cpp" "error-traces/*.cpp" "manager/*.cpp" "memory-graph/*.cpp" "summary/*.cpp" "error-diagnosis/*.cpp")

add_library(compass ${MY_SRC}) 
target_link_libraries(compass mistral sail)

file(GLOB compass_SRCS ui/*.cpp)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)



kde4_add_executable(compass_ui ${compass_SRCS})


target_link_libraries(compass_ui ${KDE4_KDEUI_LIBS} GL GLU QtOpenGL gmp ktexteditor kdeui boost_thread boost_serialization pthread util)


#custom libs build here
target_link_libraries(compass_ui compass serialization)
 
install(TARGETS compass_ui  ${INSTALL_TARGETS_DEFAULT_ARGS})




