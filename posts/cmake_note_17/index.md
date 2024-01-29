# CMake | [17] 检测外部库 -- 自定义find模块


## 一、导言

{{<admonition quote "导言" false>}}
**目前上一篇，我们了解了CMake其中一种自定义检测外部库的方式，本篇将展示通过编写一个find模块来定位系统上的`ZeroMQ`库，以能够在非操作系统上检测该库。**
{{</admonition>}}

## 二、项目结构

```
.
├── CMakeLists.txt
├── FindZeroMQ.cmake
├── zmq_client.cpp
└── zmq_server.cpp
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter3/06

注：相关cpp源码与上一篇相同。


<table><body text=red><tr><td style="text-align:right;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(test_zmq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH /opt/zmq)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(ZeroMQ REQUIRED)

add_executable(hw_server zmq_server.cpp)

target_include_directories(hw_server
  PRIVATE ${ZeroMQ_INCLUDE_DIRS}
)

target_link_libraries(hw_server
  PRIVATE ${ZeroMQ_LIBRARIES}
)

add_executable(hw_client zmq_client.cpp)

target_include_directories(hw_client
  PRIVATE ${ZeroMQ_INCLUDE_DIRS}
)

target_link_libraries(hw_client
  PRIVATE ${ZeroMQ_LIBRARIES}
)
```

{{<admonition quote "" false>}}
```c++
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
```

这行代码是用于在`CMake`中向`CMAKE_MODULE_PATH`变量中添加一个目录的路径。通常，`CMAKE_MODULE_PATH`用于存放自定义的`CMake`模块文件，这些模块文件可以在项目的`CMakeLists.txt`文件中通过`include`等命令来使用。

具体而言，这行代码的作用是将`${CMAKE_CURRENT_SOURCE_DIR}`添加到CMAKE_MODULE_PATH中。`${CMAKE_CURRENT_SOURCE_DIR}`表示当前`CMakeLists.txt`所在的目录，即源代码目录。这样做的目的是为了告诉`CMake`在这个目录中查找自定义的CMake模块。

这样`CMake`就可以找到，我们自定义的`FindZeroMQ.cmake`模块。

此示例的主`CMakeLists.txt`在使用`FindZeroMQ.cmake`时，与上一篇中使用的`CMakeLists.txt`不同。这个模块使用`find_path`和`find_library` `CMake`内置命令，搜索`ZeroMQ`头文件和库，并使用`find_package_handle_standard_args`设置相关变量。
{{</admonition>}}

<table><body text=red><tr><td style="text-align:right;font-weight:bold" bgcolor=yellow><font size="3" color="red">FindZeroMQ.cmake</font></td></tr></body></table>

