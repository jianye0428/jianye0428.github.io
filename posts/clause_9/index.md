# Effective STL [9] | 在删除选项中仔细选择


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->


## 删除指定值对象
假定你有一个容纳`int`标准STL容器:

```c++
Container<int> c;
```

而你想把c中所有值为2023的对象都去掉。

令人吃惊的是，完成这项任务的方法因不同的容器类型而不同：没有一种方法是通用的。

 - 当c是连续内存容器（vector、deque或string），最好的方法是erase-remove惯用法

```c++
c.erase(remove(c.begin(), c.end(), 2023), c.end());
// 当c是vector、string或deque时，
// erase-remove惯用法是去除特定值的元素的最佳方法
```
{{<admonition Note "Note" false>}}
- STL 和 vector中的remove的作用是**将等于value的元素放到vector的尾部**，但并不减少vector的size；
- vector中erase的作用是删除掉某个位置position或一段区域(begin, end)中的元素，减少其size，返回被删除元素下一个元素的位置。
{{</admonition>}}

 - 这方法也适合于`list`，但是`list`的成员函数`remove`更高效：
  ```c++
  // 当c是list时，remove成员函数是去除特定值的元素的最佳方法
  c.remove(1963);
  ```

- 当c是标准关联容器（即`set`、`multiset`、`map`或`multimap`）时，使用任何叫做`remove`的东西都是完全错误的。这样的容器没有叫做remove的成员函数，而且使用remove算法可能覆盖容器值，潜在地破坏容器。对于关联容器，解决问题的适当方法是调用erase：

```c++
// 当c是标准关联容器时,erase成员函数是去除特定值的元素的最佳方法
c.erase(2023);
```
这很高效，只花费对数时间，**序列容器的基于删除的技术需要线性时间**。并且，关联容器的`erase`成员函数有基于等价而不是相等的优势。

## 消除判断式

消除下面判断式，返回真的每个对象:

```c++
bool badValue(int x); // 返回x是否是“bad”
```

- 对于序列容器（`vector`、`string`、`deque`和`list`），把每个`remove`替换为`remove_if`：

```c++
c.erase(remove_if(c.begin(), c.end(), badValue), // 当c是vector、string或deque时
c.end()); // 这是去掉badValue返回真的对象的最佳方法

c.remove_if(badValue); // 当c是list时这是去掉badValue返回真的对象的最佳方法
```

- 对于标准关联容器，有两种方法处理该问题，一个更容易编码，另一个更高效。

```c++
AssocContainer<int> c; // c现在是一种标准关联容器

AssocContainer<int> goodValues; // 用于容纳不删除的值的临时容器

remove_copy_if(c.begin(), c.end(),inserter(goodValues, goodValues.end()), badValue);

c.swap(goodValues); // 交换c和goodValues的内容
```

对这种方法的**缺点**是它拷贝了所有不删除的元素。

因为关联容器没有提供类似`remove_if`的成员函数，所以必须写一个循环来迭代c中的元素，和原来一样删除元素。不幸的是，那些正确工作的代码很少是跃出脑海的代码。例如，这是很多程序员首先想到的：
```c++
AssocContainer<int> c;
...
for (AssocContainer<int>::iterator i = c.begin(); i!= c.end(); ++i) {  // 清晰，直截了当而漏洞百出的,用于删除c中badValue返回真的每个元素的代码
    if (badValue(*i)) {
        c.erase(i);
    }
} // 不要这么做！
```

这有未定义的行为。当容器的一个元素被删时，**指向那个元素的所有迭代器都失效了**。

当`c.erase(i)`返回时，`i`已经失效。那对于这个循环是个坏消息，因为在`erase`返回后，`i`通过for循环的`++i`部分自增。为了避免这个问题，我们必须保证在调用`erase`之前就得到了c中下一元素的迭代器。最容易的方法是当我们调用时在i上使用后置递增：

```c++
AssocContainer<int> c;
...
for (AssocContainer<int>::iterator i = c.begin(); i != c.end();/*nothing*/ ){// for循环的第三部分是空的；i现在在下面自增
    if (badValue(*i)) {
        c.erase(i++); // 对于坏的值，把当前的i传给erase，然后作为副作用增加i；
    }
    else {
        ++i; // 对于好的值，只增加i
    }
}
```

