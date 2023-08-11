# Effective STL [19] | 了解相等和等价的区别


## 比较对象

STL充满了比较对象是否有同样的值。比如，当你用`find`来定位区间中第一个有特定值的对象的位置，`find`必须可以比较两个对象，看看一个的值是否与另一个相等。同样，当你尝试向`set`中插入一个新元素时，`set::insert`必须可以判断那个元素的值是否已经在`set`中了。

`find`算法和`set`的`insert`成员函数是很多必须判断两个值是否相同的函数的代表。但它们实现方式不同，find对“相同”的定义是相等，基于`operator==`。`set::insert`对“相同”的定义是等价，通常基于`operator<`。因为有定义不同，所以有可能一个定义规定了两个对象有相同的值而另一个定义判定它们没有。结果，如果你想有效使用STL，那么你必须明白相等和等价的区别。

**相等**

操作上来说，相等的概念是基于`operator`的。如果表达式“`x == y`”返回true，`x`和`y`有相等的值，否则它们没有。

**<font color=red>x和y有相等的值并不意味着所有它们的成员有相等的值</font>**。比如，我们可能有一个内部记录了最后一次访问的Widget类。

```c++
class Widget {
public:
...
private:
 TimeStamp lastAccessed;
...
};
```

我们可以有一个用于Widget的忽略这个域的operator:

```c++
bool operator(const Widget& lhs, const Widget& rhs) {
// 忽略lastAccessed域的代码
}
```

在这里，两个`Widget`即使它们的`lastAccessed`域不同也可以有相等的值。

一般而言，相等意味着两个变量的值相同，但是如果比较2个对象，因为比较函数可以自定义，因此有时候对象的某些成员变量值不同也会设定2个对象相等。

**等价**

==等价是基于在一个有序区间中对象值的相对位置==。

等价一般在每种标准关联容器（比如，set、multiset、map和multimap）的一部分——排序顺序方面有意义。注意这里的应用场景是==排序==。

两个对象x和y如果在关联容器c的排序顺序中没有哪个排在另一个之前，那么它们关于c使用的排序顺序有等价的值。

举一个例子，一个set<Widget> s。两个Widget w1和w2，如果在s的排序顺序中没有哪个在另一个之前，那么关于s它们有等价的值。set<Widget>的默认比较函数是less<Widget>，而默认的less<Widget>简单地对Widget调用operator<，所以w1和w2关于operator<有等价的值如果下面表达式为真：

```c++
!(w1 < w2) // w1 < w2时它非真
&& // 而且
!(w2<w1) // w2 < w1时它非真
```

这个有意义：<font color=red>两个值如果没有哪个在另一个之前（关于某个排序标准），那么它们等价（按照那个标准）。</font>在一般情况下，用于关联容器的比较函数不是`operator<`或甚至`less`，它是用户定义的判断式。每个标准关联容器通过它的`key_comp`成员函数来访问排序判断式，所以如果下式求值为真，两个对象x和y关于一个关联容器c的排序标准有等价的值：

```c++
!c.key_comp()(x, y) && !c.key_comp()(y, x) // 在c的排序顺序中
// 如果x在y之前它非真，同时在c的排序顺序中,如果y在x之前它非真
```

要完全领会==相等==和==等价==的含义，考虑一个忽略大小写的`set<string>`，也就是set的比较函数忽略字符串中字符大小写的`set<string>`。这样的比较函数会认为“STL”和“stL”是等价的。条款35演示了怎么实现一个函数，`ciStringCompare`，它进行了忽略大小写比较，但set要一个比较函数的类型，不是真的函数。要填平这个鸿沟，我们写一个operator()调用了ciStringCompare的仿函数类:

```c++

struct CIStringCompare: // 用于忽略大小写
public // 字符串比较的类；
binary_function<string, string, bool> {
bool operator()(const string& lhs,
const string& rhs) const
{
return ciStringCompare(lhs, rhs); // 关于ciStringCompare
}
}
```

具体实现为:

```c++
int ciCharCompare(char c1, char c2) // 忽略大小写比较字符
{   // c1和c2，如果c1 < c2返回-1，
    // 如果c1==c2返回0，如果c1 > c2返回1
    int Ic1 = tolower(static_cast<unsigned char>(c1));// 转成小写
    int Ic2 = tolower(static_cast<unsigned char>(c2));
    if (Ic1 < Ic2) return -1;
    if (lc1 > Ic2) return 1;
    return 0;
}
```

