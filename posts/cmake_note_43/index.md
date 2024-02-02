# CMake 笔记 | [43] 输出库


## 一、导言

{{<admonition quote "" false>}}
**前面的笔记中（[CMake：静态库和动态库的详解(Linux/Windows)](https://mp.weixin.qq.com/s?__biz=MzkxMzI5Mjk4Mg==&mid=2247484287&idx=1&sn=df3b9fb1c0e4c2497b114cdebb6ac2bf&scene=21#wechat_redirect)），展示了如何输出动态库和静态库，但是存在一些问题，如只输出了其头文件、符号表和库文件，但实际我们希望，当其他人编译并安装了我们的库，库就能更容易找到，如配置完环境变换或者指定库的路径。这篇笔记将展示CMake如何让我们导出目标，以便其他使用CMake的项目可以轻松地获取它们。**
{{</admonition>}}

## 二、项目结构

```shell
.
├── cmake
│   └── messageConfig.cmake.in
├── CMakeLists.txt
├── src
│   ├── CMakeLists.txt
│   ├── hello_world.cpp
│   ├── message.cpp
│   └── message.hpp
└── test
    └── CMakeLists.txt
```

项目地址

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter9/03



## 三、相关源码

**CMakeLists.txt**

```c++
# CMake 3.6 needed for IMPORTED_TARGET option
# to pkg_search_module
cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(example
  LANGUAGES CXX
  VERSION 1.0.0
  )

# <<< General set up >>>

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 检查是否已经设置了安装前缀
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # 设置安装目录为项目源目录下的output文件夹
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/output/" CACHE PATH "..." FORCE)
endif()

message(STATUS "Project will be installed to ${CMAKE_INSTALL_PREFIX}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKEDIR CMake)
else()
  set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")

# Report to user
foreach(p LIB BIN INCLUDE CMAKE)
  file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} _path )
  message(STATUS "Installing ${p} components to ${_path}")
  unset(_path)
endforeach()

add_subdirectory(src)

enable_testing()

add_subdirectory(test)
```

上述cmake在上一篇[cmake:导出头文件](https://mp.weixin.qq.com/s?__biz=MzkxMzI5Mjk4Mg==&mid=2247485469&idx=1&sn=24c81bc819c02b4c01367c593117276a&scene=21#wechat_redirect)相关代码详细分析过了，请移步到上一篇参考。

**cmake/messageConfig.cmake.in**

```c++
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/messageTargets.cmake")
check_required_components(
  "message-shared"
  "message-static"
  "message-hello-world_wDSO"
  "message-hello-world_wAR"
  )

# Remove dependency on UUID if on Windows
if(NOT WIN32)
  if(NOT TARGET PkgConfig::UUID)
    find_package(PkgConfig REQUIRED QUIET)
    pkg_search_module(UUID REQUIRED uuid IMPORTED_TARGET)
  endif()
endif()
```
{{<admonition quote "" false>}}

```c++
@PACKAGE_INIT@
```

占位符将使用`configure_package_config_file`命令进行替换。如果项目成功构建，那么将在`messageConfig.cmake`文件中进行替换：

```c++
get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()
```
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
include("${CMAKE_CURRENT_LIST_DIR}/messageTargets.cmake")
```
包含为目标自动生成的导出文件。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
check_required_components(
    "message-shared"
    "message-static"
    "message-hello-world_wDSO"
    "message-hello-world_wAR"
  )
```

检查静态库和动态库，以及两个`Hello, World`可执行文件是否带有`CMake`提供的`check_required_components`函数。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
if(NOT WIN32)
  if(NOT TARGET PkgConfig::UUID)
    find_package(PkgConfig REQUIRED QUIET)
    pkg_search_module(UUID REQUIRED uuid IMPORTED_TARGET)
  endif()
endif()
```

检查目标`PkgConfig::UUID`是否存在。如果没有，再次搜索`UUID`库(只在非Windows操作系统下有效)。
{{</admonition>}}

**src/CMakeLists.txt**

```c++
# Search for pkg-config and UUID
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_search_module(UUID uuid IMPORTED_TARGET)
  if(TARGET PkgConfig::UUID)
    message(STATUS "Found libuuid")
    set(UUID_FOUND TRUE)
  endif()
endif()

# <<< Build targets >>>

# SHARED library
add_library(message-shared SHARED "")

include(GenerateExportHeader)
generate_export_header(message-shared
  BASE_NAME "message"
  EXPORT_MACRO_NAME "MESSAGE_LIB_API"
  EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/message_export.h"
  STATIC_DEFINE "MESSAGE_STATIC_DEFINE"
  DEFINE_NO_DEPRECATED
  )

target_sources(message-shared
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/message.cpp
  )

target_compile_definitions(message-shared
  PUBLIC
    $<$<BOOL:${UUID_FOUND}>:HAVE_UUID>
  INTERFACE
    $<INSTALL_INTERFACE:USING_MESSAGE>
  )

target_include_directories(message-shared
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>
  )

target_link_libraries(message-shared
  PUBLIC
    $<$<BOOL:${UUID_FOUND}>:PkgConfig::UUID>
  )

set_target_properties(message-shared
  PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "message"
    DEBUG_POSTFIX "_d"
    PUBLIC_HEADER "message.hpp;${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/message_export.h"
    MACOSX_RPATH ON
  )

# STATIC library
add_library(message-static STATIC "")

target_sources(message-static
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/message.cpp
  )

target_compile_definitions(message-static
  PUBLIC
    message_STATIC_DEFINE
    $<$<BOOL:${UUID_FOUND}>:HAVE_UUID>
  INTERFACE
    $<INSTALL_INTERFACE:USING_MESSAGE>
  )

target_include_directories(message-static
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>
  )

target_link_libraries(message-static
  PUBLIC
    $<$<BOOL:${UUID_FOUND}>:PkgConfig::UUID>
  )

set_target_properties(message-static
  PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    ARCHIVE_OUTPUT_NAME "message"
    DEBUG_POSTFIX "_sd"
    RELEASE_POSTFIX "_s"
    PUBLIC_HEADER "message.hpp;${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/message_export.h"
  )

# EXECUTABLES
add_executable(hello-world_wDSO hello_world.cpp)

target_link_libraries(hello-world_wDSO
  PUBLIC
    message-shared
  )

add_executable(hello-world_wAR hello_world.cpp)

target_link_libraries(hello-world_wAR
  PUBLIC
    message-static
  )

# <<< Install and export targets >>>

install(
  TARGETS
    message-shared
    message-static
    hello-world_wDSO
    hello-world_wAR
  EXPORT
    messageTargets
  ARCHIVE
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  RUNTIME
    DESTINATION ${INSTALL_BINDIR}
    COMPONENT bin
  LIBRARY
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  PUBLIC_HEADER
    DESTINATION ${INSTALL_INCLUDEDIR}/message
    COMPONENT dev
  )

install(
  EXPORT
    messageTargets
  NAMESPACE
    "message::"
  DESTINATION
    ${INSTALL_CMAKEDIR}
  COMPONENT
    dev
  )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/messageConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  )

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/messageConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/messageConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
  )

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/messageConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/messageConfigVersion.cmake
  DESTINATION
    ${INSTALL_CMAKEDIR}
  )
