# CMake 笔记 | [41] 安装项目


## 一、导言
{{<admonition quote "导言" false>}}
**这篇笔记，将通过一个小项目来介绍一些基本概念，这些概念也将在后面的笔记中使用。安装文件、库和可执行文件是一项非常基础的任务，但是也可能会带来一些问题。通过这篇笔记，展示如何使用CMake有效地避开这些问题。**
{{</admonition>}}



## 二、项目结构

```shell
.
├── CMakeLists.txt
├── src
│   ├── CMakeLists.txt
│   ├── hello_world.cpp
│   ├── message.cpp
│   └── message.hpp
└── test
    └── CMakeLists.txt
```

项目地址

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter9/01




## 三、相关源码

**src/CMakeLists.txt**

```c++
# Search for pkg-config and UUID
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_search_module(UUID uuid IMPORTED_TARGET)
  if(TARGET PkgConfig::UUID)
    message(STATUS "Found libuuid")
    set(UUID_FOUND TRUE)
  endif()
endif()

# SHARED library
add_library(message-shared SHARED "")

target_sources(message-shared
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/message.cpp
)

target_compile_definitions(message-shared
  PUBLIC
    $<$<BOOL:${UUID_FOUND}>:HAVE_UUID>
)

target_link_libraries(message-shared
  PUBLIC
    $<$<BOOL:${UUID_FOUND}>:PkgConfig::UUID>
)

set_target_properties(message-shared
  PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "message"
    DEBUG_POSTFIX "_d"
    PUBLIC_HEADER "message.hpp"
    MACOSX_RPATH ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_executable(hello-world_wDSO hello_world.cpp)

target_link_libraries(hello-world_wDSO
  PUBLIC
    message-shared
)

# Prepare RPATH
file(RELATIVE_PATH _rel ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR} ${CMAKE_INSTALL_PREFIX})

if(APPLE)
  set(_rpath "@loader_path/${_rel}")
else()
  set(_rpath "\$ORIGIN/${_rel}")
endif()

file(TO_NATIVE_PATH "${_rpath}/${INSTALL_LIBDIR}" message_RPATH)

set_target_properties(hello-world_wDSO
  PROPERTIES
    MACOSX_RPATH ON
    SKIP_BUILD_RPATH OFF
    BUILD_WITH_INSTALL_RPATH OFF
    INSTALL_RPATH "${message_RPATH}"
    INSTALL_RPATH_USE_LINK_PATH ON
)

# <<< Install and export targets >>>

install(
  TARGETS
    message-shared
    hello-world_wDSO
  ARCHIVE
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  RUNTIME
    DESTINATION ${INSTALL_BINDIR}
    COMPONENT bin
  LIBRARY
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  PUBLIC_HEADER
    DESTINATION ${INSTALL_INCLUDEDIR}/message
    COMPONENT dev
)
```
{{<admonition quote "tips" false>}}
```c++
target_compile_definitions(message-shared
  PUBLIC
    $<$<BOOL:${UUID_FOUND}>:HAVE_UUID>
)
```

如果我们找到了三方库`UUID`的话，该命令会将 `HAVE_UUID` 编译定义添加到消息共享目标及其依赖库中。
{{</admonition>}}

{{<admonition quote "" false>}}
这段 CMake 脚本使用了 `set_target_properties` 命令来为 `message-shared` 这个目标（通常是一个库或可执行文件）设置一系列属性。下面我会用中文解释这些属性的含义：

```c++
set_target_properties(message-shared
  PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "message"
    DEBUG_POSTFIX "_d"
    PUBLIC_HEADER "message.hpp"
    MACOSX_RPATH ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
```

