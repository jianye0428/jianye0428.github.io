# CMake 笔记 | [23] 配置时运行自定义命令


## 一、导言

{{<admonition quote "导言" false>}}
**已经好久好久没有更新这个系列了，但是无论如何这个系列一定会以较全面的形式更新完成，只是在时间上可能比较拖沓。没有更新的原因也是最近一个月在做一个项目，没日没夜的度过了一个多月的加班加点的生活。**

**我们言归正传，通过前面的学习，我们已经了解了CMake如何在配置时运行许多子任务，以便找到工作的编译器和必要的依赖项。本篇，我们将学习使用execute_process命令在配置时运行定制化命令。**
{{</admonition>}}


## 二、项目结构

本篇比较简单，只有一个简单的CMakeLists.txt。
相关源码：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter5/01



<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>


```c++
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test_exe_proc LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

# this is set as variable to prepare for abstraction using loops or functions
set(module_name "cffi")
execute_process(
  COMMAND
      ${PYTHON_EXECUTABLE} "-c" "import ${module_name}; print(${module_name}.__version__)"
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

if(stderr MATCHES "ModuleNotFoundError")
    message(STATUS "Module ${module_name} not found")
else()
    message(STATUS "Found module ${module_name} v${stdout}")
endif()
```


{{<admonition quote "代码详解" false>}}
`execute_process`命令将从当前正在执行的`CMake`进程中派生一个或多个子进程，从而提供了在配置项目时运行任意命令的方法。可以在一次调用`execute_process`时执行多个命令。注意，每个命令的输出将通过管道传输到下一个命令中。该命令接受多个参数:

- `WORKING_DIRECTORY`，指定应该在哪个目录中执行命令。
- `RESULT_VARIABLE`将包含进程运行的结果。这要么是一个整数，表示执行成功，要么是一个带有错误条件的字符串。
- `OUTPUT_VARIABLE`和`ERROR_VARIABLE`将包含执行命令的标准输出和标准错误。由于命令的输出是通过管道传输的，因此只有最后一个命令的标准输出才会保存到`OUTPUT_VARIABLE`中。
- `INPUT_FILE`指定标准输入重定向的文件名
- `OUTPUT_FILE`指定标准输出重定向的文件名
- `ERROR_FILE`指定标准错误输出重定向的文件名
- 设置`OUTPUT_QUIET`和`ERROR_QUIET`后，CMake将静默地忽略标准输出和标准错误。
- 设置`OUTPUT_STRIP_TRAILING_WHITESPACE`，可以删除运行命令的标准输出中的任何尾随空格
- 设置`ERROR_STRIP_TRAILING_WHITESPACE`，可以删除运行命令的错误输出中的任何尾随空格

```c++
execute_process(
  COMMAND
      ${PYTHON_EXECUTABLE} "-c" "import ${module_name}; print(${module_name}.__version__)"
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)
```

该命令检查`python -c "import cffi; print(cffi.__version__)"`的输出。如果没有找到模块，`stderr`将包含`ModuleNotFoundError`，我们将在`if`语句中对其进行检查。本例中，我们将打印`Module cffi not found`。如果导入成功，`Python`代码将打印模块的版本，该模块通过管道输入`stdout`，这样就可以打印如下内容:

```c++
message(STATUS "Found module ${_module_name} v${stdout}")
```
{{</admonition>}}



<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">运行结果</font></td></tr></body></table>


```shell
mkdir build
cd build
cmake ..
-- Found PythonInterp: /usr/bin/python3.8 (found version "3.8.10")
-- Module cffi not found
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter5/01/build
sudo pip3 install cffi
mkdir build
cd build
cmake ..
-- Found module cffi v1.16.0
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter5/01/build
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">补充说明</font></td></tr></body></table>


本篇，只打印了结果，但实际项目中，可以警告、中止配置，或者设置可以查询的变量，来切换某些配置选项。



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_23/  

