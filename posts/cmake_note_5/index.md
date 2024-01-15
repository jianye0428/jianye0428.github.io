# CMake 笔记 | [5] 构建类型(Debug、Release以及其他)


## 一、基本概念构建类型

`CMake`可以识别的构建类型是：
  - Debug：用于在没有优化的情况下，使用带有调试符号构建库或者可执行文件
  - Release: 用于构建的优化的库或者可执行文件，不包含调试符号
  - RelWithDebInfo：用于构建较少的优化库或者可执行文件，包含调试符号
  - MinSizeRel：用于不增加目标代码大小的优化方式，来构建库或者可执行文件

控制生成构建系统使用的配置变量是`CMAKE_BUILD_TYPE`，该变量默认为空。

这里我们仍然选择[CMake第三篇—动态库和静态库的补充中的代码](https://jianye0428.github.io/posts/cmake_note_3/)，但是这里我们对CMakeLists.txt稍作修改。

## 二、项目结构

```shell
.
├── cmake
│   └── message_config.cmake.in
├── message-module
│   ├── CMakeLists.txt
│   ├── include
│   │   ├── message_export_lib.h
│   │   └── message.h
│   └── src
│       └── message.cpp
├── CMakeLists.txt
└── hello_world.cpp

4 directories, 9 files
```

项目源码: https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_5/message_module_lib_standard

### 2.1 message-module模块下的CMakeLists.txt

```CMake

file(GLOB SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

if (BUILD_SHARED_LIBS)
    add_library(test_message SHARED ${SOURCE_FILE})
    target_compile_definitions(test_message PUBLIC -DMESSAGE_LIB_SHARED_BUILD)
    target_compile_definitions(test_message PRIVATE -DMESSAGE_LIB_EXPORTS)
else()
    add_library(test_message STATIC ${SOURCE_FILE})
endif()

# 添加别名，以便库可以在构建树中使用，例如在测试时
add_library(test_message::test_message ALIAS test_message)

target_include_directories(test_message
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    $<INSTALL_INTERFACE:include>
)

set_target_properties(test_message PROPERTIES
    CXX_STANDARD 11
    CMAKE_CXX_STANDARD_REQUIRED True
)

set_target_properties(test_message PROPERTIES
    DEBUG_POSTFIX "_debug"
    RELEASE_POSTFIX "_release"
)

install(TARGETS test_message
        EXPORT message_export_target
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION "include"
        FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION "include"
        FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT message_export_target
        FILE message_lib.cmake
        DESTINATION lib/cmake/test_message
)


include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/message_config.cmake.in
    "${CMAKE_SOURCE_DIR}/cmake/message_config.cmake"
    INSTALL_DESTINATION "lib/cmake/test_message"
)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_SOURCE_DIR}/cmake/message_config_version.cmake"
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/message_config.cmake
    ${CMAKE_SOURCE_DIR}/cmake/message_config_version.cmake
    DESTINATION lib/cmake/test_message
)

export(EXPORT message_export_target FILE ${CMAKE_SOURCE_DIR}/cmake/message_config_version.cmake)
```

代码释义:
这里，我们基本上没有做修改，只添加了以下内容：
{{<admonition quote "" false>}}
```CMake
set_target_properties(test_message PROPERTIES
    DEBUG_POSTFIX "_debug"
    RELEASE_POSTFIX "_release"
)
```
这将在 `debug` 构建模式下将库名后缀设置为 `_debug`，在 `release` 构建模式下将库名后缀设置为 `_release`。
然后，可以使用 `test_message_debug` 或 `test_message_release` 来引用库。当然，我们其实可以使用`find_package`，然后直接使用库的别名`test_message`即可。
{{</admonition>}}

### 2.2 根目录下的CMakeLists.txt

```CMake

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" OFF)

# Set install direcotory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type:                                       ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags, Debug configuration:                     ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "C flags, Release configuration:                   ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "C flags, Release configuration with Debug info:   ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
message(STATUS "C flags, minimal Release configuration:           ${CMAKE_C_FLAGS_MINSIZEREL}")
message(STATUS "C++ flags, Debug configuration:                   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags, Release configuration:                 ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "C++ flags, Release configuration with Debug info: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "C++ flags, minimal Release configuration:         ${CMAKE_CXX_FLAGS_MINSIZEREL}")

add_subdirectory(${CMAKE_SOURCE_DIR}/message-module)

include_directories(
    ${CMAKE_SOURCE_DIR}/message-module/include
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    test_message
)
```

这里，我们添加了以下内容：

{{<admonition quote "" false>}}
```CMake
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type:                                       ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags, Debug configuration:                     ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "C flags, Release configuration:                   ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "C flags, Release configuration with Debug info:   ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
message(STATUS "C flags, minimal Release configuration:           ${CMAKE_C_FLAGS_MINSIZEREL}")
message(STATUS "C++ flags, Debug configuration:                   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags, Release configuration:                 ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "C++ flags, Release configuration with Debug info: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "C++ flags, minimal Release configuration:         ${CMAKE_CXX_FLAGS_MINSIZEREL}")
```
首先，检查是否设置了**构建类型**。如果没有，则将构建类型设置为 `Release`。然后，它使用 `message` 命令输出各种编译标志，包括 `C` 和 `C++` 的 `Debug` 和 `Release` 配置，以及最小 `Release` 配置和<font color=red>带有调试信息的 `Release`</font>配置。这些信息对于调试和优化构建非常有用。
{{</admonition>}}

### 2.3 验证输出

我们执行以下命令，这里没有指定构建类型，即默认构建类型为`Release`。

```shell
mkdir build
cd build
cmake ..
```

输出:

```shell
-- The CXX compiler identification is GNU 7.5.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Build type:                                       Release
-- C flags, Debug configuration:
-- C flags, Release configuration:
-- C flags, Release configuration with Debug info:
-- C flags, minimal Release configuration:
-- C++ flags, Debug configuration:                   -g
-- C++ flags, Release configuration:                 -O3 -DNDEBUG
-- C++ flags, Release configuration with Debug info: -O2 -g -DNDEBUG
-- C++ flags, minimal Release configuration:         -Os -DNDEBUG
-- Configuring done
-- Generating done
-- Build files have been written to: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/build
```
执行`make`:

```shell
make
```

```shell
[ 25%] Building CXX object message_module/CMakeFiles/test_message.dir/src/message.cpp.o
[ 50%] Linking CXX static library ../lib/libtest_message_release.a
[ 50%] Built target test_message
[ 75%] Building CXX object CMakeFiles/hello-world.dir/hello_world.cpp.o
[100%] Linking CXX executable bin/hello-world
[100%] Built target hello-world
```

可以看到，我们执行`make`命令后，生成的库命名为`libtest_message_release.a`,并将其存入lib目录中。

执行:

```shell
make install
```

输出结果如下:

```shell
[ 50%] Built target test_message
[100%] Built target hello-world
Install the project...
-- Install configuration: "Release"
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/lib/libtest_message_release.a
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/include
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/include/message_export_lib.h
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/include/message.h
-- Up-to-date: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/include
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/lib/cmake/test_message/message_lib.cmake
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/lib/cmake/test_message/message_lib-release.cmake
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/lib/cmake/test_message/message_config.cmake
-- Installing: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/12/output/lib/cmake/test_message/message_config_version.cmake
```

设置构建类型为debug:



```shell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

则输出:

```shell
-- The CXX compiler identification is GNU 7.5.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Build type:                                       Debug
-- C flags, Debug configuration:
-- C flags, Release configuration:
-- C flags, Release configuration with Debug info:
-- C flags, minimal Release configuration:
-- C++ flags, Debug configuration:                   -g
-- C++ flags, Release configuration:                 -O3 -DNDEBUG
-- C++ flags, Release configuration with Debug info: -O2 -g -DNDEBUG
-- C++ flags, minimal Release configuration:         -Os -DNDEBUG
-- Configuring done
-- Generating done
-- Build files have been written to: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/build
```

执行`make`:

```shell
make
```

输出如下:

```shell
[ 25%] Building CXX object message_module/CMakeFiles/test_message.dir/src/message.cpp.o
[ 50%] Linking CXX static library ../lib/libtest_message_debug.a
[ 50%] Built target test_message
[ 75%] Building CXX object CMakeFiles/hello-world.dir/hello_world.cpp.o
[100%] Linking CXX executable bin/hello-world
[100%] Built target hello-world
```
可以看到，我们执行`make`命令后，生成的库命名为`libtest_message_debug.a`,并将其存入lib目录中。


```shell
make install
```

```shell
Consolidate compiler generated dependencies of target test_message
[ 50%] Built target test_message
Consolidate compiler generated dependencies of target hello-world
[100%] Built target hello-world
Install the project...
-- Install configuration: "Debug"
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/lib/libtest_message_debug.a
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/include
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/include/message_export_lib.h
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/include/message.h
-- Up-to-date: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/include
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/lib/cmake/test_message/message_lib.cmake
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/lib/cmake/test_message/message_lib-debug.cmake
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/lib/cmake/test_message/message_config.cmake
-- Installing: /home/yejian/yejian_personal/c++_playground/CMake_Learning_Notes/Note_5/message_module_lib_standard/output/lib/cmake/test_message/message_config_version.cmake
```

## 三、补充

`Release`和`Debug`在构建项目通常很有用，如评估编译器优化级别的效果。对于单配置生成器，如Unix Makefile、MSYS Makefile或者Ninja，因为要对项目重新配置，这里需要运行CMake两次。

不过，CMake也支持**符合配置生成器**。这些通常是集成开发环境提供的项目文件，最显著的是Visual Studio和XCode，它们可以同时处理多个配置。可以使用CMAKE_CONFIGURATION_TYPES变量对这些生成器的可用配置进行调整。

```shell
mkdir -p build
 cd build
 cmake .. -G"Visual Studio 14 2019 Win64" -D CMAKE_CONFIGURATION_TYPES="Release;Debug"
```

将为Release和Debug配置生成一个构建树。然后，您可以使–config标志来决定构建这两个中的哪一个:

```shell
cmake --build . --config Release
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_5/  

