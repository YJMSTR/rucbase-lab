# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_SOURCE_DIR = /home/yjmstr/rucbase-lab

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yjmstr/rucbase-lab/build

# Include any dependencies generated for this target.
include src/storage/CMakeFiles/disk_manager_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/storage/CMakeFiles/disk_manager_test.dir/compiler_depend.make

# Include the progress variables for this target.
include src/storage/CMakeFiles/disk_manager_test.dir/progress.make

# Include the compile flags for this target's objects.
include src/storage/CMakeFiles/disk_manager_test.dir/flags.make

src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o: src/storage/CMakeFiles/disk_manager_test.dir/flags.make
src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o: ../src/storage/disk_manager_test.cpp
src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o: src/storage/CMakeFiles/disk_manager_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yjmstr/rucbase-lab/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o"
	cd /home/yjmstr/rucbase-lab/build/src/storage && /usr/lib/ccache/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o -MF CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o.d -o CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o -c /home/yjmstr/rucbase-lab/src/storage/disk_manager_test.cpp

src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.i"
	cd /home/yjmstr/rucbase-lab/build/src/storage && /usr/lib/ccache/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yjmstr/rucbase-lab/src/storage/disk_manager_test.cpp > CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.i

src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.s"
	cd /home/yjmstr/rucbase-lab/build/src/storage && /usr/lib/ccache/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yjmstr/rucbase-lab/src/storage/disk_manager_test.cpp -o CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.s

# Object files for target disk_manager_test
disk_manager_test_OBJECTS = \
"CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o"

# External object files for target disk_manager_test
disk_manager_test_EXTERNAL_OBJECTS =

bin/disk_manager_test: src/storage/CMakeFiles/disk_manager_test.dir/disk_manager_test.cpp.o
bin/disk_manager_test: src/storage/CMakeFiles/disk_manager_test.dir/build.make
bin/disk_manager_test: lib/libdisk.a
bin/disk_manager_test: lib/libgtest_main.a
bin/disk_manager_test: lib/libgtest.a
bin/disk_manager_test: src/storage/CMakeFiles/disk_manager_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/yjmstr/rucbase-lab/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/disk_manager_test"
	cd /home/yjmstr/rucbase-lab/build/src/storage && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/disk_manager_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/storage/CMakeFiles/disk_manager_test.dir/build: bin/disk_manager_test
.PHONY : src/storage/CMakeFiles/disk_manager_test.dir/build

src/storage/CMakeFiles/disk_manager_test.dir/clean:
	cd /home/yjmstr/rucbase-lab/build/src/storage && $(CMAKE_COMMAND) -P CMakeFiles/disk_manager_test.dir/cmake_clean.cmake
.PHONY : src/storage/CMakeFiles/disk_manager_test.dir/clean

src/storage/CMakeFiles/disk_manager_test.dir/depend:
	cd /home/yjmstr/rucbase-lab/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yjmstr/rucbase-lab /home/yjmstr/rucbase-lab/src/storage /home/yjmstr/rucbase-lab/build /home/yjmstr/rucbase-lab/build/src/storage /home/yjmstr/rucbase-lab/build/src/storage/CMakeFiles/disk_manager_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/storage/CMakeFiles/disk_manager_test.dir/depend

