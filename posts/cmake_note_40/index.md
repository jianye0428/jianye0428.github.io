# CMake 笔记 | [40] 超级构建模式


## 一、导言

{{<admonition quote "导言" false>}}
**每个项目都需要处理依赖关系，使用****`CMake`很容易查询这些依赖关系，是否存在于配置项目中。前面的笔记中，展示了如何找到安装在系统上的依赖项，到目前为止我们一直使用这种模式。但是，当不满足依赖关系，我们只能使配置失败，并向用户警告失败的原因。然而，使用`CMake`可以组织我们的项目，如果在系统上找不到依赖项，就可以自动获取和构建依赖项。后续的几篇笔记将介绍和分析`ExternalProject.cmake`和`FetchContent.cmake`标准模块，及在`超级构建模式`中的使用。前者允许在构建时检索项目的依赖项，后者允许我们在配置时检索依赖项(`CMake`的`3.11`版本后添加)。使用超级构建模式，我们可以利用`CMake`作为包管理器：相同的项目中，将以相同的方式处理依赖项，无论依赖项在系统上是已经可用，还是需要重新构建。**

**首先通过一个简单示例，介绍超级构建模式。我们将展示如何使用****`ExternalProject_Add`命令来构建一个的`hello_world`程序。**
{{</admonition>}}


## 二、项目结构

```shell
.
├── CMakeLists.txt
└── src
      ├── CMakeLists.txt
      └── hello-world.cpp
```

项目结构：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter8/01



## 三、相关源码

**src/CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(example_core LANGUAGES CXX)

add_executable(hello-world hello_world.cpp)
```

**src/hello_world.cpp**

```c++
#include <cstdlib>
#include <iostream>
#include <string>
std::string say_hello() {
  return std::string("Hello, CMake superbuild world!");
}

int main() {
  std::cout << say_hello() << std::endl;
  return EXIT_SUCCESS;
}
```

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)

include(ExternalProject)

ExternalProject_Add(${PROJECT_NAME}_core
  SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/src
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
    -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  BUILD_ALWAYS
    1
  INSTALL_COMMAND
    ""
  )
```

{{<admonition quote "" false>}}
```c++
set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)
```

为当前目录和底层目录设置`EP_BASE`目录属性。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
include(ExternalProject)
```

包括`ExternalProject.cmake`标准模块。该模块提供了`ExternalProject_Add`函数。
{{</admonition>}}

{{<admonition quote "" false>}}
```c++
ExternalProject_Add(${PROJECT_NAME}_core
  SOURCE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/src
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
    -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  BUILD_ALWAYS
    1
  INSTALL_COMMAND
    ""
  )
