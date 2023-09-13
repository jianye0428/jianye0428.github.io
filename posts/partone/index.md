# Effective STL 精读总结 [1] | 容器


# 前言

> Effective-STL总结系列分为七部分，本文为第一部分，涉及原书第一章，内容范围Rule01~12。为方便书写，Rule12简写为R12。

{{< admonition Note "Effective-STL系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective STL(第3版)精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 [Effective STL(第3版)精读总结(二)](https://jianye0428.github.io/posts/parttwo/)</br>
💡 [Effective STL(第3版)精读总结(三)](https://jianye0428.github.io/posts/partthree/)</br>
💡 [Effective STL(第3版)精读总结(四)](https://jianye0428.github.io/posts/partfour/)</br>
{{< /admonition >}}

## R01 慎重选择容器类型

STL 容器不是简单的好，而是确实很好。

容器类型如下：
  - 标准 STL 序列容器：`vector`、`string`、`deque`、`list`。
  - 标准 STL 关联容器：`set`、`multiset`、`map`、`multimap`。
  - 非标准序列容器 `slist` 和 `rope`。`slist` 是一个单向链表，`rope` 本质上是一个 "重型" `string`。
  - 非标准的关联容器：`hash_set`、`hash_multiset`、`hash_map`、`hash_multimap`。
  - `vector` 作为 `string` 的替代。
  - `vector` 作为标准关联容器的替代：有时 `vector` 在运行时间和空间上都要优于标准关联容器。
  - 几种标准的非 STL 容器：`array`、`bitset`、`valarray`、`stack`、`queue`、`priority_queue`。

容器选择标准:
  1. vector、list和deque有着不同的复杂度，vector是默认使用的序列类型。当需要频繁在序列中间做插入和删除操作时，应使用list。当大多数插入和删除操作发生在序列的头部和尾部时，应使用deque。
  2. 可以将容器分为连续内存容器和基于节点的容器两类。连续内存容器把元素存放在一块或多块(动态分配的)内存中，当有新元素插入或已有的元素被删除时，同一块内存中的其他元素要向前或向后移动，以便为新元素让出空间，或者是填充被删除元素所留下的空隙。这种移动会影响到效率和异常安全性。标准的连续内存容器有vector、string和deque，非标准的有rope。
  3. 基于节点的容器在每一个(动态分配的)内存块中只存放一个元素。容器中元素的插入或删除只影响指向节点的指针，而不影响节点本身，所以插入或删除操作时元素是不需要移动的。链表实现的容器list和slist是基于节点的，标准关联容器也是(通常的实现方式是平衡树)，非标准的哈希容器使用不同的基于节点的实现。
  4. 是否需要在容器的任意位置插入新元素？需要则选择序列容器，关联容器是不行的。
  5. 是否关心容器中的元素是如何排序的？如果不关心，可以选择哈希容器，否则不能选哈希容器(unordered)。
  6. 需要哪种类型的迭代器？如果是随机访问迭代器，那就只能选择vector、deque和string。如果使用双向迭代器，那么就不能选slist和哈希容器。
  7. 是否希望在插入或删除元素时避免移动元素？如果是，则不能选择连续内存的容器。
  8. 容器的数据布局是否需要和C兼容？如果需要只能选`vector`。
  9. 元素的查找速度是否是关键的考虑因素？如果是就考虑哈希容器、排序的`vector`和标准关联容器。
  10. 是否介意容器内部使用引用计数技术，如果是则避免使用`string`，因为`string`的实现大多使用了引用计数，可以考虑用`vector<char>`替代。
  11. 对插入和删除操作需要提供事务语义吗？就是说在插入和删除操作失败时，需要回滚的能力吗？如果需要则使用基于节点的容器。如果是对多个元素的插入操作(针对区间)需要事务语义，则需要选择`list`，因为在标准容器中，只有`list`对多个元素的插入操作提供了事务语义。对希望编写异常安全代码的程序员，事务语义尤为重要。使用连续内存的容器也可以获得事务语义，但是要付出性能上的代价，而且代码也不那么直截了当。
  12. 需要使迭代器、指针和引用变为无效的次数最少吗？如果是则选择基于节点的容器，因为对这类容器的插入和删除操作从来不会使迭代器、指针和引用变成无效(除非它们指向一个正在删除的元素)。而对连续内存容器的插入和删除一般会使得指向该容器的迭代器、指针和引用变为无效。

## R02 不要试图编写独立于容器类型的代码

1. 容器是以类型作为参数的，而试图把容器本身作为参数，写出独立于容器类型的代码是不可能实现的。因为不同的容器支持的操作是不同的，即使操作是相同的，但是实现又是不同的，比如带有一个迭代器参数的erase作用于序列容器时，会返回一个新的迭代器，但作用于关联容器则没有返回值。这些限制的根源在于，对于不同类型的序列容器，使迭代器、指针和引用无效的规则是不同的。

2. 有时候不可避免要从一个容器类型转到另一种，可以使用封装技术来实现。最简单的方式是对容器类型和其迭代器类型使用typedef，如`typedef vector<Widget> widgetContainer`; `typedef widgetContainer::iterator WCIterator`; 如果想减少在替换容器类型时所需要修改的代码，可以把容器隐藏到一个类中，并尽量减少那些通过类接口可见的、与容器有关的信息。

**一种容器类型转换为另一种容器类型：typedef**
```c++
class Widget{...};
typedef vector<Widget> WidgetContainer;
WidgetContainer cw;
Widget bestWidget;
...
WidgetContainer::iterator i = find(cw.begin(), cw.end(), bestWidget);
```
这样就使得改变容器类型要容易得多，尤其当这种改变仅仅是增加一个自定义得分配子时，就显得更为方便（这一改变不影响使迭代器/指针/引用无效的规则）。

## R03 确保容器中的对象拷贝正确而高效
- 存在继承关系的情况下，拷贝动作会导致**剥离**（slicing）: 如果创建了一个存放基类对象的容器，却向其中插入派生类对象，那么在派生类对象（通过基类的拷贝构造函数）被拷贝进容器时，它所特有的部分（即派生类中的信息）将会丢失。
  ```c++
  vector<Widget> vw;
  class SpecialWidget:			// SpecialWidget 继承于上面的 Widget
    public Widget{...};
  SpecialWidget sw;
  vw.push_back();					// sw 作为基类对象被拷贝进 vw 中
                  // 它的派生类特有部分在拷贝时被丢掉了
  ```
- **剥离意味着向基类对象中的容器中插入派生类对象几乎总是错误的**。
- 解决剥离问题的简单方法：<u>使容器包含指针而不是对象</u>。

1. 容器中保存的对象，并不是你提供给容器的那些对象。从容器中取出对象时，也不是容器中保存的那份对象。当通过如insert或push_back之类的操作向容器中加入对象时，存入容器的是该对象的拷贝。当通过如front或back之类的操作从容器中 取出对象时，所得到的是容器中对象的拷贝。进去会拷贝，出来也是拷贝，这就是STL的工作方式。
2. 当对象被保存到容器中，它经常会进一步被拷贝。当对vector、string或deque进行元素的插入或删除时，现有元素的位置通常会被移动（拷贝）。如果使用下列任何算法，next_permutation或previous_permutation，remove、unique，rotate或reverse等等，那么对象将会被移动（拷贝），这就是STL的工作方式。
3. 如果向容器中填充的对象拷贝操作很费时，那么向容器中填充对象这一简单操作将会成为程序的性能瓶颈。而且如果这些对象的拷贝有特殊含义，那么把它们放入容器还将不可避免地会产生错误。
4. 当存在继承关系时，拷贝动作会导致剥离。也就是说，如果创建了一个存放基类对象的容器，却向其中插入派生类的对象，那么派生类对象（通过基类的拷贝构造函数）被拷贝进容器时，它派生类的部分将会丢失。
5. 使拷贝动作高效、正确，并防止剥离问题发生的一个简单办法就是使容器包含对象指针，而不是对象本身。拷贝指针的速度非常快，而且总是会按你期望的方式进行。如果考虑资源的释放，智能指针是一个很好的选择。

## R04 调用 empty 而不是检查 size()是否为0

- `empty` 通常被实现为内联函数（inline function），并且它做的仅仅是返回 size() 是否为 0.
- `empty` 对所有标准容器都是常数时间操作，而对于一些 list 实现，size 耗费线性时间

## R05 区间成员函数优先于与之对应的单元素成员函数

1. **区间成员函数是使用两个迭代器参数来确定成员操作执行的区间，像STL算法一样**。区间成员函数和for循环处理单元素成员函数在功能上是相同的，但是在效率和代码清晰度上要更胜一筹。如将一个元素插入到vector中，而它的内存满了，那么vector将申请更大容量的内容，把它的元素从旧内存拷贝到新内存，销毁旧内存中的元素，并释放旧内存，再把要插入的元素插入进来，因此插入n个新元素最多可导致次新内存的分配。
2. 几乎所有通过插入迭代器(即利用`inserter`、`back_inserter`或`front_inserter`)来操作目标区间的copy调用，都可以应该被替换为对区间成员函数的调用。
3. 对于**区间创建**，所有的标准容器都提供了如下形式的构造函数：`container::container(InputIterator begin, InputIterator end)`;
4. 对于**区间插入**，所有的标准序列容器都提供了如下形式的insert：`void container::insert(iterator position, InputIterator begin, InputIterator end)`;表示在position位置插入begin到end的元素。关联容器利用比较函数决定元素的插入位置，所以不需要提供插入位置：`void container::insert(InputIterator begin, InputIterator end)`;
5. 对于**区间删除**，所有的标准容器都提供了区间形式的erase操作，但是对于序列容器和关联容器，其返回值不同。序列容器提供了如下的形式：`iterator container::erase(iterator begin, iterator end)`;而关联容器则提供了如下形式：`void container::erase(iterator begin, iterator end)`;为什么会有这种区别呢？据说是关联容器的erase如果返回迭代器（指向被删除元素之后的元素）会导致不可接受的性能负担。
6. 对于**区间赋值**，所有的标准容器都提供了区间形式的`assign：void container::assign(InputIterator begin, InputIterator end)`;

## R06 当心C++编译器最烦人的分析机制

分析这样一段程序：有一个存有整数的文件，你想把这些整数拷贝到一个list中
```c++
ifstream dataFile("ints.dat");
list<int> data(istream_iterator<int>(dataFile), istream_iterator<int>());//小心！结果不会是你期望的那样
```

这样做的思路是把一对istream_iterator传入到list的区间构造函数，从而把文件中的整数拷贝到list中，这段代码可以通过编译，但是在运行时什么也不会做，它也不会创建list，这是因为第二条语句并没有声明创建一个list。

从最基本的说起，`int f(double d);`声明了一个接收double参数，返回int的函数，下面两种形式是做了同样的事：`int f(double (d))`;还有`int  f(double)`;再看`int g(double (*pf)())`;声明了一个函数g，参数是一个函数指针，这个函数指针不接受参数，返回值是double，下面两种形式同样是做了这样的事，int g(double pf());还有int g(double ())，第三种形式省略了函数名，double后有一个空格和()，表示这是一个函数指针。

再来看最开始的例子，第二行其实不是声明了一个变量，而是声明了一个函数，其返回值是list<int>，第一个参数名称是dataFile，类型是istream_iterator<int>，dataFile两边的括号是多余的，会被忽略。第二个参数没有名称，类型是指向不带参数的函数的指针，该函数返回一个istream_iterator<int>。

这符合C++中一个普遍规律，尽可能地解释为函数声明，比如这样一段代码，`class Widget{ ... };` `Widget w();`它并没有声明一个Widget类型的对象w，而是声明了一个名为w的函数。学会识别这类言不达意是成为C++程序员的必经之路。

而为了实现最开始那段代码想要实现的功能，可以为第一个参数参数加上一对括号，即`list<int> data((istream_iterator<int>(dataFile)), istream_iterator<int>())`;不幸地是，并不是所有编译器都知道这一点，几乎有一半都会拒绝上述正确的声明形式，而接收最开始错误的声明形式。更好的方式是在data声明中避免使用匿名的istream_iterator对象，将那句代码分成三句来写。


## R07 如果容器中包含了通过new操作创建的指针，切记在容器对象析构前将指针delete掉

STL容器很智能，但是还没有智能到知道是否该释放自己所包含的指针的程度。当你使用指针的容器，而其中的指针应该被删除时，为了避免资源泄露，你应该使用引用计数形式的智能指针（如shared_ptr）代替指针（普通指针不具有异常安全性），或者当容器被析构时手动删除其中的每个指针。

分析下面的代码：

```c++
void doSomething(){
    vector<Widget*> vwp;
    for(int i=0; i<n; ++i){
        vwp.push_back(new Widget);
    }
}
```
当vwp的作用域结束，其中的指针并没有释放，造成了资源泄露。如果希望它们被删除，你可能会写出如下的代码：
```c++
void doSomething(){
    vector<Widget*> vwp;
    for(int i=0; i<n; ++i){
        vwp.push_back(new Widget);
    }
    //do something
    for(auto i=vwp.begin(); i!=vwp.end(); ++i){
        delete *i;
    }
}
```

这样确实可以，只要你不太挑剔的话。一个问题是这段代码做的事情和for_each相同，但是不如for_each更清晰，另一个问题是这段代码不是异常安全的。如果在vwp填充指针和从其中删除指针的过程中有异常抛出的话，同样会有资源泄露。下面就要克服这两个问题。

首先使用for_each替换上面的for循环，为了做到这一点，需要把delete变成一个函数对象。

```c++
template <typename T>
struct DeleteObject : public unary_function<const T*, void>{//条款40会解释为什么有这个继承
    void operator()(const T* ptr) const {//这是函数对象，接收一个指针作为参数
        delete ptr;
    }
}

void doSomething(){
    vector<Widget*> vwp;
    for(int i=0; i<n; ++i){
        vwp.push_back(new Widget);
    }
    //do something
    for_each(vwp.begin(), vwp.end(), DeleteObject<Widget>());
}
```

不幸的是，这种形式的函数对象需要指定要删除的对象类型（这里是Widget），vector中保存的Widget*，DeleteObject当然是要删除Widget*类型的指针。这种形式不仅是多余，同样可能会导致一些难以追踪的错误，比如说有代码很不明智的从string中继承。这样做非常危险，因为同其他标准STL容器一样，string是没有虚析构函数的，从没有虚析构函数的类进行共有继承是一项重要禁忌（Effective C++有详细描述）。先抛开禁忌不谈，假如有人就是写出了这样的代码，在调用for_each(vwp.begin(), vwp.end(), DeleteObject<string>());时，因为通过基类指针删除派生类对象，而基类又没有虚析构函数的话，会产生不确定的行为。所以应该让编译器自己推断出应该删除的指针类型，如下是改进后的代码。

```c++
struct DeleteObject {
    template <typename T>
    void operator()(const T* ptr) const {//这是函数对象，接收一个指针作为参数
        delete ptr;
    }
}

void doSomething(){
    vector<Widget*> vwp;
    for(int i=0; i<n; ++i){
        vwp.push_back(new Widget);
    }
    //do something
    for_each(vwp.begin(), vwp.end(), DeleteObject());
}
```

但是上述代码依然不是类型安全的，如果在创建Widget对象和执行for_each销毁对象之间有异常被抛出，就会有资源泄露，可以使用带引用计数的智能指针来解决这个问题，如下是最终优化的版本。

```c++
void doSomething(){
    typedef std::shared_ptr<Widget> SPW;//SPW表示指向Widget的shared_ptr
    vector<SPW> vwp;
    for(int i=0; i<n; ++i){
        vwp.push_back(SPW(new Widget));
    }
}
```

## R08 切勿创建包含auto_ptr的容器对象

包含auto_ptr的容器是被禁止的，这样的代码不应该被编译通过（可惜目前有些编译器做不到这一点）。首先是因为这样的容器是不可移植的，其次，拷贝一个auto_ptr意味着它指向的对象的所有权被移交到拷入的auto_ptr上，而它自身被置为NULL（相当于是移动），这种拷贝包括调用拷贝构造函数和赋值构造函数的时候。而STL容器中的拷贝遍地都是，举一个例子，调用sort函数给包含auto_ptr<Widget>的vector排序时（定义好了排序函数谓词），在排序过程中，Widget的一个或几个auto_ptr可能会被置为NULL。这是因为sort的常见实现算法是，把容器中的某个元素当作“基准元素”，然后对大于或小于等于该元素的其他元素递归调用排序操作，会把待交换的元素首先赋值给一个临时变量，则自身被置为NULL了，该临时对象超过作用域也会被销毁，从而导致结果vector中的元素被置为NULL。

```c++
//	对vector所做的排序操作可能会改变它的内容！
vector<auto_ptr<Widget> > widgets;sort(widgets.begin(), widgets.end(), widgetAPcompare());
```

## R09 慎重选择删除元素的方法

**删除容器中有特定值的所有对象**
对标准容器 `Container<int> c`; 删除其中所有值为 1963 的元素的方法。

`erase-remove` 习惯用法（连续内存容器 vector，deque，string）：

```c++
c.erase(remove(c.begin(), c.end(), 1963), c.end());
```

list: `c.remove(1963);`

关联容器：`c.erase(1963)`; 对数时间开销，基于等价而不是相等。注意关联容器没有名为 remove 的成员函数，使用任何名为 remove 的操作都是完全错误的。

**删除容器中满足特定判别式（条件）的所有对象**

删除使下面的判别式返回 true 的每一个对象
```c++
bool badValue(int );
// 序列容器(vector,string,deque,list)
c.erase(remove_if(c.begin(), c.end(), badValue), c.end());
// list
c.remove_if(badValue);
```
高效方法：写一个循环遍历容器中的元素，并在遍历过程中删除元素。注意，对于关联容器（map，set，multimap，multiset），删除当前的 iterator，只会使当前的 iterator 失效。

原因：关联容器的底层使用红黑树实现，插入、删除一个结点不会对其他结点造成影响。erase 只会使被删除元素的迭代器失效。关联容器的 erase 返回值为 void，可以使用 erase(iter++) 的方式删除迭代器。

```c++
AssocContainer<int> c;
ofstream logFile;
...
for (AssocContainer<int>::iterator i = c.begin(); i != c.end(); /*什么也不做*/) {
  if (badValue(*i)) {
    logFile << "Erasing " << *i << '\n';	// 写日志文件
    c.erase(i++);		// 使用后缀递增删除元素，避免迭代器无效。
  } else {
     ++i;
  }
}
```
对于序列式容器（vector，string，deque），**删除当前的 iterator 会使后面所有元素的 iterator 都失效**。

原因： vector、string、deque 使用了**连续分配的内存**，删除一个元素会导致后面的所有元素都向前移动一个位置。所以不能使用 erase(iter++) 的方式，**但可以使用 erase 方法，序列容器的 erase 可以返回下一个有效的 iterator**。

## R10 了解分配器（allocator）的约定和限制

1. 像new操作符和new[]操作符一样，STL内存分配子负责分配和释放原始内存，但是多数标准容器从不向与之关联的分配子申请内存。
2.allocator是一个模板，模板参数T表示为它分配内存的对象的类型，提供类型定义pointer和reference，但是始终让pointer为T*，reference为T&。
3. 千万不能让自定义的allocator拥有随对象而不同的状态，即不应该有非静态成员变量。
4.传给allocator的allocate成员函数的参数是**对象的个数**，而**不是所需字节的大小**，这一点和new相同，和operator new，malloc相反。同时allocate的返回值是T*指针，即使尚未有T对象被构造出来，而operator new，malloc返回都是void*，void*是用来指向未初始化内存的传统方式。
5. 自定义的allocator一定要提供嵌套的rebind模板，因为标准容器依赖该模板

## R11 理解自定义分配器的正确用法

详细参考:https://jianye0428.github.io/posts/clause_11/

## R12 切勿对STL容器的线程安全性有不切实际的依赖

详细信息请参考: https://jianye0428.github.io/posts/clause_12/
STL 只支持以下多线程标准：
 - 多个线程读是安全的。
 - 多个线程对不同的容器做写入操作是安全的。

考虑一段单线程可以成功执行的代码。

```c++
//将vector中的5都替换成0
vector<int> v;
vector<int>::iterator first5(find(v.begin(), v.end(), 5));
if(first5 != v.end()){
    *first5 = 0;
}
```

但是在多线程环境中，执行第二行语句后返回的first5的值可能会被改变，导致第三行的判断不准确，甚至一些插入/删除操作会让first5无效。**所以必须在操作vector之前，再其上下位置加锁。**

```c++
vector<int> v;
getMutex(v);
vector<int>::iterator first5(find(v.begin(), v.end(), 5));
if(first5 != v.end()){
    *first5 = 0;
}
releaseMutex(v);
```

更为完善的方法是实现一个Lock类，在构造函数中加锁，在析构函数中释放锁，即RAII。

ref:
[1]. https://www.cnblogs.com/Sherry4869/p/15128250.html</br>
[2]. https://blog.csdn.net/zhuikefeng/article/details/108164117#t42


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partone/  

