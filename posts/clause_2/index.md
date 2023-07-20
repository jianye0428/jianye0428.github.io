# Effective STL 2 |  小心对“容器无关代码”的幻想


## STL 容器特点

STL是建立在**泛化**之上的

 - 数组泛化为容器，参数化了所包含的对象的类型</br>
 - 函数泛化为算法，参数化了所用的迭代器的类型</br>
 - 指针泛化为迭代器，参数化了所指向的对象的类型</br>

**独立的容器类型泛化为序列或关联容器，而且类似的容器拥有类似的功能。**

标准的内存相邻容器都提供随机访问迭代器，标准的基于节点的容器都提供双向迭代器。

序列容器支持`push_front`或`push_back`，但关联容器不支持。关联容器提供对数时间复杂度的`lower_bound`、`upper_bound`和`equal_range`成员函数，但序列容器却没有。

举例:
- 标准序列容器: vector、string、deque 和 list
- 标准关联容器: set、multiset、map 和 multimap

## 推行自己的容器

很多人会试图在他们的软件中泛化容器的不同，而不是针对容器的特殊性编程，他们会想在vector 中使用 deque 或者 list的特性，这往往会带来麻烦。

比如：

- 只有序列容器支持push_front或push_back，只有关联容器支持count和lower_bound

- 即便是 insert和erase这样的操作在名称和语义上也有差别

  - 当把对象插入序列容器中，该对象会保留在你放置的位置上;
  - 当你把对象插入到一个关联容器中，容器会按照排列顺序把对象移到它应该在的位置;
- 在序列容器上用一个迭代器作为参数调用 erase，会返回一个新的迭代器；在关联容器上什么都不返回。

**容器能力的交集**

如果你想写一个可以用在常用序列容器上的代码—— 包含vector, deque和list。你必须使用它们能力的交集来编写。

但要考虑几点：

- `deque`和`list`不支持`reserve`或`capacity`
- `list`不支持`operator[]`操作，且受限于双向迭代器的性能
- 不能使用需要随机访问迭代器的算法，包括`sort`，`stable_sort`，`partial_sort`和`nth_element`
- 如果想支持`vector`的规则，则不能使用`push_front`和`pop_front`
- `vector`和`deque`都会使`splice`和成员函数方式的`sort`失败
- 因为`deque::insert`会使所有迭代器失效，而且因为缺少`capacity`，`vector::insert`也必须假设使所有指针和引用失效，而deque是唯一一个在迭代器失效的情况下, 指针和引用仍然有效的东西
- **不能把容器里的数据传递给C风格的界面**，只有vector支持这么做
- **不能用bool作为保存的对象来实例化你的容器**，因为vector 并非总表现为一个vector，实际上它并没有真正保存bool值。
- 不能期望享受到list的常数时间复杂度的插入和删除，vector和deque的插入和删除操作是线性时间复杂度的

所以，真正开发时，如果都考虑到上面几点，那想开发的容器只剩下一个"泛化的序列容器"，但是你不能调用`reserve`、`capacity`、`operator[]`、`push_front`、`pop_front`、`splice`或任何需要随机访问迭代器的算法；调用insert和erase会有线性时间复杂度而且会使所有迭代器、指针和引用失效；而且不能兼容C风格的界面，不能存储bool。

如果你放弃了序列容器，把代码改为只能和不同的关联容器配合，这情况并没有什么改善。

 - 要同时兼容set和map几乎是不可能的，因为set保存单个对象，而map保存对象对。
 - 甚至要同时兼容set和multiset（或map和multimap）也是很难的。
 - `set/map`的`insert`成员函数只返回一个值，和他们的multi兄弟的返回类型不同，而且你必须避免对一个保存在容器中的值的拷贝份数作出任何假设。
 - 对于`map`和`multimap`，你必须避免使用`operator[]`，因为这个成员函数只存在于map中。


## 封装

如果想改变容器类型，就使用**封装**。

**Method 1: typedef**
一种最简单的方法是通过自由地对容器和迭代器类型使用typedef

```c++
class Widget {...};
vector<Widget> vw;
Widget bestWidget;
... // 给bestWidget一个值
vector<Widget>::iterator i =  // 寻找和bestWidget相等的Widget
find(vw.begin(), vw.end(), bestWidget);
```
可以简化上述写法

```c++
class Widget { ... };
typedef vector<Widget> WidgetContainer;
typedef WidgetContainer::iterator WCIterator;
WidgetContainer cw;
Widget bestWidget;
...
WCIterator i = find(cw.begin(), cw.end(), bestWidg
```

如果需要加上用户的allocator，也特别方便。（一个不影响对迭代器/指针/参考的失效规则的改变）

```c++
class Widget { ... };
template<typename T> // 关于为什么这里需要一个template
SpecialAllocator { ... }; // 请参见条款10
typedef vector<Widget, SpecialAllocator<Widget> > WidgetContainer;
typedef WidgetContainer::iterator WCIterator;
WidgetContainer cw; // 仍然能用
Widget bestWidget;
...
WCIterator i = find(cw.begin(), cw.end(), bestWidget); // 仍然能用
```
<font color=red>`typedef`只是其它类型的同义字，所以它提供的的封装是纯的词法（译注：不像#define是在预编译阶段替换的）。`typedef`并不能阻止用户使用（或依赖）任何他们不应该用的（或依赖的）。</font>

**Method 2: class**

要限制如果用一个容器类型替换了另一个容器可能需要修改的代码，就需要在类中隐藏那个容器，而且要通过类的接口限制容器特殊信息可见性的数量。

比如需要隐藏 真实的容器 list 建立客户列表：

```c++
class CustomerList {
private:
typedef list<Customer> CustomerContainer;
typedef CustomerContainer::iterator CCIterator;
CustomerContainer customers;
public: // 通过这个接口
... // 限制list特殊信息的可见性
};
```

如果使用过程中，你发现从列表的中部插入和删除客户并不像你想象的那么频繁，仅仅需要快速确定客户列表顶部的20%——一个为nth_element算法量身定做的任务。

但`nth_element`需要随机访问迭代器，不能兼容`list`。

在这种情况下，你的客户"list"可能更应该用"vector"或"deque"来实现

当你决定作这种更改的时候，你仍然**必须检查每个CustomerList的成员函数和每个友元，看看他们受影响的程度（根据性能和迭代器/指针/引用失效的情况等等）**。

但如果你做好了对CustomerList地实现细节做好封装的话，那对CustomerList的客户的影响将会很小。


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_2/  

