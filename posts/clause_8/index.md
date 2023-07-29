# Effective STL [8] | 永不建立auto_ptr的容器


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->

## 拷贝一个auto_ptr将改变它的值

当你拷贝一个`auto_ptr`时，`auto_ptr`所指向对象的所有权被转移到拷贝的`auto_ptr`，而被拷贝的`auto_ptr`被设为`NULL`。

```c++
class Widget {
 public:
  explicit Widget(int in) : randy(in) {}
  inline bool operator<(Widget& in) { return randy < in.randy; }

 public:
  int randy;
};

auto_ptr<Widget> pw1(new Widget); // pw1指向一个Widget
auto_ptr<Widget> pw2(pw1); // pw2指向pw1的Widget; pw1被设为NULL。（Widget的所有权从pw1转移到pw2。）
pw1 = pw2; // pw1现在再次指向Widget； pw2被设为NULL
```
有意思的是，如果你建立一个`auto_ptr<Widget>`的`vector`，然后使用一个指向的`Widget`的值的函数对它进行排序：

```c++
bool widgetAPCompare(const auto_ptr<Widget>& lhs, const auto_ptr<Widget>& rhs) {
 return *lhs < *rhs; // 假设Widget 存在operator<
}
auto_ptr<Widget> w1(new Widget(3));
auto_ptr<Widget> w2(new Widget(2));
widgets.push_back(w1);
widgets.push_back(w2);
vector<auto_ptr<Widget> > widgets; // 建立一个vector，然后用Widget的auto_ptr填充它；
// 记住这将不能编译！
sort(widgets.begin(), widgets.end(), widgetAPCompare);// 排序这个vector
```

这段代码将不能编译

```shell
warning: ‘template<class> class std::auto_ptr’ is deprecated [-Wdeprecated-declarations]
   30 |   std::vector<auto_ptr<Widget> >
      |               ^~~~~~~~
In file included from /usr/include/c++/9/memory:80,
                 from temp.cpp:10:
/usr/include/c++/9/bits/unique_ptr.h:53:28: note: declared here
   53 |   template<typename> class auto_ptr;
      |                            ^~~~~~~~
temp.cpp:33:3: warning: ‘template<class> class std::auto_ptr’ is deprecated [-Wdeprecated-declarations]
   33 |   auto_ptr<Widget> w1(new Widget(3));
      |   ^~~~~~~~
```

从概念上看所有东西也都很合理，但结果却完全不合理。例如，在排序过程中widgets中的一个或多个auto_ptr可能已经被设为NULL。

排序这个vector的行为可能已经改变了它的内容！

## 剖析

实现`sort`的方法是使用了**快速排序算法**的某种变体。

排序一个容器的**基本思想**是，选择容器的某个元素作为“主元”，然后对大于和小于或等于主元的值进行递归排序。

在sort内部，这样的方法看起来像这样：
```c++

template<class RandomAccessIterator, class Compare>// 这个sort的声明直接来自于标准
void sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    // 这个typedef在下面解释
    typedef typename iterator_traits<RandomAccessIterator>::value_type ElementType;
    RandomAccessIterator i;
    ... // 让i指向主元
    ElementType pivotValue(*); // 把主元拷贝到一个局部临时变量中；
    ... // wor
}
```

源码为：
```c++
template<typename _RandomAccessIterator, typename _Compare>
inline void
sort(_RandomAccessIterator __first, _RandomAccessIterator __last,
 _Compare __comp)
{
  // concept requirements
  __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<
    _RandomAccessIterator>)
  __glibcxx_function_requires(_BinaryPredicateConcept<_Compare,
    typename iterator_traits<_RandomAccessIterator>::value_type,
    typename iterator_traits<_RandomAccessIterator>::value_type>)
  __glibcxx_requires_valid_range(__first, __last);
  __glibcxx_requires_irreflexive_pred(__first, __last, __comp);

  std::__sort(__first, __last, __gnu_cxx::__ops::__iter_comp_iter(__comp));
}

// 上面 __gnu_cxx::__ops::__iter_comp_iter(__comp) 的实现如下
  template<typename _Compare, typename _Iterator>
    inline _Iter_comp_to_iter<_Compare, _Iterator>
    __iter_comp_iter(_Iter_comp_iter<_Compare> __comp, _Iterator __it)
    {
      return _Iter_comp_to_iter<_Compare, _Iterator>(
   _GLIBCXX_MOVE(__comp._M_comp), __it); // 这里有move
    }
```

当涉及`iterator_traits<RandomAccessIterator>::value_type`时，必须在它前面写上`typename`，因为它是一个**依赖于模板参数类型的名字**，在这里是`RandomAccessIterator`。

上面代码中棘手的是这一行:

```c++
ElementType pivotValue(*i);
```

因为它把一个元素从保存的区间拷贝到局部临时对象中。

在例子里，这个元素是一个`auto_ptr<Widget>`，所以这个拷贝操作默默地把被拷贝的`auto_ptr——vector`中的那个——设为`NULL`。

另外，当`pivotValue`出了生存期，它会自动删除指向的`Widget`。这时sort调用返回了，`vector`的内容已经改变了，而且至少一个`Widget`已经被删除了。

也可能有几个`vector`元素已经被设为`NULL`，而且几个`widget`已经被删除，因为快速排序是一种递归算法，递归的每一层都会拷贝一个主元。

## 结论

智能指针的容器是很好的， 但是`auto_ptr`<font color=red>完全不是那样的智能指针</font>。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_8/  

