# CMake 笔记 | [37] 使用废弃函数、宏和变量


## 一、导言

{{<admonition quote "导言" false>}}
**`废弃`是在不断发展的项目开发过程中一种重要机制，它向开发人员发出信号，表明将来某个函数、宏或变量将被删除或替换。在一段时间内，函数、宏或变量将继续可访问，但会发出警告，最终可能会上升为错误。**
{{</admonition>}}


## 二、项目结构

```shell
.
├── cmake
│   └── custom_guard.cmake
└── CMakeLists.txt
```

项目结构：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter7/06


## 三、相关源码

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(example LANGUAGES NONE)

macro(custom_include_guard)
  if(NOT DEFINED included_modules)
    set(included_modules)
  endif()

  if ("${CMAKE_CURRENT_LIST_FILE}" IN_LIST included_modules)
    message(WARNING "module ${CMAKE_CURRENT_LIST_FILE} processed more than once")
  endif()

  list(APPEND included_modules ${CMAKE_CURRENT_LIST_FILE})
endmacro()

function(deprecate_variable _variable _access)
  if(_access STREQUAL "READ_ACCESS")
    message(DEPRECATION "variable ${_variable} is deprecated")
  endif()
endfunction()

if (CMAKE_VERSION VERSION_GREATER "3.9")
  # deprecate custom_include_guard
  macro(custom_include_guard)
    message(DEPRECATION "custom_include_guard is deprecated - use built-in include_guard instead")
    _custom_include_guard(${ARGV})
  endmacro()

  # deprecate variable included_modules
  variable_watch(included_modules deprecate_variable)
endif()

include(cmake/custom_guard.cmake)

message(STATUS "list of all included modules: ${included_modules}")
```
{{<admonition quote "" false>}}
```c++
macro(custom_include_guard)
  if(NOT DEFINED included_modules)
    set(included_modules)
  endif()

  if ("${CMAKE_CURRENT_LIST_FILE}" IN_LIST included_modules)
    message(WARNING "module ${CMAKE_CURRENT_LIST_FILE} processed more than once")
  endif()

  list(APPEND included_modules ${CMAKE_CURRENT_LIST_FILE})
endmacro()
```

定义了一个自定义的`包含保护`机制，包括一个自定义模块(与上一篇内容相同)，并打印所有包含模块的列表。对于`CMake 3.10`或更高版本有内置的`include_guard`。但是，不能简单地删除`custom_include_guard`和`${included_modules}`，而是使用一个`废弃`警告来弃用宏和变量。某个时候，可以将该警告转换为`FATAL_ERROR`，使代码停止配置，并迫使开发人员对代码进行修改，切换到内置命令。
{{</admonition>}}

{{<admonition quote "" false>}}
`废弃`函数、宏和变量的方法如下:

1.定义一个函数，使用它来弃用一个变量

```c++
function(deprecate_variable _variable _access)
  if(_access STREQUAL "READ_ACCESS")
      message(DEPRECATION "variable ${_variable} is deprecated")
  endif()
endfunction()
```

2.如果`CMake`的版本大于`3.9`，我们重新定义`custom_include_guard`并将`variable_watch`附加到`included_modules`中:

```c++
if (CMAKE_VERSION VERSION_GREATER "3.9")
  # deprecate custom_include_guard
  macro(custom_include_guard)
    message(DEPRECATION "custom_include_guard is deprecated - use built-in include_guard instead")
    _custom_include_guard(${ARGV})
  endmacro()
  # deprecate variable included_modules
  variable_watch(included_modules deprecate_variable)
endif()
```
{{</admonition>}}

**cmake/custom_guard.cmake**

```c++
custom_include_guard()

message(STATUS "custom.cmake is included and processed")
```

## 四、结果展示

```shell
$ mkdir -p build
$ cd build
$ cmake ..
```

`CMake3.10`以下版本的项目会产生以下结果:

```shell
-- custom_custom.cmake is included and processed
-- list of all included modules: /home/jiangli/repo/tutorials/cmake-tutorial/chapter7/06/cmake/custom.cmake
```

`CMake3.10`以下版本的项目会产生以下结果:

```shell
CMake Deprecation Warning at CMakeLists.txt:26 (message):
  custom_include_guard is deprecated - use built-in include_guard instead
Call Stack (most recent call first):
  cmake/custom_guard.cmake:1 (custom_include_guard)
  CMakeLists.txt:34 (include)


-- custom_custom.cmake is included and processed
CMake Deprecation Warning at CMakeLists.txt:19 (message):
  variable included_modules is deprecated
Call Stack (most recent call first):
  CMakeLists.txt:9999 (deprecate_variable)
  CMakeLists.txt:36 (message)


-- list of all included modules: /home/jiangli/repo/tutorials/cmake-tutorial/chapter7/06/cmake/custom_guard.cmake
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter7/06/build
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_37/  

