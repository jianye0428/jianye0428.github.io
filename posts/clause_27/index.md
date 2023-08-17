# Effective STL [27] | 用distance和advance把const_iterator转化成iterator


## 把const_iterator转化为iterator

有些容器成员函数只接受iterator作为参数，而不是const_iterator。如果你只有一个const_iterator，要在它所指向的容器位置上插入新元素呢？

上一条款说并不存在从const_iterator到iterator之间的隐式转换，那该怎么办？

看看当你把一个const_iterator映射为iterator时会发生什么:

```c++
typedef deque<int> IntDeque; // 方便的typedef
typedef IntDeque::iterator Iter;
typedef IntDeque::const_iterator ConstIter;
ConstIter ci; // ci是const_iterator
...
Iter i(ci); // 错误！没有从const_iterator 到iterator隐式转换的途径
Iter i(const_cast<Iter>(ci)); // 仍是个错误！不能从const_iterator 映射为iterator！
```

这里只是以deque为例，但是用其它容器类——list、set、multiset、map、multimap甚至条款25描述的散列表容器——的结果一样。使用映射的行也许在vector或string的代码时能够编译，但这是我们马上要讨论的非常特殊的情形。

上述代码不能通过编译的原因在于，对于这些容器而言，<font color=red>iterator和const_iterator是完全不同的类。</font>

在两个毫无关联的类之间进行`const_cast`映射是荒谬的，所以`reinterpret_cast`、`static_cast`甚至C风格的映射也会导致同样的结果。

**不能编译的代码对于vector和string容器来说也许能够通过编译**

那是因为通常情况下大多数实现都会采用真实的指针作为那些容器的迭代器。

就这种实现而言，`vector<T>::iterator`是`T*`的`typedef`，而`vector<T>::const_iterator`是`const T*`的`typedef`，`string::iterator`是`char`的`typedef`，而`string::const_iterator`是`const char*`的`typedef`。

在这种实现的情况下，用`const_cast`把`const_iterator`映射成`iterator`当然可以编译而且没有问题，因为`const_iterator`与`iterator`之间的`const_cast`映射被最终解释成`const T*`到`T*`的映射。但是，即使是在这种实现中，`reverse_iterator`和`const_reverse_iterator`也是真正的类，所以你仍然不能直接用`const_cast`把`const_reverse_iterator`映射成`reverse_iterator`。

而且这些实现通常只会在Release模式时才使用指针表示vector和string的迭代器。

所有这些事实表明，把const迭代器映射为迭代器是病态的，即使是对vector和string来说也时，因为移植性很值得怀疑。

## const_iterator转换为iterator

有一种安全的、可移植的方法获取它所对应的iterator，而且，用不着陷入类型系统的转换。

```c++

#include <deque>
#include <iterator>

typedef deque<int> IntDeque; // 和以前一样
typedef IntDeque::iterator Iter;
typedef IntDeque::const_iterator ConstIter;
IntDeque d;
ConstIter ci;
... // 让ci指向d
Iter i(d.begin()); // 初始化i为d.begin()
advance(i, distance(i, ci)); // 把i移到指向ci位置（但请留意下面关于为什么在它编译前要调整的原因）
```

要得到与const_iterator指向同一位置的iterator:

1. 将iterator指向容器的起始位置，
2. 把它向前移到和const_iterator距离容器起始位置的偏移量一样的位置即可

这个任务得到了两个函数模板advance和distance的帮助，它们都在<iterator>中声明:
 - distance返回两个指向同一个容器的iterator之间的距离；
 - advance则用于将一个iterator移动指定的距离。

如果`i`和`ci`指向同一个容器，那么表达式`advance(i, distance(i, ci))`会将`i`移动到与`ci`相同的位置上。

上述代码编译存在问题。

先来看看distance的定义：

```c++
template<typename _InputIterator>
  inline _GLIBCXX17_CONSTEXPR
  typename iterator_traits<_InputIterator>::difference_type
  distance(_InputIterator __first, _InputIterator __last)
  {
    // concept requirements -- taken care of in __distance
    return std::__distance(__first, __last,
      std::__iterator_category(__first));
  }
```

当遇到`distance`调用时，你的编译器需要根据使用的实参类型推断出`InputIterator`的类型。

再来看看我所说的不太正确的`distance`调用:

```c++
advance(i, distance(i, ci)); // 调整i，指向ci位置
```

有两个参数传递给distance，i和ci。i的类型是Iter，即`deque<int>::iterator`的typedef。

对编译器来说，这表明调用`distance`的`InputIterator`是`deque::iterator`。但`ci`是`ConstIter`，即`deque::const_iterator`的`typedef`。

表明那个`InputIterator`是`deque<int>::const_iterator`。

`InputIterator`不可能同时有两种不同的类型，所以调用`distance`失败。

一般会造成一些冗长的出错信息，可能会也可能不会说明是编译器无法得出`InputIterator`是什么类型。

要顺利地调用`distance`，你需要排除歧义。

最简单的办法就是显式的指明`distance`调用的模板参数类型，从而避免编译器自己得出它们的类型:

```c++
advance(i, distance<ConstIter>(i, ci));
```

我们现在知道了怎么通过`advance`和`distance`获取`const_iterator`相应的`iterator`了。

**效率如何？**

答案很简单。**取决于你所转换的究竟是什么样的迭代器。**

对于**随机访问的迭代器**（比如`vector`、`string`和`deque`的）而言，这是**常数时间**的操作。

对于**双向迭代器**（也就是，所有其它容器和包括散列容器的一些实现）而言，这是**线性时间**的操作。

因为它可能花费线性时间的代价来产生一个和`const_iterator`等价的`iterator`，并且因为如果不能访问`const_iterator`所属的容器这个操作就无法完成。

从这个角度出发，也许你需要重新审视你从`const_iterator`产生`iterator`的设计。

当处理容器时尽量用`iterator`代替`const`和`reverse`迭代器。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_27/  

