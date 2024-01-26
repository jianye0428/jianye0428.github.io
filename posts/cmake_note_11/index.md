# CMake 笔记 | [11] 检测环境


## 一、 CMake:检测环境

{{<admonition quote "导言" false>}}

**通过前面几篇的学习，我们掌握`CMake`以及与`C++`相关的基本知识。尽管`CMake`跨平台，但有时源代码并不是完全可移植。为了使得我们的源代码能够跨平台、操作系统和编译器，根据平台不同的方式配置和/或构建代码是在构建项目过程中必不可少的环节。**

{{</admonition>}}


## 二、 检测操作系统

`CMake`是一组跨平台工具。在实际的开发过程中，我们需要操作系统相关的CMake代码，会根据操作系统启用条件编译，或者在可用或必要时使用特定于编译器的扩展。

这里举个特定示例说明：

`Windows`与`Unix`系统的文件结构有明显的差异，如将深度学习模型集成于`C++`开发的软件系统中时，我们想要将深度学习模型(*.pth)拷贝到指定的文件中时：

```C++
if (MSVC)
  file(GLOB MODEL "${CMAKE_SOURCE_DIR}/resource/*.pt")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${MODEL}
                     $<TARGET_FILE_DIR:${PROJECT_NAME}>)
elseif(UNIX)
  file(GLOB MODEL "${CMAKE_SOURCE_DIR}/resource/*.pt")
  file(COPY ${MODEL} DESTINATION ${EXECUTE_FILE})
endif()
```

这段`CMake`代码用于在构建项目后，根据目标平台的不同（`Windows`或`UNIX/Linux`），将模型文件复制到输出目录中，以确保运行程序时所需的模型文件（具有`.pt`扩展名）与可执行文件位于同一目录下。

对于`MSVC`（`Microsoft Visual C++`编译器，通常用于`Windows`平台）：

  - 使用`file()`命令并设置`GLOB`选项，在`CMake`源代码目录（`${CMAKE_SOURCE_DIR}`）下的`resource`目录中查找所有`.pt`模型文件。
  - 然后使用`add_custom_command()`命令将自定义的后期构建命令添加到目标``（`${PROJECT_NAME}`）中。
  - 自定义命令会将找到的所有`.pt`模型文件复制到输出目录（`$<TARGET_FILE_DIR:${PROJECT_NAME}>`）。使用`copy_if_different`参数确保仅在目标文件与源文件不同或目标目录中不存在时才复制文件。

对于`UNIX`平台（包括`Linux`）：

  - 使用`file()`命令并设置`GLOB`选项，在`CMake`源代码目录（`\${CMAKE_SOURCE_DIR}`）下的`resource`目录中查找所有`.pt`模型文件。
  - 然后使用`file()`命令并设置`COPY`选项，将找到的所有`.pt`模型文件复制到指定的目标目录（`${EXECUTE_FILE}`）。

接下来，我们将通过一个不需要编译任何源代码的示例，演示如何使用`CMake`检测操作系统。

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter2/01

**CMakeLists.txt**

```C++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(os_test)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring on/for Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring on/for macOs")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring on/for Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    message(STATUS "Configuring on/for IBM AIX")
else()
    message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()
```

**输出结果**

```shell
-- The C compiler identification is GNU 9.4.0
-- The CXX compiler identification is GNU 9.4.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring on/for Linux
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter2/01/build
```

`CMake`为目标操作系统定义了`CMAKE_SYSTEM_NAME`，因此不需要使用定制命令、工具或脚本来查询此信息。然后，可以使用此变量的值实现特定于操作系统的条件和解决方案。

- 在`macOS`上`CMAKE_SYSTEM_NAME`为`Darwin`。
- 在`Linux和Windows`上，`CMAKE_SYSTEM_NAME`分别为`Linux`和`Windows`。我们了解了如何在特定的操作系统上执行特定的`CMake`代码。当然，应该尽量减少这种定制化行为，以便简化迁移到新平台的过程。

注意：为了最小化从一个平台转移到另一个平台时的成本，应该避免直接使用`Shell`命令，还应该避免`显式的路径分隔符`(`Linux`和`macOS`上的前斜杠和`Windows`上的后斜杠)。`CMake`代码中只使用前斜杠作为路径分隔符，`CMake`将自动将它们转换为所涉及的操作系统环境。

## 四、处理与编译器相关的源代码

为了可移植性，我们尽量避免去编写新代码，但遇到有依赖的情况我们也要去解决，特别是当使用历史代码或处理编译器依赖工具。

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter2/02

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello_os LANGUAGES CXX)