```
{{<admonition quote "" false>}}
```c++
install(
  EXPORT
    messageTargets
  NAMESPACE
    "message::"
  DESTINATION
    ${INSTALL_CMAKEDIR}
  COMPONENT
    dev
  )
```

- **`EXPORT messageTargets`**:
  - 这里指定要安装的导出集名称。`messageTargets` 是一个之前定义的导出集，其中包含了项目中的一些目标（例如库或可执行文件）。
  - 这些目标是在 CMake 构建过程中定义并构建的，`EXPORT` 关键字用于指明这些目标在安装时应当被导出为一组。本篇是将动态库、静态库以及对应的头文件打包成了一组（messageTargets）
- **`NAMESPACE "message::"`**:
  - 这个命令为导出的目标设置了一个命名空间前缀。在这种情况下，任何导出的目标都将被前缀 `message::` 所标记。
  - 如，如果有一个名为 `message-shared` 的库，在其他项目中通过 `find_package` 查找和链接时，可以使用 `message::message-shared` 来引用它。
- **`DESTINATION ${INSTALL_CMAKEDIR}`**:
  - 指定导出的目标（`messageTargets`）应当被安装到的目标文件夹。
  - `${INSTALL_CMAKEDIR}` 是一个变量，它在主 `CMakeLists.txt`中定义了。
- **`COMPONENT dev`**:
  - `COMPONENT` 关键字用于指定这部分安装属于哪个组件。在这里，组件被命名为 `dev`。
  - 在 CMake 中，可以将安装分成多个组件，例如库文件、头文件、文档等，以便在安装时可以选择性地安装特定的组件。`dev` 组件可能包含开发相关的文件，比如 CMake 配置和库文件。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/messageConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  )

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/messageConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/messageConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
  )

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/messageConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/messageConfigVersion.cmake
  DESTINATION
    ${INSTALL_CMAKEDIR}
  )
```

