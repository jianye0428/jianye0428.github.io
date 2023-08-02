# Effective STL [12] | 理解自定义分配器的正确用法


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->

## 多线程维护的规则

多线程程序是很普遍的，所以大部分STL厂商努力使他们的实现在线程环境中可以正常工作。

但是，即使他们做得很好，「**大部分负担仍在你肩上，而理解为什么会这样是很重要的**」。

在STL容器（和大多数厂商的愿望）里对多线程支持的黄金规则已经由SGI定义，并且在它们的STL网站上发布：
 - 「**多个读取者是安全的**」。多线程可能同时读取一个容器的内容，在读取时「不能有任何写入者操作这个容器」。
 - 「**对不同容器的多个写入者是安全的**」。多线程可以同时写不同的容器。对同一个容器同时多线程写入是不安全的。

## 完全线程安全？

很多程序员希望STL实现是完全线程安全，这非常难以实现。

一个库可能试图以下列方式实现这样完全线程安全的容器：
- 每次调用容器的成员函数期间都要锁定该容器」
- 在「**每个容器返回的迭代器(例如通过调用begin或end)的生存期之内都要锁定该容器**」
- 在「**每个在容器上调用的算法执行期间锁定该容器**」。（这事实上没有意义，因为算法没有办法识别出它们正在操作的容器）

## Example 多线程修改

搜寻一个vector<int>中第一次出现3这个值的地方，如果它找到了，就把这个值改为2。

```c++
vector<int> v;
vector<int>::iterator first3(find(v.begin(), v.end(), 3)); // line 1
if (first3 != v.end()){ // line 2
    *first3 = 2; // line 3
}
```
多线程访问时，会有很多「问题」:

1. 在多线程环境里，另一个线程可能在行1完成之后立刻修改v中的数据。如果是那样，行2对first3和v.end的检测将是无意义的，因为v的值可能和它们在行1结束时的值不同。
2. 这样的检测会产生未定义的结果，因为另一线程可能插在行1和行2之间，使first3失效，或许通过进行一次插入操作造成vector重新分配它的内在内存。（那将使vector全部的迭代器失效）
3. 行3中对`*first3`的赋值是不安全的，因为另一个线程可能在行2和行3之间执行，并以某种方式使`first3`失效，可能通过删除它指向（或至少曾经指向）的元素。


**解决办法：加锁**

要让上面的代码成为线程安全的，「**v必须从行1到行3保持锁定**」，让最多一个线程在1-3行的过程中能访问v。很难想象STL实现怎么能自动推断出这个。

而「**同步原语（例如，信号灯，互斥量，等等）通常开销很大**」，更难想象怎么实现在程序没有明显性能损失的情况下做到前面所说的。

因此你必须「**手工对付**」这些情况中的同步控制

```c++
vector<int> v;
...
getMutexFor(v);
vector<int>::iterator first3(find(v.begin(), v.end(), 3));
if (first3 != v.end()) { // 这里现在安全了
    *first3 = 2; // 这里也是
}
releaseMutexFor(v);
```

「**改进**：」一个更面向对象的解决方案是「**创建一个Lock类**」，在它的「**构造函数里获得互斥量并在它的析构函数里释放它**」，这样使**getMutexFor**和**releaseMutexFor**的「**调用不匹配的机会减到最小**」:

```c++
template<typename Container> // 获取和释放容器的互斥量的类的模板核心；
class Lock { /
 public: // 忽略了很多细节
    Lock(const Containers container): c(container) {
        getMutexFor(c); // 在构造函数获取互斥量
    }

    ~Lock() {
        releaseMutexFor(c); // 在析构函数里释放它
    }

 private:
 const Container& c;
};
```

使用一个类（像Lock）来管理资源的生存期（例如互斥量）的办法通常称为**资源获得即初始化**。


应用到上述例子：
```c++
vector<int> v;
...
{ // 建立新块；
    Lock<vector<int> > lock(v); // 获取互斥量
    vector<int>::iterator first3(find(v.begin(), v.end(), 3));
    if (first3 != v.end()) {
      *first3 = 2;
 }
} // 关闭块，自动
// 释放互斥量
```

**基于Lock的方法在有异常的情况下是稳健的**

因为Lock对象在Lock的析构函数里释放容器的的互斥量，所以在互斥量需要释放是就销毁Lock是很重要的。为了让这件事发生，我们建立一个里面定义了Lock的新块，而且当我们不再需要互斥量时就关闭那个块。

C++保证如果抛出了异常，局部对象就会被销毁，所以即使当我们正在使用Lock对象时有异常抛出，Lock也将释放它的互斥量。如果我们依赖手工调用`getMutexFor`和`releaseMutexFor`，那么在调用`getMutexFor`之后`releaseMutexFor`之前如果有异常抛出，我们将不会释放互斥量。

## 结论

- 当涉及到线程安全和STL容器时，你可以确定库实现允许在一个容器上的多读取者和不同容器上的多写入者。

- 「你不能希望库消除对手工并行控制的需要，而且你完全不能依赖于任何线程支持」

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_12/  

