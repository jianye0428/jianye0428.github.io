# Effective STL [10] | 注意分配器的协定和约束


## 分配器

分配器最初是为抽象内存模型而开发的，允许库开发者忽略在某些16位操作系统上near和far指针的区别，也被设计成促进全功能内存管理器的发展，但事实表明那种方法在STL的一些部分会<font color=red>导致效率损失</font>。

分配器最初被设想为抽象内存模型，在那种情况下，分配器在它们定义的内存模型中提供指针和引用的`typedef`才有意义。在C++标准里，类型T的对象的默认分配器（巧妙地称为`allocator<T>`）提供`typedef allocator<T>::pointer`和`allocator<T>::reference`，而且也希望用户定义的分配器也提供这些typedef。

建立行为像引用的对象是使用代理对象的例子，而代理对象会导致很多问题。

实际上标准明确地允许库实现假设每个分配器的pointer typedef是T*的同义词，每个分配器的reference typedef与T&相同。对，库实现可以忽视typedef并直接使用原始指针和引用！

分配器是对象，那表明它们可能有成员功能，内嵌的类型和typedef（例如pointer和reference）等等，但标准允许STL实现认为所有相同类型的分配器对象都是等价的而且比较起来总是相等。

```c++
template<typename T> // 一个用户定义的分配器
class SpecialAllocator {...}; // 模板
typedef SpecialAllocator<Widget> SAW; // SAW = “SpecialAllocator for Widgets”
list<Widget, SAW> L1;
list<Widget, SAW> L2;
...
L1.splice(L1.begin(), L2); // 把L2的节点移到 L1前端
```

记住当list元素从一个list被接合到另一个时，没有拷贝什么。取而代之的是，调整了一些指针，曾经在一个list中的节点发现他们自己现在在另一个list中。这使接合操作既迅速又异常安全。

当L1被销毁时，当然，它必须销毁它的所有节点（以及回收它们的内存），而因为它现在包含最初是L2一部分的节点，L1的分配器必须回收最初由L2的分配器分配的节点。

现在清楚为什么<font color=blue>**标准允许STL实现认为相同类型的分配器等价**</font>。所以由一个分配器对象（比如L2）分配的内存可以安全地被另一个分配器对象（比如L1）回收。

STL实现可以认为相同类型的分配器等价是多严厉的约束。那意味着可移植的分配器对象——<font color=blue>**不能有状态**</font>。让我们明确这一点：它意味着可移植的分配器不能有任何非静态数据成员，至少没有会影响它们行为的。

## 分配原始内存

分配器在分配原始内存方面类似operator new，但它们的接口不同。如果你看看operator new和allocator::allocate最普通形式的声明，就会很清楚。

```c++
void* operator new(size_t bytes);
pointer allocator<T>::allocate(size_type numObjects); // 记住事实上“pointer”总是 T*的typedef
```

两者都带有一个指定要分配多少内存的参数，但对于`operator new`，这个参数指定的是字节数，而对于`allocator::allocate`，它指定的是内存里要能容纳多少个T对象。

通常`allocator::size_type`是一个`size_t`的`typedef`。

`operator new`和`allocator::allocate`的返回类型也不同。`operator new`返回`void`，那是C++传统的表示一个到未初始化内存的指针的方式。`allocator::allocate`返回一个T（通过`pointer typedef`），不仅不传统，而且是有预谋的欺诈。从`allocator::allocate`返回的指针并不指向一个T对象，因为T还没有被构造！

## 大多数标准容器从未调用它们例示的分配器

```c++
list<int> L; // 和list<int, allocator<int> >一样； allocator<int>从未用来分配内存！
set<Widget, SAW> s; // 记住SAW是一个 SpecialAllocator<Widget>的typedef； SAW从未分配内存！
```

因为`set`、`multiset`、`map`和`multimap`是基于节点的容器，即，这些容器所基于的数据结构是每当值被储存就动态分配一个新节点。对于list，节点是列表节点。

**<font color=lightseagreen>对于标准关联容器，节点通常是树节点，因为标准关联容器通常用平衡二叉搜索树实现。</font>**

`list`本身由节点组成，每个节点容纳一个T对象和到list中后一个和前一个节点的指针：

```c++
template<typename T, // list的可能
typename Allocator = allocator<T> > // 实现
class list{
private:
Allocator alloc; // 用于T类型对象的分配器
    struct ListNode{ // 链表里的节点
        T data:
        ListNode *prev;
        ListNode *next;
    };
...
};
```

当添加一个新节点到`list`时，我们需要从分配器为它获取内存，我们要的不是`T`的内存，我们要的是包含了一个`T`的`ListNode`的内存。那使我们的`Allocator`对象没用了，因为它不为`ListNode`分配内存，它为T分配内存。现在你理解list为什么从未让它的`Allocator`做任何分配了：分配器不能提供list需要的。

list需要的是从它的分配器类型那里获得用于ListNode的对应分配器的方法。

```c++
template<typename T> // 标准分配器像这样声明，
class allocator { // 但也可以是用户写的分配器模板
public:
    template<typename U>
    struct rebind{
    typedef allocator<U> other;
}
...
};
```

在list的实现代码里，需要确定我们持有的T的分配器所对应的`ListNode`的分配器类型。我们持有的T的分配器类型是模板参数`Allocator`。

在本例中，`ListNodes`的对应分配器类型是：

```c++
Allocator::rebind<ListNode>::other
```

每个分配器模板A（例如，`std::allocator`，`SpecialAllocator`等）都被认为有一个叫做`rebind`的内嵌结构体模板。`rebind`带有一个类型参数`U`, 并且只定义一个`typedef A<U> other`. other是`A<U>`的一个简单名字。

结果，List<T>可以通过`Allocator::rebind<ListNode>::other`从它用于T对象的分配器（叫做Allocator）获取对应的`ListNode`对象分配器。

## 结论
如果你想要写自定义分配器，让我们总结你需要记得的事情。

- 把你的分配器做成一个模板，带有模板参数T，代表你要分配内存的对象类型。
- 提供pointer和reference的typedef，但是总是让pointer是T*，reference是T&。
- 决不要给你的分配器每对象状态。通常，分配器不能有非静态的数据成员。- 记得应该传给分配器的allocate成员函数需要分配的对象个数而不是字节数。也应该记得这些函数返回T*指针（通过pointer typedef），即使还没有T对象被构造。
- 一定要提供标准容器依赖的内嵌rebind模板。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_10/  

