# CMake 笔记 | [2] 多目录多文件的CMake构建方式


### 一、项目结构

```shell
.
├── include
│   └── message.h
├── src
│   └── message.cpp
├── hello_world.cpp
└── CMakeLists.txt
```

项目结构是为了让我们开发人员对项目更加清晰，使代码结构更加清晰（模块化）。一般我们的项目比较简单时，可以构建为如上的项目结构。但是在构建大型项目时，项目结构会更加复杂，具体请参考下节内容。

这里我们构建了include目录和src目录，include目录主要存放的是CPP文件的头文件，即函数的声明，为使用它的文件提供API。src目录主要是存放的函数的具体实现。

> 源码地址:

### 二、message.h

```c++

#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_

#include <iostream>
#include <string>

class Message {
 public:
  Message() {}

  void Print(const std::string &message);
};
#endif // ! MESSAGE_HEADER_H_
```
### 三、message.cpp

```c++
#include "message.h"

void Message::Print(const std::string &message) {
  std::cout << message << std::endl;
}
```

### 四、hello_world.cpp

```c++
#include "message.h"

int main() {
  Message message;
  message.Print("Hello, CMake World!");
  message.Print("Goodbye, CMake World!");

  return EXIT_SUCCESS;
}
```

### 五、CMake文件

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

