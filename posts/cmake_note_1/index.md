# CMake 笔记 | [1] 单个源文件编译为可执行文件


### 一、项目结构

```txt
..

├── CMakeLists.txt
└── hello_world.cpp

0 directories, 2 files
```

本项目只包含了一个源文件hello_world.cpp和一个CMake文件。
> 源码地址: https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter1/01


{{<admonition quote "注意" true>}}
注意: CMake文件的名字只能是CMakeLists.txt，当然如果使用include命令（后续会讲到），可以以任何名字进行命名，只要以.cmake结尾即可。
{{</admonition>}}

### 二、CPP源文件

```c++

#include <cstdlib>
#include <iostream>
#include <string>

std::string SayHello();

int main() {
  std::cout << SayHello() << std::endl;
  return 0;
}

std::string SayHello() { return std::string("Hello, CMake World!"); }
```

### 三、CMake文件

```cmake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    hello_world.cpp
}
```

- **`cmake_minimum_required(VERSION 3.10 FATAL_ERROR)`**

  设置CMake所需的最低版本的最低版本，如果使用的CMake版本低于该版本，则会发出致命错误。
  今后，笔记中CMake的版本要求都为3.10。vscode中CMake Tool插件使用debug功能要求CMake版本不低于3.10。如果使用的系统ubuntu 16.04的话，需要升级CMake版本，windows请自行安装高于3.10版本的CMake。

- **`project(hello-world LANGUAGES CXX)`**
  声明了项目的名称和支持的编程语言，且该命令必须生命在cmake_minimum_required之后。一旦声明了项目的名称后，可以使用宏定义${PROJECT_NAME}进行调用。
  在CMake中，C++是默认的编程语言。不过在实际编写代码过程中，仍建议使用LANGUAGES选项在project命令中显示地声明项目的语言。

- `add_executable(${PROJECT_NAME} hello_world.cpp}`
  CMake创建一个新可执行文件，其名字为${PROJECT_NAME}（也可以使用其他任意的名字）。这个可执行文件是通过编译和链接源文件hello_world.cpp生成的。

  CMake将为编译器使用默认设置，并自动生成工具。

### 四、操作

在工程文件夹下，执行如下操作:
```shell
mkdir build
cd build
cmake ..
```

输出结果:
```shell
-- The CXX compiler identification is GNU 9.4.0
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/01/build
```

CMake的构建方式有两种: **源内构建**和**源外构建**。源内构建即在CMakeLists.txt同级目录中进行构建，构建出来的CMake文件将在该目录中，<font color=red><u>这通常是不推荐的，因为这会混合源代码和项目的目录树</u></font>。源外构建的构建方式，就是笔记的构建方式，往后我们都采用源外构建的方式。

CMake是一个构建系统生成器。将描述构建系统，如Unix Makefile、Ninja、sln等应当如何操作才能编译代码。然后，CMake为所选的构建系统生成相应的指令。默认情况下，在GNU/Linux和macOS系统上，CMake使用Unix Makefile生成器。Windows上，sln是默认的生成器。

GNU/Linux上，CMake默认生成 Makefile来构建项目:
  1. **Makefile**: make将运行指令来构建项目。
  2. **CMakefile**: 包含临时文件的目录，CMake用于检测操作系统、编译器等。此外，根据所选的生成器，它还包含特定的文件。
  3. **cmake_install.cmake**: 处理安装规则的CMake脚本，在项目安装时使用（命令为执行为make后执行make install）。
  4. **CMakeCache.txt**: 如文件名所示，CMake缓存。CMake在重新运行配置时使用这个文件。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_1/  