```C++
if(NOT ZeroMQ_ROOT)
  set(ZeroMQ_ROOT "$ENV{ZeroMQ_ROOT}")
endif()

if(NOT ZeroMQ_ROOT)
  find_path(_ZeroMQ_ROOT NAMES include/zmq.h)
else()
  set(_ZeroMQ_ROOT "${ZeroMQ_ROOT}")
endif()

find_path(ZeroMQ_INCLUDE_DIRS NAMES zmq.h HINTS ${_ZeroMQ_ROOT}/include)

if(ZeroMQ_INCLUDE_DIRS)
  set(_ZeroMQ_H ${ZeroMQ_INCLUDE_DIRS}/zmq.h)

  function(_zmqver_EXTRACT _ZeroMQ_VER_COMPONENT _ZeroMQ_VER_OUTPUT)
    set(CMAKE_MATCH_1 "0")
    set(_ZeroMQ_expr "^[ \\t]*#define[ \\t]+${_ZeroMQ_VER_COMPONENT}[ \\t]+([0-9]+)$")
    file(STRINGS "${_ZeroMQ_H}" _ZeroMQ_ver REGEX "${_ZeroMQ_expr}")
    string(REGEX MATCH "${_ZeroMQ_expr}" ZeroMQ_ver "${_ZeroMQ_ver}")
    set(${_ZeroMQ_VER_OUTPUT} "${CMAKE_MATCH_1}" PARENT_SCOPE)
  endfunction()

  _zmqver_EXTRACT("ZMQ_VERSION_MAJOR" ZeroMQ_VERSION_MAJOR)
  _zmqver_EXTRACT("ZMQ_VERSION_MINOR" ZeroMQ_VERSION_MINOR)
  _zmqver_EXTRACT("ZMQ_VERSION_PATCH" ZeroMQ_VERSION_PATCH)

  // We should provide version to find_package_handle_standard_args in the same format as it was requested,
  // otherwise it can't check whether version matches exactly.

  if(ZeroMQ_FIND_VERSION_COUNT GREATER 2)
    set(ZeroMQ_VERSION "${ZeroMQ_VERSION_MAJOR}.${ZeroMQ_VERSION_MINOR}.${ZeroMQ_VERSION_PATCH}")
  else()
  // User has requested ZeroMQ version without patch part => user is not interested in specific patch =>
  // any patch should be an exact match.
    set(ZeroMQ_VERSION "${ZeroMQ_VERSION_MAJOR}.${ZeroMQ_VERSION_MINOR}")
  endif()

  if(NOT ${CMAKE_C_PLATFORM_ID} STREQUAL "Windows")
    find_library(ZeroMQ_LIBRARIES
        NAMES zmq HINTS
        ${_ZeroMQ_ROOT}/lib
        ${_ZeroMQ_ROOT}/lib/x86_64-linux-gnu
    )
  else()
    find_library(ZeroMQ_LIBRARIES
        NAMES libzmq
        "libzmq-mt-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
        "libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
        libzmq_d
        "libzmq-mt-gd-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
        "libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-gd-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
        HINTS
        ${_ZeroMQ_ROOT}/lib
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZeroMQ
  FOUND_VAR
    ZeroMQ_FOUND
  REQUIRED_VARS
    ZeroMQ_INCLUDE_DIRS
    ZeroMQ_LIBRARIES
  VERSION_VAR
    ZeroMQ_VERSION
)
```

{{<admonition quote "" false>}}
```c++
if(NOT ZeroMQ_ROOT)
  set(ZeroMQ_ROOT "$ENV{ZeroMQ_ROOT}")
endif()
```

如果`ZeroMQ_ROOT`变量没有被设置，就尝试从环境变量中获取该路径并设置到`ZeroMQ_ROOT`变量中。这样的设计可以让我们在不修改`CMakeLists.txt`文件的情况下，通过设置环境变量来指定ZeroMQ库的路径。
{{</admonition>}}



{{<admonition quote "" false>}}
```C++
if(NOT ZeroMQ_ROOT)
    find_path(_ZeroMQ_ROOT NAMES include/zmq.h)
else()
    set(_ZeroMQ_ROOT "${ZeroMQ_ROOT}")
endif()
find_path(ZeroMQ_INCLUDE_DIRS NAMES zmq.h HINTS ${_ZeroMQ_ROOT}/include)
```

用于查找`ZeroMQ`库的头文件路径，并将路径存储在变量`ZeroMQ_INCLUDE_DIRS`中。首先检查是否定义了`ZeroMQ_ROOT`变量，如果未定义，则尝试在系统路径中查找`zmq.h`头文件；如果已定义，则直接使用`ZeroMQ_ROOT`变量的值。
{{</admonition>}}

{{<admonition quote "" false>}}
```C++
set(_ZeroMQ_H ${ZeroMQ_INCLUDE_DIRS}/zmq.h)
function(_zmqver_EXTRACT _ZeroMQ_VER_COMPONENT _ZeroMQ_VER_OUTPUT)
set(CMAKE_MATCH_1 "0")
set(_ZeroMQ_expr "^[ \\t]*#define[ \\t]+${_ZeroMQ_VER_COMPONENT}[ \\t]+([0-9]+)$")
file(STRINGS "${_ZeroMQ_H}" _ZeroMQ_ver REGEX "${_ZeroMQ_expr}")
string(REGEX MATCH "${_ZeroMQ_expr}" ZeroMQ_ver "${_ZeroMQ_ver}")
set(${_ZeroMQ_VER_OUTPUT} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()
_zmqver_EXTRACT("ZMQ_VERSION_MAJOR" ZeroMQ_VERSION_MAJOR)
_zmqver_EXTRACT("ZMQ_VERSION_MINOR" ZeroMQ_VERSION_MINOR)
_zmqver_EXTRACT("ZMQ_VERSION_PATCH" ZeroMQ_VERSION_PATCH)
```