```

`Hello, World`源代码通过调用`ExternalProject_Add`函数作为外部项目添加的。外部项目的名称为`example_core`。

`ExternalProject_Add`命令可用于添加第三方源。本篇通过将自己的项目，分为不同`CMake`项目的集合管理。本例中，主`CMakeLists.txt`和子`CMakeLists.txt`都声明了一个`CMake`项目，它们都使用了`project`命令。

`ExternalProject_Add`有许多选项，可用于外部项目的配置和编译等所有方面。这些选择可以分为以下几类:

- Directory:用于调优源码的结构，并为外部项目构建目录。本篇，我们使用`SOURCE_DIR`选项让`CMake`知道源文件在`${CMAKE_CURRENT_LIST_DIR}/src`文件夹中。用于构建项目和存储临时文件的目录，也可以在此类选项或目录属性中指定。通过设置`EP_BASE`目录属性，`CMake`将按照以下布局为各个子项目设置所有目录:

  ```
  TMP_DIR = <EP_BASE>/tmp/<name>
  STAMP_DIR = <EP_BASE>/Stamp/<name>
  DOWNLOAD_DIR = <EP_BASE>/Download/<name>
  SOURCE_DIR = <EP_BASE>/Source/<name>
  BINARY_DIR = <EP_BASE>/Build/<name>
  INSTALL_DIR = <EP_BASE>/Install/<name>
  ```

- Download：外部项目的代码可能需要从在线存储库或资源处下载。

- Update和Patch：可用于定义如何更新外部项目的源代码或如何应用补丁。

- Configure：默认情况下，`CMake`会假定外部项目是使用`CMake`配置的。如下所示，我们并不局限于这种情况。如果外部项目是`CMake`项目，`ExternalProject_Add`将调用`CMake`可执行文件，并传递选项。对于本篇示例，我们通过`CMAKE_ARGS`和`CMAKE_CACHE_ARGS`选项传递配置参数。前者作为命令行参数直接传递，而后者通过`CMake`脚本文件传递。实际，脚本文件位于`build/subprojects/tmp/example_core/example_core- cache-.cmake`。然后，配置如以下所示:

  ```shell
  loading initial cache file /home/jiangli/repo/tutorials/cmake-tutorial/chapter8/01/build/subprojects/tmp/example_core/example_core-cache-.cmake
  -- The CXX compiler identification is GNU 9.4.0
  -- Check for working CXX compiler: /usr/bin/c++
  -- Check for working CXX compiler: /usr/bin/c++ -- works
  -- Detecting CXX compiler ABI info
  -- Detecting CXX compiler ABI info - done
  -- Detecting CXX compile features
  -- Detecting CXX compile features - done
  -- Configuring done
  -- Generating done
  -- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter8/01/build/subprojects/Build/example_core
  ```

- Build：可用于调整外部项目的实际编译。我们使用`BUILD_ALWAYS`选项确保外部项目总会重新构建。

- Install：这些选项用于配置应该如何安装外部项目。我们将`INSTALL_COMMAND`保留为空。

- Test：为基于源代码构建的软件运行测试。`ExternalProject_Add`的这类选项可以用于此目的。我们的没有使用这些选项，因为`Hello, World`示例没有任何测试。
{{</admonition>}}

## 四、结果展示

```shell
$ mkdir -p build
$ cmake ..
$ cmake --build .
```

构建目录的结构稍微复杂一些，`subprojects`文件夹的内容如下:

```shell
build/subprojects/
├── Build
│    └── example_core
│        ├── CMakeCache.txt
│        ├── CMakeFiles
│        ├── cmake_install.cmake
│        ├── hello-world
│        └── Makefile
├── Download
│    └── example_core
├── Install
│    └── example_core
├── Stamp
│    └── exampleq_core
│        ├── example_core-configure
│        ├── example_core-done
│        ├── example_core-download
│        ├── example_core-install
│        ├── example_core-mkdir
│        ├── example_core-patch
│        └── example_core-update
└── tmp
    └── example_core
        ├── example_core-cache-.cmake
        ├── example_core-cfgcmd.txt
        └── example_core-cfgcmd.txt.in
```

**补充内容**

`ExternalProject.cmake`定义了`ExternalProject_Get_Property`命令，该命令对于检索外部项目的属性非常有用。外部项目的属性是在首次调用`ExternalProject_Add`命令时设置的。例如，在配置`example_core`时，检索要传递给`CMake`的参数可以通过以下方法实现:

```c++
ExternalProject_Get_Property(${PROJECT_NAME}_core CMAKE_ARGS)
message(STATUS "CMAKE_ARGS of ${PROJECT_NAME}_core ${CMAKE_ARGS}")
```

`ExternalProject.cmake`模块定义了以下附加命令:

- `ExternalProject_Add_Step`: 当添加了外部项目，此命令允许将附加的命令作为自定义步骤锁定在其上。
- `ExternalProject_Add_StepTargets`:允许将外部项目中的步骤(例如：构建和测试步骤)定义为单独的目标。这意味着可以从完整的外部项目中单独触发这些步骤，并允许对项目中的复杂依赖项，进行细粒度控制。
- `ExternalProject_Add_StepDependencies`:外部项目的步骤有时可能依赖于外部目标，而这个命令的设计目的就是处理这些情况。



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_40/  

