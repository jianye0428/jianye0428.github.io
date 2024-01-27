# CMake 笔记 | [14] 检测Python模块和包


## 一、检测Python模块和包

{{<admonition quote "" false>}}
**上一篇，我们基本了解了如何检测python的解释器和python库。通常，代码是依赖于特定的python模块的，无论是python工具、嵌入python的程序，还是扩展python的库。例如，numpy包。依赖于python模块或包的项目中，确定满足对这些python模块的依赖非常重要。**
{{</admonition>}}


## 二、项目结构

```shell
├── CMakeLists.txt
├── py3_pure_embedding.cpp
└── use_numpy.py
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter3/03


**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(python_module LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PythonInterp REQUIRED)
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "-c" "import re, numpy; print(re.compile('/__init__.py.*').sub('',numpy.__file__))"
    RESULT_VARIABLE numpy_status
    OUTPUT_VARIABLE numpy_location
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT numpy_status)
    set(NumPy ${numpy_location} CACHE STRING "Location of NumPy")
endif()

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "-c" "import numpy; print(numpy.__version__)"
    OUTPUT_VARIABLE numpy_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NumPy
    FOUND_VAR NumPy_FOUND
    REQUIRED_VARS NumPy
    VERSION_VAR numpy_version
)

add_executable(pure-embedding py3_pure_embedding.cpp)

target_include_directories(pure-embedding
  PRIVATE ${PYTHON_INCLUDE_DIRS}
)

target_link_libraries(pure-embedding
  PRIVATE ${PYTHON_LIBRARIES}
)

add_custom_command(
  OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
  COMMAND
      ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py
      ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
  DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py
)

# make sure building pure-embedding triggers the above custom command
target_sources(pure-embedding
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
)
```

{{<admonition quote "Tip" false>}}
```c++
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "-c" "import re, numpy; print(re.compile('/__init__.py.*').sub('',numpy.__file__))"
    RESULT_VARIABLE numpy_status
    OUTPUT_VARIABLE numpy_location
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
```

使用了`CMake`的`execute_process`函数来运行一个`python`脚本。该脚本导入了`re`和`numpy`模块，然后使用`re.compile`函数来替换`numpy`模块路径中的一个模式。`RESULT_VARIABLE`用于捕获`python`脚本执行的状态，而`OUTPUT_VARIABLE`用于捕获修改后的`numpy`模块文件的位置。通过使用`ERROR_QUIET`来抑制进程生成的任何错误，并且使用`OUTPUT_STRIP_TRAILING_WHITESPACE`来移除输出中的尾随空格。
{{</admonition>}}


{{<admonition quote "Tip" false>}}
```c++
if(NOT numpy_status)
    set(NumPy ${numpy_location} CACHE STRING "Location of NumPy")
endif()
```

如果`numpy_status`不为空，那么设置了一个名为`NumPy`的CMake缓存变量，其值为`numpy_location`，这个变量用于存储`NumPy`库的位置信息。这个操作允许在`CMake`配置过程中指定`NumPy`的位置，以便后续的构建过程可以使用它。如果`numpy_status`为空，则不进行任何操作。
{{</admonition>}}


{{<admonition quote "Tip" false>}}
```c++
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "-c" "import numpy; print(numpy.__version__)"
    OUTPUT_VARIABLE numpy_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
```

这段代码使用`execute_process`命令来执行一个`python`脚本。

- `${PYTHON_EXECUTABLE}` 是一个`CMake`变量，用于指定`python`可执行文件的路径。
- `-c` 选项告诉`python`解释器后面紧跟着的字符串是要执行的`python`代码。
- 在这个`python`代码中，首先导入了`numpy`库，然后使用`print`函数输出了`numpy`库的版本号。
- `OUTPUT_VARIABLE` 选项用于捕获`python`代码的输出，即`numpy`库的版本号。
- `ERROR_QUIET` 选项用于忽略可能的错误信息。
- `OUTPUT_STRIP_TRAILING_WHITESPACE` 选项用于移除输出字符串末尾的空格。

