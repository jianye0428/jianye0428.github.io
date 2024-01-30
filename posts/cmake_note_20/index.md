# CMake 笔记 | [20] 利用Google Test库进行单元测试


## 一、导言


{{<admonition quote "导言" false>}}
**本篇，我们将学习如何在CMake的帮助下使用Google Test框架实现单元测试。与前一个配置（Catch2）相比，Google Test框架不仅仅是一个头文件，也是一个库，包含两个需要构建和链接的文件。可以将它们与我们的代码项目放在一起，但是为了使项目更加轻量级，我们将选择在配置时，下载一个定义良好的Google Test，然后构建框架并链接它。我们将使用较新的FetchContent模块（从CMake版本3.11开始可用）。关于相关使用将在后续的笔记中学习。**

**此外我们将在相关测试内同学习完成后挑选一个测试框架（目前Google Test更加流行）\**写一个小的项目实践，尽可能多的将该框架下的功能加以熟悉。\****
{{</admonition>}}


## 二、项目结构

<mark>NOTE:</mark>

`main.cpp`、`sum_integers.cpp`和`sum_integers.hpp`与上一篇内容相同，我们对test.cpp将做相关的修改。

```shell
.
├── CMakeLists.txt
├── main.cpp
├── sum_integers.cpp
├── sum_integers.h
└── test.cpp
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/03



## 三、项目代码

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(test_gtest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(sum_integers sum_integers.cpp)

add_executable(sum_up main.cpp)
target_link_libraries(sum_up sum_integers)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if(ENABLE_UNIT_TESTS)
  include(FetchContent)

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://gitcode.net/mirrors/google/googletest.git
    GIT_TAG        release-1.8.0
  )

  FetchContent_GetProperties(googletest)

  if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Prevent GoogleTest from using PThreads
    set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

    # adds the targers: gtest, gtest_main, gmock, gmock_main
    add_subdirectory(
      ${googletest_SOURCE_DIR}
      ${googletest_BINARY_DIR}
    )

    # Silence std::tr1 warning on MSVC
    if(MSVC)
      foreach(_tgt gtest gtest_main gmock gmock_main)
        target_compile_definitions(${_tgt}
          PRIVATE
            "_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
        )
      endforeach()
    endif()
  endif()

  add_executable(cpp_test "")

  target_sources(cpp_test
    PRIVATE
      test.cpp
  )

  target_link_libraries(cpp_test
    PRIVATE
      sum_integers
      gtest_main
  )

  enable_testing()

  add_test(
    NAME google_test
    COMMAND $<TARGET_FILE:cpp_test>
  )
endif()
```

**<font color=red>注意:</font>**CMake 3.11版本以后才可以使用`FetchContent`模块。

{{<admonition quote "" false>}}
```c++
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
```

告诉`CMake`在 `Windows` 平台上自动从共享库（`DLL`）中导出所有符号（函数、变量、类等）。

当将此选项设置为 `ON` 时，`CMake`会自动在库的代码中插入导出指令，确保它们可以被外部链接。这在 `Windows` 上特别重要，因为需要明确的导出声明才能让符号从 `DLL` 外部访问。

然而，启用此选项可能会导致生成较大的二进制文件，并可能意外地暴露符号。如果链接了多个库，还可能导致符号冲突。因此，虽然它简化了符号的导出，但需要仔细考虑其影响，以及是否适用于自己的项目。

{{</admonition>}}

{{<admonition quote "" false>}}
```c++
option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
if(ENABLE_UNIT_TESTS)
    # all the remaining CMake code will be placed here
endif()
```

检查`ENABLE_UNIT_TESTS`。默认情况下，它为`ON`，但有时需要设置为`OFF`，以免在没有网络连接时，也能使用`Google Test`。

{{</admonition>}}


{{<admonition quote "" false>}}
```c++
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://gitcode.net/mirrors/google/googletest.git
  GIT_TAG        release-1.8.0
)
```

使用了`FetchContent`模块来下载和集成`Google Test`库（`googletest`）。

- `include(FetchContent)`：用于包含`FetchContent`模块，该模块允许在项目中获取外部依赖项。
- `FetchContent_Declare(googletest ...)`：使用`FetchContent_Declare`宏来声明要获取的外部依赖项。在这种情况下，它声明了一个名为`googletest`的外部依赖项。
- `GIT_REPOSITORY`：这是指定用于获取库的`Git`存储库的`URL`。
- `GIT_TAG`：这是`Git`存储库中的特定标签或分支，指定希望获取的库的版本。在这里，它指定了`Google Test`库的版本为`release-1.8.0`。

使用这段代码后，当构建项目时，`CMake`将尝试下载并集成`Google Test`库，以便可以在项目中进行单元测试。请注意，实际项目中可能还需要在测试目标中链接`Google Test`库，并设置测试用例等。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Prevent GoogleTest from using PThreads
    set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

    # adds the targers: gtest, gtest_main, gmock, gmock_main
    add_subdirectory(
      ${googletest_SOURCE_DIR}
      ${googletest_BINARY_DIR}
    )

    # Silence std::tr1 warning on MSVC
    if(MSVC)
      foreach(_tgt gtest gtest_main gmock gmock_main)
        target_compile_definitions(${_tgt}
          PRIVATE
            "_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
        )
      endforeach()
    endif()
  endif()
```

- `if(NOT googletest_POPULATED)`：检查是否已经获取并集成了`Google Test`库。如果`googletest_POPULATED`未定义，则会执行其中的代码块。
- `FetchContent_Populate(googletest)`：将下载的`Google Test`库内容填充到指定的目录，以便后续构建和集成。
- `set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)`：在使用`Visual Studio`构建时，将强制`Google Test`使用共享运行时（`C Runtime`）库。可以避免构建时的链接错误。
- `set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)`：禁用`Google Test`对`pthreads`的使用。这可能在某些环境中是必需的，例如在没有`pthreads`支持的平台上。
- `add_subdirectory(...)`：将`Google Test`库添加到项目中。它会在指定的源码目录和二进制目录中进行构建。
- 对于`Microsoft Visual Studio（MSVC）`，在构建`Google Test`库时，通过`target_compile_definitions`为`gtest`、`gtest_main`、`gmock`和`gmock_main`目标添加了宏定义以消除`MSVC`下的警告。

{{</admonition>}}

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>

**test.cpp**

```c++
#include "sum_integers.h"
#include "gtest/gtest.h"
#include <vector>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(example, sum_zero) {
  auto integers = {1, -1, 2, -2, 3, -3};
  auto result = sum_integers(integers);
  ASSERT_EQ(result, 0);
}

TEST(example, sum_five) {
  auto integers = {1, 2, 3, 4, 5};
  auto result = sum_integers(integers);
  ASSERT_EQ(result, 15);
}
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出:</font></td></tr></body></table>

```shell
mkdir build
cd build
cmake ..
ctest
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/03/build
    Start 1: google_test
1/1 Test #1: google_test ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
./cpp_test
[==========] Running 2 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 2 tests from example
[ RUN      ] example.sum_zero
[       OK ] example.sum_zero (0 ms)
[ RUN      ] example.sum_five
[       OK ] example.sum_five (0 ms)
[----------] 2 tests from example (0 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test case ran. (0 ms total)
[  PASSED  ] 2 tests.
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_20/  

