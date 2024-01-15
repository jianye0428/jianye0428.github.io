# CMake 笔记 | 条件语句、选项命令以及指定编译器


## 一、条件语句

首先，我们还是拿我们上一篇根目录下的CMakeLists.txt文件进行讲解。

源码地址为：https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter1/11

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

这里，我们使用了一个条件语句`if()...endif()`和一个选项命令`option`。这一节我们先对条件语句`if()...endif()`进行讲解，关于选项`option`命令，将在下一节进行讲解。

{{<admonition quote "Tip" false>}}
```CMake
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
endif()
```
判断CMake自带的宏`CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT`有没有处于开启状态，如果是默认状态的话，则`CMAKE_INSTALL_PREFIX`将被设置为`${CMAKE_SOURCE_DIR}/output/`。
{{</admonition>}}

这里，我们只写一个`CMakeLists.txt`对条件语句`if()...else()`和宏`CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT`、`CMAKE_INSTALL_PREFIX`进行探索。


```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(condition)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
endif()

message("CMAKE_INSTALL_PREFIX is " ${CMAKE_INSTALL_PREFIX})
```

如果我们对`CMAKE_INSTALL_PREFIX`提前进行了定义

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(condition)

set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT off)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
else()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/test)
endif()

message("CMAKE_INSTALL_PREFIX is " ${CMAKE_INSTALL_PREFIX})
```

## 二、命令选项

当然，上述内容我们也可以在编译时，使用如下命令，而不用显式地在`CMakeLists.txt`中对`CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT`进行设置，但我们需要在`CMakeLists.txt`中添加`option`命令。

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(condition)

option(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT "Set default install path" off)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
else()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/test)
endif()

message("CMAKE_INSTALL_PREFIX is " ${CMAKE_INSTALL_PREFIX})
```

```shell
cmake .. -DCMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT=OFF
```

其中，-D开关用于为CMake设置任何类型的变量：逻辑变量、路径等等。

源码地址: https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter1/11

## 三、指定编译器

到目前为止，我们还没有考虑过使用CMake如何选择编译器。`CMake`可以根据平台和生成器选择编译器，还能将编译器标志设置为默认值。

在实际工作中，指定编译器十分重要，比如我们要交叉编译嵌入式的项目，亦或是我们可以将`Qt`、`OpenCV`等三方库交叉编译到我们的嵌入式项目中。具体关于如何编译其他三方库生成指定的形式，我们将在之后讲到交叉编译时会详细讲解。

### 3.1 通过命令行的形式指定

```shell
cmake -D CMAKE_CXX_COMPILER=clang++ ..
```
指定`c++`的编译器为`clang++`

```shell
CMAKE_CXX_COMPILER 指定C++编译器
CMAKE_C_COMPILER 指定C编译器
CMAKE_Fortran_COMPILER 指定Fortran编译器
```

### 3.2 通过导出环境变量指定

```shell
env CXX=clang++ cmake ..
```

导出环境变量，指定C++的编译器为Clang++

`CXX`(`C++`编译器)、`CC`(C编译器)和`FC`(Fortran编译器)

{{<admonition quote "" false>}}

注意: CMake了解运行环境，可以通过-D开关或者环境变量设置许多选项。第一种方法覆盖第二种方法，但是建议使用-D的显式设置选项。显式由于隐式，因为环境变量可能被设置为不合适的值（当前项目）。
注意: 这里，我们假设，其他的编译器，如clang++在标准的路径中可以用，CMake可以在标准路径中执行查找编译器。如果在标准路径中不可以用使用，则用户需要将完整的编译器可执行文件或者包装器路径传递给CMake。如：
```shell
cmake -D CMAKE_CXX_COMPILER=/mnt/usr/bin/clang++ ..
```
注意: 这里，建议使用`-DCMAKE_<LANG>_COMPILER`选项设置编译器，而不是导出CXX、CC和FC。这是确保跨平台并与非POSIX兼容的唯一方法。为了避免变量污染环境，这些变量会影响与项目一起构建的外部库环境。
{{</admonition>}}

### 3.3 在CMake中指定

```CMake
set(CMAKE_CXX_COMPILER /mnt/usr/bin/clang++)
```
在使用CMake进行构建时，CMake会进行一系列的平台测试，以确定哪些编译器可以使用以及它们是否适合当前的项目。

一个合适的编译器不仅取决于我们使用的平台，还取决于我们想要的生成器。CMake执行的第一个测试基于项目语言的编译器名称。如，CC是一个工作i的C编译器，那么它将用作C项目的默认编译器。

GNU/Linux上，使用Unix Makefile或Ninja时，GCC家族中的编译器很可能是C++、C和Fortran的默认选择。Windows上，将选择Visual Studio中C++和C编译器。如果选择MinGW或MSYS Makefile作为生成器，则默认使用MinGW编译器。

### 3.4 编译器的补充

我们平台上的CMake可以找到可用的编译器和编译器标志的方法是：`CMake`提供`--system-information`标志，他将把关于系统的所有信息转储到屏幕或者文件中。

```shell
cmake --system-information information.txt
```

文件中可以看到`CMAKE_CXX_COMPILER`、`CMAKE_C_COMPILER`和`CMAKE_Fortran_COMPILER`的默认值，以及默认标志。

CMake提供了额外的变量来与编译器交互：
  - `CMAKE_<LANG>_COMPILER_LOADED`:如果为项目启用了语言<LANG>，则将设置为TRUE。
  - `CMAKE_<LANG>_COMPILER_ID`:编译器标识字符串，编译器供应商所特有。例如，GCC用于GNU编译器集合，AppleClang用于macOS上的Clang, MSVC用于Microsoft Visual Studio编译器。注意，不能保证为所有编译器或语言定义此变量。
  - `CMAKE_COMPILER_IS_GNU<LANG>`:如果语言<LANG>是GNU编译器集合的一部分，则将此逻辑变量设置为TRUE。注意变量名的<LANG>部分遵循GNU约定：C语言为CC, C++语言为CXX, Fortran语言为G77。
  - `CMAKE_<LANG>_COMPILER_VERSION`:此变量包含一个字符串，该字符串给定语言的编译器版本。版本信息在major[.minor[.patch[.tweak]]]中给出。但是，对于CMAKE_<LANG>_COMPILER_ID，不能保证所有编译器或语言都定义了此变量。

我们可以使用不同的编译器，构建下面的CMakeLists.txt。

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(recipe-06 LANGUAGES C CXX)
message(STATUS "Is the C++ compiler loaded? ${CMAKE_CXX_COMPILER_LOADED}")
if(CMAKE_CXX_COMPILER_LOADED)
    message(STATUS "The C++ compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Is the C++ from GNU? ${CMAKE_COMPILER_IS_GNUCXX}")
    message(STATUS "The C++ compiler version is: ${CMAKE_CXX_COMPILER_VERSION}")
endif()
message(STATUS "Is the C compiler loaded? ${CMAKE_C_COMPILER_LOADED}")
if(CMAKE_C_COMPILER_LOADED)
    message(STATUS "The C compiler ID is: ${CMAKE_C_COMPILER_ID}")
    message(STATUS "Is the C from GNU? ${CMAKE_COMPILER_IS_GNUCC}")
    message(STATUS "The C compiler version is: ${CMAKE_C_COMPILER_VERSION}")
endif()
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_4/  

