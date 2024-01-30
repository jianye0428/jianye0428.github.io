# CMake 笔记 | [22] 测试的其他补充(重要)


## 一、导言

Original jjjstephen [Hope Hut](javascript:void(0);) *2023-09-21 09:03* *Posted on 天津*

![Image](https://mmbiz.qpic.cn/sz_mmbiz_jpg/sd9Xic2LWs1WwT1A0HSI6pMVScgZJicEnnOickMWJc4aXcy2uPpUYEqHPRDL3xoljDdtjySrsKjen7TWJQyFMCo8w/640?wx_fmt=jpeg&wxfrom=5&wx_lazy=1&wx_co=1)



![Image](https://mmbiz.qpic.cn/sz_mmbiz_gif/sd9Xic2LWs1WwT1A0HSI6pMVScgZJicEnnoLFl1sLEZ3ic5sljf3eCAA7Xjap5dRibPCNDX1icQzXzqS635pziaqDxfg/640?wx_fmt=gif&wxfrom=5&wx_lazy=1&wx_co=1)


{{<admonition quote "导言" false>}}
**通过前几篇的学习，我们基本掌握了关于单元测试的相关内容。当然，随着技术的不断发展，根据不同业务的需求测试框架层出不穷，我们没有办法一一列举。本篇我们将补充几个测试的相关技术，如预期失败、并行测试以及测试子集等。**
{{</admonition>}}



## 二、预期测试失败

理想情况下，我们希望所有的测试能在每个平台上通过。然而，也可能想要测试预期的失败或异常是否会在受控的设置中进行。这种情况下，我们将把预期的失败定义为成功。我们认为，这通常应该交给测试框架(例如：`Catch2`或`Google Test`)的任务，它应该检查预期的失败并向`CMake`报告成功。但是，在某些情况下，可能希望将测试的非零返回代码定义为成功；换句话说，可能想要颠倒成功和失败的定义。


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">项目结构</font></td></tr></body></table>

```shell
.
├── CMakeLists.txt
└── test.py
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/05


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>


```c++
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test_error LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

enable_testing()

add_test(example ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py)

set_tests_properties(example PROPERTIES WILL_FAIL true)
```
{{<admonition quote "" false>}}
定义测试并告诉`CMake`，测试预期会失败:
```
set_tests_properties(example PROPERTIES WILL_FAIL true)
```
{{</admonition>}}


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>


**test.py**

```python
import sys
# simulate a failing test
sys.exit(1)
```


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果</font></td></tr></body></table>



```shell
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/05/build
    Start 1: example
1/1 Test #1: example ..........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.02 sec
```

## 三、使用超时测试运行时间过长的测试

理想情况下，测试集应该花很短的时间进行，以便开发人员经常运行测试，并使每个提交(变更集)进行测试成为可能(或更容易)。然而，有些测试可能会花费更长的时间或者被卡住(例如，由于高文件I/O负载)，可能需要设置超时来终止耗时过长的测试，它们延迟了整个测试，并阻塞了部署管道。本节，将通过一种设置超时的方法，可以针对每个测试设置不同的超时。

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">项目结构</font></td></tr></body></table>


```shell
.
├── CMakeLists.txt
└── test.py
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/06


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(test_long_test LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

enable_testing()

add_test(example ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py)
set_tests_properties(example PROPERTIES TIMEOUT 10)
set_tests_properties(example PROPERTIES TIMEOUT 10)
```

为测试指定时限，设置为10秒

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>

**test.py**

```python
import sys
import time
# wait for 2 seconds
time.sleep(2)
# report success
sys.exit(0)
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果</font></td></tr></body></table>

```shell
mkdir build & cd build
cmake ..
ctest
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/06/build
    Start 1: example
1/1 Test #1: example ..........................   Passed    2.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   2.02 sec
```

为了验证超时是否有效，将`test.py`中的`sleep`命令增加到`11`秒，并重新运行测试:

```shell
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/06/build
    Start 1: example
1/1 Test #1: example ..........................***Timeout  10.02 sec

0% tests passed, 1 tests failed out of 1

Total Test time (real) =  10.02 sec

The following tests FAILED:
          1 - example (Timeout)
Errors while running CTest
```

## 四、并行测试

大多数现代计算机都有`4`个或更多个`CPU`核芯。`CTest`有个非常棒的特性，能够并行运行测试，如果有多个可用的核。这可以减少测试的总时间。


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">项目结构</font></td></tr></body></table>

```shell
.
├── CMakeLists.txt
└── test
    ├── a.py
    ├── b.py
    ├── c.py
    ├── d.py
    ├── e.py
    ├── f.py
    ├── g.py
    ├── h.py
    ├── i.py
    └── j.py
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/07



<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txts</font></td></tr></body></table>



```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(recipe-08 LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

enable_testing()
add_test(a ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/a.py)
add_test(b ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/b.py)
add_test(c ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/c.py)
add_test(d ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/d.py)
add_test(e ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/e.py)
add_test(f ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/f.py)
add_test(g ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/g.py)
add_test(h ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/h.py)
add_test(i ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/i.py)
add_test(j ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/j.py
```


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>


这里我们只给出了一个`python`文件，其他文件按照如下表格设置时间即可。

|测试用例|该单元耗时|
|:--|:--|
|a,b,c,d|0.5|
|e,f,g|1.5|
|h|2.5|
|i|3.5|
|j|4.5|

**a.py**

```python
import sys
import time
# wait for 0.5 seconds
time.sleep(0.5)
# finally report success
sys.exit(0)
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果</font></td></tr></body></table>

```shell
mkdir build & cd build
cmake ..
ctest
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/07/build
      Start  1: a
 1/10 Test  #1: a ................................   Passed    0.51 sec
      Start  2: b
 2/10 Test  #2: b ................................   Passed    0.52 sec
      Start  3: c
 3/10 Test  #3: c ................................   Passed    0.52 sec
      Start  4: d
 4/10 Test  #4: d ................................   Passed    0.52 sec
      Start  5: e
 5/10 Test  #5: e ................................   Passed    1.52 sec
      Start  6: f
 6/10 Test  #6: f ................................   Passed    1.52 sec
      Start  7: g
 7/10 Test  #7: g ................................   Passed    1.52 sec
      Start  8: h
 8/10 Test  #8: h ................................   Passed    2.52 sec
      Start  9: i
 9/10 Test  #9: i ................................   Passed    3.52 sec
      Start 10: j
10/10 Test #10: j ................................   Passed    4.52 sec

100% tests passed, 0 tests failed out of 10

Total Test time (real) =  17.20 sec
ctest --parallel 4
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/07/build
      Start 10: j
      Start  9: i
      Start  8: h
      Start  7: g
 1/10 Test  #7: g ................................   Passed    1.51 sec
      Start  6: f
 2/10 Test  #8: h ................................   Passed    2.51 sec
      Start  5: e
 3/10 Test  #6: f ................................   Passed    1.51 sec
      Start  4: d
 4/10 Test  #9: i ................................   Passed    3.52 sec
      Start  3: c
 5/10 Test  #4: d ................................   Passed    0.51 sec
      Start  2: b
 6/10 Test  #5: e ................................   Passed    1.51 sec
      Start  1: a
 7/10 Test  #3: c ................................   Passed    0.51 sec
 8/10 Test  #2: b ................................   Passed    0.51 sec
 9/10 Test #10: j ................................   Passed    4.51 sec
10/10 Test  #1: a ................................   Passed    0.51 sec

100% tests passed, 0 tests failed out of 10

Total Test time (real) =   4.54 sec
ctest --parallel 8
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/07/build
      Start 10: j
      Start  9: i
      Start  8: h
      Start  7: g
      Start  6: f
      Start  5: e
      Start  4: d
      Start  2: b
 1/10 Test  #4: d ................................   Passed    0.52 sec
      Start  3: c
 2/10 Test  #2: b ................................   Passed    0.52 sec
      Start  1: a
 3/10 Test  #3: c ................................   Passed    0.51 sec
 4/10 Test  #1: a ................................   Passed    0.51 sec
 5/10 Test  #7: g ................................   Passed    1.52 sec
 6/10 Test  #6: f ................................   Passed    1.52 sec
 7/10 Test  #5: e ................................   Passed    1.52 sec
 8/10 Test  #8: h ................................   Passed    2.52 sec
 9/10 Test  #9: i ................................   Passed    3.52 sec
10/10 Test #10: j ................................   Passed    4.52 sec

100% tests passed, 0 tests failed out of 10

Total Test time (real) =   4.52 sec
```


## 五、运行测试子集

前几节，我们学习了如何在`CMake`的帮助下并行运行测试，并讨论了从最长的测试开始是最高效的。虽然，这种策略将总测试时间最小化，但是在特定特性的代码开发期间，或者在调试期间，我们可能不希望运行整个测试集。对于调试和代码开发，我们只需要能够运行选定的测试子集。t通过本节我们对这一策略进行进一步探究。

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">项目结构</font></td></tr></body></table>



```shell
.
├── CMakeLists.txt
└── test
    ├── benchmark-a.py
    ├── benchmark-b.py
    ├── feature-a.py
    ├── feature-b.py
    ├── feature-c.py
    └── feature-d.py
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/08


<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(test_subset LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

enable_testing()
add_test(
  NAME feature-a
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-a.py
)

add_test(
  NAME feature-b
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-b.py
)

add_test(
  NAME feature-c
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-c.py
)

add_test(
  NAME feature-d
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-d.py
)

add_test(
  NAME benchmark-a
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/benchmark-a.py
)

add_test(
  NAME benchmark-b
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/benchmark-b.py
)

set_tests_properties(
  feature-a
  feature-b
  feature-c
  PROPERTIES
      LABELS "quick"
)

set_tests_properties(
  feature-d
  benchmark-a
  benchmark-b
  PROPERTIES
      LABELS "long"
)
```

给较短的测试贴上`quick`的标签，给较长的测试贴上`long`的标签:

```c++
set_tests_properties(
  feature-a
  feature-b
  feature-c
  PROPERTIES
      LABELS "quick"
)

set_tests_properties(
  feature-d
  benchmark-a
  benchmark-b
  PROPERTIES
      LABELS "long"
)
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>



我们假设总共有六个测试：前三个测试比较短，名称分别为`feature-a`、`feature-b`和`feature-c`，还有三个长测试，名称分别是`feature-d`、`benchmark-a`和`benchmark-b`。我们只给出`feature-a.py`，其他只是睡眠时间的不同。

```python
import sys
import time
# wait for 0.1 seconds
time.sleep(1)
# finally report success
sys.exit(0)
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果</font></td></tr></body></table>

```shell
ctest -R feature
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/08/build
    Start 1: feature-a
1/4 Test #1: feature-a ........................   Passed    0.11 sec
    Start 2: feature-b
2/4 Test #2: feature-b ........................   Passed    0.11 sec
    Start 3: feature-c
3/4 Test #3: feature-c ........................   Passed    0.11 sec
    Start 4: feature-d
4/4 Test #4: feature-d ........................   Passed    1.01 sec

100% tests passed, 0 tests failed out of 4

Label Time Summary:
long     =   1.01 sec*proc (1 test)
quick    =   0.33 sec*proc (3 tests)

Total Test time (real) =   1.36 sec
ctest -L long
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/08/build
    Start 4: feature-d
1/3 Test #4: feature-d ........................   Passed    1.01 sec
    Start 5: benchmark-a
2/3 Test #5: benchmark-a ......................   Passed    1.01 sec
    Start 6: benchmark-b
3/3 Test #6: benchmark-b ......................   Passed    1.01 sec

100% tests passed, 0 tests failed out of 3

Label Time Summary:
long    =   3.04 sec*proc (3 tests)

Total Test time (real) =   3.04 sec
ctest -L quick
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/08/build
    Start 1: feature-a
1/3 Test #1: feature-a ........................   Passed    0.11 sec
    Start 2: feature-b
2/3 Test #2: feature-b ........................   Passed    0.11 sec
    Start 3: feature-c
3/3 Test #3: feature-c ........................   Passed    0.12 sec

100% tests passed, 0 tests failed out of 3

Label Time Summary:
quick    =   0.34 sec*proc (3 tests)

Total Test time (real) =   0.34 sec
```

## 六、使用测试固件


本节将学习如何使用测试固件。这对于更复杂的测试非常有用，这些测试需要在测试运行前进行设置，以及在测试完成后执行清理操作(例如：创建示例数据库、设置连接、断开连接、清理测试数据库等等)。我们需要运行一个设置或清理操作的测试，并能够以一种可预测和健壮的方式自动触发这些步骤，而不需要引入代码重复。这些设置和清理步骤可以委托给测试框架(例如`Google Test`或`Catch2`)。

**项目结构**

```shell
.
├── CMakeLists.txt
└── test
    ├── cleanup.py
    ├── feature-a.py
    ├── feature-b.py
    └── setup.py
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter4/09

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">CMakeLists.txt</font></td></tr></body></table>

```c++
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(test_firmware LANGUAGES NONE)

find_package(PythonInterp REQUIRED)

enable_testing()

add_test(
  NAME setup
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/setup.py
)

set_tests_properties(
  setup
  PROPERTIES
    FIXTURES_SETUP my-fixture
)

add_test(
  NAME feature-a
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-a.py
)

add_test(
  NAME feature-b
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-b.py
)

set_tests_properties(
  feature-a
  feature-b
  PROPERTIES
    FIXTURES_REQUIRED my-fixture
)

add_test(
  NAME cleanup
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/cleanup.py
)

set_tests_properties(
  cleanup
  PROPERTIES
    FIXTURES_CLEANUP my-fixture
)
add_test(
  NAME setup
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/setup.py
  )
set_tests_properties(
  setup
  PROPERTIES
      FIXTURES_SETUP my-fixture
  )
add_test(
  NAME feature-a
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-a.py
  )
add_test(
  NAME feature-b
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/feature-b.py
  )
set_tests_properties(
  feature-a
  feature-b
  PROPERTIES
      FIXTURES_REQUIRED my-fixture
  )
add_test(
  NAME cleanup
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/cleanup.py
  )
set_tests_properties(
  cleanup
  PROPERTIES
      FIXTURES_CLEANUP my-fixture
  )
```

定义了一个文本固件，并将其称为`my-fixture`。我们为安装测试提供了`FIXTURES_SETUP`属性，并为清理测试了`FIXTURES_CLEANUP`属性，并且使用`FIXTURES_REQUIRED`，我们确保测试`feature-a`和`feature-b`都需要安装和清理步骤才能运行。将它们绑定在一起，可以确保在定义良好的状态下，进入和离开相应的步骤。

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">相关源码</font></td></tr></body></table>

**setup.py**

```python
import sys

print("tearing down")

# report success
sys.exit(0)
```

**feature-a.py**

```python
import sys

print("running test a")

# report success
sys.exit(0)
```

**feature-b.py**

```python
import sys

print("running test b")

# report success
sys.exit(0)
```

**clearup.py**

```python
import sys

print("tearing down")

# report success
sys.exit(0)
```

<table><body text=red><tr><td style="text-align:left;font-weight:bold" bgcolor=yellow><font size="3" color="red">输出结果</font></td></tr></body></table>

```shell
ctest
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/09/build
    Start 1: setup
1/4 Test #1: setup ............................   Passed    0.01 sec
    Start 2: feature-a
2/4 Test #2: feature-a ........................   Passed    0.01 sec
    Start 3: feature-b
3/4 Test #3: feature-b ........................   Passed    0.01 sec
    Start 4: cleanup
4/4 Test #4: cleanup ..........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.05 sec
ctest -R feature-a
Test project /home/jiangli/repo/tutorials/cmake-tutorial/chapter4/09/build
    Start 1: setup
1/3 Test #1: setup ............................   Passed    0.01 sec
    Start 2: feature-a
2/3 Test #2: feature-a ........................   Passed    0.01 sec
    Start 4: cleanup
3/3 Test #4: cleanup ..........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 3

Total Test time (real) =   0.03 sec
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_22/  

