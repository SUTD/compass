# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lijiaying/Research/GitHub/compass

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lijiaying/Research/GitHub/compass/build

# Include any dependencies generated for this target.
include util/CMakeFiles/util.dir/depend.make

# Include the progress variables for this target.
include util/CMakeFiles/util.dir/progress.make

# Include the compile flags for this target's objects.
include util/CMakeFiles/util.dir/flags.make

util/CMakeFiles/util.dir/util.o: util/CMakeFiles/util.dir/flags.make
util/CMakeFiles/util.dir/util.o: ../util/util.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/lijiaying/Research/GitHub/compass/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object util/CMakeFiles/util.dir/util.o"
	cd /home/lijiaying/Research/GitHub/compass/build/util && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/util.dir/util.o -c /home/lijiaying/Research/GitHub/compass/util/util.cpp

util/CMakeFiles/util.dir/util.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/util.dir/util.i"
	cd /home/lijiaying/Research/GitHub/compass/build/util && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/lijiaying/Research/GitHub/compass/util/util.cpp > CMakeFiles/util.dir/util.i

util/CMakeFiles/util.dir/util.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/util.dir/util.s"
	cd /home/lijiaying/Research/GitHub/compass/build/util && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/lijiaying/Research/GitHub/compass/util/util.cpp -o CMakeFiles/util.dir/util.s

util/CMakeFiles/util.dir/util.o.requires:
.PHONY : util/CMakeFiles/util.dir/util.o.requires

util/CMakeFiles/util.dir/util.o.provides: util/CMakeFiles/util.dir/util.o.requires
	$(MAKE) -f util/CMakeFiles/util.dir/build.make util/CMakeFiles/util.dir/util.o.provides.build
.PHONY : util/CMakeFiles/util.dir/util.o.provides

util/CMakeFiles/util.dir/util.o.provides.build: util/CMakeFiles/util.dir/util.o

# Object files for target util
util_OBJECTS = \
"CMakeFiles/util.dir/util.o"

# External object files for target util
util_EXTERNAL_OBJECTS =

lib/libutil.a: util/CMakeFiles/util.dir/util.o
lib/libutil.a: util/CMakeFiles/util.dir/build.make
lib/libutil.a: util/CMakeFiles/util.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library ../lib/libutil.a"
	cd /home/lijiaying/Research/GitHub/compass/build/util && $(CMAKE_COMMAND) -P CMakeFiles/util.dir/cmake_clean_target.cmake
	cd /home/lijiaying/Research/GitHub/compass/build/util && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/util.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
util/CMakeFiles/util.dir/build: lib/libutil.a
.PHONY : util/CMakeFiles/util.dir/build

util/CMakeFiles/util.dir/requires: util/CMakeFiles/util.dir/util.o.requires
.PHONY : util/CMakeFiles/util.dir/requires

util/CMakeFiles/util.dir/clean:
	cd /home/lijiaying/Research/GitHub/compass/build/util && $(CMAKE_COMMAND) -P CMakeFiles/util.dir/cmake_clean.cmake
.PHONY : util/CMakeFiles/util.dir/clean

util/CMakeFiles/util.dir/depend:
	cd /home/lijiaying/Research/GitHub/compass/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lijiaying/Research/GitHub/compass /home/lijiaying/Research/GitHub/compass/util /home/lijiaying/Research/GitHub/compass/build /home/lijiaying/Research/GitHub/compass/build/util /home/lijiaying/Research/GitHub/compass/build/util/CMakeFiles/util.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : util/CMakeFiles/util.dir/depend
