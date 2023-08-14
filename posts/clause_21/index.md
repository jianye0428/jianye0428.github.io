# Effective STL [21] | 永远让比较函数对相等的值返回false


## Example set

建立一个set，比较类型用`less_equal`，然后插入整型数字33(称为 $33_{A}$):

```C++
set<int, less_equal<int> > s; // s以“<=”排序
s.insert(33); // 插入33
```

现在尝试再插入一次33(称为 $33_{B}$):

```C++
s.insert(33);
```

对于这个insert的调用，set必须先要判断出 $33_{A}$ 是否已经位于其中了，查找哪儿适合插入 $33_{B}$。最终，它总要检查 $33_{B}$是否与 $33_{A}$ 相同。

关联容器对“相同”的定义是等价，因此set测试 $33_{B}$是否等价于 $33_{A}$。

当执行这个测试时，它自然是使用set的比较函数。在这一例子里，是`operator<=`，因为我们指定`set`的比较函数为`less_equal`，而`less_equal`意思就是`operator<=`。

可以看看`less_equal`的源码实现：

```c++
/// One of the @link comparison_functors comparison functors@endlink.
template<typename _Tp>
struct less_equal : public binary_function<_Tp, _Tp, bool>
{
  _GLIBCXX14_CONSTEXPR
  bool
  operator()(const _Tp& __x, const _Tp& __y) const
  { return __x <= __y; } // 此处是 <=
};
```

于是，set将计算这个表达式是否为真:

```c++
!(33A <= 33B) && !(33B <= 33A) // 测试33A和33B是否等价
```

$33_{A}$ 和 $33_{B}$ 都是33，因此， $33_{A}$<=$33_{B}$ 肯定为真。同样清楚的是，$33_{B}$ <= $33_{A}$。于是上述的表达式简化为:

```c++
!(true) && !(true)
```

再简化就是

```c++
false && false
```

结果当然是false。

也就是说，set得出的结论是 $33_{B}$ 与 $33_{B}$ 不等价，因此不一样，于是它将 $33_{B}$ 插入容器中的旁边。

在技术上而言，这个做法导致未定义的行为，但是通常的结果是set以拥有了两个为33的值的拷贝而告终，也就是说它不再是一个set了。通过使用less_equal作为我们的比较类型，我们破坏了容器！

**测试**

```c++
template<typename T>
void print(const T t) {
 cout << t << endl;
}

int main(int argc, char** argv) {
  set<int, less_equal<int> > snumber;
  snumber.insert(33);
  snumber.insert(33);
  for_each(snumber.begin(), snumber.end(), print<int>);
}
```

结果:

```c++
33
33
```

## Example string*

`条款20`描述了该如何写一个比较函数以使得容纳`string*`指针的容器根据string的值排序，而不是对指针的值排序。那个比较函数是按升序排序的，但现在假设你需要`string*`指针的容器的降序排序的比较函数。

自然是抓现成的代码来修改了。如果不细心，可能会这么干:

```c++
struct StringPtrGreater:
  public binary_function<const string*, const string*, bool> { // 这代码是有瑕疵的！
  bool operator()(const string *ps1, const string *ps2) const
  {
    return !(*ps1 < *ps2); // 只是相反了旧的测试；这是不对的！
  }
};
```

这里的想法是通过将比较函数内部结果取反来达到反序的结果。很不幸，取反“<”不会给你（你所期望的）“>”，它给你的是 “>=”。

而你现在知道，因为它将对相等的值返回true，对关联容器来说，它是一个无效的比较函数。

你真正需要的比较类型是这个:

```c++
struct StringPtrGreater: // 对关联容器来说这是有效的比较类型
  public binary_function<const string*, const string*, bool> {
    bool operator()(const string *ps1, const string *ps2) const {
      return *ps2 < *ps1; // 返回*ps2是否大于*ps1（也就是交换操作数的顺序）
    }
};
```

要避免掉入这个陷阱，你所要记住的就是比较函数的返回值表明的是在此函数定义的排序方式下，一个值是否大于另一个。

**相等的值绝不该一个大于另一个，所以比较函数总应该对相等的值返回`false`**

## Example multiset

multiset和multimap那些容器可以容纳复本可能包含副本，如果容器认为两个值相等的对象不等价，它将会把两个都存储进去的，这正是multi系列容器的所要支持的事情。

```c++
multiset<int, less_equal<int> > msnum; // 仍然以“<=”排序
msnum.insert(22); // 插入22A
msnum.insert(22); // 插入22B
for_each(msnum.begin(), msnum.end(), print<int>);
```

结果:

```c++
22
22
```

s里有两个22的拷贝，**因此我们期望如果我们在它上面做一个`equal_range`，我们将会得到一对指出包含这两个拷贝的范围的迭代器。但那是不可能的。**

`equal_range`，虽然叫这个名字，但不是指示出相等的值的范围，而是等价的值的范围。在这个例子中，s的比较函数说22A和22B是不等价的，所以不可能让它们同时出现在equal_range所指示的范围内。

```c++
pair<multiset<int>::iterator, multiset<int>::iterator> range2;
//在 myvector 容器中找到所有的元素 22
range2 = equal_range(msnum.begin(), msnum.end(), 22, less_equal<int>{});
cout << "\nmy multiset：";
for (auto it = range2.first; it != range2.second; ++it) {
 cout << *it << " ";
}
```

结果没有得到期望结果:

```shell
my multiset:
```

如果改为`less`比较:

```c++
range2 = equal_range(msnum.begin(), msnum.end(), 22, less<int>{});
```

结果:

```c++
my multiset：22 22
```

<font color=red>**除非你的比较函数总是为相等的值返回false，你将会打破所有的标准关联型容器，不管它们是否允许存储复本。**</font>

## 总结

从技术上说，用于排序关联容器的比较函数必须在它们所比较的对象上定义一个“**严格的弱序化(strict weakordering)**”。（传给sort等算法（参见条款31）的比较函数也有同样的限制）。

**任何一个定义了严格的弱序化的函数都必须在传入相同的值的两个拷贝时返回false。**

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_21/  

