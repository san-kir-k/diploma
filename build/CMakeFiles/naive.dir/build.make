# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.21.3_1/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.21.3_1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/ak/Desktop/diploma/diploma

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/ak/Desktop/diploma/diploma/build

# Include any dependencies generated for this target.
include CMakeFiles/naive.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/naive.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/naive.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/naive.dir/flags.make

CMakeFiles/naive.dir/src/main.cpp.o: CMakeFiles/naive.dir/flags.make
CMakeFiles/naive.dir/src/main.cpp.o: ../src/main.cpp
CMakeFiles/naive.dir/src/main.cpp.o: CMakeFiles/naive.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/ak/Desktop/diploma/diploma/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/naive.dir/src/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/naive.dir/src/main.cpp.o -MF CMakeFiles/naive.dir/src/main.cpp.o.d -o CMakeFiles/naive.dir/src/main.cpp.o -c /Users/ak/Desktop/diploma/diploma/src/main.cpp

CMakeFiles/naive.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/naive.dir/src/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/ak/Desktop/diploma/diploma/src/main.cpp > CMakeFiles/naive.dir/src/main.cpp.i

CMakeFiles/naive.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/naive.dir/src/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/ak/Desktop/diploma/diploma/src/main.cpp -o CMakeFiles/naive.dir/src/main.cpp.s

# Object files for target naive
naive_OBJECTS = \
"CMakeFiles/naive.dir/src/main.cpp.o"

# External object files for target naive
naive_EXTERNAL_OBJECTS =

naive: CMakeFiles/naive.dir/src/main.cpp.o
naive: CMakeFiles/naive.dir/build.make
naive: CMakeFiles/naive.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/ak/Desktop/diploma/diploma/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable naive"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/naive.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/naive.dir/build: naive
.PHONY : CMakeFiles/naive.dir/build

CMakeFiles/naive.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/naive.dir/cmake_clean.cmake
.PHONY : CMakeFiles/naive.dir/clean

CMakeFiles/naive.dir/depend:
	cd /Users/ak/Desktop/diploma/diploma/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/ak/Desktop/diploma/diploma /Users/ak/Desktop/diploma/diploma /Users/ak/Desktop/diploma/diploma/build /Users/ak/Desktop/diploma/diploma/build /Users/ak/Desktop/diploma/diploma/build/CMakeFiles/naive.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/naive.dir/depend

