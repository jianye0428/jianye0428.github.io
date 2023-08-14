# Effective STL [23] | 考虑用有序vector代替关联容器


当需要一个提供快速查找的数据结构时，很多STL程序员立刻会想到标准关联容器：`set`、`multiset`、`map`和`multimap`。

如果使用了合适的散列函数，则可以认为散列容器提供了常数时间的查找。

对于多数应用，被认为是常数时间查找的散列容器要好于保证了对数时间查找的`set`、`map`和它们的`multi`同事。

即使你需要的就只是==对数时间==查找的保证，标准关联容器仍然可能不是你的最佳选择。和直觉相反，对于标准关联容器，所提供的性能也经常劣于本该比较次的`vector`。

## 关联容器数据结构

标准关联容器的典型实现是**平衡二叉查找树**。

一个平衡二叉查找树是一个对**插入**、**删除**和**查找**的混合操作优化的数据结构。

换句话说，它被设计为应用于进行一些插入，然后一些查找，然后可能再进行一些插入，然后也许一些删除，然后再来一些查找，然后更多的插入或删除，然后更多的查找等。这个事件序列的关键特征是插入、删除和查找都是混合在一起的。

一般来说，没有办法预测对树的下一个操作是什么。

**使用数据结构的3阶段**

1. **建立**。通过插入很多元素建立一个新的数据结构。在这个阶段，几乎所有的操作都是插入和删除。几乎没有或根本没有查找。
2. **查找**。在数据结构中查找指定的信息片。在这个阶段，几乎所有的操作都是查找。几乎没有或根本没有插入和删除。
3. **重组**。修改数据结构的内容，通过删除所有现有数据和在原地插入新数据。从动作上说，这个阶段等价于阶段1。一旦这个阶段完成，应用程序返回阶段2。

## 有序vector更高效

一个vector可能比一个关联容器能提供更高的性能（时间和空间上都是）。

但不是任意的`vector`都会，只有有序vector。因为只有有序容器才能正确地使用查找算法——`binary_search`、`lower_bound`、`equal_range`等。

Q: 为什么一个（有序的）vector的二分法查找比一个二叉树的二分法查找提供了更好的性能？

A: 其中的一个是**大小问题**，其中的一个是**引用局部性问题**。


**大小问题**

假设我们需要一个容器来容纳Widget对象，而且，因为查找速度对我们很重要，我们考虑一个Widget的关联容器和一个有序`vector<Widget>`。

**关联容器数据结构**

如果选择一个关联容器，我们几乎确定了要使用平衡二叉树。这样的树是由树节点组成，每个都不仅容纳了一个Widget，而且保存了一个该节点到左孩子的指针，一个到它右孩子的指针，和（典型的）一个到它父节点的指针。

这意味着**在关联容器中用于存储一个Widget的空间开销至少会是三个指针**。

**vector数据结构**

与之相对的是，当在vector中存储Widget并没有开销：简单地存储一个Widget。

当然，**vector本身有开销**，在vector结尾也可能有空的（保留）空间，但是每个vector开销是可以忽略的（通常是三个机器字，比如，三个指针或两个指针和一个int），而且如果必要的话，末尾空的空间可以通过“交换技巧”去掉。即使这个附加的空间没有去掉，也并不影响下面的分析，因为当查找时不会引用那段内存。

**内存大小**

假设我们的数据结构足够大，它们可以分成多个内存页面，但是vector比关联容器需要的页面要少。

因为vector不需要每个Widget的开销，而关联容器给每个Widget上附加了三个指针。

假设在你使用的系统上一个Widget的大小是12个字节，指针是4个字节，一个内存页面是4096（4K）字节。

忽略每个容器的开销，当用vector保存时，你可以在一页面上放置341个Widget $(4096\div12\approx341)$，但使用关联容器时你最多只能放170个 $4096\div(12+4\times3)\approx170)$。

**因此关联容器和vector比起来，你将会使用大约两倍的内存。**

如果你使用的环境可以用**虚拟内存**，就很可以容易地看出那会**造成大量的页面错误，因此一个系统会因为大数据量而明显慢下来**。

**引用局部性问题**

假设在二叉树中的节点都群集在一个相关的小内存页面集中，实际情况下关联容器很乐观的。

**大部分STL实现使用自定义内存管理器来达到这样的群集，但是如果你的STL实现没有改进树节点中的引用局部性，这些节点会分散在所有你的内存空间。那会导致更多的页面错误。**

即使使用了自定义群集内存管理器，关联容器也会导致很多页面错误，因为，不像连续内存容器，比如vector，**基于节点的容器更难保证在容器的遍历顺序中一个挨着一个的元素在物理内存上也是一个挨着一个。**

**当进行二分查找时那种内存组织方式（译注：遍历顺序中一个挨着一个的元素在物理内存上也是一个挨着一个）正好是页面错误最少的。**

## vector的缺点

<font color=red>有序vector的大缺点是必须保持有序！</font>

因为vector中所有的元素都必须拷贝，所以：

1. 当一个新元素插入时，大于这个新元素的所有东西都必须向上移一位，非常昂贵；

2. 如果vector必须重新分配它的内在内存，则会更昂贵；

3. 如果一个元素从vector中被删除，所有大于它的元素都要向下移动。

vector的插入和删除都很昂贵，但是关联容器的插入和删除则很轻量。

这就是为什么只有当你知道你的数据结构使用的时候查找几乎不和插入和删除混合时，使用有序vector代替关联容器才有意义。

**概要**