通过这个操作，可以在`CMake`配置过程中获取并保存`numpy`库的版本号，以便后续的构建过程可以使用。
{{</admonition>}}


{{<admonition quote "Tip" false>}}
```c++
add_custom_command(
  OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
  COMMAND
      ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py
      ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
  DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py
)
```

使用 `CMake` 中的 `add_custom_command` 命令，用于定义自定义的构建步骤，以及生成相应的输出文件。

- `OUTPUT` 指定了生成的输出文件，这里是 `${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py`。
- `COMMAND` 指定了生成输出文件所需要执行的命令，这里是将 `${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py` 复制到 `${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py`。
- `DEPENDS` 列出了生成输出文件所依赖的文件，这里是 `${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py`。

这段代码的作用是在构建过程中，如果 `${CMAKE_CURRENT_SOURCE_DIR}/use_numpy.py` 发生变化，就执行指定的命令来将该文件复制到构建目录 `${CMAKE_CURRENT_BINARY_DIR}` 下的相同路径。这可以确保在构建过程中，始终使用最新的 `use_numpy.py` 文件。
{{</admonition>}}


{{<admonition quote "Tip" false>}}
```c++
target_sources(pure-embedding
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
)
```

在 `CMake` 的构建过程中为名为 `pure-embedding` 的目标（通常是一个可执行文件或库）指定了源文件。在这里，并没有直接添加 `C++` 源代码，而是添加了一个 `python` 脚本文件 `${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py`。

这意味着在构建 `pure-embedding` 目标时，`CMake` 会将 `${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py` 视为目标的源文件之一，并确保在构建过程中该文件已经生成。
{{</admonition>}}


**相关源码**



**py3_pure_embedding.cpp**

```c++
#include <Python.h>

int main(int argc, char* argv[]) {
  PyObject *py_name, *py_module, *py_dict, *py_func;
  PyObject *py_args, *py_value;
  if (argc < 3) {
    fprintf(stderr, "Usage: pure-embedding python_file funcname [args]\n");
    return 1;
  }
  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.append(\".\")");
  py_name = PyUnicode_DecodeFSDefault(argv[1]);
  /* Error checking of py_name left out */
  py_module = PyImport_Import(py_name);
  Py_DECREF(py_name);
  if (py_module != NULL) {
    py_func = PyObject_GetAttrString(py_module, argv[2]);
    /* py_func is a new reference */
    if (py_func && PyCallable_Check(py_func)) {
      py_args = PyTuple_New(argc - 3);
      for (int i = 0; i < argc - 3; ++i) {
        py_value = PyLong_FromLong(atoi(argv[i + 3]));
        if (!py_value) {
          Py_DECREF(py_args);
          Py_DECREF(py_module);
          fprintf(stderr, "Cannot convert argument\n");
          return 1;
        }
        /* py_value reference stolen here: */
        PyTuple_SetItem(py_args, i, py_value);
      }
      py_value = PyObject_CallObject(py_func, py_args);
      Py_DECREF(py_args);
      if (py_value != NULL) {
        printf("Result of call: %ld\n", PyLong_AsLong(py_value));
        Py_DECREF(py_value);
      } else {
        Py_DECREF(py_func);
        Py_DECREF(py_module);
        PyErr_Print();
        fprintf(stderr, "Call failed\n");
        return 1;
      }
    } else {
      if (PyErr_Occurred()) PyErr_Print();
      fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
    }
    Py_XDECREF(py_func);
    Py_DECREF(py_module);
  } else {
    PyErr_Print();
    fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
    return 1;
  }
  Py_Finalize();
  return 0;
}
```
**use_numpy.py**

```python
import numpy as np
def print_ones(rows, cols):
  A = np.ones(shape=(rows, cols), dtype=float)
  print(A)
  # we return the number of elements to verify
  # that the C++ code is able to receive return values
  num_elements = rows*cols
  return num_elements
```




---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_14/  

