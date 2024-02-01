# CMake 笔记 | [34] 编写函数来测试和设置编译器标志


## 一、导言
{{<admonition quote "导言" false>}}
**前两篇，我们使用了宏。本篇，将使用一个函数来抽象细节并避免代码重复。我们将实现一个接受编译器标志列表的函数。该函数将尝试用这些标志逐个编译测试代码，并返回编译器理解的第一个标志。这样，我们将了解几个新特性：****`函数`、`列表操作`、`字符串操作`，以及检查编译器是否支持相应的标志。**
{{</admonition>}}




## 二、项目结构

```shell
.
├── cmake
│   └── set_compiler_flag.cmake
└── CMakeLists.txt
```

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter7/03




<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>

**cmake/set_compiler_flag.cmake**

```c++
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFortranCompilerFlag)
function(set_compiler_flag _result _lang)
  set(_list_of_flags)
  set(_flag_is_required FALSE)
  foreach(_arg IN ITEMS ${ARGN})
      string(TOUPPER "${_arg}" _arg_uppercase)
      if(_arg_uppercase STREQUAL "REQUIRED")
          set(_flag_is_required TRUE)
      else()
          list(APPEND _list_of_flags "${_arg}")
      endif()
  endforeach()
  set(_flag_found FALSE)
  foreach(flag IN ITEMS ${_list_of_flags})
      unset(_flag_works CACHE)
      if(_lang STREQUAL "C")
          check_c_compiler_flag("${flag}" _flag_works)
      elseif(_lang STREQUAL "CXX")
          check_cxx_compiler_flag("${flag}" _flag_works)
      elseif(_lang STREQUAL "Fortran")
          check_Fortran_compiler_flag("${flag}" _flag_works)
      else()
          message(FATAL_ERROR "Unknown language in set_compiler_flag: ${_lang}")
      endif()
    if(_flag_works)
      set(${_result} "${flag}" PARENT_SCOPE)
      set(_flag_found TRUE)
      break()
    endif()
  endforeach()
  if(_flag_is_required AND NOT _flag_found)
      message(FATAL_ERROR "None of the required flags were supported")
  endif()
endfunction()
```

{{<admonition quote "tips" false>}}
```c++
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFortranCompilerFlag)
```

这都是标准的`CMake`模块，`CMake`将在`${CMAKE_MODULE_PATH}`中找到它们。这些模块分别提供`check_c_compiler_flag`、`check_cxx_compiler_flag`和`check_fortran_compiler_flag`宏。
{{</admonition>}}

{{<admonition quote "tips" false>}}
```c++
function(set_compiler_flag _result _lang)
    ...
endfunction()
```

`set_compiler_flag`函数需要两个参数，`_result`(保存成功编译标志或为空字符串)和`_lang`(指定语言:`C`、`C++`或`Fortran`)。

我们也能这样调用函数:

```
set_compiler_flag(working_compile_flag C REQUIRED "-Wall" "-warn all")
```

这里有五个调用参数，但是函数头只需要两个参数。这意味着`REQUIRED`、`-Wall`和`-warn all`将放在`${ARGN}`中。
{{</admonition>}}

{{<admonition quote "tips" false>}}

```c++
# build a list of flags from the arguments
set(_list_of_flags)
# also figure out whether the function
# is required to find a flag
set(_flag_is_required FALSE)
foreach(_arg IN ITEMS ${ARGN})
  string(TOUPPER "${_arg}" _arg_uppercase)
  if(_arg_uppercase STREQUAL "REQUIRED")
      set(_flag_is_required TRUE)
  else()
      list(APPEND _list_of_flags "${_arg}")
  endif()
endforeach()
```

使用`foreach`构建一个标志列表。同时，从标志列表中过滤出`REQUIRED`，并使用它来设置`_flag_is_required`，将标志列表中的其他参数放到`_list_of_flags`中。
{{</admonition>}}

