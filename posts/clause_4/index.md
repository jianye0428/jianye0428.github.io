# Effective STL [4] | 用empty来代替检查size()是否为0


## 判断容器是否为空

对于任意容器 randy, 当判断是否为空的时候，会使用到以下判断语句：
```c++
if(randy.size() == 0) {
... // work
}

// or

if(randy.empty()) {
... // work
}
```
应该首选empty的构造，而且理由很简单：**对于所有的标准容器，empty是一个常数时间的操作，但对于一些`list`实现，`size`花费线性时间**


## list size耗时

**Q**：但是什么造成list这么麻烦？为什么不能也提供一个常数时间的size？

**A**：对于list特有的splice有很多要处理的东西。

考虑这段代码：

```c++
list<int> list1;
list<int> list2;
...
list1.splice( // 把list2中
list1.end(), list2, // 从第一次出现5到
find(list2.begin(), list2.end(), 5), // 最后一次出现10
find(list2.rbegin(), list2.rend(), 10).base() // 的所有节点移到list1的结尾。
);
```

除非`list2`在5的后面有一个10，否则这段代码无法工作，但是咱们假设这不是问题。

**Q**：假设上述代码可以正常运行，那么接合后`list1`有多少元素？

**A**：结合后`list1`的元素个数=接合之后前`list1`的元素个数 + 接合进去的元素个数

**Q**：有多少元素接合进去了？

**A**：元素个数为`find(list2.rbegin(), list2.rend(), 10).base()`所定义的区间的元素个数。

到底有多少？

在没有遍历这个区间并计数之前无法知道。

**问题剖析**

如果size是一个常数时间操作，当操作时每个list成员函数必须更新list的大小。也包括了splice。

让区间版本的splice更新它所更改的list大小的**唯一的方法**是算出接合进来的元素的个数，但**那么做就会使它不可能有你所希望的常数时间的性能。**

如果你去掉了splice的区间形式要更新它所修改的list的大小的需求，splice就可以是常数时间，但**size就变成线性时间的操作**。

一般来说，**必须遍历它的整个数据结构来才知道包含多少元素**。

不同的list实现用不同的方式解决这个矛盾，依赖于他们的作者选择的是让size或splice的区间形式达到最高效率。

如果你碰巧使用了一个常数时间的splice的区间形式比常数时间的size优先级更高的list实现，**调用empty比调用size更好，因为empty总是常数时间操作。**

## 结论

不同的list实现用不同的方式解决这个矛盾，依赖于它们的作者选择的是让size或splice的区间形式达到最高效率。

如果你碰巧使用了一个常数时间的splice的区间形式比常数时间的size优先级更高的list实现，<font color=blue>调用empty比调用size更好，因为empty总是常数时间操作。</font>

## STL vector 和 list的empty 及 size  实现源码

**vector**
```c++
// vector
  size_type
  size() const _GLIBCXX_NOEXCEPT
  { return size_type(end() - begin()); }

  bool
  empty() const _GLIBCXX_NOEXCEPT
  { return begin() == end(); }
```

**list**

```c++
// list

/**  Returns the number of elements in the %list.  */
size_type
size() const _GLIBCXX_NOEXCEPT
{ return _M_node_count(); }

      // return the stored size
      size_t
      _M_node_count() const
      { return this->_M_get_size(); }

// size() 调用 _M_get_size
 size_t _M_get_size() const { return _M_impl._M_node._M_size; }


// [23.2.2.2] capacity
/**
*  Returns true if the %list is empty.  (Thus begin() would equal
*  end().)
*/
_GLIBCXX_NODISCARD bool
empty() const _GLIBCXX_NOEXCEPT
{ return this->_M_impl._M_node._M_next == &this->_M_impl._M_node; }
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_4/  

