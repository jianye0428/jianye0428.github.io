# Effective C++(第3版)精读总结(一)


{{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}}

ref:
[1]. https://blog.csdn.net/cltcj/category_12098441.html

<!--more-->
# 序言

这本C++的经典之作，作者是大佬`Scott Meyers`👉[大佬主页](https://www.aristeia.com/books.html)，还写过其他几本影响深远的C++经典，例如`《Effective STL》`,`《More Effective C++》`,`《Effective Mordern C++》`,`《Overview of the New C++(C++11/14)》`等等。本人看的是中文版，侯捷老师翻译的，精读分析并实践推敲后，整理成博客记录下来。

> (Effective-C++总结系列分为四部分，本文为第一部分，涉及原书第1~2章，内容范围Rule01~12。为方便书写，Rule01简写为R01)。

{{< admonition Note "Effective-C++系列List" false >}}
本博客站点系列内容如下：</br>
💡 Effective C++(第3版)精读总结(一)</br>
💡 Effective C++(第3版)精读总结(二)</br>
💡 Effective C++(第3版)精读总结(三)</br>
💡 Effective C++(第3版)精读总结(四)</br>
{{< /admonition >}}

# CH1.让自己习惯C++

## R01 视C++为一个语言联邦

如今的C++已经是个多重范式(multiparadigm)语言，同时支持面向过程形式、面向对象形式、函数形式、泛型形式、元编程形式。 要理解这么多特性，可以简单的归结为`四种次语言`(sublanguage)组成：

- **C语言：**`C++仍以C为基础`。C++是C的超集，区块 、语句、预处理、内置数据类型、数组、指针等全部来自于C语言；
  - 说到底 C++ 仍然以 C 为基础。区块、语句、预处理器、内置数据类型 、数组、指针等统统来自C，许多时候C++对问题的解决其实不过就是较高级的 C 解法，但当你C++内的 C 成分工作时，高效编程守则映照出 C 语言的局限：没有模板(template) ，没有异常(exceptions)，没有重载(overloading)……
- **Object-Oriented C++:** 面向对象特性。这部分也就是 C with classes 所诉求的：classes(包括构造函数和析构函数)，封装(encapsulation)、继承(inheritance)、多态(polymorhpism)、virtual函数(动态绑定)……等等，这一部分是面向对象设计之古典守则在C++ 上的直接实施。
- **Template C++:**C++的泛型(generic)编程的部分，也带来了黑魔法-模板元编程(TMP,Metaprogramming)；
- **STL：**STL(Standard Temlate Library)即标准模板库，它是template程序库。封装了各类容器(container)、配置器(allocator)、迭代器(iterator)、算法以及常用对象。

**总结:**
C++高效编程守则视状况而变化，取决于你使用C++的哪一部分

## R02 尽量以`const`,`enum`,`inline`替换`#define`

- **对于宏定义的常量，建议用const常量或者枚举enum替换**
  这样做的好处是方便调试，因为宏报错就是个常数值，没有符号表；并且宏不具有封装性(宏的作用域是在编译时是其定义之事)。
  ```c++
  // 举例：MAX_DATA_COUNT在预处理阶段就会被替换，编译器不会见到它，所以一旦有相关报错，给的是100这个值
  #define MAX_DATA_COUNT   100
  const  int MAX_DATA_COUNT = 100 ;//常量只有一份，宏会导致多份常量值
  class Buffer{
  public://...类其他部分省略
      static const double factor_ ;//static常量,类内声明
      static const int  times_ = 2;// int类型允许类内初始化,规范上还是建议拿到类外
  private:
      static const  int ArrLength = 5;
      int arr[ArrLength];
  };
  const double Buffer::factor_  = 0.1;//类外初始化,一般写在实现文件*.cpp,*.cc中
  ```
  如果编译器不允许声明时"in-class初值设定",如果是整形常量，可以让枚举值来替代，而且<font color=red>`枚举值不能被取地址`</font>。
- **对于宏定义的函数，建议用内联inline函数替换**
  宏函数没办法单行debug调试，而内联函数可以；
  宏的写法即使小心翼翼的加好了括号，也可能造成意想不到的<font color=red>`宏函数重复计算`</font>的问题。


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partone/  

