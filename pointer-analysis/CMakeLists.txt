#haiyan zhu 3/12
set (CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "-std=c++0x -O2 -mtune=native -march=native  -funroll-loops -fstrict-aliasing  -fno-stack-protector  -Wall -fmessage-length=0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -O2 -mtune=native -march=native  -funroll-loops -fstrict-aliasing  -fno-stack-protector -Wall -fmessage-length=0 -g")

include_directories(./access-path ./mem-graph/ ../compass ./static-assert-builder 
./constraints  ./ )

file(GLOB MY_SRC "*.cpp" "mem-graph/*.cpp" "access-path/*.cpp" "../compass/compass_assert.cpp" 
"static-assert-builder/*.cpp" "constraints/*.cpp" )

add_executable(pointer-analysis ${MY_SRC})

target_link_libraries(pointer-analysis dex-parser sail boost_thread boost_serialization util mistral gmp)


