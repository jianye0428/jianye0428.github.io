# Effective STL 精读总结 [3] | 关联容器


# 前言

> Effective-STL总结系列分为七部分，本文为第一部分，涉及原书第一章，内容范围Rule01~12。为方便书写，Rule12简写为R12。

{{< admonition Note "Effective-STL系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective STL(第3版) 精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 [Effective STL(第3版) 精读总结(二)](https://jianye0428.github.io/posts/parttwo/)</br>
💡 [Effective STL(第3版) 精读总结(三)](https://jianye0428.github.io/posts/partthree/)</br>
💡 [Effective STL(第3版) 精读总结(四)](https://jianye0428.github.io/posts/partfour/)</br>
{{< /admonition >}}

## R19: 理解相等（equality）和等价（equivalence）的区别

1. find算法和set的insert成员函数都需要比较两个值是否相同，find返回指定元素位置的迭代器，set::insert需要在插入前确定元素是否已经存在于set中了。但是这两个函数是不同的方法判断两个值是否相同。find对相同的定义是相等，基于operator==，set::insert对相同的定义是等价，基于operator<。但是相等也不一定意味着对象的所有成员都相等，因为可以重写operator==，制定我们自己的相等。等价是以在已排序区间中对象值的相对顺序为基础的，对于两个关联容器的对象x和y，如果它们都不在另一个的前面，那么称这两个对象具有等价的值，即!(x < y) && !(y < x)成立。但是一般情况下，关联容器的比较函数并不是operator<，甚至不是less，它是用户自定义的判别式。每个关联容器都通过key_comp成员函数使排序判别式可被外界使用，所以更一般的等价是 !c.key_comp() (x, y) && !c.key_comp() (y, x)成立，key_comp()返回一个比较函数。

2. 为了进一步理解相等和等价的区别，考虑这样一个不区分大小写的set<string>，它认为STL和stl是等价的，下面是实现：
```c++
struct CIStringCompare : public binary_function<string, string, bool>{//该基类信息参考条款40
    bool operator()(const string& lhs, const string& rhs) const{
        return ciStringCompare(lhs, rhs);//不区分大小写的函数对象，具体实现参考条款35
    }
}

set<string, CIStringCompare> ciss;
```
ciss就是一个不区分大小写的集合，如果在set中插入STL和stl，只有第一个字符串会被插入，因为第二个和第一个等价。如果使用set的find成员函数查找stl，是可以查找成功的，但是如果使用非成员的find算法就会查找失败，因为STL和stl并不相等。这个例子也印证了条款44中的，优先使用成员函数，而不是与之对应的非成员函数算法。

3.  那么为什么关联容器要使用等价，而不是相等呢？标准容器总是保持排列顺序的，所以每个容器必须有一个比较函数（默认是less），如果关联容器使用相等来决定两个对象是否相同的话，意味着要提供另一个比较函数来判断相等。同样是那个不区分大小写的例子，STL和stl因为不相等，所以都会被插入到set中，但是它们之间的顺序是什么呢？因为排序是用的less，所以之间的顺序是判断不了的。

## R20: 为包含指针的关联容器指定比较类型

1. 假定有一个包含string*指针的set，你将一些字符串指针放入其中，你可能期望set会按照字符串的字母顺序来排序，实则不然。如果想要按照期望的形式输出，就必须编写比较函数子类。
    ```c++
    struct stringPtrLess : public binary_function<const string*, const string*, bool> {
      bool operator()(const string* ps1, const string* ps2) const {
        return *ps1 < *ps2;
      }
    }

    typedef set<string*, stringPtrLess> stringPtrSet;
    stringPtrSet sps;

    void print(const string* ps){
      cout << *ps << endl;
    }
    for_each(sps.begin(), sps.end(), print);//对sps的每个对象调用print
    ```
    这里需要注意的是set模板的三个参数都是一个类型，所以给参数传递一个比较函数是不行的，无法通过编译。set不需要函数，它需要一个类型，在内部用它创建函数，所以下面的代码是不能通过编译的。
    ```c++
    bool stringPtrLess(const string* ps1, const string* ps2) const {
        return *ps1 < *ps2;
    }
    typedef set<string*, stringPtrLess> stringPtrSet;//不能通过编译
    stringPtrSet sps;
    ```
    每当创建包含指针的关联容器时，一般同时需要指定容器的比较类型，所以可以准备一个模板比较函数。
    ```c++
    struct dereferenceLess{
        template <typename PtrType>
        bool operator()(PtrType pt1, PtrType pt2) const{
            return *pt1 < *pt2;
        }
    }

    set<string*, dereferenceLess> sps;
    ```
    最后一件事，本条款是关于关联容器的，但它也同样适用于其他一些容器，这些容器包含指针，智能指针或迭代器，那么同样需要为这些容器指定一个比较类型。

## R21: 总是让比较函数在等值情况下返回false
1. 看一个例子，set<int, less_equal<int> > s;其中less_equal是指定的比较类型，相当于<=。当执行s.insert(10);，容器中有一个10的元素了，然后再执行一次s.insert(10);，容器会先判断内部有没有和10等价的元素，即调用判断 !(10 <= 10) && !(10 <= 10)， &&两边都是false，所以结果也是false，意思为容器中没有与当前待插入元素等价的元素！看出问题了吧？相等却不等价。当第二个10被插入到set中，意味着set不是一个set了，就破坏了这个容器。所以一定要保证对关联容器适用的比较函数总是对相等值返回false。

2. 再看一个例子，就是条款20中的stringPtrLess比较类型，实现的是string*按照字母升序排列，加入我们希望按照字幕降序排序，可以直接将它的判断置反吗？不可以！将判断直接置反得到的新判断是>=，而不是>。
  ```c++
  struct stringPtrLess : public binary_function<const string*, const string*, bool> {
      bool operator()(const string* ps1, const string* ps2) const {
          return !(*ps1 < *ps2);//这是错误演示
      }
  }
  ```

## R22: 切勿直接修改set或multiset中的键

1. 所有的关联容器都会按照一定顺序存储自己的元素，如果改变了关联容器的元素的键，那么新的键可能不在原来的位置上，这就会打破容器的有序性。对于map和multimap很简单，因为键的类型是const的，但是set和multiset中的元素却不是const的。首先考虑一下为什么set中的元素不能是const的，加入有一个雇员类，其中有id和salary两个成员，set是按照id的顺序进行排序的，所以更改salary不会影响雇员对象的位置，正因为可以更改雇员对象，这意味着set中存储的对象不能是const的。正因为更改set中的元素是如此简单，所以才要提醒你，如果你改变了set或multiset中的元素，一定不能改变键部分，如果你改变了，那么可能会破坏容器，再使用该容器将导致不确定的结果。
2. 尽管set和multiset的元素不是const，但是STL有办法防止其被修改。有种实现会使set<T>::iterator的operator*返回一个const T&，在这种情况下是无法修改set和multiset中的元素的。
3. 第一条提到可以更改雇员对象中非键的成员变量，但是有的编译器不允许这样的行为，所以修改set或multiset中元素的值是不可移植的代码。如果你不重视移植性，那么就可以更改对象中的非键成员，如果你重视移植性，那么就不能改变set和multiset中的对象。不对不允许改变非键的成员变量，可以先执行const_cast转换之后再改变。但是要注意转换成引用，即const_cast<Employee&>(*i)，如果不是引用的话，类型转换首先会产生一个临时对象，在临时对象上做更改salary的动作，而*i本身是并没有被更改的。
4. 对于修改map或multimap情况又有所不同，map<K, V>或multimap<K, V>包含的是pair<const K, V>类型的元素，如果把const属性去掉，就意味着可以改变键部分。理论上，一种STL实现可以将这样的值卸载一个只读的内存区域，一旦写入后，将由一个系统调用进行写保护，这是若试图修改它，最好的结果就是没有效果。但是如果要坚持C++标准的规则，那就永远不要试图修改map或multimap中的键部分。
5. 除了强制类型转换，还有一种安全的方式完成更改对象的工作。第一步找到要修改的对象的位置。第二步为将被修改的元素做一份拷贝。第三步修改该拷贝。第四步把容器中的元素删除，通常是使用erase。第五步是把新的值插入到容器中，通常是使用insert。

对于一个 map<K, V> 或 map<K, V> 类型的对象，其中的元素类型是 pair<const K, V>，因为键的类型是 const K，所以它不能修改。

set / multiset 中的值不是 const，所以对这些值进行修改的代码可以通过编译。

## R23: 考虑用排序的vector替代关联容器

1.当你需要一个快速查找功能的数据结构时，一般会立即想到标准关联容器。但是哈希容器的查找速度更快，通常提供常数时间的查找能力，而关联容器时对数时间的查找能力。如果你觉得对数时间的查找能力也可，那么可能排序的vector可能更符合你的要求。这是因为标准关联容器通常被实现为平衡二叉树，这种数据结构对混合的插入、删除和查找做了优化，即它适用于程序插入、删除和查找混在一起，没有明显的阶段的操作。但是很多应用程序使用数据结构的方式并没有这么乱，一般可以明显地分成三个阶段。设置阶段，这个阶段主要是插入和删除，几乎没有查找。查找阶段，这个阶段主要是查找，几乎没有插入和删除。重组阶段，这个阶段主要是插入和删除，几乎没有查找。对于这种方式，vector可能比关联容器提供了更好的性能，但是必须是排序的容器才可以，因为只有对排序的vector容器才能够正确底使用查找算法binary_search、lower_bound和equal_range等。
2. 下面探究为什么排序的vector在查找性能上会比关联容器要快呢？第一个原因是大小，平衡二叉树存储对象，除了对象本身以外，还通常包含了三个指针，一个指向左儿子，一个指向右儿子，通常还有一个指向父节点，而使用vector存储对象的话，除了对象本身以外，就没有多余的开销了。假设我们的数据足够大，它们被分割后将跨越多个内存页面，但是vector将比关联容器需要更少的页面。第二个原因是vector是连续内存容器，关联容器是基于节点的容器，虽然绝大多数STL实现使用了自定义的内存管理器使得二叉树的节点聚集在相对较少的内存页面，但是如果你的STL并没有这样做，那这些节点就会散布在全部地址空间中，这会导致更多的页面错误。与vector这样的内存连续容器不同，基于节点的容器想保证容器中相邻的元素在物理内存中也是相邻是十分困难的。
3. 但是需要注意的是，插入和删除操作对于vector来说是昂贵的，尤其是对于需要保持有序的vector。因为每当有元素被插入，新元素之后的元素都要向后移动一个位置，当有元素被删除，删除位置之后的元素都要向前移动一个位置。所以只有删除插入操作不和查找操作混在一起的才考虑使用排序的vector替代关联容器。
4. 当使用vector替换map或multimap时，存储在vector中的数据必须是pair<K, V>，而不是pair<const K, V>。因为当对vector进行排序时，他的元素的值将通过赋值操作被移动，这意味着pair的两个部分都必须是可以被赋值的。map和multimap在排序时只看元素的键部分，所以你需要为自己的pair写一个自定义的比较函数，因为pair的operator<对pair的两个部分都会检查。而且你需要另一个比较函数来执行查找，用来做排序的比较函数需要两个pair对象作为参数，但是查找的比较函数的一个参数是与键相同类型的对象，另一个是pair对象，只需要一个键值对。另外你不知道传进来的第一个参数是键还是pair，所以实际上需要两个查找的比较函数，一个是假定键部分作为第一个参数传入，另一个是假定pair先传入。

标准的关联容器通常被实现为平衡二叉树。适合插入、删除、查找的混合操作，提供对数时间的查找能力。但比较浪费内存空间（父指针，左儿子指针，右儿子指针）。如果节点散布在全部地址空间，将会导致更多的页缺失。

散列容器：提供常数时间的查找能力。

使用数据结构的一般过程：
  1. 设置阶段：创建一个新的数据结构，并插入大量元素。在这个阶段，几乎所有的操作都是插入和删除操作，很少或几乎没有查找操作。
  2. 查找阶段：查询该数据结构以找到特定的信息。在这个阶段，几乎所有的操作都是查找操作，很少或几乎没有插入和删除操作。
  3. 重组阶段：改变该数据结构的内容，或许是删除所有的当前数据，再插入新的数据。在行为上，这个阶段与第1阶段类似。但这个阶段结束以后，应用程序又回到了第2阶段。

使用 vector 替代标准关联容器：
 - 在排序的 vector 中存储数据可能比在标准关联容器中存储同样的数据要耗费更少的内存。
 - 考虑到页面错误的因素，通过二分搜索法来查找一个排序的 vector 可能比查找一个标准关联容器要更快一点。
 - 存储在 vector 中的数据必须是 pair<K, V> ，因为排序时它的元素的值将通过赋值操作被移动。
 - 对 vector 做排序时，必须为 pair 写一个自定义的比较类型。（P85）

## R24: 当效率至关重要时，请在 map::operator[] 与 map::insert 之间谨慎做出选择。

`map::operator[]`的功能是**添加和更新**，当map中没有`[]`中指定的键时，则加入一个新pair，如果`[]`中有指定的键时，则更新这个键的值。假如有一个map的值是Widget对象，键是一个简单类型（如int），Widget有一个默认无参构造函数和一个接受一个参数的有参构造函数和赋值构造函数。当map中没有相应的key时，map::insert是比`map::operator[]`更快的，因为`map::operator[]`会构造一个临时对象（调用无参构造函数），再将赋给他新值，而map::insert是直接调用有参构造函数。但是当map中有相应的key时，`map::operator[]`是比map::insert更快的，因为map::insert需要构造和析构对象，而map::operator[]不需要。

总结：当向映射表中添加元素时，要优先选用 insert 而不是 `operator[]`；当更新已经在映射表中的元素的值时，要优先选择 operator[]。


## R25: 熟悉非标准的散列容器

非标准的散列容器有 `hash_map`，`hash_set`，`hash_multimap`，`hash_multiset`

SGI 的散列容器：

```c++
template<typename T,
	typename HashFunction = hash<T>,
	typename CompareFunction = equal_to<T>,
	typename Allocator = allocator<T> >
class hash_set;
```

注意: 与标准关联容器不同， SGI 的散列容器使用 equal_to 作为默认的比较函数，通过测试两个对象是否相等而不是等价来决定容器中的两个对象是否相等。
SGI 的实现把表的元素放在一个单向链表中，而 Dinkumware 的实现则使用了双向链表。


ref:
[1]. https://www.cnblogs.com/Sherry4869/p/15128250.html</br>
[2]. https://blog.csdn.net/zhuikefeng/article/details/108164117#t42


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partthree/  

