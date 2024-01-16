# CMake 笔记 | [6] 设置编译选项


## 一、编译器选项相关概念

`编译器选项`是指在编译程序时，可以通过设置不同的选项来控制编译器的行为和生成的代码的特性。常见的编译器选项包括`优化选项`、`调试选项`、`警告选项`、`链接选项`等。
  - `优化选项`可以控制编译器对代码进行优化的程度，以提高程序的性能。
  - `调试选项`可以生成调试信息，以便在程序出现问题时进行调试。
  - `警告选项`可以控制编译器是否生成警告信息，以帮助开发者发现潜在的问题。
  - `链接选项`可以控制编译器如何将多个目标文件链接在一起，以生成最终的可执行文件。

不同的编译器可能支持不同的选项，具体的选项和使用方法可以参考编译器的文档或者官方网站。

本篇内容涉及到的编译器选项有:
  - 优化选项: `-fPIC`、`-fno-rtti`、`-fno exception`
    - `-fPIC`:表示生成位置无关代码。具体来说，位置无关代码可以在不同的进程空间中加载和执行，而不需要进行重定位操作。fPIC选项通常用于生成动态库，因为动态库需要在不同的进程空间中加载和执行。使用-fPIC选项可以确保动态库中的代码可以在不同的进程空间中正确地执行。需要注意的是，使用`-fPIC`选项会增加代码的大小和运行时开销，因此需要根据具体情况来决定是否使用该选项。
    - `-fno-rtti`: 选项可以控制编译器是否生成与C++运行时类型信息（RTTI）相关的代码，以减小程序的大小和运行时开销。需要注意的是，禁用RTTI可能会影响程序的可靠性和可维护性，因为RTTI可以帮助开发者在运行时获取对象的类型信息。因此，需要根据具体情况来决定是否使用该选项。
    - `-fno exception`: 表示禁用C++异常处理机制。具体来说，使用该选项可以使编译器不生成与异常处理相关的代码，从而减小程序的大小和运行时开销。需要注意的是，禁用异常处理机制可能会影响程序的可靠性和可维护性，因为异常处理机制可以帮助开发者处理程序中的异常情况。因此，需要根据具体情况来决定是否使用该选项。
  - 警告选项:`-Wall`、`-Wextra`和`-Wpedantic`
    - `-Wall`: 表示开启所有警告信息。具体来说，编译器会生成所有可能的警告信息，包括一些可能会被忽略的警告信息。开启`-Wall`选项可以帮助开发者发现潜在的问题，提高代码的质量和可靠性。但是，由于`-Wall`会生成大量的警告信息，有时候会影响开发效率，因此需要根据具体情况来决定是否开启该选项。
    - `-Wextra`: 表示开启额外的警告信息。具体来说，编译器会生成一些不属于`-Wall`选项的警告信息，例如一些不符合标准的代码风格、一些未使用的变量等。同理，开启`-Wextra`选项可以帮助开发者发现更多的潜在问题，提高代码的质量和可靠性。但是，由于`-Wextra`会生成更多的警告信息，有时候会影响开发效率，因此需要根据具体情况来决定是否开启该选项。
    - `-Wpedantic`: 表示开启严格的警告信息。具体来说，编译器会生成一些不符合C或C++标准的代码警告信息，例如使用了不推荐的语法、未定义的行为等。开启`-Wpedantic`选项可以帮助开发者编写符合标准的代码，提高代码的可移植性和可靠性。同理，由于`-Wpedantic`会生成更多的警告信息，有时候会影响开发效率，因此需要根据具体情况来决定是否开启该选项

## 二、编译器选项设置

为目标准备了标志列表，其中一些将无法在Windows上使用：

```CMake
list(APPEND compile_flags "-fPIC" "-Wall" "-fPIC")
if(NOT WIN32)
    list(APPEND compile_flags "-Wextra" "-Wpedantic")
endif()
```

为库设置编译选项：

```CMake
target_compile_options(test_message
    PRIVATE ${compile_flags}
)
```

编译选项可以添加三个级别的可见性：`INTERFACE`、`PUBLIC`和`PRIVATE`。
  - `PRIVATE`:编译选项会应用于给定的目标，不会传递给与目标相关的目标。
  - `INTERFACE`:给定的编译选项将只应用于指定目标，并传递给与目标相关的目标。
  - `PUBLIC`:编译选项将应用于指定目标和使用它的目标。

<font color=red>如何确定</font>项目在`CMake`构建时，实际使用了哪些编译标志？

