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
CMAKE_SOURCE_DIR = /home/yufeng/research/others/compass

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yufeng/research/others/compass

# Include any dependencies generated for this target.
include serialization/CMakeFiles/serialization.dir/depend.make

# Include the progress variables for this target.
include serialization/CMakeFiles/serialization.dir/progress.make

# Include the compile flags for this target's objects.
include serialization/CMakeFiles/serialization.dir/flags.make

serialization/CMakeFiles/serialization.dir/compass-serialization.o: serialization/CMakeFiles/serialization.dir/flags.make
serialization/CMakeFiles/serialization.dir/compass-serialization.o: serialization/compass-serialization.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/yufeng/research/others/compass/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object serialization/CMakeFiles/serialization.dir/compass-serialization.o"
	cd /home/yufeng/research/others/compass/serialization && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/serialization.dir/compass-serialization.o -c /home/yufeng/research/others/compass/serialization/compass-serialization.cpp

serialization/CMakeFiles/serialization.dir/compass-serialization.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/serialization.dir/compass-serialization.i"
	cd /home/yufeng/research/others/compass/serialization && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/yufeng/research/others/compass/serialization/compass-serialization.cpp > CMakeFiles/serialization.dir/compass-serialization.i

serialization/CMakeFiles/serialization.dir/compass-serialization.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/serialization.dir/compass-serialization.s"
	cd /home/yufeng/research/others/compass/serialization && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/yufeng/research/others/compass/serialization/compass-serialization.cpp -o CMakeFiles/serialization.dir/compass-serialization.s

serialization/CMakeFiles/serialization.dir/compass-serialization.o.requires:
.PHONY : serialization/CMakeFiles/serialization.dir/compass-serialization.o.requires

serialization/CMakeFiles/serialization.dir/compass-serialization.o.provides: serialization/CMakeFiles/serialization.dir/compass-serialization.o.requires
	$(MAKE) -f serialization/CMakeFiles/serialization.dir/build.make serialization/CMakeFiles/serialization.dir/compass-serialization.o.provides.build
.PHONY : serialization/CMakeFiles/serialization.dir/compass-serialization.o.provides

serialization/CMakeFiles/serialization.dir/compass-serialization.o.provides.build: serialization/CMakeFiles/serialization.dir/compass-serialization.o

# Object files for target serialization
serialization_OBJECTS = \
"CMakeFiles/serialization.dir/compass-serialization.o"

# External object files for target serialization
serialization_EXTERNAL_OBJECTS =

lib/libserialization.a: serialization/CMakeFiles/serialization.dir/compass-serialization.o
lib/libserialization.a: serialization/CMakeFiles/serialization.dir/build.make
lib/libserialization.a: serialization/CMakeFiles/serialization.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library ../lib/libserialization.a"
	cd /home/yufeng/research/others/compass/serialization && $(CMAKE_COMMAND) -P CMakeFiles/serialization.dir/cmake_clean_target.cmake
	cd /home/yufeng/research/others/compass/serialization && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/serialization.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
serialization/CMakeFiles/serialization.dir/build: lib/libserialization.a
.PHONY : serialization/CMakeFiles/serialization.dir/build

serialization/CMakeFiles/serialization.dir/requires: serialization/CMakeFiles/serialization.dir/compass-serialization.o.requires
.PHONY : serialization/CMakeFiles/serialization.dir/requires

serialization/CMakeFiles/serialization.dir/clean:
	cd /home/yufeng/research/others/compass/serialization && $(CMAKE_COMMAND) -P CMakeFiles/serialization.dir/cmake_clean.cmake
.PHONY : serialization/CMakeFiles/serialization.dir/clean

serialization/CMakeFiles/serialization.dir/depend:
	cd /home/yufeng/research/others/compass && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yufeng/research/others/compass /home/yufeng/research/others/compass/serialization /home/yufeng/research/others/compass /home/yufeng/research/others/compass/serialization /home/yufeng/research/others/compass/serialization/CMakeFiles/serialization.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : serialization/CMakeFiles/serialization.dir/depend
