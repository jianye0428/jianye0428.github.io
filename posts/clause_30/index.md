# Effective STL [30] | 确保目标区间足够大


STL容器在被添加时（通过insert、push_front、push_back等）自动扩展它们自己来容纳新对象。

## 插入数据

**尾部插入 back_inserter**

当你想向容器中插入对象但并没有告诉STL他们所想的时，问题出现了:

```C++
int transmogrify(int x); // 自定义的这个函数从x产生一些新值
vector<int> values;
...
// 把数据放入values
vector<int> results;
// 把transmogrify应用于values中的每个对象
// 把这个返回的values附加到results
transform(values.begin(), values.end(),results.end(),transmogrify);

// 这段代码有bug！
```

transform被告知它的目的区间是从`results.end()`开始的，所以那就是开始写在values的每个元素上调用`transmogrify`的结果的地方。

就像所有使用目标区间的算法，`transform`通过对目标区间的元素赋值的方法写入结果，`transform`会把`transmogrify`应用于`values[0]`并把结果赋给`*results.end()`。

然后它会把`transmogrify`应用于`value[1]`并把结果赋给`*(results.end()+1)`。

那只能带来灾难，因为在`*results.end()`没有对象，`*(results.end()+1)`也没有！因为`transform`并没有在尾部创造新的对象。

调用`transform`是错误的，因为它会给不存在的对象赋值。

**正确做法**

把`transform`的结果放入`results`容器的结尾的方式是调用`back_inserter`来产生指定目标区间起点的迭代器:

```C++
vector<int> results;
// 把transmogrify应用于values中的每个对象，在results的结尾插入返回的values
transform(values.begin(), values.end(),back_inserter(results),transmogrify);
```

在内部，`back_inserter`返回的迭代器会调用`push_back`，所以你可以在任何提供`push_back`的容器上使用`back_inserter`(也就是任何标准序列容器: `vector`、`string`、`deque`和`list`)。

**前端插入 front_inserter**

如果你想让一个算法在容器的前端插入东西，你可以使用`front_inserter`。

在内部，`front_inserter`利用了`push_front`，所以`front_insert`只和提供那个成员函数的容器配合(也就是`deque`和`list`):

```C++
...
// 同上
list<int> results;
// results现在是list
transform(values.begin(), values.end(),front_inserter(results),transmogrify);
// 在results前端   以反序   插入transform的结果
```

因为`front_inserter`用`push_front`把每个对象添加到`results`，`results`中的对象顺序会和`values`中对应的对象顺序相反。

`vector`不提供`push_front`，所以`front_inserter`不能用于`vector`。

**同序插入**

如果你要`transform`把输出结果放在`results`前端，但你也要输出和`values`中对应的对象顺序相同，只要以相反的顺序迭代`values`:

```c++
list<int> results;
// 同上
// 在results前端 插入transform的结果
transform(values.rbegin(), values.rend(),front_inserter(results),transmogrify);

// 保持相对的对象顺序
```

**任意位置插入 inserter**

`front_inserter`让你强制算法在容器前端插入它们的结果，`back_inserter`让你告诉它们把结果放在容器后端，有点惊人的是`inserter`允许你强制算法把它们的结果插入容器中的任意位置:

```c++
vector<int> values;
...
vector<int> results;
...
// 同上
// 同上，除了现在在调用transform前 results已经有一些数据
transform(values.begin(), values.end(),
// 把transmogrify的结果插入results的中间
inserter(results, results.begin() + results.size()/2), transmogrify);
```

## 插入效率

不管你是否使用了`back_inserter`、`front_inserter`或`inserter`，`transform`会对目的区间每次写入一个值，你无法改变。

当你要插入的容器是`vector`或`string`时，你可以最小化这个代价，**预先调用`reserve`**。

你仍然要承受每次发生插入时移动元素的开销，但至少你避免了重新分配容器的内在内存:

```c++
vector<int> values;
// 同上
vector<int> results;
...
results.reserve(results.size() + values.size());
// 确定results至少还能装得下values.size()个元素
transform(values.begin(), values.end(),
// 同上，但results没有任何重新分配操作
inserter(results, results.begin() + results.size() / 2), transmogrify);
```

当使用`reserve`来提高一连串插入的效率时，总是应该记住`reserve`**只增加容器的容量：容器的大小仍然没有改变**。

即使调用完`reserve`，当你想要让容器把新元素加入到`vector`或`string`时，你也必须对算法使用插入迭代器(比如，从`back_inserter`、`front_inserter`或`inserter`返回的迭代器之一)，因为赋值只在两个对象之间操作时有意义，而不是在一个对象和一块原始的比特之间。

第一个例子正确的写法:

```c++
vector<int> values;
// 同上
vector<int> results;
results.reserve(results.size() + values.size());
// 同上
// 把transmogrify的结果写入results的结尾，处理时避免了重新分配
transform(values.begin(), values.end(), back_inserter(results) , transmogrify);
```

## 覆盖原始数据

有时候你要覆盖现有容器的元素而不是插入新的。

**当这种情况时，你不需要插入迭代器，但你仍然需要按照本条款的建议来确保你的目的区间足够大。**

假设你让`transform`覆盖`results`的元素。如果`results`至少有和`values`一样多的元素，那很简单。如果没有， 你也必须使用`resize`来确保它有。

```c++
vector<int> values;
vector<int> results;
...
// 确保results至少和values一样大
if (results.size() < values.size()){
 results.resize(values.size());
}
// 覆盖values.size()个 results的元素
transform(values.begin(), values.end(), results.begin(), transmogrify);
```

或者你可以清空results然后用通常的方式使用插入迭代器:

```c++
...
// 销毁results中的所有元素
results.clear();
// 保留足够空间
results.reserve(values.size());
// 把transform地返回值// 放入results
transform(values.begin(), values.end(), back_inserter(results), transmogrify);
```

## 结论

无论何时你使用一个要求指定目的区间的算法，确保目的区间已经足够大或者在算法执行时可以增加大小。

如果你选择增加大小，就使用插入迭代器，比如`ostream_iterators`或从`back_inserter`、`front_inserter`或`inserter`返回的迭代器。


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_30/  