add_executable(${PROJECT_NAME} hello_os.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(${PROJECT_NAME} PUBLIC "IS_LINUX")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_definitions(${PROJECT_NAME} PUBLIC "IS_MACOS")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(${PROJECT_NAME} PUBLIC "IS_WINDOWS")
endif()
```

通过`target_compile_definitions()`命令向目标（`hello_os`）添加预定义宏`IS_LINUX`、`IS_MACOS`或者`IS_WINDOWS`，该宏在编译过程中将生效。

`target_compile_definitions`会将将定义限制于一个特定的目标，以及通过`PRIVATE`|`PUBLIC`|`INTERFACE`限定符，限制这些定义可见性:

- `PRIVATE`，编译定义将只应用于给定的目标，而不应用于相关的其他目标。
- `INTERFACE`，对给定目标的编译定义将只应用于使用它的目标。
- `PUBLIC`，编译定义将应用于给定的目标和使用它的所有其他目标。

当然，在`C++`中，可以直接使用预定义的宏来识别不同的平台和操作系统。这些预定义的宏是由编译器或操作系统提供的，可以在源代码中使用它们来编写平台相关的代码。以下是一些常用的平台识别宏：

- `__APPLE__`：在苹果（`Apple`）平台（例如 `macOS` 和 `iOS`）上定义。
- `__linux__`：在`Linux`平台上定义。
- `_WIN32`：在`32`位`Windows`操作系统上定义。
- `_WIN64`：在`64`位`Windows`操作系统上定义。
- `_MSC_VER`：在使用`Microsoft Visual C++`编译器时定义，表示编译器的版本号。
- `__GNUC__`：在使用`GNU`编译器（例如`g++`）时定义，表示编译器的版本号。

**hello_os.cpp**

```c++
#include <string>
#include <iostream>
std::string HelloOS();

int main() {
  std::cout << HelloOS() << std::endl;
  return EXIT_SUCCESS;
}

std::string HelloOS() {
#ifdef IS_WINDOWS
  return std::string("Hello from Windows!");
#elif IS_LINUX
  return std::string("Hello from Linux!");
#elif IS_MACOS
  return std::string("Hello from macOS!");
#else
  return std::string("Hello from an unknown system!");
#endif
}
```

`Windows`系统上，将看到来自`Windows`的`Hello`。其他操作系统将产生不同的输出。


## 三、检测与处理器体系结构

19世纪70年代，出现的`64位`整数运算和本世纪初出现的用于个人计算机的`64位寻址`，扩大了内存寻址范围，开发商投入了大量资源来移植为`32位`体系结构硬编码，以支持`64位寻址`。虽然，避免显式硬编码的方式非常明智，但需要在使用`CMake`配置的代码中适应硬编码限制。
项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter2/03

**CMakeLists.txt**

```C++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(arch_dependent LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/arch_dependent.cpp
)

# 检查空指针类型的大小
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(
        ${PROJECT_NAME}
        PUBLIC "IS_64_BIT_ARCH"
    )
    message(STATUS "Target is 64 bits")
else()
    target_compile_definitions(
        ${PROJECT_NAME}
        PUBLIC "IS_32_BIT_ARCH"
    )
    message(STATUS "Target is 32 bits")
endif()

# 通过定义目标编译定义，让预处理器了解主机处理器架构
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i386")
    message(STATUS "i386 architecture detected")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
    message(STATUS "i686 architecture detected")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "x86_64 architecture detected")
else()
    message(STATUS "host processor architecture is unknown")
endif()

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC "ARCHITECTURE=${CMAKE_HOST_SYSTEM_PROCESSOR}"
)
```

`CMake`定义了`CMAKE_HOST_SYSTEM_PROCESSOR`变量，以包含当前运行的处理器的名称。可以设置为`i386`、`i686`、`x86_64`、`AMD64`等等，当然，这取决于当前的`CPU`。

`CMAKE_SIZEOF_VOID_P`为`void指针`的大小。可以在`CMake`配置时进行查询，以便修改目标或目标编译定义。可以基于检测到的主机处理器体系结构，使用预处理器定义，确定需要编译的分支源代码。

当然，编写新代码时应该避免这种依赖，但在处理遗留代码或交叉编译时，这种依赖是有用的。

注意：使用`CMAKE_SIZEOF_VOID_P`是检查当前`CPU`是否具有`32位`或`64位`架构的唯一“真正”可移植的方法。

**arch_dependent.cpp**

```C++
#include <cstdlib>
#include <iostream>
#include <string>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

std::string ArchInfo();

int main() {
  std::cout << ArchInfo() << std::endl;
  return EXIT_SUCCESS;
}

