# CMake 笔记 | [13] 检测python解释器和python库


## 一、检测python解释器和python库

{{<admonition quote "导言" false>}}

**python是一种非常流行的语言。许多项目用python编写的工具，从而将主程序和库打包在一起，或者在配置或构建过程中使用python脚本。这种情况下，确保运行时python解释器的依赖也需要得到满足。本篇将展示如何检测和使用python解释器。**

**除此之外，还有其他方法可以将解释语言(如python)与编译语言(如C或C++)组合在一起使用。一种是扩展python，通过编译成共享库的C或C++模块在这些类型上提供新类型和新功能。另一种是将python解释器嵌入到C或C++程序中。两种方法都需要下列条件:**

  - **python解释器的工作版本**
  - **python头文件python.h的可用性**
  - **python运行时库libpython**
{{</admonition>}}



## 二、检测python解释器

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter3/01

**CMakeLists.txt文件**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(python_interperter LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "-c" "print('Hello, python interpreter!')"
    RESULT_VARIABLE RESULT_STATUS
    OUTPUT_VARIABLE RESULT_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# message(STATUS "RESULT_VARIABLE is: ${RESULT_STATUS}")
# message(STATUS "OUTPUT_VARIABLE is: ${RESULT_OUTPUT}")

include(CMakePrintHelpers)
cmake_print_variables(RESULT_STATUS RESULT_OUTPUT)
```

{{<admonition quote "" false>}}
```c++
find_package(PythonInterp REQUIRED)
```


使用`find_package`命令找到`python`解释器。

`find_package`是用于发现和设置包的`CMake`模块的命令。这些模块包含`CMake`命令，用于标识系统标准位置中的包。`CMake`模块文件称为`Find<name>.cmake`，当调用`find_package(<name>)`时，模块中的命令将会运行。

除了在系统上实际查找包模块之外，查找模块还会设置了一些有用的变量，反映实际找到了什么，也可以在自己的`CMakeLists.txt`中使用这些变量。对于`python`解释器，相关模块为`FindPythonInterp.cmake`附带的设置了一些`CMake`变量:

- `PYTHONINTERP_FOUND`：是否找到解释器
- `PYTHON_EXECUTABLE`：`python`解释器到可执行文件的路径
- `PYTHON_VERSION_STRING`：`python`解释器的完整版本信息
- `PYTHON_VERSION_MAJOR`：`python`解释器的主要版本号
- `PYTHON_VERSION_MINOR` ：`python`解释器的次要版本号
- `PYTHON_VERSION_PATCH`：`python`解释器的补丁版本号

可以强制`CMake`，查找特定版本的包。例如，要求`python`解释器的版本大于或等于`2.7`：`find_package(PythonInterp 2.7)`。

`CMake`有很多查找软件包的模块。建议在`CMake`在线文档中查询`Find<package>.cmake`模块，并在使用它们之前详细阅读它们的文档。`find_package`命令的文档可以参考 :

> https://cmake.org/cmake/help/v3.5/command/find_ackage.html
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
execute_process(
  COMMAND
      ${PYTHON_EXECUTABLE} "-c" "print('Hello, world!')"
  RESULT_VARIABLE _status
  OUTPUT_VARIABLE _hello_world
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
```

执行`python`命令并捕获它的输出和返回值。

{{</admonition>}}

**输出**

```shell
-- Found PythonInterp: /usr/bin/python3.8 (found version "3.8.10")
-- RESULT_VARIABLE is: 0
-- OUTPUT_VARIABLE is: Hello, python interpreter!
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter3/01/build
```

**附录**

软件包没有安装在标准位置时，`CMake`无法正确定位它们。用户可以使用`-D`参数传递相应的选项，告诉`CMake`查看特定的位置。`python`解释器可以使用以下配置:

```shell
$ cmake -D PYTHON_EXECUTABLE=/custom/location/python ..
```

这将指定非标准`/custom/location/python`安装目录中的`python`可执行文件。

注意:每个包都是不同的，`Find<package>.cmake`模块试图提供统一的检测接口。当`CMake`无法找到模块包时，可以阅读相应检测模块的文档，以了解如何正确地使用`CMake`模块。可以在终端中直接浏览文档，可使用`cmake --help-module FindPythonInterp`查看。

除了检测包之外，我们还想提到一个便于打印变量的`helper`模块:

```c++
message(STATUS "RESULT_VARIABLE is: ${_status}")
message(STATUS "OUTPUT_VARIABLE is: ${_hello_world}")
```

使用以下工具进行调试:

```c++
include(CMakePrintHelpers)
cmake_print_variables(_status _hello_world)
```

将产生以下输出:

```c++
-- _status="0" ; _hello_world="Hello, world!"
```

# 三、检测python库

**项目结构**

```shell
.
├── CMakeLists.txt
└── hello_embedded_python.c
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter3/02


**CMakeLists.txt文件**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(link_python LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PythonInterp REQUIRED)

find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)

add_executable(hello_embedded_python hello_embedded_python.c)
target_include_directories(hello_embedded_python
  PRIVATE ${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(hello_embedded_python
  PRIVATE ${PYTHON_LIBRARIES}
)
```

为了确保可执行文件、头文件和库都有一个匹配的版本。这对于不同版本，可能在运行时导致崩溃。通过`FindPythonInterp.cmake`中定义的`PYTHON_VERSION_MAJOR`和`PYTHON_VERSION_MINOR`来实现:

```c++
find_package(PythonInterp REQUIRED)
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)
```

可执行文件包含`python.h`头文件。因此，这个目标的`include`目录必须包含`python`的`include`目录，可以通过`PYTHON_INCLUDE_DIRS`变量进行指定:

```c++
target_include_directories(hello_embedded_python
  PRIVATE ${PYTHON_INCLUDE_DIRS}
)
```

将可执行文件链接到`python`库，通过`PYTHON_LIBRARIES`变量访问:

```
target_link_libraries(hello_embedded_python
  PRIVATE ${PYTHON_LIBRARIES}
)
```


**相关源码**

```c++
#include <Python.h>
int main(int argc, char *argv[]) {
  /* optional but recommended */
  Py_SetProgramName(argv[0]);
  Py_Initialize();
  PyRun_SimpleString("print('Today is Tuesday!')\n");
  Py_Finalize();
  return 0;
}
```

**附录**

当`python`不在标准安装目录中，如何确定`python`头文件和库的位置是正确的？

对于`python`解释器，可以通过`-D`选项传递`PYTHON_LIBRARY`和`PYTHON_INCLUDE_DIR`选项来强制`CMake`查找特定的目录。这些选项指定了以下内容:

- `PYTHON_LIBRARY`：指向`python`库的路径
- `PYTHON_INCLUDE_DIR`：`python.h`所在的路径

这样，就能获得所需的`python`版本。

注意：有时需要将`-D PYTHON_EXECUTABLE`、`-D PYTHON_LIBRARY`和`-D PYTHON_INCLUDE_DIR`传递给`CMake CLI`，以便找到及定位相应的版本的组件。

要将`python`解释器及其开发组件匹配为完全相同的版本可能非常困难，对于那些将它们安装在非标准位置或系统上安装了多个版本的情况尤其如此。`CMake 3.12`版本中增加了新的`python`检测模块，来解决这个问题。`CMakeLists.txt`的检测部分也将简化为:

```c++
find_package(Python COMPONENTS Interpreter Development REQUIRED)
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_13/  