file(GLOB HEADER ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(
    ${PROJECT_NAME}
    ${HEADER}
    ${SOURCE}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)
```

**代码释义:**

  {{<admonition quote "Tip" false>}}
  ```CMake
  include_directories(
      ${CMAKE_SOURCE_DIR}/include
  )
  ```
  将`include`目录下的所有文件包含进来，这样include目录下的message.h将会被包含到整个项目中。如果我们在细分目录中使用包含某一模块的头文件，我们可以在具体模块的CMakeLists.txt中使用该命令，且要包含的头文件的可见性只有该模块，其他模块不可见，具体使用方法，请参考下节内容。
  {{</admonition>}}

  {{<admonition quote "Tip" false>}}
  ```CMake
  file(GLOB HEADER ${CMAKE_SOURCE_DIR}/include/*.h)
  file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/src/*.cpp)
  ```
  如果`include`目录和`src`目录中有多个头文件和源文件，使用如上命令可以将所有头文件集合到HEADER和SOURCE自定义宏定义中，使用时的命令为`${HEADER}`和`${SOURCE}`。
  {{</admonition>}}

  {{<admonition quote "Tip" false>}}
  ```CMake
  add_executable(
      ${PROJECT_NAME}
      ${HEADER}
      ${SOURCE}
      ${CMAKE_SOURCE_DIR}/hello_world.cpp
  )
  ```
  这将结合`include`目录下的文件和src目录下的文件以及`hello_world.cpp`生成名为`hello-world`的可执行文件。
  {{</admonition>}}

### 六、构建及编译

```shell
mkdir build
cd build
cmake ..
```

构建过程:

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
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/02/build
```

构建可执行文件并输出:
```shell
make
```

```shell
Scanning dependencies of target hello-world
[ 33%] Building CXX object CMakeFiles/hello-world.dir/src/message.cpp.o
[ 66%] Building CXX object CMakeFiles/hello-world.dir/hello_world.cpp.o
[100%] Linking CXX executable hello-world
[100%] Built target hello-world
```
上一篇我们没有讲将执行cmake命令后生成的MakeFile文件，其如何构建出可执行文件的具体操作，只是简单的说MakeFile需要make命令执行。

在我们执行完cmake ..后，将生成MakeFile文件，然后执行make后便可以生成可执行文件。

这里我们进行补充说明：如果我们在GNU/Linux上，执行CMake ..后会生成MakeFile文件，然后执行make命令即可生成可执行文件；在Windows上，执行cmake ..后会生成sln文件，需要使用VS进行打开，然后对其进行生成操作。Windows生成sln文件后的具体操作过程请参考最后一些补充内容。除此之外，我们可以执行以下命令，不分平台直接构建出可执行文件:

```shell
cmake --build .
```

### 七、静态库和动态库简介

首先，如果对程序的生命周期的不清楚，请先移步[这里](https://mp.weixin.qq.com/s?__biz=MzkxMzI5Mjk4Mg==&mid=2247483658&idx=1&sn=70144d3480b23fe13f447e3ef715a6a9&chksm=c17eaad3f60923c50560d65017f44bff5e95b79765d4fce5e3f0f2f586c95799d1d671c4d945&token=1465071518&lang=zh_CN&scene=21#wechat_redirect)进行学习。

#### 7.1 静态库

在链接阶段，会将汇编生成的目标文件`.o`与引用到的库**一起链接打包到可执行文件中**。这个链接方式为**静态链接**，所需要的.o（unix系统）称为静态库。
  - 静态库对函数库的链接是放在编译时期完成的。
  - 程序在运行时与函数库再无瓜葛，移植方便。
  - 浪费空间和资源，因为所有相关的目标文件与牵涉到的函数库被链接成一个可执行文件。
  - 静态库对程序的更新、部署和发布会带来麻烦。如果静态库libxxx.o更新了，所有使用它的应用程序都需要重新编译、发布给用户。

#### 7.2 动态库

动态库在程序编译时并不会链接到目标代码中，而是**在程序运行时才被载入**。不同的应用程序如果调用相同的库，那么在内存只需要有一份该共享库的实例，规避了空间浪费。

动态库在程序运行时才被载入，也解决了静态库对程序的更新、部署和发布带来的问题，用户只需要更新动态库即可，增量更新。

Windows与Linux执行文件格式不同，在创建动态库的时候有一些差异：
  - 在Windows系统下的执行文件格式是PE格式，动态库需要一个DllMain函数做初始化的入口，通常在导出函数的声明时需要有_declspec(dllexport)关键字。
  - Linux下gcc编译的执行文件默认是ELF格式，不需要初始化入口，亦不需要函数做特别的声明，编写比较方便

### 八、构建和链接静态库

#### 8.1 项目结构

```shell
.
├── message-module
│   ├── include
│   │   └── message.h
│   ├── src
│   │   └── message.cpp
│   └── CMakeLists.txt
├── hello_world.cpp
└── CMakeLists.txt
```

在实际的项目开发过程中，我们的项目结构往往会由很多个模块组成，每个模块通过一个单独的CMakeLists.txt去控制，最后在根目录下的CMakeLists.txt中将各个模块组合使用。

本项目中为了简化学习，只构建了一个message-module模块，构建多个模块的方式同理。其中项目中的所有CPP源文件与第一节内容相同，这里就不展开描述了。

源码地址:https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_2/message_static_lib

#### 8.2 message-module目录下的CMakeLists

```CMake
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(
    test_message
    STATIC
    ${HEADER}
    ${SOURCE}
)
```

**代码释义:**
{{<admonition quote "Tip" false>}}
```CMake
add_library(
    test_message
    STATIC
    ${HEADER}
    ${SOURCE}
)
```
`add_library`生成必要的构建指令，将指定的源码编译到库中。第一个参数是目标名。整个项目中，可使用相同的名称来引用库。生成的库的实际名称将由CMake通过在前面添加前缀lib和适当的扩展名作为后缀来形成。生成库是根据第二个参数(STATIC或SHARED)和操作系统确定的，本项目是将目标文件生成静态库。
{{</admonition>}}

#### 8.3 根目录下的CMakeLists

```CMake

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

# 设置可执行文件到bin文件夹下
set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

# 设置静态库到lib文件夹下
set(LIB_FILE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${LIB_FILE})

include_directories(
    ${CMAKE_SOURCE_DIR}/message-module/include
)

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/message-module
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    test_message
)
```
**代码释义:**

{{<admonition quote "Tip" false>}}
```CMake
# 设置可执行文件到bin文件夹下
set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

# 设置静态库到lib文件夹下
set(LIB_FILE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${LIB_FILE})
```
在构建项目时，我们为了使得项目结构更加清晰，使得生成的可执行文件、静态库以及动态库等文件能够存放在合适的位置。这样的构建方式有助于我们在项目重构、项目优化、debug的时候逻辑更加清晰。
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```CMake
include_directories(
    ${CMAKE_SOURCE_DIR}/message-module/include
)
```
这个命令同第一节内容，因为hello_world.cpp要使用message-module模块的API，且与该CMakeLists.txt在相同层级的目录，所以需要将message-module模块的API包含进去。
{{</admonition>}}

如果hello_world.cpp中使用到了多个模块，则此处可以包含多个模块的API：
```CMake
include_directories(
    ${CMAKE_SOURCE_DIR}/message-module/include
    ${CMAKE_SOURCE_DIR}/xxx-module/include
)
```

{{<admonition quote "Tip" false>}}
```CMake
add_subdirectory(
    ${CMAKE_SOURCE_DIR}/message-module
)
```
将我们的message-module添加进来进行编译，这个函数命令将寻找message-module目录下的CMakeLists.txt，如果该目录下没有CMakeLists.txt将报错。
{{</admonition>}}

由于在本项目中，hello_world.cpp要使用message-module模块中编译生成的静态库，所以add_subdirectory命令将message-module添加到项目中, add_subdirectory的顺序必须要先于add_executable命令。

{{<admonition quote "Tip" false>}}
```CMake
add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    test_message
)
```
`add_executable`命令将`hello_world.cpp`编译成可执行文件，其名字为项目名称`hello-world`，该可执行文件使用`target_link_libraries`命令将`message-module`模块下编译生成的静态库`test_message`链接到可执行文件中。

注意：在子模块message-module中编译生成的test_message是全局可见的，即任何模块或者根目录下的CMakeLists.txt都可以直接使用test_message进行调用。
{{</admonition>}}

#### 8.4 构建以及编译

```shell
mkdir build
cd build
cmake ..
```

构建及编译过程:

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
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/03/build
```

```shell
# cmake --build .
make
```

```
Scanning dependencies of target test_message
[ 25%] Building CXX object message-module/CMakeFiles/test_message.dir/src/message.cpp.o
[ 50%] Linking CXX static library ../lib/libtest_message.a
[ 50%] Built target test_message
Scanning dependencies of target hello-world
[ 75%] Building CXX object CMakeFiles/hello-world.dir/hello_world.cpp.o
[100%] Linking CXX executable bin/hello-world
[100%] Built target hello-world
```

可以通过编译日志看到，首先编译了`message-module`模块，并将编译生成的`libtest_message.a`存档到了`../lib/`，即build文件夹中的lib目录中。然后链接`hello-world`所需要的依赖项，此时便将`test-message`链接到了`hello-world`中，最终生成可执行文件`hello-world`，并将其存放到`bin`目录中，即build文件夹下的bin目录。

### 九、只链接链接三方库（静态）

我们在构建实际项目过程中，一个项目往往需要链接许多的三方库，抑或是我们将自己的算法以静态库的形式发布，通常需要为我们的项目链接三方库。本节讲其中的一种，后续涉及到三方库的链接将讲述所有链接的方式。关于`third-party`模块下`include`文件夹下的`message.h`头文件与前面相同，lib文件夹下的`libtest_message.a`是第三节编译生成的静态库。

#### 9.1 项目结构

```shell
.
├── third-party
│   ├── include
│   │   └── message.h
│   └── lib
│       └── libtest_message.a
├── hello_world.cpp
└── CMakeLists.txt
```

一般，我们将三方库放到项目中一个`third-party`的文件夹下，当然你也可以随意命名。三方库`third-party`中包含`include`和`lib`分别存放三方库的API和静态库。

源码地址:https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_2/message_static_lib_third_party

#### 9.2 CMakeLists.txt

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

# 设置可执行文件到bin文件夹下
set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

set(TEST_MESSAGE ${CMAKE_SOURCE_DIR}/third-party/lib/libtest_message.a)

include_directories(
    ${CMAKE_SOURCE_DIR}/third-party/include
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    ${TEST_MESSAGE}
)
```

代码释义:

{{<admonition quote "Tip" false>}}
```CMake
set(TEST_MESSAGE ${CMAKE_SOURCE_DIR}/third-party/lib/libtest_message.a)
```
将三方库中的静态库定义为TEST_MESSAGE，方便后续使用${TEST_MESSAGE}进行调用。当然你也可以直接在target_link_libraries命令中使用${CMAKE_SOURCE_DIR}/third-party/lib/libtest_message.a进行链接，但是这么做是不推荐的。如果多个模块都使用到了该库，那么定义为宏的方式更加方便和清晰。

今后，我们都将定义出来的宏统一采用了大写，意和CMake自身变量命名对其。
{{</admonition>}}

#### 9.3 构建及编译

```shell
mkdir build
cd build
cmake ..
```
构建及编译过程:

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
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/04/build
```

```shell
cmake --build .
```

```shell
Scanning dependencies of target hello-world
[ 50%] Building CXX object CMakeFiles/hello-world.dir/hello_world.cpp.o
[100%] Linking CXX executable bin/hello-world
[100%] Built target hello-world
```


### 十、编译和连接动态库

动态库的编写需要区分平台，在GNU/Linux平台上，动态库的编写和调用与静态库没有差别，但是在Windows平台上动态库的编写和调用需要做一定的修改。

#### 10.1 GNU/Linux平台上动态库的编译和链接

在GNU/Linux上生成动态库的方法和静态库生成的方法类似，其目录结构等都与静态库相同，只有在使用`add_library`命令时，参数`STATIC`改为`SHARE`即可,相关项目结构和`CMakeLists.txt`如下。

```shell
.
├── message-module
│   ├── include
│   │   └── message.h
│   ├── src
│   │   └── message.cpp
│   └── CMakeLists.txt
├── hello_world.cpp
└── CMakeLists.txt
```

源码地址: https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_2/message_dynamic_lib

配置动态库的`CMakeLists.txt`:

```shell
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

# 设置可执行文件到bin文件夹下
set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

# 设置动态库到lib文件夹下
set(LIB_FILE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${LIB_FILE})

include_directories(
    ${CMAKE_SOURCE_DIR}/message-module/include
)

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/message-module
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    test_message
)
```

这里我们设置动态库存放的路径的宏为CMAKE_LIBRARY_OUTPUT_DIRECTORY。

源码地址：


#### 10.2 GNU/Linux只链接三方库（动态库）

在`GNU/Linux`上链接动态库的方法和静态库生成的方法类似，其目录结构等都与静态库相同，只有在使用add_library命令时，参数STATIC改为SHARE即可,相关项目结构和`CMakeLists.txt`如下。

目录结构:
```shell
.
├── third-party
│   ├── include
│   │   └── message.h
│   └── lib
│       └── libtest_message.so
├── hello_world.cpp
└── CMakeLists.txt
```
源码地址: https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_2/message_dynamic_lib_third_party

`CMakeLists.txt`文件:

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

# 设置可执行文件到bin文件夹下
set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

set(TEST_MESSAGE ${CMAKE_SOURCE_DIR}/third-party/lib/libtest_message.so)

include_directories(
    ${CMAKE_SOURCE_DIR}/third-party/include
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    ${TEST_MESSAGE}
)
```

#### 10.3 Windows平台上动态库的编译和链接

注意：通过实践发现，Windows中`CMAKE_LIBRARY_OUTPUT_DIRECTORY`没有作用。在Windows中生成的动态库将会自动生成到可执行文件所在的目录。

前面我们说Windows平台中生成动态库的源码和静态库是不同的，在Windows平台中，在导出动态库时除了会生成`.dll`动态库之外，还会生成一个`.lib`文件。注意，这个`.lib`文件和静态库的`.lib`文件是不同的，它里面并不保存代码生成的二进制文件，而是所有需要导出符号的符号表。因此这个`.lib`文件和编译生成的静态库`.lib`相比较而言会小的多。

符号表是需要我们在编写源码时进行指定的，如果我们将一个符号导出（符号可以指类、函数等各种类型）,需要在其前面加上`__declspec(dllexport)`标志，这样这个符号的相关信息就会在导出的`.lib`中的符号表中了。

如果在源码中没有任何的`__declspec(dllexport)`,依然可以成功的编译出动态库，但是并不会生成保存符号表的`.lib`文件。

```c++
class __declspec(dllexport) Message {

public:
  Message() {}

    void Print(const std::string &message);
};
```
除了导出符号标识符`__declspec(dllexport)`以外，作为用户使用动态库的时候，对应的头文件的符号还需要`__declspec(dllimport)`标识符来表示这个符号是从动态库导入的。

```c++
class __declspec(dllimport) Message {

public:
  Message() {}

    void Print(const std::string &message);
};
```

一般，一个库文件我们并不想对导入和导出分别写两个几乎同样的头文件，因此可以使用宏定义来代替直接使用`__declspec(dllexport)`和`__declspec(dllimport)`关键字。

```c++

#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_

#ifdef SHARED_LIB_EXPORT

#define SHARED_LIB_EXPORT __declspec(dllexport)

#else

#define SHARED_LIB_EXPORT __declspec(dllimport)

#endif

class SHARED_LIB_EXPORT Message {

public:
  Message() {}
    void Print(const std::string &message);
};

#endif // ! MESSAGE_HEADER_H_
```

这样我们只需要在编译（导出）这个库的时候，给编译器添加`SHARED_LIB_EXPORT`宏。而在使用该库的时候什么都不定义即可。

<font color=red>我们通常编写一个头文件来专门管理`SHARED_LIB_EXPORT`宏定义。</font>为了使得我们的代码在Linux中平台以及静态库的情况，我们的头文件编写如下：

```c++
#ifndef EXPORT_LIB_HEADER_H_
#define EXPORT_LIB_HEADER_H_

#ifdef SHARED_LIB_BUILD
#ifdef _WIN32
#ifdef SHARED_LIB_EXPORT
#define SHARED_LIB_API __declspec(dllexport)
#else
#define SHARED_LIB_API __declspec(dllimport)
#endif  // SHARED_LIB_EXPORT
#else
#define SHARED_LIB_API
#endif  // _WIN32
#else
#define SHARED_LIB_API
#endif  // SHARED_LIB_BUILD

#endif // ! EXPORT_LIB_HEADER_H_
```

我们除了使用`SHARED_LIB_API`宏定义来判断是否导出为动态库以外，还使用了编译器自带的`_WIN32`宏来判断是实在windows平台上以及使用。`SHARED_LIB_BUILD`来判断是否正在编译动态库。

有了这个头文件之后，我们只需要在导出符号表的头文件中包含该头文件，就可以使用`SHARED_LIB_API`宏定义了。

除此之外，上述的头文件可以通过CMake提供的`GenerateExportHeader`命令自动生成。关于该命令的使用在后续介绍中会详细的进行探索。

**项目结构:**
```shell
.
├── message-module
│   ├── include
│   │  └── message.h
│   ├── src
│   │   └── message.cpp
│   └── CMakeLists.txt
├── hello_world.cpp
└── CMakeLists.txt
```

源码地址:https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_2/message_shared_lib_in_windows

**message.h**

```c++

#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_

#include <iostream>
#include <string>

class __declspec(dllexport) Message {
 public:
  Message() {}

  void Print(const std::string &message);
};
#endif // ! MESSAGE_HEADER_H_
```

**message.cpp**

```c++
#include "message.h"

void Message::Print(const std::string &message) {
  std::cout << message << std::endl;
}
```

**message-module下的CMakeLists.txt**

```CMake
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

file(GLOB HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(test_message SHARED ${SOURCE})

add_library(test_message::test_message ALIAS test_message)

target_include_directories(test_message
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    $<INSTALL_INTERFACE:include>
)

set_target_properties(test_message PROPERTIES
    CXX_STANDARD 11
    CMAKE_CXX_STANDARD_REQUIRED True
)

install(TARGETS test_message
        EXPORT message_export_target
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION "include"
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION "include"
        FILES_MATCHING PATTERN "*.hpp")
```

代码释义:

{{<admonition quote "Tip" false>}}
```CMake
add_library(test_message::test_message ALIAS test_message)
```
添加别名，以便库可以在构建树中使用，例如在测试时。
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```CMake
target_include_directories(test_message
    PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    $<INSTALL_INTERFACE:include>
)
```
设置 test_message 的包含路径。其中，`PUBLIC` 表示这些头文件路径将会被暴露给该目标的依赖项，即其他目标可以通过依赖该目标来访问这些头文件路径。
`BUILD_INTERFACE`表示在构建时使用的头文件路径;INSTALL_INTERFACE表示在安装时使用的头文件路径，即将该目标安装到其他地方时，头文件将会被安装到 include 目录下。
综上，这段代码的作用是将当前项目的根目录添加到 “test_message” 目标的头文件包含路径中，以便在编译和安装时能够正确地访问这些头文件。
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```CMake
set_target_properties(test_message PROPERTIES
    CXX_STANDARD 11
    CMAKE_CXX_STANDARD_REQUIRED True
)
```
设置 `test_message` 的相关属性，关于`target_include_directories`和`set_target_properties`的具体使用情况，我们将在以后做详细讲解。
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```CMake
install(TARGETS test_message
        EXPORT message_export_target
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib")
```
安装一个名为 “test_message” 的目标（在Windows上需要将编译出来的sln文件使用vs打开，然后再属性栏中的INSTALL右键生成，即可执行install命令）。其中，TARGETS 表示要安装的目标名称，即 test_message。
`EXPORT message_export_target` 表示将该目标导出到一个名为 “message_export_target” 的 CMake 配置文件中，以便其他项目可以使用该目标。
**注意:** 我们在根目录下已经指定了’install’安装目录的根目录。
`RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/output/bin`表示将该目标的可执行文件安装到当前项目的 bin 目录下。
`LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/output/lib`表示将该目标的共享库文件安装到 当前项目的 lib 目录下。
`ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/output/lib`)表示将该目标的静态库文件安装到 当前项目的 lib 目录下。
**注意:** 在Windows构建动态库时，动态库dll会被安装到当前项目的bin目录下，目标文件.lib文件会被安装到项目的lib目录下。
综上，这段代码的作用是将 “test_message” 目标的可执行文件、共享库文件和静态库文件安装到指定的目录下，并将该目标导出到一个 CMake 配置文件中，以便其他项目可以使用该目标。关于生成cmake相关配置文件详见下一篇内容。本项目仅用此命令做输出库的相关文件的功能。
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```c++
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_SOURCE_DIR}/output/include
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_SOURCE_DIR}/output/include
        FILES_MATCHING PATTERN "*.hpp")
```
将当前目录下的`include`目录中的`.hpp`和`.h`安装到当前项目下的`output`目录下的`include`文件中。关于install命令的具体使用方法，我们将以后做详细解释。
{{</admonition>}}

**根目录下的CMakeLists.txt**

```CMake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

# 设置可执行文件到bin文件夹下
set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

# 设置动态库到lib文件夹下
set(LIB_FILE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${LIB_FILE})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
endif()

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/message-module
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    test_message
)
```

**代码释义:**

{{<admonition quote "Tip" false>}}
```CMake
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
endif()
```
如果没有指定`install`的目录，则设置install的目录为`${CMAKE_SOURCE_DIR}/output/`。
{{</admonition>}}

#### 10.4 Windows上只链接第三方库（动态库）

Windows平台上链接动态库的使用方法和GNU/Linux平台上有所不同，我们在链接时需要将符号表文件lib进行连接，然后将对应的动态库文件dll文件拷贝到环境变量或者可执行文件所在的目录下，可知行文件才可以正常过执行。

**项目结构**

```shell
.
├── third-party
│   ├── include
│   │   └── message.h
│   ├── lib
│   │   └── test_message.lib
│  └── bin
│    └── test_message.dll
├── hello_world.cpp
└── CMakeLists.txt
```

项目中使用的`third-party`使用的是上一节内容中在Windows上生成的动态库。文件hello_world.cpp与上一节内容相同，我们不再对其进行描述。

源码地址:https://github.com/jianye0428/CMake_Learning_Notes/tree/main/Note_2/message_shared_lib_third_party_in_windows

**CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(hello-world LANGUAGES CXX)

set(EXECUTE_FILE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTE_FILE})

set(TEST_MESSAGE ${CMAKE_SOURCE_DIR}/third-party/lib/test_message.lib)

include_directories(
    ${CMAKE_SOURCE_DIR}/third-party/include
)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/hello_world.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    ${TEST_MESSAGE}
)

if (MSVC)
  file(GLOB MODEL "${CMAKE_SOURCE_DIR}/third-party/bin/*.dll")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${MODEL}
                     $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
```

代码释义:

{{<admonition quote "Tip" false>}}
```CMake
set(TEST_MESSAGE ${CMAKE_SOURCE_DIR}/third-party/lib/test_message.lib)
```
链接三方库的符号表
{{</admonition>}}

{{<admonition quote "Tip" false>}}
```CMake
if (MSVC)
  file(GLOB MODEL "${CMAKE_SOURCE_DIR}/third-party/bin/*.dll")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${MODEL}
                     $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
```
将我们之前生成的动态库文件dll文件手动复制到可执行文件所在的目录中。关于add_custom_command用法我们之后章节将作详细详解。
{{</admonition>}}

### 十一、补充

在window上使用vs生成可执行文件和执行install命令。
首先，我们首先新建一个build目录，并进入该目录。

```shell
mkdir build
cd build
```
然后使用`cmake`进行构建项目。

```shell
cmake ..
```
然后在`build`目录下可以看到`sln`文件，使用`vs`打开。

**构建所有**: 在vs中的解决方案资源管理器中右键ALL_BUILD，然后点击生成

编译生成hello-world进程，右键hello-world,然后点击生成。如果我们使用CMake在一个项目中生成了多个进程，我们在测试某一个进程时，在对应的进程上右键设为启动项目即可。

**执行install安装命令**: 在INSTALL上右键，然后点击生成即可。注意：只有当我们的CMake中有install命令时，VS中才会出现INTALL选项。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_2/  

