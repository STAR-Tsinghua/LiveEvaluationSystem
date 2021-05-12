# Install script for directory: /Users/lihongsheng/Desktop/About_DTP/submodule/DTP/deps/boringssl

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/crypto/cmake_install.cmake")
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/ssl/cmake_install.cmake")
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/ssl/test/cmake_install.cmake")
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/tool/cmake_install.cmake")
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/util/fipstools/cavp/cmake_install.cmake")
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/util/fipstools/acvp/modulewrapper/cmake_install.cmake")
  include("/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/decrepit/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/lihongsheng/Desktop/About_DTP/submodule/DTP/target/release/build/quiche-7e3276a7bbab95c9/out/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
