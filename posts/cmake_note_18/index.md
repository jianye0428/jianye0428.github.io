# CMake | [18] 利用CTest进行单元测试


## 一、导言

{{<admonition quote "导言" false>}}
***\*`CTest`\**是\**`CMake`\**的测试工具，本篇通过编写和测试能够对整数求和的代码，以窥探\**`CTest`\**其中的功能一二。为了说明\**`CMake`\**没有对实际测试的语言进行任何限制，我们不仅使用\**`C++`\**可执行文件测试代码，还使用\**`Python`\**脚本和\**`shell`\**脚本作为测试代码。为了简单起见，我们将不使用任何测试库来实现，但是我们将在后面的笔记中熟悉\**`C++`\**测试框架\**。\****

{{</admonition>}}


## 二、项目结构

```shell
.
├── CMakeLists.txt
├── main.cpp
├── sum_integers.cpp
├── sum_integers.h
├── test.cpp
├── test.py
└── test.sh
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/01



<table><body text=red><tr><td style="text-align:right;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>


```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(test_ctest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PythonInterp REQUIRED)
find_program(BASH_EXECUTABLE NAMES bash REQUIRED)

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
  NAME bash_test
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:sum_up>
)

add_test(
  NAME cpp_test
  COMMAND $<TARGET_FILE:cpp_test>
)

add_test(
  NAME python_test_long
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py --executable $<TARGET_FILE:sum_up>
)

add_test(
  NAME python_test_short
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py --short --executable $<TARGET_FILE:sum_up>
)
```

{{<admonition quote "" false>}}
```c++
find_program(BASH_EXECUTABLE NAMES bash REQUIRED)
```

用于查找系统中的`bash`可执行文件，并将可执行文件的路径存储在变量`BASH_EXECUTABLE`中。这可以用于在`CMake`构建过程中执行`bash`脚本或命令。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
enable_testing()
```

测试这个目录和所有子文件夹(因为我们把它放在主`CMakeLists.txt`)。
{{</admonition>}}

{{<admonition quote "Tips" false>}}

```c++
add_test(
  NAME cpp_test
  COMMAND $<TARGET_FILE:cpp_test>
)
```

定义了一个新的测试，并设置测试名称和运行命令。

上述代码中，使用了生成器表达式:`$<TARGET_FILE:cpp_test>`。生成器表达式，是在生成**构建系统生成时**的表达式。我们将在后续的学习内容中介绍和学习生成器表达式。然后，可以声明`$<TARGET_FILE:cpp_test>`变量，将使用`cpp_test`可执行目标的完整路径进行替换。

<mark>NOTE:</mark>

生成器表达式在测试时非常方便，因为不必显式地将可执行程序的位置和名称，可以硬编码到测试中。以一种可移植的方式实现这一点非常麻烦，因为可执行文件和可执行后缀(例如，`Windows`上是`.exe`后缀)的位置在不同的操作系统、构建类型和生成器之间可能有所不同。使用生成器表达式，我们不必显式地了解位置和名称。

也可以将参数传递给要运行的`test`命令：

```c++
add_test(
  NAME python_test_short
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py --short --executable $<TARGET_FILE:sum_up>
)
```

<mark>NOTE</mark>

`CTest`可以以任何编程语言运行测试集。`CTest`关心的是，通过命令的返回码测试用例是否通过。`CTest`遵循的标准约定是：返回零意味着成功，非零返回意味着失败。可以返回零或非零的脚本，都可以做测试用例。
{{</admonition>}}

<table><body text=red><tr><td style="text-align:right;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>


**sum_integers.h**

```c++
#ifndef SUM_INTEGERS_H
#define SUM_INTEGERS_H
#include <vector>
int sum_integers(const std::vector<int> &integers);
#endif // ! SUM_INTEGERS_H
```

**sum_integers.cpp**

```c++
#include "sum_integers.h"

int sum_integers(const std::vector<int>& integers) {
  auto sum = 0;
  for (auto i : integers) {
    sum += i;
  }
  return sum;
}
```

**main.cpp**