std::string ArchInfo() {
  std::string arch_info(TOSTRING(ARCHITECTURE));
  arch_info += std::string(" architecture.  ");
#ifdef IS_32_BIT_ARCH
  return arch_info + std::string("Compiled on a 32 bit host processor.");
#elif IS_64_BIT_ARCH
  return arch_info + std::string("Compiled on a 64 bit host processor.");
#else
  return arch_info + std::string("Neither 32 not 64 bit, puzzling ...");
#endif
}
```

**输出结果**

```shell
mkdir build
cd build
cmake ..
-- The CXX compiler identification is GNU 9.4.0
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Target is 64 bits
-- x86_64 architecture detected
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter2/03/build
make
Scanning dependencies of target arch_dependent
[ 50%] Building CXX object CMakeFiles/arch_dependent.dir/arch_dependent.cpp.o
[100%] Linking CXX executable arch_dependent
[100%] Built target arch_dependent
./arch_dependent
x86_64 architecture.  Compiled on a 64 bit host processor.
```

## 四、附录

除了`CMAKE_HOST_SYSTEM_PROCESSOR`, `CMake`还定义了`CMAKE_SYSTEM_PROCESSOR`变量。前者包含当前运行的`CPU`在`CMake`的名称，而后者将包含当前正在为其构建的CPU的名称。这是一个细微的差别，在交叉编译时起着非常重要的作用。

另一种让`CMake`检测主机处理器体系结构，是使用`C`或`C++`中定义的符号，结合`CMake`的`try_run`函数:

```c++
#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #error cmake_arch i386
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
    #error cmake_arch x86_64
#endif
```

这种策略也是检测目标处理器体系结构的推荐策略，因为`CMake`似乎没有提供可移植的内在解决方案。

另一种选择，将只使用`CMake`，完全不使用预处理器，代价是为每种情况设置不同的源文件，然后使用`target_source`命令将其设置为可执行目标`arch_dependent`依赖的源文件:

```c++
add_executable(arch-dependent "")
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i386")
    message(STATUS "i386 architecture detected")
    target_sources(arch_dependent
        PRIVATE
        arch_dependent_i386.cpp
    )
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
    message(STATUS "i686 architecture detected")
    target_sources(arch_dependent
        PRIVATE
            arch_dependent_i686.cpp
    )
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "x86_64 architecture detected")
    target_sources(arch_dependent
        PRIVATE
            arch_dependent_x86_64.cpp
    )
else()
    message(STATUS "host processor architecture is unknown")
endif()
```

这种方法，显然需要对现有项目进行更多的工作，因为源文件需要分离。此外，不同源文件之间的代码复制肯定也会成为问题。

## 五、检测处理器指令集**

`CMake`可以检测主机处理器支持的指令集。这个功能是较新版本添加到`CMake`中的，需要`CMake 3.10`或更高版本。检测到的主机系统信息，可用于设置相应的编译器标志，或实现可选的源代码编译，或根据主机系统生成源代码。

将使用`config.h.in`生成`config.h`文件。config.h.in如下:

```c++
#ifndef CONFIG_HEADER_IN_H
#define CONFIG_HEADER_IN_H

#define NUMBER_OF_LOGICAL_CORES @_NUMBER_OF_LOGICAL_CORES@
#define NUMBER_OF_PHYSICAL_CORES @_NUMBER_OF_PHYSICAL_CORES@
#define TOTAL_VIRTUAL_MEMORY @_TOTAL_VIRTUAL_MEMORY@
#define AVAILABLE_VIRTUAL_MEMORY @_AVAILABLE_VIRTUAL_MEMORY@
#define TOTAL_PHYSICAL_MEMORY @_TOTAL_PHYSICAL_MEMORY@
#define AVAILABLE_PHYSICAL_MEMORY @_AVAILABLE_PHYSICAL_MEMORY@
#define IS_64BIT @_IS_64BIT@
#define HAS_FPU @_HAS_FPU@
#define HAS_MMX @_HAS_MMX@
#define HAS_MMX_PLUS @_HAS_MMX_PLUS@
#define HAS_SSE @_HAS_SSE@
#define HAS_SSE2 @_HAS_SSE2@
#define HAS_SSE_FP @_HAS_SSE_FP@
#define HAS_SSE_MMX @_HAS_SSE_MMX@
#define HAS_AMD_3DNOW @_HAS_AMD_3DNOW@
#define HAS_AMD_3DNOW_PLUS @_HAS_AMD_3DNOW_PLUS@
#define HAS_IA64 @_HAS_IA64@
#define OS_NAME "@_OS_NAME@"
#define OS_RELEASE "@_OS_RELEASE@"
#define OS_VERSION "@_OS_VERSION@"
#define OS_PLATFORM "@_OS_PLATFORM@"