如果成功找到头文件，则将`ZeroMQ_INCLUDE_DIRS`设置为其位置。我们继续通过使用字符串操作和正则表达式，寻找相应版本的`ZeroMQ`库。
{{</admonition>}}

{{<admonition quote "" false>}}
```C++
if(ZeroMQ_FIND_VERSION_COUNT GREATER 2)
    set(ZeroMQ_VERSION "${ZeroMQ_VERSION_MAJOR}.${ZeroMQ_VERSION_MINOR}.${ZeroMQ_VERSION_PATCH}")
else()
    set(ZeroMQ_VERSION "${ZeroMQ_VERSION_MAJOR}.${ZeroMQ_VERSION_MINOR}")
endif()
```

为`find_package_handle_standard_args`准备`ZeroMQ_VERSION`变量。
{{</admonition>}}

{{<admonition quote "" false>}}
```C++
if(NOT ${CMAKE_C_PLATFORM_ID} STREQUAL "Windows")
  find_library(ZeroMQ_LIBRARIES
    NAMES
        zmq
    HINTS
      ${_ZeroMQ_ROOT}/lib
      ${_ZeroMQ_ROOT}/lib/x86_64-linux-gnu
    )
else()
  find_library(ZeroMQ_LIBRARIES
    NAMES
        libzmq
      "libzmq-mt-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
      "libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
      libzmq_d
      "libzmq-mt-gd-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
      "libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-gd-${ZeroMQ_VERSION_MAJOR}_${ZeroMQ_VERSION_MINOR}_${ZeroMQ_VERSION_PATCH}"
    HINTS
        ${_ZeroMQ_ROOT}/lib
    )
endif()
```

使用`find_library`命令搜索`ZeroMQ`库。因为库的命名有所不同，这里我们需要区分`Unix`的平台和`Windows`平台。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQ
  FOUND_VAR
      ZeroMQ_FOUND
  REQUIRED_VARS
  ZeroMQ_INCLUDE_DIRS
  ZeroMQ_LIBRARIES
  VERSION_VAR
  ZeroMQ_VERSION
  )