1. **`POSITION_INDEPENDENT_CODE 1`**: 设置代码为位置无关代码。这对于创建共享库是重要的，因为它允许代码在内存中的任何位置运行，这对于共享库来说是必需的。
2. **`SOVERSION ${PROJECT_VERSION_MAJOR}`**: 设置共享对象版本号，这里使用的是项目的主版本号。
3. **`OUTPUT_NAME "message"`**: 指定输出的名称。尽管目标名称是 `message-shared`，在构建时生成的文件将被命名为 `message`（例如，`message.dll` 或 `message.so`）。
4. **`DEBUG_POSTFIX "_d"`**: 为调试版本的输出添加后缀。在构建调试版本时，输出文件的名称会有一个额外的 `_d` 后缀，有助于区分调试和发布版本。
5. **`PUBLIC_HEADER "message.hpp"`**: 指定公共头文件。
6. **`MACOSX_RPATH ON`**: 在 macOS 系统上启用 RPATH。这是一种动态库查找路径的设置方法，有助于应用程序在运行时找到其依赖的共享库。
7. **`WINDOWS_EXPORT_ALL_SYMBOLS ON`**: 在 Windows 上自动导出所有符号。这对于创建 DLL（动态链接库）特别有用，因为它简化了符号导出的过程。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
file(RELATIVE_PATH _rel ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR} ${CMAKE_INSTALL_PREFIX})

if(APPLE)
  set(_rpath "@loader_path/${_rel}")
else()
  set(_rpath "\$ORIGIN/${_rel}")
endif()
```

在这个 CMake 脚本命令中，`file(RELATIVE_PATH ...)` 用于计算两个路径之间的相对路径。

这个命令的目的是为了找出 `${CMAKE_INSTALL_PREFIX}` 相对于 `${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR}` 的相对路径。换句话说，它在寻找从安装的二进制目录（`INSTALL_BINDIR`）到安装的根目录（`CMAKE_INSTALL_PREFIX`）的路径。在大多数情况下，这将简单地解析为从二进制目录向上到达根目录的相对路径（比如 `../` 或者更多级的 `../../`，取决于 `INSTALL_BINDIR` 的深度）。

这种类型的计算在处理安装和打包时非常有用，尤其是当需要处理可移植性和不同系统结构时。通过这样的相对路径设置，可以确保不管你的软件安装在哪里，文件和资源的相互引用都是正确的。

这段 CMake 脚本代码使用 `file(RELATIVE_PATH ...)` 命令计算了一个相对路径，并根据操作系统类型（Apple 系统或其他）设置了一个名为 `_rpath` 的变量，用于指定动态库的运行时搜索路径（RPATH）。下面是详细解释：

1. **`if(APPLE)`** 和 **`else()`**: 这两行代码检查当前是否在 Apple 系统（比如 macOS）上进行构建。如果是，在 Apple 系统上使用一种 RPATH 设置方法；如果不是（比如在 Linux 或 Windows 上），使用另一种方法。
2. **`set(_rpath "@loader_path/${_rel}")`**: 在 Apple 系统上，`_rpath` 被设置为 `"@loader_path/${_rel}"`。这里的 `@loader_path` 是一个特殊的标记，它表示加载动态库的可执行文件的位置。这种方法允许动态库在与可执行文件相对的路径中被找到。
3. **`set(_rpath "\$ORIGIN/${_rel}")`**: 在非 Apple 系统上，`_rpath` 被设置为 `"\$ORIGIN/${_rel}"`。这里的 `$ORIGIN` 是一个特殊的标记，它也表示加载动态库的可执行文件的位置。与 Apple 系统的方法类似，它使得动态库可以在相对于可执行文件的路径中被找到。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
set_target_properties(hello-world_wDSO
  PROPERTIES
    MACOSX_RPATH ON
    SKIP_BUILD_RPATH OFF
    BUILD_WITH_INSTALL_RPATH OFF
    INSTALL_RPATH "${message_RPATH}"
    INSTALL_RPATH_USE_LINK_PATH ON
)
```

使用 `set_target_properties` 命令来为名为 `hello-world_wDSO` 的目标设置一系列属性。

