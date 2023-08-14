# Effective STL [22] | 避免原地修改set和multiset的键


**所有标准关联容器，set和multiset保持它们的元素有序，这些容器的正确行为依赖于它们保持有序。**

如果你改了关联容器里的一个元素的值（例如，把10变为1000），新值可能不在正确的位置，而且那将破坏容器的有序性。

## 修改map和multimap值

试图改变这些容器里的一个键值的程序将不能编译:

```c++
map<int, string> m;
...
m.begin()->first = 10; // 错误！map键不能改变
multimap<int, string> mm;
...
mm.begin()->first = 20; // 错误！multimap键也不能改变
```

{{<admonition tip "Note">}}
因为`map<K, V>`或`multimap<K, V>`类型的对象中元素的类型是`pair<const K, V>`。因为键的类型const K，它不能改变。
{{</admonition>}}

```c++
template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
    typename _Alloc = std::allocator<std::pair<const _Key, _Tp> > >
class map {
public:
  typedef _Key     key_type;
  typedef _Tp     mapped_type;
  typedef std::pair<const _Key, _Tp>  value_type;
  typedef _Compare     key_compare;
  typedef _Alloc     allocator_type;
  ...
}
```
如果你使用一个`const_cast`，或许能改变它，后面会讨论到。

## 修改set和multiset值

对于`set<T>`或`multiset<T>`类型的对象来说，储存在容器里的元素类型只不过是`T`，并非`const T`。因此，`set`或`multiset`里的元素可能在你想要的任何时候改变。不需要映射。

```c++
template<typename _Key, typename _Compare = std::less<_Key>,
  typename _Alloc = std::allocator<_Key> >
class set {
public:
  // typedefs:
  ///@{
  /// Public typedefs.
  typedef _Key     key_type;
  typedef _Key     value_type;
  typedef _Compare key_compare;
  typedef _Compare value_compare;
  typedef _Alloc   allocator_type;
  ///@}
  ...
}
```

**为什么set或multiset里的元素不是常数？**

假设我们有一个雇员的类:

```c++
class Employee {
public:
  ...
  const string& name() const;       // 获取雇员名
  void setName(const string& name); // 设置雇员名
  const string& getTitle() const;   // 获取雇员头衔
  void setTitle(string& title);     // 设置雇员头衔
  int idNumber() const;             // 获取雇员ID号
  ...
}
```

让我们做合理的假设，每个雇员有唯一的ID号，就是`idNumber`函数返回的数字。然后，建立一个雇员的`set`，很显然应该只以`ID`号来排序`set`:

```c++
struct IDNumberLess {
  public binary_function<Employee, Employee, bool> { // 参见条款40
    bool operator()(const Employees lhs,
    const Employee& rhs) const {
        return lhs.idNumber() < rhs.idNumber();
    }
};
typedef set<Employee, IDNumberLess> EmpIDSet;
EmpIDSet se; // se是雇员的set， 按照ID号排序
```

实际上，雇员的`ID`号是`set`中元素的键。**其余的雇员数据只是虚有其表**。在这里，没有理由不能把一个特定雇员的头衔改成某个有趣的东西:

```c++
Employee selectedID; // 容纳被选择的雇员
... // ID号的变量
EmpIDSet::iterator i = se.find(selectedID);
if (i != se.end()){
    i->setTitle("Corporate Deity"); // 给雇员新头衔
}
```

因为在这里我们只是改变雇员的一个与set排序的方式无关的方面（一个雇员的非键部分），所以这段代码不会破坏set。那是它合法的原因。但它的合法排除了`set/multiset`的元素是`const`的可能。而且那是它们为什么不是的原因。

因为`set`或`multiset`里的值不是`const`，所以试图改变它们可以编译。

<font color=red>如果你改变`set`或`multiset`里的元素， 你必须**确保不改变一个键部分——影响容器有序性的元素部分**。如果你做了，你会破坏容器，再使用那个容器将产生未定义的结果， 而且那是你的错误。另一方面，这个限制只应用于被包含对象的键部分。对被包含元素的所有其他部分来说，是开放的: 随便改变！</font>

## 阻止修改set和multiset值

即使`set`和`multiset`的元素不是`const`，实现仍然有很多方式可以阻止它们被修改。

例如，实现可以让用于`set<T>::iterator的operator*`返回一个常数`T&`。即，它可以让`set`的迭代器解引用的结果是`set`元素的常量引用。

在这样的实现下，将没有办法修改`set`或`multiset`的元素，因为所有访问那些元素的方法都将在让你访问之前加一个`const`。

## 要不要修改set和multiset值

