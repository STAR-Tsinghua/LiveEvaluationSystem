# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.14.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.14.4/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build

# Include any dependencies generated for this target.
include crypto/test/CMakeFiles/test_support_lib.dir/depend.make

# Include the progress variables for this target.
include crypto/test/CMakeFiles/test_support_lib.dir/progress.make

# Include the compile flags for this target's objects.
include crypto/test/CMakeFiles/test_support_lib.dir/flags.make

crypto/test/CMakeFiles/test_support_lib.dir/abi_test.cc.o: crypto/test/CMakeFiles/test_support_lib.dir/flags.make
crypto/test/CMakeFiles/test_support_lib.dir/abi_test.cc.o: /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/abi_test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object crypto/test/CMakeFiles/test_support_lib.dir/abi_test.cc.o"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_support_lib.dir/abi_test.cc.o -c /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/abi_test.cc

crypto/test/CMakeFiles/test_support_lib.dir/abi_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_support_lib.dir/abi_test.cc.i"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/abi_test.cc > CMakeFiles/test_support_lib.dir/abi_test.cc.i

crypto/test/CMakeFiles/test_support_lib.dir/abi_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_support_lib.dir/abi_test.cc.s"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/abi_test.cc -o CMakeFiles/test_support_lib.dir/abi_test.cc.s

crypto/test/CMakeFiles/test_support_lib.dir/file_test.cc.o: crypto/test/CMakeFiles/test_support_lib.dir/flags.make
crypto/test/CMakeFiles/test_support_lib.dir/file_test.cc.o: /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/file_test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object crypto/test/CMakeFiles/test_support_lib.dir/file_test.cc.o"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_support_lib.dir/file_test.cc.o -c /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/file_test.cc

crypto/test/CMakeFiles/test_support_lib.dir/file_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_support_lib.dir/file_test.cc.i"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/file_test.cc > CMakeFiles/test_support_lib.dir/file_test.cc.i

crypto/test/CMakeFiles/test_support_lib.dir/file_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_support_lib.dir/file_test.cc.s"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/file_test.cc -o CMakeFiles/test_support_lib.dir/file_test.cc.s

crypto/test/CMakeFiles/test_support_lib.dir/malloc.cc.o: crypto/test/CMakeFiles/test_support_lib.dir/flags.make
crypto/test/CMakeFiles/test_support_lib.dir/malloc.cc.o: /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/malloc.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object crypto/test/CMakeFiles/test_support_lib.dir/malloc.cc.o"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_support_lib.dir/malloc.cc.o -c /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/malloc.cc

crypto/test/CMakeFiles/test_support_lib.dir/malloc.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_support_lib.dir/malloc.cc.i"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/malloc.cc > CMakeFiles/test_support_lib.dir/malloc.cc.i

crypto/test/CMakeFiles/test_support_lib.dir/malloc.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_support_lib.dir/malloc.cc.s"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/malloc.cc -o CMakeFiles/test_support_lib.dir/malloc.cc.s

crypto/test/CMakeFiles/test_support_lib.dir/test_util.cc.o: crypto/test/CMakeFiles/test_support_lib.dir/flags.make
crypto/test/CMakeFiles/test_support_lib.dir/test_util.cc.o: /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/test_util.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object crypto/test/CMakeFiles/test_support_lib.dir/test_util.cc.o"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_support_lib.dir/test_util.cc.o -c /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/test_util.cc

crypto/test/CMakeFiles/test_support_lib.dir/test_util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_support_lib.dir/test_util.cc.i"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/test_util.cc > CMakeFiles/test_support_lib.dir/test_util.cc.i

crypto/test/CMakeFiles/test_support_lib.dir/test_util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_support_lib.dir/test_util.cc.s"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/test_util.cc -o CMakeFiles/test_support_lib.dir/test_util.cc.s

crypto/test/CMakeFiles/test_support_lib.dir/wycheproof_util.cc.o: crypto/test/CMakeFiles/test_support_lib.dir/flags.make
crypto/test/CMakeFiles/test_support_lib.dir/wycheproof_util.cc.o: /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/wycheproof_util.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object crypto/test/CMakeFiles/test_support_lib.dir/wycheproof_util.cc.o"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_support_lib.dir/wycheproof_util.cc.o -c /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/wycheproof_util.cc

crypto/test/CMakeFiles/test_support_lib.dir/wycheproof_util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_support_lib.dir/wycheproof_util.cc.i"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/wycheproof_util.cc > CMakeFiles/test_support_lib.dir/wycheproof_util.cc.i

crypto/test/CMakeFiles/test_support_lib.dir/wycheproof_util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_support_lib.dir/wycheproof_util.cc.s"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test/wycheproof_util.cc -o CMakeFiles/test_support_lib.dir/wycheproof_util.cc.s

# Object files for target test_support_lib
test_support_lib_OBJECTS = \
"CMakeFiles/test_support_lib.dir/abi_test.cc.o" \
"CMakeFiles/test_support_lib.dir/file_test.cc.o" \
"CMakeFiles/test_support_lib.dir/malloc.cc.o" \
"CMakeFiles/test_support_lib.dir/test_util.cc.o" \
"CMakeFiles/test_support_lib.dir/wycheproof_util.cc.o"

# External object files for target test_support_lib
test_support_lib_EXTERNAL_OBJECTS =

crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/abi_test.cc.o
crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/file_test.cc.o
crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/malloc.cc.o
crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/test_util.cc.o
crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/wycheproof_util.cc.o
crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/build.make
crypto/test/libtest_support_lib.a: crypto/test/CMakeFiles/test_support_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX static library libtest_support_lib.a"
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && $(CMAKE_COMMAND) -P CMakeFiles/test_support_lib.dir/cmake_clean_target.cmake
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_support_lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
crypto/test/CMakeFiles/test_support_lib.dir/build: crypto/test/libtest_support_lib.a

.PHONY : crypto/test/CMakeFiles/test_support_lib.dir/build

crypto/test/CMakeFiles/test_support_lib.dir/clean:
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test && $(CMAKE_COMMAND) -P CMakeFiles/test_support_lib.dir/cmake_clean.cmake
.PHONY : crypto/test/CMakeFiles/test_support_lib.dir/clean

crypto/test/CMakeFiles/test_support_lib.dir/depend:
	cd /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl/crypto/test /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/test/CMakeFiles/test_support_lib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : crypto/test/CMakeFiles/test_support_lib.dir/depend