- **`include(CMakePackageConfigHelpers)`**:
  - 命令包含了 CMake 模块 `CMakePackageConfigHelpers`，该模块提供了一些辅助宏和函数，用于生成和配置包配置文件。
  - 这些辅助工具简化了创建兼容不同项目的配置文件的过程。
- **`write_basic_package_version_file`**:
  - 生成一个包版本文件，该文件包含了关于包版本和兼容性的信息。
  - `${CMAKE_CURRENT_BINARY_DIR}/messageConfigVersion.cmake` 指定生成文件的路径和名称。
  - `VERSION ${PROJECT_VERSION}` 指定了包的当前版本，`${PROJECT_VERSION}` 是在 CMake 中定义的项目版本变量。
  - `COMPATIBILITY SameMajorVersion` 指定了版本兼容性规则。在本篇，它表明包是与同一主要版本号的其他版本兼容的。
- **`configure_package_config_file`**:
  - 命令用于从一个 `.in` 模板文件生成一个包配置文件。
  - `${PROJECT_SOURCE_DIR}/cmake/messageConfig.cmake.in` 是模板文件的路径，这个文件通常包含了配置信息，如库的位置、依赖项等。
  - `${CMAKE_CURRENT_BINARY_DIR}/messageConfig.cmake` 是输出文件的路径和名称。
  - `INSTALL_DESTINATION ${INSTALL_CMAKEDIR}` 指定了安装时配置文件的目标目录。
- **`install` 命令**:
  - 这个 `install` 命令用于安装前面生成的配置文件。
  - `FILES` 关键字后面列出了要安装的文件：生成的 `messageConfig.cmake` 和 `messageConfigVersion.cmake` 文件。
  - `DESTINATION ${INSTALL_CMAKEDIR}` 指定了这些文件的安装目录。
{{</admonition>}}


**src/message.hpp**

```c++
#pragma once

#include <iosfwd>
#include <string>
#include "message_export.h"
class MESSAGE_LIB_API Message {
public:
  Message(const std::string &m) : message_(m) {}

  friend std::ostream &operator<<(std::ostream &os, Message &obj) {
    return obj.PrintObject(os);
  }

private:
  std::string message_;
  std::ostream &PrintObject(std::ostream &os);
};

std::string GetUUID();
```

**src/message.cpp**

```c++
#include "message.hpp"

#include <iostream>
#include <string>

#ifdef HAVE_UUID
#include <uuid/uuid.h>
#endif

std::ostream &Message::PrintObject(std::ostream &os) {
  os << "This is my very nice message: " << std::endl;
  os << message_ << std::endl;
  os << "...and here is its UUID: " << GetUUID();

  return os;
}

#ifdef HAVE_UUID
std::string GetUUID() {
  uuid_t uuid;
  uuid_generate(uuid);
  char uuid_str[37];
  uuid_unparse_lower(uuid, uuid_str);
  uuid_clear(uuid);
  std::string uuid_cxx(uuid_str);
  return uuid_cxx;
}
#else
std::string GetUUID() { return "Ooooops, no UUID for you!"; }
#endif
```

**src/hello_world.cpp**

```c++
#include <cstdlib>
#include <iostream>

#include "message.hpp"

int main() {
  Message say_hello("Hello, CMake World!");

  std::cout << say_hello << std::endl;

  Message say_goodbye("Goodbye, CMake World");

  std::cout << say_goodbye << std::endl;

  return EXIT_SUCCESS;
}
```

**test/CMakeLists.txt**

```c++
add_test(
  NAME test_shared
  COMMAND $<TARGET_FILE:hello-world_wDSO>
  )

add_test(
  NAME test_static
  COMMAND $<TARGET_FILE:hello-world_wAR>
  )
```



## 四、结果展示

```shell
$ mkdir -p build
$ cd build
$ cmake ..
$ cmake --build . --target install
```

安装树结构：

```shell
output
   ├── bin
   │   ├── hello-world_wAR
   │   └── hello-world_wDSO
   ├── include
   │   └── message
   │       ├── message_export.h
   │       └── message.hpp
   ├── lib
   │   ├── libmessage_s.a
   │   ├── libmessage.so -> libmessage.so.1
   │   └── libmessage.so.1
   └── share
       └── cmake
           └── example
               ├── messageConfig.cmake
               ├── messageConfigVersion.cmake
               ├── messageTargets.cmake
               └── messageTargets-release.cmake
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_43/  

