# (c) Isil Dillig (isil@cs.stanford.edu), Thomas Dillig (tdillig@cs.stanford.edu)

set (CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++0x -O2 -mtune=native -march=native -ftree-vectorize -funroll-loops -fstrict-aliasing  -fno-stack-protector -ffast-math -mfpmath=sse -msse3 -Wall -fmessage-length=0 -g ")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -O2 -mtune=native -march=native -ftree-vectorize -funroll-loops -fstrict-aliasing  -fno-stack-protector -ffast-math -mfpmath=sse -msse3 -Wall -fmessage-length=0 -g ")

file(GLOB MY_SRC "*.cpp")


add_executable(abductor ${MY_SRC})

target_link_libraries(abductor mistral sail gmp boost_thread boost_serialization util)
