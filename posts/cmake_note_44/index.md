# CMake 笔记 | [44] 验证自己生成的库


## 一、导言
{{<admonition quote "" false>}}
**经过上一篇（[CMake:输出库（像其他优秀的库一样优雅）](https://mp.weixin.qq.com/s?__biz=MzkxMzI5Mjk4Mg==&mid=2247485493&idx=1&sn=153d11f6db08ef0f70ce8bb95d7e0417&scene=21#wechat_redirect)），我们已经构建出了优雅的库。本篇，我们将基于上一篇的内容，写一个简单的使用demo进行验证！**
{{</admonition>}}

## 二、项目结构

```shell
.
├── CMakeLists.txt
└── use_message.cpp
```

项目地址：

> https://gitee.com/jiangli01/tutorials/tree/master/cmake-tutorial/chapter9/04



## 三、相关源码

**CMakeLists.txt**

```c++
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(use-message
  LANGUAGES CXX
  )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH /home/jiangli/repo/tutorials/cmake-tutorial/chapter9/03/output/share/cmake/example)
find_package(message REQUIRED CONFIG)

if(message_FOUND)
  message(STATUS "Found message: (found version ${message_VERSION})")
endif()

include_directories(/home/jiangli/repo/tutorials/cmake-tutorial/chapter9/03/output/include)

add_executable(use_message use_message.cpp)

target_link_libraries(use_message message::message-shared)
```

{{<admonition quote "" false>}}
这里，我们没有对我们的库添加到环境变量中，所以为了使`find_package`命令能够找到我们的库，需要在`find_package`命令前设置库所在的`cmake`文件路径。
```
set(CMAKE_PREFIX_PATH /home/jiangli/repo/tutorials/cmake-tutorial/chapter9/03/output/share/cmake/example)
```
如果我们把生成的库设置到环境变量中，那么我们就可以省略掉这一步骤！
{{</admonition>}}


**use_message.cpp**

```c++
#include <cstdlib>
#include <iostream>

#include <message/message.hpp>

int main() {
  Message say_hello("Hello, CMake World!");

  std::cout << say_hello << std::endl;

  Message say_goodbye("Goodbye, CMake World");

  std::cout << say_goodbye << std::endl;

  return EXIT_SUCCESS;
}
```

## 四、结果展示

```shell
mkdir build & cd build
cmake ..
make .
./use_message
```

<br>
<center>
  <img src="images/4_01.png" width="640" height="320" align=center style="border-radius: 0.3125em; box-shadow: 0 2px 4px 0 rgba(34,36,38,.12),0 2px 10px 0 rgba(34,36,38,.08);">
  <br>
  <div style="color:orange; border-bottom: 1px solid #d9d9d9; display: inline-block; color: #999; padding: 2px;">验证生成的库</div>
</center>
<br>







![Image](data:image/svg+xml,%3C%3Fxml version='1.0' encoding='UTF-8'%3F%3E%3Csvg width='1px' height='1px' viewBox='0 0 1 1' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'%3E%3Ctitle%3E%3C/title%3E%3Cg stroke='none' stroke-width='1' fill='none' fill-rule='evenodd' fill-opacity='0'%3E%3Cg transform='translate(-249.000000, -126.000000)' fill='%23FFFFFF'%3E%3Crect x='249' y='126' width='1' height='1'%3E%3C/rect%3E%3C/g%3E%3C/g%3E%3C/svg%3E)





![Image](data:image/svg+xml,%3C%3Fxml version='1.0' encoding='UTF-8'%3F%3E%3Csvg width='1px' height='1px' viewBox='0 0 1 1' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'%3E%3Ctitle%3E%3C/title%3E%3Cg stroke='none' stroke-width='1' fill='none' fill-rule='evenodd' fill-opacity='0'%3E%3Cg transform='translate(-249.000000, -126.000000)' fill='%23FFFFFF'%3E%3Crect x='249' y='126' width='1' height='1'%3E%3C/rect%3E%3C/g%3E%3C/g%3E%3C/svg%3E)



cmake45

cmake · 目录

上一篇CMake:输出库（像其他优秀的库一样优雅）下一篇CMake:使用Doxygen构建文档



People who liked this content also liked

CMake:超级构建模式



Hope Hut

不喜欢

不看的原因

OK

- 内容质量低
- 不看此公众号

![img](https://mmbiz.qpic.cn/sz_mmbiz_jpg/sd9Xic2LWs1UWYaO93spsRvKt3OOuS6wEZYsTAEFh3wErfaBLzIL3Rga9mUibLiaZ8NYc21AFPQ37qEckj64DNWkQ/0?wx_fmt=jpeg)

CMake:使用函数和宏重用代码



Hope Hut

不喜欢

不看的原因

OK

- 内容质量低
- 不看此公众号

![img](https://mmbiz.qpic.cn/sz_mmbiz_jpg/sd9Xic2LWs1XSITicZy4hiaRDWMNVf5Sw28aX23cbibXMTEiaS0JjEt8hqR1ur7uVk5ZtSGUicic1Sd5N9hOQvXiboqiaqw/0?wx_fmt=jpeg)

CMake:使用Doxygen构建文档



Hope Hut

不喜欢

不看的原因

OK

- 内容质量低
- 不看此公众号

![img](https://mmbiz.qpic.cn/sz_mmbiz_jpg/sd9Xic2LWs1WGpostfpm9PtqeEibqiaGX5J6MEJqn9ggdLlzlBpEn2Ohd2KQHE7Ac9pDK823S9brsfxcPqMa1RRzw/0?wx_fmt=jpeg)

![img](https://mp.weixin.qq.com/mp/qrcode?scene=10000004&size=102&__biz=MzkxMzI5Mjk4Mg==&mid=2247485507&idx=1&sn=5ed4771fb64fe63e64ecc4c2be2de5d2&send_time=)

Scan to Follow

people underline

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_44/  

