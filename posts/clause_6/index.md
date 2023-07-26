# Effective STL [6] | 警惕C++最令人恼怒的解析


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->

## 函数声明的几种方式

1. 声明一个函数f带有一个double而且返回一个int：

```c++
int f(double d);
```

2. 名为d的参数左右的括号是多余的，被忽略：

```c++
int f(double (d)); // 同上；d左右的括号被忽略
```

3. 省略了参数名：

```c++
int f(double); // 同上；参数名被省略
```
4. 第一个声明了一个函数g，它带有一个参数，那个参数是指向一个没有参数、返回double的函数的指针：

```c++
int g(double (*pf)()); // g带有一个指向函数的指针作为参数
```

5. 唯一的不同是pf使用非指针语法来声明(一个在C和C++中都有效的语法):

```c++
int g(double pf()); // 同上；pf其实是一个指针
```

6. 照常，参数名可以省略，所以这是g的第三种声明，去掉了pf这个名字：

```c++
int g(double ()); // 同上；参数名省略
```

注意参数名左右的括号（就像f的第二种声明中的d）和单独的括号（正如本例）之间的区别。

**参数名左右的括号被忽略，但单独的括号指出存在一个参数列表：它们声明了存在指向函数的指针的参数。**

## 问题探讨

假设有一个int的文件，想要把那些int拷贝到一个list中，可能会使用下面代码：

```c++
ifstream dataFile("ints.dat");
// 警告！这完成的并不是像你想象的那样
list<int> data(istream_iterator<int>(dataFile),
istream_iterator<int>());
```

这里的想法是传一对`istream_iterator`给`list`的区间构造函数，因此把int从文件拷贝到list中。

这段代码可以编译，但是运行时什么都不会做，不会从文件中读出任何数据，甚至不会构建1个`list`。

第二句并不声明list，也不调用构造函数。

这声明了一个函数data，它的返回类型是`list<int>`。这个函数data带有两个参数：

● 第1个参数叫做dataFile。它的类型是`istream_iterator<int>`。dataFile左右的括号是多余的而且被忽略。

● 第2个参数没有名字。它的类型是指向一个没有参数而且返回`istream_iterator<int>`的函数的指针。

就像下面具有这条规则的代码：

```c++
class Widget {...}; // 假设Widget有默认构造函数
Widget w();
```

这并没有声明一个叫做w的Widget，它声明了一个叫作w的没有参数且返回Widget的函数。

本来代码的初衷，是用一个文件的内容来初始化一个`list<int>`对象，现在并没有达到我们的期望。

## 解决办法

1. 函数调用前后增加括号
用括号包围一个实参的声明是不合法的，但用括号包围一个函数调用的观点是合法的，所以通过增加一对括号，代码变为：

```c++
list<int> data((istream_iterator<int>(dataFile)), // 注意在list构造函数的第一个实参左右的新括号
istream_iterator<int>());
```

这是可能的声明数据方法，给予`istream_iterators`的实用性和区间构造函数。

2. 命名迭代器对象

```c++
ifstream dataFile("ints.dat");
istream_iterator<int> dataBegin(dataFile);
istream_iterator<int> dataEnd;
list<int> data(dataBegin, dataEnd);
```

命名迭代器对象的使用和普通的STL编程风格相反，但是你得判断这种方法对编译器和必须使用编译器的人都模棱两可的代码是一个值得付出的代价。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_6/  

