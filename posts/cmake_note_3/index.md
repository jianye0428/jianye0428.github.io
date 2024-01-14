# CMake 笔记 | [3] 静态库和动态的补充


## 一、导言

对CMake在不同平台上构建动态库和静态库做进一步的探索，即如何利用一个比较统一的模板在不同的平台构建静态库和动态库，以及对add_library命令的其他参数做进一步的探索。

## 二、库模板

### 2.1 项目结构

```shell
.
├── cmake
│  └── message_config.cmake.in
├── message-module
│   ├── include
│   │  ├── message_export_lib.h
│   │  └── message.h
│   ├── src
│   │   └── message.cpp
│   └── CMakeLists.txt
├── hello_world.cpp
└── CMakeLists.txt
```
本项目的结构相对比较复杂，为了能够生成一套比较标准的库（静态库/动态库），所以项目中会包含很多配置项，接下来我们会对项目中的所有内容进行一一讲解。

### 2.2
根目录下的CMakeLists.txt

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

{{<admonition quote "Tip" false>}}
```CMake
option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" OFF)
```
首先，我们提供了在执行`cmake`命令时的参数选项，默认默认情况下`BUILD_SHARED_LIBS`的状态是关闭的(OFF)，即默认情况下我们默认构建的是静态库。

当我们要构建动态库时，我们需执行以下命令（假设你已经在项目中构建了build文件夹,并且你现在在build目录中）
```shell
cmake .. -DBUILD_SHARED_LIBS=ON
```
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```CMake
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
endif()
```
如果我们没有指定在`make install`后的输出路径，则`CMAKE_INSTALL_PREFIX` 将设置为`${CMAKE_SOURCE_DIR}/output/`，即当前项目下的`output`目录。
{{</admonition>}}

### 2.3 cmake目录下的message_config.cmake.in

```CMake
@PACKAGE_INIT@

include ( "${CMAKE_CURRENT_LIST_DIR}/message_config.cmake" )
```

为了能够生成一个标准的库，我们写了一个`message_config.cmake.in`文件，该文件执行`make install`时将被调用，调用时我们再进行讲解。

### 2.4 message-module下message_export_lib.h

```c++
#ifndef MESSAGE_EXPORT_LIB_H_
#define MESSAGE_EXPORT_LIB_H_

#ifdef MESSAGE_LIB_SHARED_BUILD
    #ifdef _WIN32
        #ifdef MESSAGE_LIB_EXPORTS
            #define MESSAGE_LIB_API __declspec(dllexport)
        #else
            #define MESSAGE_LIB_API __declspec(dllimport)
        #endif  // MESSAGE_LIB_EXPORTS
    #else
        #define MESSAGE_LIB_API
    #endif  // _WIN32
#else
    #define MESSAGE_LIB_API
#endif  // MESSAGE_LIB_SHARED_BUILD

#endif // ! MESSAGE_EXPORT_LIB_H_
```

首先，如果我们定义了宏定义<font color=red>`MESSAGE_LIB_SHARED_BUILD`</font>(即我们要构建动态库时)，我们将执行以下命令：如果是`Windows`平台，并且是在生成动态库时，即定义了(MESSAGE_LIB_EXPORTS)，将`__declspec(dllexport)`定义为`MESSAGE_LIB_API`；如果是使用动态库时，即未定义(MESSAGE_LIB_EXPORTS)，则将`__declspec(dllimport)`定义为`MESSAGE_LIB_API`。如果是非Windows平台，则对宏MESSAGE_LIB_API不做任何操作(因为在非Windows平台上可以动态库的生成和使用与静态库是一样的)。

然后，如果我们未定义宏定义MESSAGE_LIB_SHARED_BUILD(即我们要构建静态库时)，我们对宏MESSAGE_LIB_API不做任何操作。

注意：关于宏定义MESSAGE_LIB_SHARED_BUILD和MESSAGE_LIB_EXPORTS是否要添加利用message-module下的CMakeLists.txt进行配置。

### 2.5 message-module下message.h

```c++
#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_

#include <iostream>
#include <string>

#include "message_export_lib.h"
class MESSAGE_LIB_API Message {
 public:
  Message() {}

  void Print(const std::string &message);
};
#endif
```

我们将头文件`message_export_lib.h`包含进来，来控制在不同平台生成不同的库时的选项。

### 2.6 message-module下的CMakeLists.txt

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

如果我们让宏`BUILD_SHARED_LIBS`为开启状态(即我们要构建动态库)，我们要将宏定义`MESSAGE_LIB_SHARED_BUILD`和`MESSAGE_LIB_EXPORTS`添加到编译器中，这样我们的头文件`message_export_lib.h`中便知道我们要构建**动态库**。

否则(即我们没有让BUILD_SHARED_LIBS开启)，我们将构建**静态库**。

利用`CMakePackageConfigHelpers`模块生成关于`test_message`库的cmake的配置文件，且可以使用命令`find_package`命令找到库`test_message`库。关于以上命令的具体参数使用，我们将在具体的命令学习章节进行具体的讲解和学习。

## 三、add_library其他参数的一些探索

### 3.1 生成对象库

```CMake
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(recipe-03 LANGUAGES CXX)

add_library(message-objs
    OBJECT
        Message.hpp
        Message.cpp
    )
# this is only needed for older compilers
# but doesn't hurt either to have it
set_target_properties(message-objs
    PROPERTIES
        POSITION_INDEPENDENT_CODE 1
    )

add_library(message-shared
    SHARED
        $<TARGET_OBJECTS:message-objs>
    )

add_library(message-static
    STATIC
        $<TARGET_OBJECTS:message-objs>
    )

add_executable(hello-world hello-world.cpp)

target_link_libraries(hello-world message-static)
```

为了保证编译的目标文件与生成位置无关，可以通过使用set_target_properties命令，设置message-objs目标的相应属性来实现。

**注意:**可能在某些平台和/或使用较老的编译器上，需要显式地为目标设置`POSITION_INDEPENDENT_CODE`属性。

现在，可以使用这个对象库来获取静态库(message-static)和动态库(message-shared)。要注意引用对象库的生成器表达式语法:$<TARGET_OBJECTS:message-objs>。生成器表达式是CMake在生成时(即配置之后)构造，用于生成特定于配置的构建输出。

### 3.2 将静态库和动态库同时命名为同名的两个库

```CMake

add_library(message-shared
  SHARED
    $<TARGET_OBJECTS:message-objs>
    )
set_target_properties(message-shared
    PROPERTIES
        OUTPUT_NAME "message"
    )
add_library(message-static
    STATIC
        $<TARGET_OBJECTS:message-objs>
    )
set_target_properties(message-static
    PROPERTIES
        OUTPUT_NAME "message"
    )
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_3/  