1. **`MACOSX_RPATH ON`**: 这个选项用于在 macOS 系统上启用 RPATH。这意味着在 macOS 上构建时，CMake 会自动设置运行时路径，这有助于程序在运行时找到其动态库依赖。
2. **`SKIP_BUILD_RPATH OFF`**: 当此选项为 OFF 时，CMake 会在构建阶段使用 RPATH。这样做确保在构建时（例如，当运行测试时），可以找到动态库。
3. **`BUILD_WITH_INSTALL_RPATH OFF`**: 这个选项表示构建时不使用安装后的 RPATH。这意味着构建时和安装后使用的 RPATH 是不同的。通常，构建时的 RPATH 指向构建目录中的库，而安装后的 RPATH 指向安装目录中的库。
4. **`INSTALL_RPATH "${message_RPATH}"`**: 这个选项设置了安装后的 RPATH。`${message_RPATH}` 是一个变量，它应该在其他地方被定义，并包含了运行时库（比如动态链接库）的路径。这意味着一旦 `hello-world_wDSO` 被安装，它会使用这个变量指定的路径来查找其运行时依赖。
5.  **`INSTALL_RPATH_USE_LINK_PATH ON`**: 当此选项为 ON 时，CMake 会在设置安装后的 RPATH 时考虑目标的链接路径。这意味着安装后的 RPATH 不仅会包括 `INSTALL_RPATH` 指定的路径，还会包括目标链接时使用的所有路径。这有助于确保运行时能找到所有必需的动态库，尤其是当这些库位于非标准或非默认的位置时。

这些属性共同确保了 `hello-world_wDSO` 在构建、安装和运行时能正确地找到其动态链接库依赖。这是在跨平台开发和部署应用程序时非常重要的一部分，尤其是在涉及到动态链接库的情况下。
{{</admonition>}}


{{<admonition quote "" false>}}
```c++
install(
  TARGETS
    message-shared
    hello-world_wDSO
  ARCHIVE
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  RUNTIME
    DESTINATION ${INSTALL_BINDIR}
    COMPONENT bin
  LIBRARY
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  PUBLIC_HEADER
    DESTINATION ${INSTALL_INCLUDEDIR}/message
    COMPONENT dev
)
```

使用 `install()` 命令来定义如何安装 `message-shared` 和 `hello-world_wDSO` 这两个目标。

1. **`TARGETS message-shared hello-world_wDSO`**: 这指定了要安装的目标。这里有两个目标：`message-shared` 和 `hello-world_wDSO`。
2. **`ARCHIVE DESTINATION ${INSTALL_LIBDIR} COMPONENT lib`**: 这部分指定静态库（`.a` 或 `.lib` 文件）的安装位置。它们将被安装到由 `${INSTALL_LIBDIR}` 变量定义的目录中，并标记为 `lib` 组件的一部分。通常，`${INSTALL_LIBDIR}` 会指向类似 `lib` 或 `lib64` 的目录。
3. **`RUNTIME DESTINATION ${INSTALL_BINDIR} COMPONENT bin`**: 这定义了可执行文件（`hello-world_wDSO`）的安装位置。它们将被安装到由 `${INSTALL_BINDIR}` 变量指定的目录，通常是类似于 `bin` 的目录。这些文件被标记为 `bin` 组件的一部分。
4. **`LIBRARY DESTINATION ${INSTALL_LIBDIR} COMPONENT lib`**: 这指定动态库（`.so`、`.dll` 或 `.dylib` 文件）的安装位置。就像静态库一样，它们将被安装到 `${INSTALL_LIBDIR}` 目录，并标记为 `lib` 组件的一部分。
5. **`PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDEDIR}/message COMPONENT dev`**: 这指定了公共头文件（如 `message.hpp`）的安装位置。这些文件将被安装到 `${INSTALL_INCLUDEDIR}/message` 目录，通常是类似于 `include/message` 的目录，作为 `dev` 组件的一部分。这使得其他开发者可以在他们自己的项目中轻松地找到并使用这些头文件。
{{</admonition>}}

**src/message.h**

```c++
class Message {
public:
  Message(const std::string &m) : message_(m) {}

  friend std::ostream &operator<<(std::ostream &os, Message &obj) {
    return obj.PrintObject(os);
  }

private:
  std::string message_;
  std::ostream &PrintObject(std::ostream &os);
};

std::string GetUUID();
```

**src/message.cpp**

```c++
#include "message.hpp"

#include <iostream>
#include <string>

#ifdef HAVE_UUID
#include <uuid/uuid.h>
#endif

std::ostream &Message::PrintObject(std::ostream &os) {
  os << "This is my very nice message: " << std::endl;
  os << message_ << std::endl;
  os << "...and here is its UUID: " << GetUUID();

  return os;
}

#ifdef HAVE_UUID
std::string GetUUID() {
  uuid_t uuid;
  uuid_generate(uuid);
  char uuid_str[37];
  uuid_unparse_lower(uuid, uuid_str);
  uuid_clear(uuid);
  std::string uuid_cxx(uuid_str);
  return uuid_cxx;
}
#else
std::string GetUUID() { return "Ooooops, no UUID for you!"; }
#endif
```