```c++
#include <iostream>
#include <string>

#include "sum_integers.h"

int main(int argc, char *argv[]) {
  std::vector<int> integers;
  for (auto i = 1; i < argc; i++) {
    integers.push_back(std::stoi(argv[i]));
  }
  auto sum = sum_integers(integers);
  std::cout << sum << std::endl;

  return 0;
}
```

**test.cpp**

```c++
#include "sum_integers.h"

int main() {
  auto integers = {1, 2, 3, 4, 5};
  if (sum_integers(integers) == 15) {
    return 0;
  } else {
    return 1;
  }
}
```

**test.py**

```python
import subprocess
import argparse
# test script expects the executable as argument
parser = argparse.ArgumentParser()
parser.add_argument('--executable', help='full path to executable')
parser.add_argument('--short',
					default=False,
                    action='store_true',
                    help='run a shorter test')
args = parser.parse_args()
def execute_cpp_code(integers):
    result = subprocess.check_output([args.executable] + integers)
    return int(result)
if args.short:
    # we collect [1, 2, ..., 100] as a list of strings
    result = execute_cpp_code([str(i) for i in range(1, 101)])
    assert result == 5050, 'summing up to 100 failed'
else:
    # we collect [1, 2, ..., 1000] as a list of strings
    result = execute_cpp_code([str(i) for i in range(1, 1001)])
    assert result == 500500, 'summing up to 1000 failed'
```

**test.sh**

```shell
#!/usr/bin/env bash
EXECUTABLE=$1
OUTPUT=$($EXECUTABLE 1 2 3 4)
if [ "$OUTPUT" = "10" ]
then
    exit 0
else
    exit 1
fi
```



## 三、附录

**1. 考虑以下定义:**

```c++
add_test(
  NAME python_test_long
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py --executable $<TARGET_FILE:sum_up>
)
```

前面的定义可以通过显式指定脚本运行的`WORKING_DIRECTORY`重新表达，如下:

```c++
add_test(
  NAME python_test_long
  COMMAND ${PYTHON_EXECUTABLE} test.py --executable $<TARGET_FILE:sum_up>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
```

测试名称可以包含`/`字符，按名称组织相关测试也很有用，例如：

```c++
add_test(
  NAME python/long
  COMMAND ${PYTHON_EXECUTABLE} test.py --executable $<TARGET_FILE:sum_up>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
```

有时候，我们需要为测试脚本设置环境变量。这可以通过`set_tests_properties`实现:

```c++
set_tests_properties(python_test
  PROPERTIES
    ENVIRONMENT
      ACCOUNT_MODULE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
      ACCOUNT_HEADER_FILE=${CMAKE_CURRENT_SOURCE_DIR}/account/account.h
      ACCOUNT_LIBRARY_FILE=$<TARGET_FILE:account>
)
```

这种方法在不同的平台上并不总可行，`CMake`提供了解决这个问题的方法。下面的代码片段与上面给出的代码片段相同，在执行实际的`Python`测试脚本之前，通过`CMAKE_COMMAND`调用`CMake`来预先设置环境变量：

```c++
add_test(
  NAME
      python_test
  COMMAND
    ${CMAKE_COMMAND} -E env
    ACCOUNT_MODULE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    ACCOUNT_HEADER_FILE=${CMAKE_CURRENT_SOURCE_DIR}/account/account.h
    ACCOUNT_LIBRARY_FILE=$<TARGET_FILE:account>
    ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/account/test.py
)
```

同样，要注意使用生成器表达式`$<TARGET_FILE:account>`来传递库文件的位置。

**2. 不同平台测试命令**
我们已经使用`ctest`命令执行测试，`CMake`还将为生成器创建目标(`Unix Makefile`生成器为`make test`，`Ninja`工具为`ninja test`，或者`Visual Studio`为`RUN_TESTS`)。这意味着，还有另一种(几乎)可移植的方法来运行测试：

```
$ cmake --build . --target test
```

当使用`Visual Studio`生成器时，我们需要使用`RUN_TESTS`来代替:

```
$ cmake --build . --target RUN_TESTS
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_18/  