给定`CIStringCompare`，要建立一个忽略大小写的set<string>就很简单了:

```c++
set<string, CIStringCompare> ciss; // ciss = “case-insensitive
// string set”
```

如果我们向这个set中插入“`Persephone`”和“`persephone`”，只有第一个字符串加入了，因为第二个等价于第一个：

```c++
ciss.insert("Persephone"); // 一个新元素添加到set中
ciss.insert("persephone"); // 没有新元素添加到set中
```

如果我们现在使用set的find成员函数搜索字符串“persephone”，搜索会成功，

```c++
if (ciss.find("persephone") != ciss.end())... // 这个测试会成功
```

但如果我们用非成员的find算法，搜索会失败：

```c++
if (find(ciss.begin(), ciss.end(),
"persephone") != ciss.end())... // 这个测试会失败
```

那是因为“`persephone`”等价于“`Persephone`”（关于比较仿函数CIStringCompare），但不等于它（因为string("persephone") != string("Persephone")）。你可能会奇怪为什么标准关联容器是基于等价而不是相等。毕竟，大多数程序员对相等有感觉而缺乏等价的感觉。

<font color=red>标准关联容器保持有序，所以每个容器必须有一个定义了怎么保持东西有序的比较函数（默认是less）。等价是根据这个比较函数定义的，所以标准关联容器的用户只需要为他们要使用的任意容器指定一个比较函数（决定排序顺序的那个）。

如果关联容器使用相等来决定两个对象是否有相同的值，那么每个关联容器就需要，除了用于排序的比较函数，还需要一个用于判断两个值是否相等的比较函数。（默认的，这个比较函数大概应该是equal_to，但有趣的是equal_to从没有在STL中用做默认比较函数。当在STL中需要相等时，习惯是简单地直接调用operator==。比如，这是非成员find算法所作的。）</font>

**<font color=blue>说白了，就是等价是为容器排序服务的，如果容器想要插入多个转化为小写后相等的单词，容器会判定这些单词等价，虽然它们本身不相等，这样就能够很好地保持容器内单词的唯一性和单词的有序性。否则通过查找或插入该单词的时候，容器内部就会发生歧义，造成不符合使用者逻辑。</font>**

让我们假设我们有一个类似set的STL容器叫做`set2CF`，“set with two comparison functions”。**<font color=red>第一个比较函数用来决定set的排序顺序，第二个用来决定是否两个对象有相同的值。</font>** 现在考虑这set2CF：

```c++
set2CF<string, CIStringCompare, equal_to<string> > s;
```

在这里，s内部排序它的字符串时不考虑大小写，等价标准直觉上是这样：如果两个字符串中一个等于另一个，那么它们有相同的值。让我们向s中插入哈迪斯强娶的新娘（Persephone）的两个拼写:

```c++
s.insert("Persephone");
s.insert("persephone");
```

> 如果我们说`"Persephone" != "persephone"`然后两个都插入s，它们应该是什么顺序？

记住排序函数不能分别告诉它们。我们可以以任意顺序插入，因此放弃以确定的顺序遍历set内容的能力吗？（不能以确定的顺序遍历关联容器元素已经折磨着`multiset`和`multimap`了，因为标准没有规定等价的值（对于multiset）或键（对于multimap）的相对顺序。）或者我们坚持s的内容的一个确定顺序并忽略第二次插入的尝试（“persephone”的那个）? 如果我们那么做，这里会发生什么？

```c++
if (s.find("persephone") != s.end())... // 这个测试成功或失败？
```

大概**<font color=red>find使用了等价检查</font>**，但如果我们为了维护s中元素的一个确定顺序而忽略了第二个insert的调用，这个find会失败，即使“persephone”的插入由于它是一个重复的值的原则而被忽略！

## 总结

通过只使用一个比较函数并使用等价作为两个值“相等”的意义的仲裁者，标准关联容器避开了很多会由允许两个比较函数而引发的困难。

一开始行为可能看起来有些奇怪（特别是当你发现成员和非成员find可能返回不同结果），但最后，它避免了会由在标准关联容器中混用相等和等价造成的混乱。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_19/  