**src/hello_world.cpp**

```c++
#include <cstdlib>
#include <iostream>

#include "message.hpp"

int main() {
  Message say_hello("Hello, CMake World!");

  std::cout << say_hello << std::endl;

  Message say_goodbye("Goodbye, CMake World");

  std::cout << say_goodbye << std::endl;

  retu
```

**test/CMakeLists.txt**

```c++
add_test(
  NAME test_shared
  COMMAND $<TARGET_FILE:hello-world_wDSO>
)
```

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(
  example
  LANGUAGES CXX
  VERSION 1.0.0
)

# <<< General set up >>>

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output/)
message(STATUS "Project will be installed to ${CMAKE_INSTALL_PREFIX}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKEDIR CMake)
else()
  set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")

# Report to user
foreach(p LIB BIN INCLUDE CMAKE)
  file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} _path )
  message(STATUS "Installing ${p} components to ${_path}")
  unset(_path)
endforeach()

add_subdirectory(src)

enable_testing()

add_subdirectory(test)
```

{{<admonition quote "" false>}}
```c++
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKEDIR CMake)
else()
  set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")
```

1. **`set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")`**: 这行设置了一个名为 `INSTALL_LIBDIR` 的变量，它定义了库文件（静态库和动态库）的安装目录。这个目录默认为 `CMAKE_INSTALL_LIBDIR` 的值，通常指向系统默认的库安装路径（如 `lib` 或 `lib64`）。
2. **`set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")`**: 这行设置了一个名为 `INSTALL_BINDIR` 的变量，用于定义可执行文件的安装目录。这个目录默认为 `CMAKE_INSTALL_BINDIR` 的值，通常是系统默认的可执行文件安装路径（如 `bin`）。
3. **`set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")`**: 这行设置了一个名为 `INSTALL_INCLUDEDIR` 的变量，它定义了头文件的安装目录。这个目录默认为 `CMAKE_INSTALL_INCLUDEDIR` 的值，通常指向系统默认的头文件安装路径（如 `include`）。
4. **`if(WIN32 AND NOT CYGWIN)`** 和 **`else()`**: 这两行代码是一个条件语句，用于区分 Windows 系统和其他系统（如 Linux 或 macOS）。
   - 在 Windows 系统上，如果不是在 Cygwin 环境下，`DEF_INSTALL_CMAKEDIR` 被设置为 `CMake`。这意味着 CMake 配置文件将安装到一个名为 `CMake` 的目录中。
   - 在其他系统上，`DEF_INSTALL_CMAKEDIR` 被设置为 `share/cmake/${PROJECT_NAME}`。这里 `${PROJECT_NAME}` 是项目名称的变量，这个路径通常用于存放项目相关的 CMake 配置文件。
5. **`set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")`**: 这行设置了一个名为 `INSTALL_CMAKEDIR` 的变量，它定义了 CMake 配置文件的安装目录，值为前面根据平台条件设置的 `DEF_INSTALL_CMAKEDIR`。

这些设置的目的是确保在不同的操作系统和环境中，项目的库文件、可执行文件、头文件和 CMake 文件都能被安装到合适的位置。通过使用这些变量，CMake 脚本可以灵活地适应不同系统的目录结构和用户的自定义安装路径。这种做法提高了项目的可移植性和灵活性，使得在不同环境下的构建和安装过程更加一致和可预测。此外，使用缓存变量（`CACHE PATH`）允许用户在配置（CMake 的 configure 阶段）时覆盖这些路径，进一步增加了灵活性。

{{</admonition>}}



## 四、结果展示

```shell
mkdir build & cd build
cmake ..
cmake --build . --target install
```

GNU/Linux构建目录的内容如下:

```shell
├── build
    ├── bin
    │   └── hello-world_wDSO
    ├── CMakeCache.txt
    ├── cmake_install.cmake
    ├── CTestTestfile.cmake
    ├── install_manifest.txt
    ├── lib
    │   ├── libmessage.so -> libmessage.so.1
    │   └── libmessage.so.1
    ├── Makefile
    ├── src
    ├── test
    └── Testing
```

