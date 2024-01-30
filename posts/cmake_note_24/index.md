# CMake 笔记 | [24] 构建时运行自定义命令add_custom_command


## 一、导言

{{<admonition quote "" false>}}

**项目的构建目标取决于命令的结果，这些命令只能在构建系统生成完成后的构建执行。CMake提供了三个选项来在构建时执行自定义命令:**

- **使用add_custom_command编译目标，生成输出文件。**
- **add_custom_target的执行没有输出。**
- **构建目标前后，add_custom_command的执行可以没有输出。**

**这三个选项强制执行特定的语义，并且不可互换。接下来的我们将分别学习具体的用法。**

{{</admonition>}}

## 二、项目结构


本项目比较简单，我们通过对`add_custom_command`的简单使用，来探索它的功能。

```shell
.
├── CMakeLists.txt
├── config
│   └── config.txt
└── test.cpp
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter5/02


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.0)

project(custom_command_example)

# Set static library to lib file
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# Set dynamic library to lib file
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# Set dynamic runtime library or exetuable file to bin file
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} test.cpp)

if (MSVC)
  file(GLOB config_file "${CMAKE_SOURCE_DIR}/config/*.txt")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${config_file}
                     $<TARGET_FILE_DIR:${PROJECT_NAME}>)
elseif(UNIX)
  file(GLOB config_file "${CMAKE_SOURCE_DIR}/config/*.txt")
  file(COPY ${config_file} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
```

{{<admonition quote "" false>}}
`add_custom_command` 是 `CMake` 中的一个命令，<font color=green>用于向构建系统添加自定义的构建规则或自定义命令</font>。这可以用于执行各种任务，如生成源文件、拷贝文件、执行脚本等。它允许在 `CMake` 构建过程中定义一些额外的操作。
{{</admonition>}}


以下是 `add_custom_command` 命令的一般语法和参数：

```c++
add_custom_command(
    OUTPUT output1 [output2...]
    COMMAND command1 [ARGS] [command2 [ARGS] ...]
    [MAIN_DEPENDENCY depend]
    [DEPENDS depend [depend ...]]
    [WORKING_DIRECTORY dir]
    [COMMENT comment]
    [VERBATIM]
    [APPEND]
    [USES_TERMINAL]
)
```

- `OUTPUT output1 [output2...]`: 指定命令执行后生成的输出文件。这些文件通常是构建过程的目标，可以是可执行文件、库文件、数据文件等。
- `COMMAND command1 [ARGS] [command2 [ARGS] ...]`: 定义要执行的命令。这可以是外部命令、脚本或自定义操作。
- `MAIN_DEPENDENCY depend`: 指定主要的依赖项，通常是影响命令执行的文件。如果 `depend` 被修改，命令将重新运行。
- `DEPENDS depend [depend ...]`: 指定其他依赖项。这些文件会触发命令重新运行，如果它们被修改。
- `WORKING_DIRECTORY dir`: 指定命令执行的工作目录。
- `COMMENT comment`: 可选，用于描述自定义命令的文本注释。
- `VERBATIM`: 可选，告诉 CMake 保持命令的参数不变，不进行任何转义。
- `APPEND`: 可选，将新的自定义命令追加到同一输出文件上。
- `USES_TERMINAL`: 可选，指示命令是否使用终端。

通常，`add_custom_command` 用于在构建期间执行一些非标准的操作，例如生成代码、转换文件格式、运行测试或其他自定义任务。这可以帮助您在 CMake 构建系统中添加额外的步骤，以满足项目的特定需求。

本项目需要在`Windows`系统中构建，以探究`add_custom_command` 命令的功能。该命令在本项目中的主要功能是将根目录下`config`·文件夹下的所有`txt`本文文件拷贝到可执行文件所在的目录。

之所以在`Windows`中的命令和`Linux`中的命令不一样，根本原因是由于操作系统的文件系统的不同造成的。


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>

**test.cpp**

```c++
#include <fstream>
#include <iostream>
#include <string>

int main() {
  // 文件路径
  std::string file_name = "config.txt";

  // 打开文件
  std::ifstream file(file_name);

  // 检查文件是否成功打开
  if (!file.is_open()) {
    std::cerr << "无法打开文件: " << file_name << std::endl;
    return 1;
  }

  std::string line;

  // 逐行读取文件内容
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }

  // 关闭文件
  file.close();

  return 0;
}
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">运行结果</font></td></tr></body></table>

```
version: 0.0.1
author: jiangli
email: 1316762810@qq.com
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_24/  