1. 如果**不关心移植性**，你想要改变set或multiset中元素的值，而且你的STL实现让你侥幸成功，继续做。只是要确定不要改变元素的键部分，即，会影响容器有序性的元素部分。
2. 如果**在乎移植性**，就认为set和multiset中的元素不能被修改，至少不能在没有映射的情况下

**Solution: 映射到一个引用**

有时候完全有理由改变`set`或`multiset`元素的非键部分。例如刚看的不能在一些实现下编译的`setTitle`调用:

```c++
EmpIDSet::iterator i = se.find(selectedID);
if (i != se.end()) {
    i->setTitle("Corporate Deity"); // 有些STL实现会拒绝这样，因为*i是const
}
```

为了让它可以编译并且行为正确，我们必须映射掉*i的常量性。这是那么做的正确方法:

```c++
if (i != se.end()) {
 const_cast<Employee&>(*i).setTitle("Corporate Deity"); // 映射掉*i的常量性
}
```

**这可以得到i指向的对象，告诉你的编译器把映射的结果当作一个（非常数）Employee的引用，然后在那个引用上调用setTitle。**

**错误版本的映射**

```c++
if (i != se.end()){
  static_cast<Employee>(*i).setTitle("Corporate Deity"); // 把*i映射到一个Employee
}
```

它也等价于如下内容:

```c++
if (i != se.end()) {
  ((Employee)(*i)).setTitle("Corporate Deity"); // 使用C映射语法
}
```

这两个都能编译，而且因为它们等价，所以它们错的原因也相同。

**在运行期，它们不能修改`*i`！**

**在这两个情况里，映射的结果是一个`*i`副本的临时匿名对象，而setTitle是在匿名的物体上调用，不在`*i`上`！*i`没被修改，因为setTitle从未在那个对象上调用，它在那个对象的副本上调用。**

两个句法形式等价于这个:

```c++
if (i != se.end()){
  Employee tempCopy(*i); // 把*i拷贝到tempCopy
  tempCopy.setTitle("Corporate Deity"); // 修改tempCopy
}
```

通过映射到引用，我们避免了建立一个新对象。

取而代之的是，映射的结果是一个现有对象的引用，`i`指向的对象。

当我们在有这个引用指定的对象上调用setTitle时，我们是在`*i`上调用setTitle，而且那正是我们想要的。

**为什么不能去掉map和multimap的常量性**

注意`map<K, V>`或`multimap<K, V>`包含`pair<const K, V>`类型的元素。

那个`const`表明`pair`的第一个组件被定义为常量，而那意味着试图修改它是未定义的行为（即使映射掉它的常量性）。

理论上，一个STL实现可能把这样的值写到一个只读的内存位置（比如，一旦写了就通过系统调用进行写保护的虚拟内存页），而且试图映射掉它的常量性，最多，没有效果。

如果你是一个坚持遵循标准拟定的规则的人，你绝不会试图映射掉`map`或`multimap`键的常量性。

## 安全修改元素步骤

想要总是可以工作而且总是安全地改变set、multiset、map或multimap里的元素，按5个简单的步骤去做：

  1. 定位你想要改变的容器元素。如果你不确定最好的方法， 条款45提供了关于怎样进行适当搜寻的指导。
  2. 拷贝一份要被修改的元素。对map或multimap而言，确定不要把副本的第一个元素声明为const。毕竟，你想要改变它！
  3. 修改副本，使它有你想要在容器里的值。
  4. 从容器里删除元素，通常通过调用erase（参见条款9）。
  5. 把新值插入容器。如果新元素在容器的排序顺序中的位置正好相同或相邻于删除的元素，使用insert的“提示”形式把插入的效率从对数时间改进到分摊的常数时间。使用你从第一步获得的迭代器作为提示。


**一句话概括就是：先删除，再插入新的。**

**Example:**

这是同一个累人的雇员例子，这次以安全、可移植的方式写:

```c++
EmpIDSet se; // 同前，se是一个以ID号排序的雇员set
Employee selectedID; // 同前，selectedID是一个带有需要ID号的雇员
...
EmpIDSet::iterator i = se.find(selectedID); // 第一步：找到要改变的元素
if (i!=se.end()) {
  Employee e(*i); // 第二步：拷贝这个元素
  se.erase(i++);  // 第三步：删除这个元素；
                  // 自增这个迭代器以
                  // 保持它有效（参见条款9）
  e.setTitle("Corporate Deity"); // 第四步：修改这个副本
  se.insert(i, e);               // 第五步：插入新值；提示它的位置和原先元素的一样
}
```



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_22/  