在安装位置，可以找到如下的目录结构:

```shell
.
├── bin
│   └── hello-world_wDSO
├── include
│   └── message
│       └── message.hpp
└── lib
    ├── libmessage.so -> libmessage.so.1
    └── libmessage.so.1
```

## 五、补充内容

**安装到标准位置**

对于项目的安装来说，什么是好的布局呢？如果只有自己使用该项目，那就无所谓好或坏的布局。然而，一旦向外部发布产品，和他人共用该项目，就应该在安装项目时提供一个合理的布局。

我们可以遵循一些标准，CMake可以帮助我们做到这一点。实际上，`GNUInstallDirs.cmake`模块所做的就是定义这样一组变量，这些变量是安装不同类型文件的子目录的名称。

  - **`CMAKE_INSTALL_BINDIR`**：用于定义用户可执行文件所在的子目录，即所选安装目录下的bin目录。
  - **`CMAKE_INSTALL_LIBDIR`**：将扩展到目标代码库(即静态库和动态库)所在的子目录。在64位系统上，它是`lib64`，而在32位系统上，它只是`lib`。
  - **`CMAKE_INSTALL_INCLUDEDIR`**：使用这个变量为头文件获取正确的子目录，该变量为`include`。

用户可能希望覆盖这些选项。允许在主CMakeLists.txt文件中使用以下方式覆盖选项:

```c++
# Offer the user the choice
of overriding the installation directories
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH
"Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH
"Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE
PATH "Installation directory for header files")
```

重新定义了在项目中使用的`INSTALL_BINDIR`、`INSTALL_LIBDIR`和`INSTALL_INCLUDEDIR`变量。

当只要求安装库：

```c++
$ cmake -D COMPONENT=lib -P cmake_install.cmake
```

正确设置`RPATH`可能相当麻烦，但这对于用户来说无法避免。默认情况下，`CMake`设置可执行程序的`RPATH`，假设它们将从构建树运行。但是，安装之后`RPATH`被清除，当用户想要运行`hello-world_wDSO`时，就会出现问题。使用Linux上的`ldd`工具，我们可以检查构建树中的`hello-world_wDSO`可执行文件，运行`ldd hello-world_wDSO`将得到以下结果:

```c++
libmessage.so.1 => /home/jiangli/repo/tutorials/cmake-tutorial/chapter9/01/build/lib/libmessage.so.1 (0x00007f43a4df7000)
```

在安装目录中运行ldd hello-world_wDSO将得到以下结果:

```c++
libmessage.so.1 => Not found
```

这显然是不行的。但是，总是硬编码`RPATH`来指向构建树或安装目录也是错误的：这两个位置中的任何一个都可能被删除，从而导致可执行文件的损坏。给出的解决方案为构建树和安装目录中的可执行文件设置了不同的`RPATH`，因此它总是指向“有意义”的位置；也就是说，尽可能接近可执行文件。在构建树中运行`ldd`显示相同的输出:

```c++
libmessage.so.1 => /home/jiangli/repo/tutorials/cmake-tutorial/chapter9/01/output/bin/./../lib/libmessage.so.1 (0x00007f0ebfc4a000)

libmessage.so.1 => /home/jiangli/repo/tutorials/cmake-tutorial/chapter9/01/build/lib/libmessage.so.1 (0x00007f43a4df7000)
```

使用了带有目标参数的`CMake`安装命令，该命令还有另外4个参数:

- `FILES`和`PROGRAMS`，分别用于安装文件或程序。安装后，并设置安装文件适当的权限。对于文件，对所有者具有读和写权限，对组以及其他用户和组具有读权限。对于程序，将授予执行权限。注意，`PROGRAMS`要与非构建目标的可执行程序一起使用。
- `DIRECTORY`，用于安装目录。当只给出一个目录名时，它通常被理解为相对于当前源目录。可以对目录的安装粒度进行控制。
- `SCRIPT`，可以使用它在CMake脚本中定义自定义安装规则。
- `EXPORT`，该参数用于导出目标。




---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_41/  

