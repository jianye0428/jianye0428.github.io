# Effective STL [7] | 当使用new得指针的容器时，记得在销毁容器前delete那些指针


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->

## STL容器能够做的事情
- 提供了前向和逆向遍历的迭代器（通过`begin`、`end`、`rbegin`等）；

- 能告诉你所容纳的对象类型（通过`value_type`的`ttypedef`）；

- 在插入和删除中，负责任何需要的内存管理；

- 报告容纳了多少对象和最多可能容纳的数量（分别通过`size`和`max_size`）；

- 当容器自己被销毁时会自动销毁容纳的每个对象。

## 容器内包含指针

虽然STL容器被销毁时，能够自动销毁容纳的每个对象，但是如果这些对象是通过new分配的对象的指针时，它不会调用`delete`，销毁指针所指向的对象。

**Example**
```c++
void doSomething() {
    vector<Widget*> vwp;
    for (int i = 0; i < SOME_MAGIC_NUMBER; ++i)
    vwp.push_back(new Widget);
    ... // work
} // Widgets在这里泄漏！
```

这段代码将直接导致内存泄露。

当`vwp`结束其生命周期后，`vwp`的每个元素都被销毁，但不会`delete`每个`new`得到的对象。

那样的删除是你的职责，而不是vector的。这是一个特性。只有你知道一个指针是否应该被删除。

可以很简单地实现：

```c++
void doSomething()
{
    vector<Widget*> vwp;
    ... // work
    for (vector<Widget*>::iterator i = vwp.begin();
    i != vwp.end(), ++i) {
     delete *i;
    }
}
```

这段销毁的代码，仍然有2个问题：
- 新的for循环代码比for_each多得多，没有使用for_each来的清楚
- 这段代码不是异常安全的。如果在用指针填充了vwp的时候和你要删除它们之间抛出了一个异常，你会再次资源泄漏。

**for_each删除对象**

要把你的类似for_each的循环转化为真正使用for_each，你需要把delete转入一个函数对象中。

```c++
template<typename T>
struct DeleteObject :
public unary_function<const T*, void> { // 这里有这个继承
    void operator()(const T* ptr) const
    {
     delete ptr;
    }
};
```

现在可以这么删除对象
```c++
void HappyWork()
{
    ... // work
    for_each(vwp.begin(), vwp.end(), DeleteObject<Widget>);
}
```

**问题**

如果有人编写了一个类，该类继承了 string

```c++
class SpecialString: public string { ... };
```

这是很危险的行为，因为string，就像所有的标准STL容器，**缺少虚析构函数**，而从没有虚析构函数的类公有继承是一个大的C++禁忌。

当他删除 SpecialString 时就会资源泄露

```c++
void doSomething()
{
    deque<SpecialString*> dssp;
    ...
    for_each(dssp.begin(), dssp.end(), // 行为未定义！通过没有
    DeleteObject<string>()); // 虚析构函数的基类
} // 指针来删除派生对象
```

**解决**

可以通过编译器推断传给`DeleteObject::operator()`的指针的类型来消除这个错误（也减少DeleteObject的用户需要的击键次数）。

**把模板化从DeleteObject移到它的operator()**：

```c++
struct DeleteObject { // 删除这里的
    // 模板化和基类
    template<typename T> // 模板化加在这里
    void operator()(const T* ptr) const
    {
     delete ptr;
    }
}
```

通过传给`DeleteObject::operator()`的指针的类型，自动实例化一个`operator()`。这种类型演绎下降让我们放弃使`DeleteObject`可适配的能力

现在删除 SpecialString 就会正常了
```c++
void doSomething()
{
    deque<SpecialString*> dssp;
    ...
    for_each(dssp.begin(), dssp.end(), DeleteObject()); // good！
}
```

<font color=red>现在仍不是异常安全的。</font>

果在SpecialString被new但在调用for_each之前抛出一个异常，就会发生泄漏。

这个问题可以以多种方式被解决，但最简单的可能是用**智能指针的容器来代替指针的容器，典型的是引用计数指针**。

## Boost库中的shared_ptr

利用Boost的shared_ptr，本条款的原始例子可以重写为这样：

```c++
void doSomething()
{
    typedef boost::shared_ ptr<Widget> SPW; //SPW = "shared_ptr to Widget"
    vector<SPW> vwp;
    for (int i = 0; i < SOME_MAGIC_NUMBER; ++i)
     vwp.push_back(SPW(new Widget)); // 从一个Widget建立SPW,然后进行一次push_back
    ... // work
} // 这里没有Widget泄漏，甚至在上面代码中抛出异常
```

## 结论
STL容器很智能，但它们没有智能到知道是否应该删除它们所包含的指针。

当你要删除指针的容器时要避免资源泄漏，你必须**用智能引用计数指针对象**（比如`Boost`的`shared_ptr`）来代替指针，或者你**必须在容器销毁前手动删除容器中的每个指针**。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_7/  