1. 在有序vector中存储数据很有可能比在标准关联容器中保存相同的数据消耗更少的内存；

2. 当页面错误值得重视的时候，在有序vector中通过二分法查找可能比在一个标准关联容器中查找更快。

## Example

> vector代替set

```c++
 // 代替set<Widget>
vector<Widget> vw;

// 建立阶段：很多插入，几乎没有查找
...

// 结束建立阶段。（当模拟一个multiset时，你可能更喜欢用stable_sort 来代替；
sort(vw.begin(), vw.end());

// 用于查找的值的对象
Widget w;

...// 开始查找阶段

// 通过binary_search查找
if (binary_search(vw.begin(), vw.end(), w))...

// 通过lower_bound查找
vector<Widget>::iterator i = lower_bound(vw.begin(), vw.end(), w);

// 条款19解释了“!(w < *i)”测试
if (i != vw.end() && !(w < *i))...

// 通过equal_range查找
pair<vector<Widget>::iterator, vector<Widget>::iterator> range =
                       equal_range(vw.begin(), vw.end(), w);

if (range.first != range.second)...

... // 结束查找阶段，开始重组阶段

// 开始新的查找阶段...
sort(vw.begin(), vw.end());
```
里面最难的东西就是**怎么在搜索算法中做出选择**（比如，`binary_search`、`lower_bound`等）

> vector代替map或multimap

当你决定用vector代替map或multimap时，事情会变得更有趣，因为**vector必须容纳pair对象**。

但是要注意，如果你声明一个map<K, V>的对象（或者等价的multimap），保存在map中的元素类型是`pair<const K, V>`。

如果要用vector模拟`map`或者`multimap`，**必须去掉const**，因为当你对vector排序时，元素的值将会通过赋值移动，那意味着pair的两个组件都必须是可赋值的。

当使用vector来模拟map<K, V>时，**保存在vector中数据的类型将是`pair<K, V>`**，而不是`pair<const K, V>`。

map和multimap以顺序的方式保存他们的元素，但**用于排序目的时它们只作用于元素的key部分**（pair的第一个组件），所以当排序vector时你必须做一样的事情。

你需要为你的pair写一个自定义比较函数，因为`pair`的`operator<`作用于pair的两个组件。

**2个比较函数来进行查找**

用来排序的比较函数将作用于两个pair对象，但是查找只用到key值。

必须传给用于查找的比较函数一个key类型的对象（要查找的值）和一个pair（存储在vector中的一个pair）——两个不同的类型。

还有一个附加的麻烦，你不会知道key还是pair是作为第一个实参传递的，所以你真的需要两个用于查找的比较函数：**一个key值先传递，一个pair先传递**

```c++
typedef pair<string, int> Data; // 在这个例子里"map"容纳的类型

class DataCompare { // 用于比较的类
public:
    // 用于排序的比较函数keyLess在下面
  bool operator()(const Data& lhs, const Data& rhs) const {
      return keyLess(lhs.first, rhs.first);
  }

  // 用于查找的比较函数（形式1）
  bool operator()(const Data& Ihs, const Data::first_type& k) const  {
    return keyLess(lhs.first, k);
  }

  // 用于查找的比较函数（形式2）
  bool operator()(const Data::first_type& k,  const Data& rhs) const {
    return keyLessfk, rhs.first);
  }

private:
  // “真的”比较函数
  bool keyLess(const Data::first_type& k1, const Data::first_type& k2) const {
    return k1 < k2;
  }
};
```

我们假设有序vector将模拟`map<string, int>`。

这段代码几乎是上面讨论的字面转换，除了存在成员函数`keyLess`。

那个函数的存在是用来**保证几个不同的`operator()`函数之间的一致性**。

每个这样的函数只是简单地比较两个key的值，所以我们把这个测试放在keyLess中并让operator()函数返回keyLess所做的事情，这比复制那个逻辑要好。

这个软件工程中绝妙的动作增强了`DataCompare`的可维护性，但**有一个小缺点，它提供了有不同参数类型的operator()函数，这将导致函数对象无法适配。**

把有序vector用作map本质上和用作set一样。

**唯一大的区别是必须把DataCompare对象用作比较函数:**

```c++
// 代替map<string, int>
vector<Data> vd;

 // 建立阶段：很多插入，几乎没有查找
...

// 结束建立阶段。（当模拟multimap时，你可能更喜欢用stable_sort来代替）；
sort(vd.begin(), vd.end(), DataCompare());

// 用于查找的值的对象
string s;

... // 开始查找阶段

// 通过binary_search查找
if (binary_search(vd.begin(), vd.end(), s, DataCompare()))...

// 再次通过lower_bound查找，
vector<Data>::iterator i = lower_bound(vd.begin(), vd.end(), s, DataCompare());

// 条款45解释了“!DataCompare()(s, *i)”测试
if (i != vd.end() && !DataCompare()(s, *i))...

// 通过equal_range查找
pair<vector<Data>::iterator, vector<Data>::iterator> range =
    equal_range(vd.begin(), vd.end(), s, DataCompare());

if (range.first != range.second)...

... // 结束查找阶段，开始重组阶段

// 开始新的查找阶段...
sort(vd.begin(), vd.end(), DataCompare());
```

正如你所见，一旦你写了DataCompare，东西都很好地依序排列了。

而一旦位置合适了，它们往往比相应的使用真的map的设计运行得更快而且使用更少内存。

如果你的程序不是按照阶段的方式操作数据结构，那么使用有序vector代替标准关联容器几乎可以确定是在浪费时间。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_23/  