```

最后，包含了标准`FindPackageHandleStandardArgs.cmake`，并调用相应的`CMake`命令。如果找到所有需要的变量，并且版本匹配，则将`ZeroMQ_FOUND`变量设置为`TRUE`。
{{</admonition>}}



## 三、附录



`find-module`通常遵循特定的模式:

- 检查用户是否为所需的包提供了自定义位置。
- 使用`find_`家族中的命令搜索所需包的必需组件，即头文件、库、可执行程序等等。我们使用`find_path`查找头文件的完整路径，并使用`find_library`查找库。`CMake`还提供`find_file`、`find_program`和`find_package`。这些命令的参数示意如下:

```
find_path(<VAR> NAMES name PATHS paths)
```

如果搜索成功，`<VAR>`将保存搜索结果；如果搜索失败，则会设置为`<VAR>-NOTFOUND`。`NAMES`和`PATHS`分别是`CMake`应该查找的文件的名称和搜索应该指向的路径。

初步搜索的结果中，可以提取版本号。本例中，`ZeroMQ`头文件包含库版本，可以使用字符串操作和正则表达式提取库版本信息。

最后，调用`find_package_handle_standard_args`命令。处理`find_package`命令的`REQUIRED`、`QUIET`和版本参数，并设置`ZeroMQ_FOUND`变量。

总结：有四种方式可用于找到依赖包。

- 使用由包供应商提供`CMake`文件`<package>Config.cmake `，`<package>ConfigVersion.cmake`和`<package>Targets.cmake`，通常会在包的标准安装位置查找。
- 无论是由`CMake`还是第三方提供的模块，为所需包使用`find-module`。
- 使用`pkg-config`，如本篇的示例所示。
- 如果这些都不可行，那么编写自己的`find`模块。

这四种可选方案按相关性进行了排序，每种方法也都有其挑战。

目前，并不是所有的包供应商都提供`CMake`的`Find`文件，不过正变得越来越普遍。因为导出`CMake`目标，使得第三方代码很容易使用它所依赖的库和/或程序附加的依赖。

从一开始，`Find-module`就一直是`CMake`中定位依赖的主流手段。但是，它们中的大多数仍然依赖于设置依赖项使用的变量，比如`Boost_INCLUDE_DIRS`、`PYTHON_INTERPRETER`等等。这种方式很难在第三方发布自己的包时，确保依赖关系被满足。

使用`pkg-config`的方法可以很好地进行适配，因为它已经成为`Unix`系统的标准。然而，也由于这个原因，它不是一个完全跨平台的方法。此外，如`CMake`文档所述，在某些情况下，用户可能会意外地覆盖检测包，并导致`pkg-config`提供不正确的信息。

最后的方法是编写自己的查找模块脚本，就像本示例中那样。这是可行的，并且依赖于`FindPackageHandleStandardArgs.cmake`。然而，编写一个全面的查找模块脚本绝非易事，需要考虑很多可能性。



最后祝大家变得更强！





![Image](data:image/svg+xml,%3C%3Fxml version='1.0' encoding='UTF-8'%3F%3E%3Csvg width='1px' height='1px' viewBox='0 0 1 1' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'%3E%3Ctitle%3E%3C/title%3E%3Cg stroke='none' stroke-width='1' fill='none' fill-rule='evenodd' fill-opacity='0'%3E%3Cg transform='translate(-249.000000, -126.000000)' fill='%23FFFFFF'%3E%3Crect x='249' y='126' width='1' height='1'%3E%3C/rect%3E%3C/g%3E%3C/g%3E%3C/svg%3E)





![Image](data:image/svg+xml,%3C%3Fxml version='1.0' encoding='UTF-8'%3F%3E%3Csvg width='1px' height='1px' viewBox='0 0 1 1' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'%3E%3Ctitle%3E%3C/title%3E%3Cg stroke='none' stroke-width='1' fill='none' fill-rule='evenodd' fill-opacity='0'%3E%3Cg transform='translate(-249.000000, -126.000000)' fill='%23FFFFFF'%3E%3Crect x='249' y='126' width='1' height='1'%3E%3C/rect%3E%3C/g%3E%3C/g%3E%3C/svg%3E)



cmake44

cmake · 目录

上一篇CMake:检测外部库---使用pkg-config下一篇CMake:利用CTest进行单元测试



People who liked this content also liked

CMake:超级构建模式



Hope Hut

不喜欢

不看的原因

OK

- 内容质量低
- 不看此公众号

![img](https://mmbiz.qpic.cn/sz_mmbiz_jpg/sd9Xic2LWs1UWYaO93spsRvKt3OOuS6wEZYsTAEFh3wErfaBLzIL3Rga9mUibLiaZ8NYc21AFPQ37qEckj64DNWkQ/0?wx_fmt=jpeg)

IDEA 2023.3 爆了！



架构汪

不喜欢

不看的原因

OK

- 内容质量低
- 不看此公众号

![img](https://mmbiz.qpic.cn/sz_mmbiz_jpg/elOZBLGApYY4ia5Z8cUAQ7untxqn8D8TebLhMTh5VWiawukpec1vCZ9VLUO0rAia7iclbibSplxKHK5M9bVraNb87dA/0?wx_fmt=jpeg)

CMake: 构建时记录Git Hash值



Hope Hut

不喜欢

不看的原因

OK

- 内容质量低
- 不看此公众号

![img](https://mmbiz.qpic.cn/sz_mmbiz_jpg/sd9Xic2LWs1VztH1NqYjqLSsmNyGWwH3ppfHYEQibn3HlDhjTZCqnx7P0pZAuliaqE4T0OLtCYLYvnrVUPjMTXtFA/0?wx_fmt=jpeg)

![img](https://mp.weixin.qq.com/mp/qrcode?scene=10000004&size=102&__biz=MzkxMzI5Mjk4Mg==&mid=2247485047&idx=1&sn=d1fa67a4801303c8e836c4640ae41ac0&send_time=)

Scan to Follow

people underline

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_17/  