精髓的地方在于：这种调用`erase`的解决方法可以工作，因为表达式i++的值是i的旧值，但作为副作用，i增加了。

因此，我们把i的旧值（没增加的）传给`erase`，但在`erase`开始执行前i已经自增了。

现在不仅删除`badValue`返回真的每个元素，而且每当一个元素被删掉时，我们也想把一条消息写到日志文件中。

 - 可以通过**直接从原容器删除元素来避开拷贝**。
 - “更容易但效率较低”的解决方案用`remove_copy_if`**把需要的值拷贝到一个新容器中，然后把原容器的内容和新的交换**：

对于**关联容器**，这说多容易就有多容易，因为只需要对刚才开发的循环做一个微不足道的修改就行了：

```c++
ofstream logFile; // 要写入的日志文件
AssocContainer<int> c;
...
for (AssocContainer<int>::iterator i = c.begin(); i !=c.end();){// 循环条件和前面一样
    if (badValue(*i)){
        logFile << "Erasing " << *i <<'\n'; // 写日志文件
        c.erase(i++); // 删除元素
    }
    else {
        ++i;
    }
}
```

现在是`vector`、`string`和`deque`不能再使用`erase-remove`惯用法，因为没有办法让`erase`或`remove`写日志文件。

而且，我们不能使用刚刚为关联容器开发的循环，因为它为`vector`、`string`和`deque`产生未定义的行为！

要记得对于那样的容器，**调用`erase`不仅使所有指向被删元素的迭代器失效，也使被删元素之后的所有迭代器失效**。

包括所有i之后的迭代器。我们写`i++`，`++i`或你能想起的其它任何东西都没有用，因为没有能导致迭代器有效的。必须利用erase的返回值。那个返回值正是我们需要的：**一旦删除完成，它就是指向紧接在被删元素之后的元素的有效迭代器。**

我们这么写：
```c++
for (SeqContainer<int>::iterator i = c.begin(); i != c.end();){
    if (badValue(*i)){
        logFile << "Erasing " << *i << '\n';
        i = c.erase(i); // 通过把erase的返回值
    } // 赋给i来保持i有效
    else {
       ++i;
    }
}
```

**<font color=blue>这可以很好地工作，但只用于标准序列容器</font>**。

标准关联容器的`erase`的返回类型是`void`。对于那些容器，你必须使用“**后置递增你要传给erase的迭代器**”技术。为了避免你奇怪`list`的适当方法是什么，事实表明对于迭代和删除，你可以像`vector/string/deque`一样或像关联容器一样对待list；两种方法都可以为list工作。

## 结论

1. **去除一个容器中有特定值的所有对象**：
- 如果容器是`vector`、`string`或`deque`，使用`erase-remove`惯用法
- 如果容器是`list`，使用`list::remove`
- 如果容器是标准关联容器，使用它的`erase`成员函数
2. **去除一个容器中满足一个特定判定式的所有对象**：
- 如果容器是`vector`、`string`或`deque`，使用`erase-remove_if`惯用法
- 如果容器是`list`，使用`list::remove_if`
- 如果容器是标准关联容器，使用`remove_copy_if`和`swap`，或写一个循环来遍历容器元素，当你把迭代器传给`erase`时记得后置递增它
3. **在循环内做某些事情（除了删除对象之外）**：
- 如果容器是标准**序列容器**，写一个循环来遍历容器元素，**每当调用`erase`时记得都用它的返回值更新你的迭代器**。
- 如果容器是标准**关联容器**，写一个循环来遍历容器元素，当**你把迭代器传给`erase`时记得后置递增它**。

如你所见，与仅仅调用erase相比，有效地删除容器元素有更多的东西。

**解决问题的最好方法取决于你是怎样鉴别出哪个对象是要被去掉的，储存它们的容器的类型，和当你删除它们的时候你还想要做什么（如果有的话）。**

这仅对带有迭代器实参的`erase`形式是正确的。关联容器也提供一个带有一个值的实参的`erase`形式，而那种形式返回被删掉的元素个数。但这里，我们只关心通过迭代器删除东西。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_9/  