一种方法是，使用CMake将额外的参数传递给本地构建工具。本例中会设置环境变量VERBOSE=1：

```shell
mkdir -p build
$ cd build
$ cmake ..
$ cmake --build . -- VERBOSE=1
```

```shell
... lots of output ...
Scanning dependencies of target test_message
make[2]: 离开目录“/home/jiangli/repo/tutorials/cmake-tutorial/chapter1/13/build”
/usr/bin/make -f message-module/CMakeFiles/test_message.dir/build.make message-module/CMakeFiles/test_message.dir/build
make[2]: 进入目录“/home/jiangli/repo/tutorials/cmake-tutorial/chapter1/13/build”
[ 25%] Building CXX object message-module/CMakeFiles/test_message.dir/src/message.cpp.o
cd /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/13/build/message-module && /usr/bin/c++   -I/home/jiangli/repo/tutorials/cmake-tutorial/chapter1/13/message-module/include  -O3 -DNDEBUG   -fPIC -Wall -Wextra -Wpedantic -std=gnu++11 -o CMakeFiles/test_message.dir/src/message.cpp.o -c /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/13/message-module/src/message.cpp
[ 50%] Linking CXX static library ../lib/libtest_message_release.a
... lots of output ...
```

第二种，使用CMake参数进行配置：

```shell
$ cmake -D CMAKE_CXX_FLAGS="-fno-exceptions -fno-rtti" ..
```

这个命令将编译项目，禁用异常和运行时类型标识(RTTI)。

## 三、补充

大多数时候，编译器有特性标示。当前的例子只适用于GCC和Clang；其他编译器不确定是否会理解这些标志。如果项目是真正跨平台，那么这个问题就必须得到解决，以下为两种解决方案：

第一种，所需编译器标志列表附加到每个配置类型CMake变量`CMAKE_<LANG>_FLAGS_<CONFIG>`。标志确定设置为给定编译器有效的标志，因此将包含在if-endif子句中，用于检查`CMAKE_<LANG>_COMPILER_ID`变量：

```CMake
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  list(APPEND CMAKE_CXX_FLAGS "-fno-rtti" "-fno-exceptions")
  list(APPEND CMAKE_CXX_FLAGS_DEBUG "-Wsuggest-final-types" "-Wsuggest-final-methods" "-Wsuggest-override")
  list(APPEND CMAKE_CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  list(APPEND CMAKE_CXX_FLAGS "-fno-rtti" "-fno-exceptions" "-Qunused-arguments" "-fcolor-diagnostics")
  list(APPEND CMAKE_CXX_FLAGS_DEBUG "-Wdocumentation")
  list(APPEND CMAKE_CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
endif()
```

第二种，定义特定的标志列表：

```CMake
set(COMPILER_FLAGS)
set(COMPILER_FLAGS_DEBUG)
set(COMPILER_FLAGS_RELEASE)
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  list(APPEND CXX_FLAGS "-fno-rtti" "-fno-exceptions")
  list(APPEND CXX_FLAGS_DEBUG "-Wsuggest-final-types" "-Wsuggest-final-methods" "-Wsuggest-override")
  list(APPEND CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  list(APPEND CXX_FLAGS "-fno-rtti" "-fno-exceptions" "-Qunused-arguments" "-fcolor-diagnostics")
  list(APPEND CXX_FLAGS_DEBUG "-Wdocumentation")
  list(APPEND CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
endif()
```

稍后，使用生成器表达式来设置编译器标志的基础上，为每个配置和每个目标生成构建系统:
```CMake
target_compile_option(test_message
  PRIVATE
    ${CXX_FLAGS}
    "$<$<CONFIG:Debug>:${CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${CXX_FLAGS_RELEASE}>"
  )
```

这里我们推荐使用第二种方法。

两种方法都有效，并在许多项目中得到广泛应用。不过，每种方式都有缺点。`CMAKE_<LANG>_COMPILER_ID`不能保证为所有编译器都定义。此外，一些标志可能会被弃用，或者在编译器的较晚版本中引入。

与`CMAKE_<LANG>_COMPILER_ID`类似，`CMAKE_<LANG>_COMPILER_VERSION`变量不能保证为所有语言和供应商都提供定义。尽管检查这些变量的方式非常流行，但我们认为更健壮的替代方法是检查所需的标志集是否与给定的编译器一起工作，这样项目中实际上只使用有效的标志。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_6/  

