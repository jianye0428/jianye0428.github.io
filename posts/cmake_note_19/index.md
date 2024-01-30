# CMake 笔记 | [19] 利用Catch2库进行单元测试


## 一、导言

{{<admonition quote "导言" false>}}
**上一篇，我们通过返回码来表示\**`test.cpp`\**测试的成功或者失败。对于简单的功能没问题，但是通常情况下，我们想要使用一个测试框架，它提供了相关基础设施来运行更复杂的测试，包括固定方式进行测试，与数值公差进行比较，以及在测试失败时输出更好的错误报告。本篇，通过使用目前比较流行的测试库Catch2来进行探索相关内容。这个测试框架有个很好的特性，它可以通过单个头文件包含在项目中进行测试，这使得编译和更新框架特别容易。通过CMake和Catch2结合使用，来测试上一篇的求和代码。**

{{</admonition>}}



## 二、项目结构

<mark>NOTE</mark>
本项目中`Catch`使用的版本是`2.0.1`，目前已经更新到`3.4.0`。关于最新版本的使用我们将在本系列最后开启一个具体的小型项目探索测试框架。

```shell
.
├── catch.hpp
├── CMakeLists.txt
├── main.cpp
├── sum_integers.cpp
├── sum_integers.h
└── test.cpp
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/02

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
# set minimum cmake version
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
# project name and language
project(test_catch2 LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# example library
add_library(sum_integers sum_integers.cpp)
# main code
add_executable(sum_up main.cpp)
target_link_libraries(sum_up sum_integers)

# testing binary
add_executable(cpp_test test.cpp)
target_link_libraries(cpp_test sum_integers)

enable_testing()
add_test(
  NAME catch_test
  COMMAND $<TARGET_FILE:cpp_test> --success
)
```


{{<admonition quote "" false>}}
```c++
add_test(
  NAME catch_test
  COMMAND $<TARGET_FILE:cpp_test> --success
)
```

`--success`选项可传递给单元测试的可执行文件。这是一个`Catch2`选项，测试成功时，也会有输出。
{{</admonition>}}

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>


> `main.cpp`、`sum_integers.cpp`和`sum_integers.h`与之前的示例相同。

**test.cpp**

```c++
#include <vector>

#include "sum_integers.h"
// this tells catch to provide a main()
// only do this in one cpp file
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Sum of integers for a short vector", "[shirt]") {
  auto integers = {1, 2, 3, 4, 5};
  REQUIRE(sum_integers(integers) == 15);
}

TEST_CASE("Sum of integers for a longer vector", "[long]") {
  std::vector<int> integers;
  for (int i = 1; i < 1001; ++i) {
    integers.push_back(i);
  }
  REQUIRE(sum_integers(integers) == 500500);
}
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果:</font></td></tr></body></table>

```shell
mkdir -p build
cd build
cmake ..
cmake --build .
ctest -V
UpdateCTestConfiguration  from :/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/build/DartConfiguration.tcl
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: catch_test

1: Test command: /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/build/cpp_test "--success"
1: Test timeout computed to be: 10000000
1:
1: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1: cpp_test is a Catch v2.0.1 host application.
1: Run with -? for options
1:
1: -------------------------------------------------------------------------------
1: Sum of integers for a short vector
1: -------------------------------------------------------------------------------
1: /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:13
1: ...............................................................................
1:
1: /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:15:
1: PASSED:
1:   REQUIRE( sum_integers(integers) == 15 )
1: with expansion:
1:   15 == 15
1:
1: -------------------------------------------------------------------------------
1: Sum of integers for a longer vector
1: -------------------------------------------------------------------------------
1: /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:18
1: ...............................................................................
1:
1: /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:23:
1: PASSED:
1:   REQUIRE( sum_integers(integers) == 500500 )
1: with expansion:
1:   500500 (0x7a314) == 500500 (0x7a314)
1:
1: ===============================================================================
1: All tests passed (2 assertions in 2 test cases)
1:
1/1 Test #1: catch_test .......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```

测试`cpp_test`的二进制文件，可以直接从`Catch2`中看到输出：

```shell
./cpp_test --success
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cpp_test is a Catch v2.0.1 host application.
Run with -? for options

-------------------------------------------------------------------------------
Sum of integers for a short vector
-------------------------------------------------------------------------------
/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:13
...............................................................................

/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:15:
PASSED:
  REQUIRE( sum_integers(integers) == 15 )
with expansion:
  15 == 15

-------------------------------------------------------------------------------
Sum of integers for a longer vector
-------------------------------------------------------------------------------
/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:18
...............................................................................

/home/jiangli/repo/tutorials/cmake-tutorial/chapter4/02/test.cpp:23:
PASSED:
  REQUIRE( sum_integers(integers) == 500500 )
with expansion:
  500500 (0x7a314) == 500500 (0x7a314)

===============================================================================
All tests passed (2 assertions in 2 test cases)
```



## 三、附录

`Catch2`是一个单头文件测试框架，所以不需要定义和构建额外的目标。只需要确保`CMake`能找到`catch.hpp`，从而构建`test.cpp`即可。为了方便起见，将它放在与`test.cpp`相同的目录中，我们可以选择一个不同的位置，并使用`target_include_directory`指示该位置。另一种方法是将头部封装到接口库中：

```c++
# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR
${CMAKE_CURRENT_SOURCE_DIR}/catch)
add_library(Catch
INTERFACE)
target_include_directories(Catch INTERFACE
${CATCH_INCLUDE_DIR})
```

然后，对库进行如下链接:

```
target_link_libraries(cpp_test Catch)
```

`Catch2`提供了更多功能。有关`Catch2`框架的完整文档，可访问：

> https://github.com/catchorg/Catch2

`Catch2`代码库包含有`CMake`函数，用于解析`Catch`测试并自动创建`CMake`测试，不需要显式地输入`add_test()`函数，可见：

> https://github.com/catchorg/Catch2/blob/master/contrib/ParseAndAddCatchTests.cmake





---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_19/  

