# Effective STL [25] | 熟悉非标准散列容器


**STL没有散列表。**

兼容STL的散列关联容器可以从多个来源获得，而且它们甚至有事实上的标准名字：`hash_set`、`hash_multiset`、`hash_map`和`hash_multimap`。在C++标准委员会的议案中，散列容器的名字是`unordered_set`、 `unordered_multiset`、`unordered_map`和`unordered_multimap`。

**它们在接口、能力、内在数据结构和支持操作的相关效率方面不同。**

最常见的两个来自`SGI`和`Dinkumware`，`STLport`也提供散列容器，但是`STLport`的散列容器是基于来自`SGI`的。

散列容器是关联容器，它们需要知道储存在容器中的对象类型，用于这些对象的比较函数，以及用于这些对象的分配器。


## 散列容器声明

散列容器需要散列函数的说明。下面是散列容器声明：

```c++
template<typename T,
 typename HashFunction,
 typename CompareFunction,
 typename Allocator = allocator<T> >
class hash_container;
```

这非常接近于散列容器的SGI声明，主要差别是SGI为HashFunction和CompareFunction提供了默认类型。

**SGI 设计举例**

`hash_set`的SGI声明看起来基本上像这样：

```c++
template<typename T,
 typename HashFunction = hash<T>,
 typename CompareFunction = equa_to<T>,
 typename Allocator = allocator<T> >
class hash_set;
```

`SGI`设计的一个值得注意的方面是使用`equal_to`作为默认比较函数。**这违背标准关联容器的约定——默认比较函数是less。**

`SGI`的散列容器确定在一个散列容器中的两个对象是否有相同的值是通过相等测试，而不是等价。

因为散列关联容器，不像它们在标准中的（通常基于树）兄弟，不需要保持有序。


**Dinkumware 设计举例**

Dinkumware设计的散列容器采取一些不同的策略。它仍然允许你指定对象类型、散列函数类型、比较函数类型和分配器类型，但是**它把默认的散列和比较函数移进一个单独的类似特性的叫做hash_compare的类**，而且它把`hash_compare`作为容器模板的`HashingInfo`实参的默认值。

这是`Dinkumware`的hash_set声明（再次为演示而调整过）:

```c++
template<typename T, typename CompareFunction>
class hash_compare;
template<typename T,
typename HashingInfo = hash_compare<T, less<T> >
typename Allocator = allocator<T> >
class hash_set;
```

这种接口设计有趣的地方是`HashingInfo`的使用。

**容器的散列和比较函数储存在`HashingInfo`中，但HashingInfo类型也容纳了控制表中桶（bucket）最小数量，以及容器元素对桶的最大允许比率的枚举。**

当这比率被超过时，表中桶的数量就增加，而表中的一些元素需要重新散列。（SGI提供具有类似控制表中桶和表中元素对桶比率的成员函数。）

**hash_compare**

`hash_compare`（HashingInfo的默认值）看起来或多或少像这样:

```c++
template<typename T, typename CompareFunction = less<T> >
class hash_compare {
public:
    enum {
        bucket_size = 4, // 元素对桶的最大比率
        min_buckets = 8 // 桶的最小数量
    };
    size_t operator()(const T&) const; // 散列函数比较函数
    bool operator()(const T&, const T&) const;
    ... // 忽略一些东西，包括 CompareFunction的使用
}
```

重载`operator()`（在这里是实现散列和比较函数）是比你可以想象的更经常出现的一个策略。

`Dinkumware`设计允许你写你自己的类似hash_compare的类（也许通过从`hash_compare`本身派生而来），而且只要你的类定义了`bucket_size`、`min_buckets`、两个`operator()`函数（一个带有一个实参，一个带有两个），加上已经省去的一些东西，就能使用它来控制Dinkumware的`hash_set`或`hash_multiset`的配置和行为。

`hash_map`和`hash_multimap`的配置控制也相似。

## 决策留给实现

注意不管是SGI还是Dinkumware的设计，你都能把全部决策留给实现:

```c++
// 建立一个int的散列表
hash_set<int> intTable;
```

要让这个可以编译，散列表必须容纳一个整数类型（例如int），因为**默认散列函数一般局限于整数类型。**

**实现方法不同**

在后端，SGI和Dinkumware的实现方法非常不同。

1. SGI利用常用的一个元素的单链表的指针数组（桶）组成的开放散列法。
2. Dinkumware也利用了开放散列法，但是它的设计是基于一种新颖的数据结构——由迭代器（本质是桶）的数组组成的元素双向链表，迭代器的相邻对表示在每个桶里元素的范围。

**链表不同**

1. SGI实现在**单链表**中储存表的元素
2. Dinkumware实现使用一个双向链表

**迭代器种类不同**

1. SGI的散列容器提供了前向迭代器，因此你得放弃进行反向迭代的能力：在SGI的散列容器中没有rbegin或者rend成员函数。
2. 用于Dinkumware散列容器的迭代器是双向的，所以它们可以提供前向和反向遍历。

**内存使用量方面不同**

SGI的设计比Dinkumware的节俭一点点。

## 结论

虽然STL本身缺乏散列容器，兼容STL的散列容器（有不同的接口、能力和行为权衡）不难得到。就SGI和STLport的实现而言，你甚至可以免费下载得到它们。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_25/  