{{<admonition quote "tips" false>}}
```c++
set(_flag_found FALSE)
# loop over all flags, try to find the first which works
foreach(flag IN ITEMS ${_list_of_flags})
  unset(_flag_works CACHE)
  if(_lang STREQUAL "C")
      check_c_compiler_flag("${flag}" _flag_works)
  elseif(_lang STREQUAL "CXX")
      check_cxx_compiler_flag("${flag}" _flag_works)
  elseif(_lang STREQUAL "Fortran")
      check_Fortran_compiler_flag("${flag}" _flag_works)
  else()
      message(FATAL_ERROR "Unknown language in set_compiler_flag: ${_lang}")
  endif()
  # if the flag works, use it, and exit
  # otherwise try next flag
  if(_flag_works)
      set(${_result} "${flag}" PARENT_SCOPE)
      set(_flag_found TRUE)
      break()
  endif()
endforeach()
```

现在，我们将循环`${_list_of_flags}`，尝试每个标志，如果`_flag_works`被设置为`TRUE`，我们将`_flag_found`设置为`TRUE`，并中止进一步的搜索。

`unset(_flag_works CACHE)`确保`check_*_compiler_flag`的结果，不会在使用`_flag_works result`变量时，使用的是缓存结果。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
if(_flag_works)
      set(${_result} "${flag}" PARENT_SCOPE)
      set(_flag_found TRUE)
      break()
  endif()
```

如果找到了标志，并且`_flag_works`设置为`TRUE`，我们就将`_result`映射到的变量。

这需要使用`PARENT_SCOPE`来完成，因为我们正在修改一个变量，希望打印并在函数体外部使用该变量。请注意，如何使用`${_result}`语法解引用，从父范围传递的变量`_result`的值。不管函数的名称是什么，这对于确保工作标志被设置非常有必要。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
# raise an error if no flag was found
if(_flag_is_required AND NOT _flag_found)
    message(FATAL_ERROR "None of the required flags were supported")
endif()
```

如果没有找到任何标志，并且该标志设置了`REQUIRED`，那我们将使用一条错误消息停止配置。
{{</admonition>}}


**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(example LANGUAGES C CXX)

include(cmake/set_compiler_flag.cmake)

set_compiler_flag(
  working_compile_flag C REQUIRED
  "-foo" # this should fail
  "-wrong" # this should fail
  "-wrong" # this should fail
  "-Wall" # this should work with GNU
  "-warn all" # this should work with Intel
  "-Minform=inform" # this should work with PGI
  "-nope" # this should fail
)
message(STATUS "working C compile flag: ${working_compile_flag}")

set_compiler_flag(
  working_compile_flag CXX REQUIRED
  "-foo" # this should fail
  "-g" # this should work with GNU, Intel, PGI
  "/RTCcsu" # this should work with MSVC
)
message(STATUS "working CXX compile flag: ${working_compile_flag}")
```

{{<admonition quote "tips" false>}}
```c++
set_compiler_flag(
  working_compile_flag C REQUIRED
  "-foo" # this should fail
  "-wrong" # this should fail
  "-wrong" # this should fail
  "-Wall" # this should work with GNU
  "-warn all" # this should work with Intel
  "-Minform=inform" # this should work with PGI
  "-nope" # this should fail
)
```

设置`C`标志列表。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
set_compiler_flag(
  working_compile_flag CXX REQUIRED
  "-foo" # this should fail
  "-g" # this should work with GNU, Intel, PGI
  "/RTCcsu" # this should work with MSVC
)
```

设置`C++`标志列表。
{{</admonition>}}




## 三、结果展示

```shell
$ mkdir -p build
$ cd build
$ cmake ..
-- ...
-- Performing Test _flag_works
-- Performing Test _flag_works - Failed
-- Performing Test _flag_works
-- Performing Test _flag_works - Failed
-- Performing Test _flag_works
-- Performing Test _flag_works - Failed
-- Performing Test _flag_works
-- Performing Test _flag_works - Success
-- working C compile flag: -Wall
-- Performing Test _flag_works
-- Performing Test _flag_works - Failed
-- Performing Test _flag_works
-- Performing Test _flag_works - Success
-- working CXX compile flag: -g
-- ...
```



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_34/  

