# CMake 笔记 | [21] 利用Valgrind来检测内存缺陷


## 一、导言

{{<admonition quote "导言" false>}}
**目前，内存缺陷：写入或读取越界，或者内存泄漏（已分配但从未释放的内存），会生产难以跟踪的\**`bug`\**，最好尽早将它们检查出来。\**`Valgrind`\**是一个通用的工具，用来检测内存缺陷和内存泄漏。本篇将在使用\**`CMake/CTest`\**测试时使用\**`Valgrind`\**对内存问题进行警告。**
{{</admonition>}}

## 二、Valgrind安装


- 下载`Valgrind`

  ```
  wget  https://sourceware.org/pub/valgrind/valgrind-3.21.0.tar.bz2
  ```

- 解压

  ```
  tar -xjvf valgrind-3.15.0.tar.bz2
  cd valgrind-3.21.0
  ```

- 配置

  ```
  ./configure
  ```

- 编译

  ```
  make
  ```

- 安装

  ```
  sudo make install
  ```
## 三、项目结构

```shell
.
├── CMakeLists.txt
├── leaky_implementation.cpp
├── leaky_implementation.h
└── test.cpp
```

项目结构：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/04


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(test_leaky LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(example_library leaky_implementation.cpp)

add_executable(cpp_test test.cpp)
target_link_libraries(cpp_test example_library)

find_program(MEMORYCHECK_COMMAND NAMES valgrind)

set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
# add memcheck test action

include(CTest)

enable_testing()

add_test(
  NAME cpp_test
  COMMAND $<TARGET_FILE:cpp_test>
)
```
{{<admonition quote "" false>}}
```c++
find_program(MEMORYCHECK_COMMAND NAMES valgrind)
```

查找`valgrind`，并将`MEMORYCHECK_COMMAND`设置为其绝对路径。

{{</admonition>}}

{{<admonition quote "" false>}}
```c++
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
```
将相关参数传递给`Valgrind`。内存检查会创建一个日志文件，该文件可用于详细记录内存缺陷信息。
{{</admonition>}}



<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>


**leaky_implementation.h**

```c++
#ifndef LEAKY_IMPLEMENTATION_H
#define LEAKY_IMPLEMENTATION_H

int DoSomeWork();
#endif // ！LEAKY_IMPLEMENTATION_H
```

**leaky_implementation.cpp**

```c++
#include "leaky_implementation.h"

int DoSomeWork() {
  // we allocate an array
  double *default_array = new double[1000];
  // do some work
  // ...
  // we forget to deallocate it
  // delete[] default_array;
  return 0;
}
```

**test.cpp**

```c++
#include "leaky_implementation.h"

int main() {
  int ret = DoSomeWork();

  return ret;
}
```



<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果</font></td></tr></body></table>

```shell
   Site: jiangli-virtual-machine
   Build name: Linux-g++
Memory check project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/04/build
    Start 1: cpp_test
1/1 MemCheck #1: cpp_test .........................   Passed    1.03 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   1.03 sec
-- Processing memory checking output:
1/1 MemCheck: #1: cpp_test .........................   Defects: 1
MemCheck log files can be found here: ( * corresponds to test number)
/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/04/build/Testing/Temporary/MemoryChecker.*.log
Memory checking results:
Memory Leak - 1
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_21/  

