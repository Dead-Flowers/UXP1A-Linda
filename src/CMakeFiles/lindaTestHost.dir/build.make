# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/daniell/dev/UXP1A-Linda

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/daniell/dev/UXP1A-Linda

# Include any dependencies generated for this target.
include src/CMakeFiles/lindaTestHost.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/lindaTestHost.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/lindaTestHost.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/lindaTestHost.dir/flags.make

src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o: src/CMakeFiles/lindaTestHost.dir/flags.make
src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o: src/test-host/main.cpp
src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o: src/CMakeFiles/lindaTestHost.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/daniell/dev/UXP1A-Linda/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o"
	cd /home/daniell/dev/UXP1A-Linda/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o -MF CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o.d -o CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o -c /home/daniell/dev/UXP1A-Linda/src/test-host/main.cpp

src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lindaTestHost.dir/test-host/main.cpp.i"
	cd /home/daniell/dev/UXP1A-Linda/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/daniell/dev/UXP1A-Linda/src/test-host/main.cpp > CMakeFiles/lindaTestHost.dir/test-host/main.cpp.i

src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lindaTestHost.dir/test-host/main.cpp.s"
	cd /home/daniell/dev/UXP1A-Linda/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/daniell/dev/UXP1A-Linda/src/test-host/main.cpp -o CMakeFiles/lindaTestHost.dir/test-host/main.cpp.s

# Object files for target lindaTestHost
lindaTestHost_OBJECTS = \
"CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o"

# External object files for target lindaTestHost
lindaTestHost_EXTERNAL_OBJECTS =

src/lindaTestHost: src/CMakeFiles/lindaTestHost.dir/test-host/main.cpp.o
src/lindaTestHost: src/CMakeFiles/lindaTestHost.dir/build.make
src/lindaTestHost: lib/linda/liblinda.a
src/lindaTestHost: src/CMakeFiles/lindaTestHost.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/daniell/dev/UXP1A-Linda/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lindaTestHost"
	cd /home/daniell/dev/UXP1A-Linda/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lindaTestHost.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/lindaTestHost.dir/build: src/lindaTestHost
.PHONY : src/CMakeFiles/lindaTestHost.dir/build

src/CMakeFiles/lindaTestHost.dir/clean:
	cd /home/daniell/dev/UXP1A-Linda/src && $(CMAKE_COMMAND) -P CMakeFiles/lindaTestHost.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/lindaTestHost.dir/clean

src/CMakeFiles/lindaTestHost.dir/depend:
	cd /home/daniell/dev/UXP1A-Linda && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/daniell/dev/UXP1A-Linda /home/daniell/dev/UXP1A-Linda/src /home/daniell/dev/UXP1A-Linda /home/daniell/dev/UXP1A-Linda/src /home/daniell/dev/UXP1A-Linda/src/CMakeFiles/lindaTestHost.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/lindaTestHost.dir/depend

