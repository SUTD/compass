# (c) Isil Dillig (idillig@cs.wm.edu), Thomas Dillig (tdillig@cs.wm.edu)




set (CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++0x -O2 -mtune=native -march=native  -funroll-loops -fstrict-aliasing  -fno-stack-protector  -Wall -fmessage-length=0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -O2 -mtune=native -march=native  -funroll-loops -fstrict-aliasing  -fno-stack-protector -Wall -fmessage-length=0 -g")


file(GLOB MY_SRC "*.cpp")


include_directories(.)

add_library(util ${MY_SRC})
