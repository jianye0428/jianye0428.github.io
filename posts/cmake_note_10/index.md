# CMake 笔记 | [10] 使用控制流


## 一、CMake:使用控制流

{{<admonition quote "导言" false>}}
<font color=green>在前面的示例中，我们已经使用过if-else-endif。CMake还提供了创建循环的语言工具：foreach-endforeach和while-endwhile。两者都可以与break结合使用，以便尽早从循环中跳出。而本篇也是为第一章的完结篇，算是正式的踏入了CMake学习的大门了。</font>
{{</admonition>}}

## 二、项目结构

```shell
.
├── cal_add.h
├── cal_subtract.cpp
├── cal_subtract.h
├── CMakeLists.txt
├── main.cpp
├── message.cpp
└── message.h
```

本项目结构回归到了简单的结构，目的主要是为了快速展示本篇的主旨，即如何使用控制流实现一些功能。

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter1/13

## 三、CMakeLists.txt

```Cmake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(control-flow LANGUAGES CXX)

set(CXX_STANDARD 11)

add_library(test_lib
    STATIC
    ${CMAKE_SOURCE_DIR}/message.h
    ${CMAKE_SOURCE_DIR}/message.cpp
    ${CMAKE_SOURCE_DIR}/cal_add.h
    ${CMAKE_SOURCE_DIR}/cal_subtract.h
    ${CMAKE_SOURCE_DIR}/cal_subtract.cpp
)

# 使用-O3编译器优化级别编译库，对目标设置一个私有编译器选项
target_compile_options(test_lib PRIVATE -O3)

# 生成一个源文件列表，以较低的优化选项进行编译
list(APPEND sources_with_lower_optimization
     cal_subtract.cpp
)

# 循环这些源文件，将它们的优化级别调到-O2。使用它们的源文件属性完成
message(STATUS "Setting source properties using IN LISTS syntax:")
foreach(_source IN LISTS sources_with_lower_optimization)
  set_source_files_properties(${_source} PROPERTIES COMPILE_FLAGS -O2)
  message(STATUS "Appending -O2 flag for ${_source}")
endforeach()

# 为了确保设置属性，再次循环并在打印每个源文件的COMPILE_FLAGS属性
message(STATUS "Querying sources properties using plain syntax:")
foreach(_source ${sources_with_lower_optimization})
  get_source_file_property(_flags ${_source} COMPILE_FLAGS)
  message(STATUS "Source ${_source} has the following extra COMPILE_FLAGS: ${_flags}")
endforeach()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} test_lib)
set_source_files_properties(file PROPERTIES property value)
```

将属性设置为给定文件的传递值。与目标非常相似，文件在CMake中也有属性，允许对构建系统进行非常细粒度的控制。

{{<admonition quote "Tip" false>}}
```Cmake
get_source_file_property(VAR file property)
```

检索给定文件所需属性的值，并将其存储在`CMakeVAR`变量中。

注意：`CMake`中，列表是用`分号`分隔的字符串组。列表可以由`list`或`set`命令创建。例如，`set(var a b c d e)`和`list(APPEND a b c d e)`都创建了列表`a;b;c;d;e`。
{{</admonition>}}

{{<admonition quote "" false>}}
```Cmake
foreach(loop_var range total)
or
foreach(loop_var range start stop [step])
```

通过指定一个范围，可以对整数进行循环
{{</admonition>}}






✦

## 四、相关源码


**cal_add.h**

```c++
#ifndef CALCULATE_ADD_HEADER
#define CALCULATE_ADD_HEADER

template <typename T, typename U>
auto Add(T t, U u) -> decltype(t + u){
    return t + u;
}
#endif // ! CALCULATE_ADD_HEADER
```

**cal_substruct.h**

```c++
#ifndef CALCULATE_SUBSTRACT_HEADER
#define CALCULATE_SUBSTRACT_HEADER

float Substract(float a, float b);
#endif // ! CALCULATE_SUBSTRACT_HEADER
```

**cal_substruct.cpp**

```c++
float Substract(float a, float b) { return a - b; }
```

**message.h**

```c++
#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_
#include <string>

class Message {
 public:
  Message() {}
  void Print(const std::string& message);
};
#endif // ! MESSAGE_HEADER_H_
```

**message.cpp**

```c++
#include "message.h"

#include <iostream>
void Message::Print(const std::string& message) {
  std::cout << message << std::endl;
}
```

**main.cpp**

```c++
#include "message.h"
#include "cal_add.h"
#include "cal_subtract.h"
int main() {
  int a = 1;
  int b = 2;
  auto c = Add(a , b);
  Message message;
  message.Print(std::to_string(c));
  return 0;
}
```

## 五、编译结果

```shell
-- The CXX compiler identification is GNU 9.4.0
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Setting source properties using IN LISTS syntax:
-- Appending -O2 flag for cal_add.cpp
-- Appending -O2 flag for cal_subtract.cpp
-- Configuring done
-- Generating done
-- Build files have been written to: /home/jiangli/repo/tutorials/cmake-tutorial/chapter1/13/build
```

## 五、附录

`foreach()`的四种使用方式:

- `foreach(loop_var arg1 arg2 ...)`: 其中提供循环变量和显式项列表。当为`sources_with_lower_optimization`中的项打印编译器标志集时，使用此表单。注意，如果项目列表位于变量中，则必须显式展开它；也就是说，`${sources_with_lower_optimization}`必须作为参数传递。
- 通过指定一个范围，可以对整数进行循环，例如：`foreach(loop_var range total)`或`foreach(loop_var range start stop [step])`。
- 对列表值变量的循环，例如：`foreach(loop_var IN LISTS [list1[...]])` 。参数解释为列表，其内容就会自动展开。
- 对变量的循环，例如：`foreach(loop_var IN ITEMS [item1 [...]])`。参数的内容没有展开。



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_10/  

