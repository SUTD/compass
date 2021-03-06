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
include mistral/smtparser/CMakeFiles/smtparser.dir/depend.make

# Include the progress variables for this target.
include mistral/smtparser/CMakeFiles/smtparser.dir/progress.make

# Include the compile flags for this target's objects.
include mistral/smtparser/CMakeFiles/smtparser.dir/flags.make

mistral/smtparser/smtparser.cpp: ../mistral/smtparser/smtparser.y
	$(CMAKE_COMMAND) -E cmake_progress_report /home/lijiaying/Research/GitHub/compass/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "[BISON][SMTParser] Building parser with bison 3.0.2"
	cd /home/lijiaying/Research/GitHub/compass/mistral/smtparser && /usr/bin/bison -d -o /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtparser.cpp smtparser.y

mistral/smtparser/smtparser.hpp: mistral/smtparser/smtparser.cpp

mistral/smtparser/smtlexer.cpp: ../mistral/smtparser/smtlexer.l
	$(CMAKE_COMMAND) -E cmake_progress_report /home/lijiaying/Research/GitHub/compass/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "[FLEX][SMTLexer] Building scanner with flex 2.5.35"
	cd /home/lijiaying/Research/GitHub/compass/mistral/smtparser && /usr/bin/flex -o/home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtlexer.cpp smtlexer.l

mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o: mistral/smtparser/CMakeFiles/smtparser.dir/flags.make
mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o: mistral/smtparser/smtlexer.cpp
mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o: mistral/smtparser/smtparser.hpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/lijiaying/Research/GitHub/compass/build/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/smtparser.dir/smtlexer.o -c /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtlexer.cpp

mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/smtparser.dir/smtlexer.i"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtlexer.cpp > CMakeFiles/smtparser.dir/smtlexer.i

mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/smtparser.dir/smtlexer.s"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtlexer.cpp -o CMakeFiles/smtparser.dir/smtlexer.s

mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.requires:
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.requires

mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.provides: mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.requires
	$(MAKE) -f mistral/smtparser/CMakeFiles/smtparser.dir/build.make mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.provides.build
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.provides

mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.provides.build: mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o

mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o: mistral/smtparser/CMakeFiles/smtparser.dir/flags.make
mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o: mistral/smtparser/smtparser.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/lijiaying/Research/GitHub/compass/build/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/smtparser.dir/smtparser.o -c /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtparser.cpp

mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/smtparser.dir/smtparser.i"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtparser.cpp > CMakeFiles/smtparser.dir/smtparser.i

mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/smtparser.dir/smtparser.s"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/smtparser.cpp -o CMakeFiles/smtparser.dir/smtparser.s

mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.requires:
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.requires

mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.provides: mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.requires
	$(MAKE) -f mistral/smtparser/CMakeFiles/smtparser.dir/build.make mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.provides.build
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.provides

mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.provides.build: mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o

mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o: mistral/smtparser/CMakeFiles/smtparser.dir/flags.make
mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o: ../mistral/smtparser/smt-parser.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/lijiaying/Research/GitHub/compass/build/CMakeFiles $(CMAKE_PROGRESS_5)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/smtparser.dir/smt-parser.o -c /home/lijiaying/Research/GitHub/compass/mistral/smtparser/smt-parser.cpp

mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/smtparser.dir/smt-parser.i"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/lijiaying/Research/GitHub/compass/mistral/smtparser/smt-parser.cpp > CMakeFiles/smtparser.dir/smt-parser.i

mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/smtparser.dir/smt-parser.s"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/lijiaying/Research/GitHub/compass/mistral/smtparser/smt-parser.cpp -o CMakeFiles/smtparser.dir/smt-parser.s

mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.requires:
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.requires

mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.provides: mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.requires
	$(MAKE) -f mistral/smtparser/CMakeFiles/smtparser.dir/build.make mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.provides.build
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.provides

mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.provides.build: mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o

# Object files for target smtparser
smtparser_OBJECTS = \
"CMakeFiles/smtparser.dir/smtlexer.o" \
"CMakeFiles/smtparser.dir/smtparser.o" \
"CMakeFiles/smtparser.dir/smt-parser.o"

# External object files for target smtparser
smtparser_EXTERNAL_OBJECTS =

lib/libsmtparser.a: mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o
lib/libsmtparser.a: mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o
lib/libsmtparser.a: mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o
lib/libsmtparser.a: mistral/smtparser/CMakeFiles/smtparser.dir/build.make
lib/libsmtparser.a: mistral/smtparser/CMakeFiles/smtparser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library ../../lib/libsmtparser.a"
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && $(CMAKE_COMMAND) -P CMakeFiles/smtparser.dir/cmake_clean_target.cmake
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/smtparser.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
mistral/smtparser/CMakeFiles/smtparser.dir/build: lib/libsmtparser.a
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/build

mistral/smtparser/CMakeFiles/smtparser.dir/requires: mistral/smtparser/CMakeFiles/smtparser.dir/smtlexer.o.requires
mistral/smtparser/CMakeFiles/smtparser.dir/requires: mistral/smtparser/CMakeFiles/smtparser.dir/smtparser.o.requires
mistral/smtparser/CMakeFiles/smtparser.dir/requires: mistral/smtparser/CMakeFiles/smtparser.dir/smt-parser.o.requires
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/requires

mistral/smtparser/CMakeFiles/smtparser.dir/clean:
	cd /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser && $(CMAKE_COMMAND) -P CMakeFiles/smtparser.dir/cmake_clean.cmake
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/clean

mistral/smtparser/CMakeFiles/smtparser.dir/depend: mistral/smtparser/smtparser.cpp
mistral/smtparser/CMakeFiles/smtparser.dir/depend: mistral/smtparser/smtparser.hpp
mistral/smtparser/CMakeFiles/smtparser.dir/depend: mistral/smtparser/smtlexer.cpp
	cd /home/lijiaying/Research/GitHub/compass/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lijiaying/Research/GitHub/compass /home/lijiaying/Research/GitHub/compass/mistral/smtparser /home/lijiaying/Research/GitHub/compass/build /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser /home/lijiaying/Research/GitHub/compass/build/mistral/smtparser/CMakeFiles/smtparser.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : mistral/smtparser/CMakeFiles/smtparser.dir/depend