#endif // ! CONFIG_HEADER_IN_H
```

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(progressor_info LANGUAGES CXX)

add_executable(${PROJECT_NAME} "")

target_sources(${PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/processor_info.cpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
)

foreach(key
  IN ITEMS
    NUMBER_OF_LOGICAL_CORES
    NUMBER_OF_PHYSICAL_CORES
    TOTAL_VIRTUAL_MEMORY
    AVAILABLE_VIRTUAL_MEMORY
    TOTAL_PHYSICAL_MEMORY
    AVAILABLE_PHYSICAL_MEMORY
    IS_64BIT
    HAS_FPU
    HAS_MMX
    HAS_MMX_PLUS
    HAS_SSE
    HAS_SSE2
    HAS_SSE_FP
    HAS_SSE_MMX
    HAS_AMD_3DNOW
    HAS_AMD_3DNOW_PLUS
    HAS_IA64
    OS_NAME
    OS_RELEASE
    OS_VERSION
    OS_PLATFORM
)
  cmake_host_system_information(RESULT _${key} QUERY ${key})
endforeach()

configure_file(config.h.in config.h @ONLY)
```

`foreach`循环会查询多个键值，并定义相应的变量。`cmake_host_system_information`查询运行`CMake`的主机系统的系统信息。本例中，对每个键使用了一个函数调用。然后，使用这些变量来配置`config.h.in`中的占位符，输入并生成`config.h`。

此配置使用`configure_file`命令完成。最后，`config.h`包含在`processor_info.cpp`中。编译后，它将把值打印到屏幕上。

```
target_include_directories(${PROJECT_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
)
```

这将生成的可执行文件链接到可执行文件所在的文件夹中。

我们编译后，`config.h`将于`build`中生成，在本机中生成的内容如下：

```c++
#ifndef CONFIG_HEADER_IN_H
#define CONFIG_HEADER_IN_H

#define NUMBER_OF_LOGICAL_CORES 16
#define NUMBER_OF_PHYSICAL_CORES 16
#define TOTAL_VIRTUAL_MEMORY 2047
#define AVAILABLE_VIRTUAL_MEMORY 2047
#define TOTAL_PHYSICAL_MEMORY 7903
#define AVAILABLE_PHYSICAL_MEMORY 6007
#define IS_64BIT 1
#define HAS_FPU 1
#define HAS_MMX 1
#define HAS_MMX_PLUS 0
#define HAS_SSE 1
#define HAS_SSE2 1
#define HAS_SSE_FP 0
#define HAS_SSE_MMX 0
#define HAS_AMD_3DNOW 0
#define HAS_AMD_3DNOW_PLUS 0
#define HAS_IA64 0
#define OS_NAME "Linux"
#define OS_RELEASE "5.15.0-78-generic"
#define OS_VERSION "#85~20.04.1-Ubuntu SMP Mon Jul 17 09:42:39 UTC 2023"
#define OS_PLATFORM "x86_64"

#endif // ! CONFIG_HEADER_IN_H
```

**processor_info.cpp**

```c++
#include <cstdlib>
#include <iostream>

#include "config.h"
int main() {
  std::cout << "Number of logical cores: " << NUMBER_OF_LOGICAL_CORES
            << std::endl;
  std::cout << "Number of physical cores: " << NUMBER_OF_PHYSICAL_CORES
            << std::endl;
  std::cout << "Total virtual memory in megabytes: " << TOTAL_VIRTUAL_MEMORY
            << std::endl;
  std::cout << "Available virtual memory in megabytes: "
            << AVAILABLE_VIRTUAL_MEMORY << std::endl;
  std::cout << "Total physical memory in megabytes: " << TOTAL_PHYSICAL_MEMORY
            << std::endl;
  std::cout << "Available physical memory in megabytes: "
            << AVAILABLE_PHYSICAL_MEMORY << std::endl;
  std::cout << "Processor is 64Bit: " << IS_64BIT << std::endl;
  std::cout << "Processor has floating point unit: " << HAS_FPU << std::endl;
  std::cout << "Processor supports MMX instructions: " << HAS_MMX << std::endl;
  std::cout << "Processor supports Ext. MMX instructions: " << HAS_MMX_PLUS
            << std::endl;
  std::cout << "Processor supports SSE instructions: " << HAS_SSE << std::endl;
  std::cout << "Processor supports SSE2 instructions: " << HAS_SSE2
            << std::endl;
  std::cout << "Processor supports SSE FP instructions: " << HAS_SSE_FP
            << std::endl;
  std::cout << "Processor supports SSE MMX instructions: " << HAS_SSE_MMX
            << std::endl;
  std::cout << "Processor supports 3DNow instructions: " << HAS_AMD_3DNOW
            << std::endl;
  std::cout << "Processor supports 3DNow+ instructions: " << HAS_AMD_3DNOW_PLUS
            << std::endl;
  std::cout << "IA64 processor emulating x86 : " << HAS_IA64 << std::endl;
  std::cout << "OS name: " << OS_NAME << std::endl;
  std::cout << "OS sub-type: " << OS_RELEASE << std::endl;
  std::cout << "OS build ID: " << OS_VERSION << std::endl;
  std::cout << "OS platform: " << OS_PLATFORM << std::endl;

  return EXIT_SUCCESS;
}
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_11/  

