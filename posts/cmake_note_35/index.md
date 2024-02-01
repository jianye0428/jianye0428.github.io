# CMake 笔记 | [35] 重新定义函数和宏


## 一、导言

{{<admonition quote "" false>}}
**模块包含不应该用作函数调用，因为模块可能被包含多次。本篇，我们将编写我们自己的`包含保护`机制，如果多次包含一个模块，将触发警告。内置的`include_guard`命令从`3.10`版开始可以使用，对于`C/C++`头文件，它的行为就像`#pragma`一样。对于当前版本的`CMake`，我们将演示如何重新定义函数和宏，并且展示如何检查`CMake`版本，对于低于`3.10`的版本，我们将使用定制的`包含保护`机制。**
{{</admonition>}}


## 二、项目结构

```shell
.
├── cmake
│   ├── custom.cmake
│   └── include_guard.cmake
└── CMakeLists.txt
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter7/05





<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>


**cmake/include_guard.cmake**

```c++
macro(include_guard)
  if (CMAKE_VERSION VERSION_LESS "3.10")
    # for CMake below 3.10 we define our
    # own include_guard(GLOBAL)
    message(STATUS "calling our custom include_guard")

    # if this macro is called the first time
    # we start with an empty list
    if(NOT DEFINED included_modules)
        set(included_modules)
    endif()

    if ("${CMAKE_CURRENT_LIST_FILE}" IN_LIST included_modules)
      message(WARNING "module ${CMAKE_CURRENT_LIST_FILE} processed more than once")
    endif()

    list(APPEND included_modules ${CMAKE_CURRENT_LIST_FILE})
  else()
    # for CMake 3.10 or higher we augment
    # the built-in include_guard
    message(STATUS "calling the built-in include_guard")

    _include_guard(${ARGV})
  endif()
endmacro()
```

{{<admonition quote "" false>}}
```c++
macro(include_guard)
  if (CMAKE_VERSION VERSION_LESS "3.10")
      # ...
  else()
      # ...
  endif()
endmacro()
```

`include_guard`宏包含两个分支，一个用于`CMake`低于`3.10`，另一个用于`CMake`高于`3.10`。

{{</admonition>}}

{{<admonition quote "" false>}}
```c++
message(STATUS "calling our custom include_guard")
# if this macro is called the first time
# we start with an empty list
if(NOT DEFINED included_modules)
    set(included_modules)
endif()
if ("${CMAKE_CURRENT_LIST_FILE}" IN_LIST included_modules)
    message(WARNING "module ${CMAKE_CURRENT_LIST_FILE} processed more than once")
endif()
list(APPEND included_modules ${CMAKE_CURRENT_LIST_FILE})
```

如果`CMake`版本低于`3.10`，进入第一个分支，并且内置的`include_guard`不可用，所以我们自定义了一个。

如果第一次调用宏，则`included_modules`变量没有定义，因此我们将其设置为空列表。然后检查`${CMAKE_CURRENT_LIST_FILE}`是否是`included_modules`列表中的元素。如果是，则会发出警告；如果没有，我们将`${CMAKE_CURRENT_LIST_FILE}`追加到这个列表。`CMake`输出中，我们可以验证自定义模块的第二个包含确实会导致警告。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
macro(include_guard)
  if (CMAKE_VERSION VERSION_LESS "3.10")
      # ...
  else()
      message(STATUS "calling the built-in include_guard")
      _include_guard(${ARGV})
  endif()
endmacro()
```

`CMake 3.10`及更高版本的情况有所不同；在这种情况下，存在一个内置的`include_guard`，我们用自己的宏接收到参数并调用它。

这里，`_include_guard(${ARGV})`指向内置的`include_guard`。这里，我们使用自定义消息(`调用内置的include_guard`)进行了扩展。这种模式为我们提供了一种机制，来重新定义自己的或内置的函数和宏，这对于调试或记录日志来说非常有用。
{{</admonition>}}

**cmake/custom.cmake**

```c++
include_guard(GLOBAL)

message(STATUS "custom.cmake is included and processed")
```

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(example LANGUAGES NONE)

# (re)defines include_guard
include(cmake/include_guard.cmake)

# this is some custom module
include(cmake/custom.cmake)

# we simulate that we accidentally include the module a second time
include(cmake/custom.cmake)
```


## 三、结果展示

```shell
$ mkdir -p build
$ cd build
$ cmake ..
```

`CMake 3.10`及更高版本的结果如下:

```shell
-- calling the built-in include_guard
-- custom.cmake is included and processed
-- calling the built-in include_guard
```

`CMake 3.10`以下的结果如下:

```shell
- calling our custom include_guard
-- custom.cmake is included and processed
-- calling our custom include_guard
CMake Warning at cmake/include_guard.cmake:7 (message):
module
/home/user/example/cmake/custom.cmake
processed more than once
Call Stack (most recent call first):
cmake/custom.cmake:1 (include_guard)
CMakeLists.txt:12 (include)
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_35/  

