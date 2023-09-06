# Effective STL [35] | 通过mismatch或lexicographical比较实现简单的忽略大小写字符串比较


## 怎么使用STL来进行忽略大小写的字符串比较?

如果你**忽略国际化问题而且只关注于设计成字符串strcmp那样的类型**，这个任务很简单。

如果你要有strcmp不具有的按语言处理字符串中的字符的能力（即，容纳文本的字符串是除了英语以外的语言）或程序使用了区域设置而不是默认的，这个任务很困难。

想要使用**忽略大小写**比较的程序员通常需要两种不同的调用接口：

 - 一种类似strcmp（返回一个负数、零或正数）
 - 另一种类似operator（返回true或false）

## 确定两个字符除了大小写之外是否相等

类似`strcmp`进行的字符串比较，只考虑类似`strcmp`的字符串比较：

```C++
// 忽略大小写比较字符
// c1和c2，如果c1 < c2返回-1，
// 如果c1==c2返回0，如果c1 > c2返回1
int ciCharCompare(char c1, char c2) {
  // 这些语句的解释看下文
  int Ic1 = tolower(static_cast<unsigned char>(c1));
  int Ic2 = tolower(static_cast<unsigned char>(c2));
  if (Ic1 < Ic2) return -1;
  if (Ic1 > Ic2) return 1;
  return 0;
}
```
这个函数遵循了strcmp，可以返回一个负数、零或正数，依赖于c1和c2之间的关系。

**tolower**

与strcmp不同的是，`ciCharCompare`在进行比较前把两个参数转化为小写，从而忽略大小写的字符比较。

正如`<cctype>`（也是`<ctype.h>`）里的很多函数，tolower的参数和返回值类型是int，但除非这个int是EOF，它的值必须能表现为一个unsigned char。

在C和C++中，char可能或可能不是有符号的（依赖于实现），当char有符号时，唯一确认它的值可以表现为unsigned char的方式是在调用tolower之前转换一下。

**基于mismatch实现**

给定了`ciCharCompare`，就很容易写出我们的第一个忽略大小写的两个字符串比较函数，提供了一个类似`strcmp`的接口。

`ciStringCompare`这个函数，返回一个负数、零或正数，依赖于要比较的字符串的关系。

它基于`mismatch`算法，因为`mismatch`确定了两个区间中第一个对应的不相同的值的位置。

我们必须确定一个字符串是否比另一个短，短的字符串作为第一个区间传递。

因此我们可以把真正的工作放在一个叫做`ciStringCompareImpl`的函数，然后让`ciStringCompare`简单地确保传进去的实参顺序正确，如果实参交换了就调整返回值：

```c++
int ciStringCompareImpl(const string &s1, const string &s2);
// 实现请看下文
int ciStringCompare(const string &s1, const string &s2) {
  if (s1.size() <= s2.size())
    return ciStringCompareImpl(s1, s2);
  else
    return -ciStringCompareImpl(s2, s1);
}
```

在ciStringCompareImpl中，大部分工作由mismatch来完成。它返回一对迭代器，表示了区间中第一个对应的字符不相同的位置：

```c++
int ciStringCompareImpl(const string &si, const strings s2) {
  // PSCI = “pair of string::const_iterator”
  typedef pair<string::const_iterator, string::const_iterator> PSCI;
  // 下文解释了为什么我们需要not2；
  PSCI p =
      mismatch(s1.begin(), s1.end(), s2.begin(), not2(ptr_fun(ciCharCompare)));

  if (p.first == s1.end()) {
    // 如果为真，s1等于s2或s1比s2短
    if (p.second == s2.end())
      return 0;
    else
      return -1;
  }
  return ciCharCompare(*p.first,
                       *p.second);  // 两个字符串的关系 和不匹配的字符一样
}
```

**比较过程**

当字符匹配时这个判断式返回true，因为当判断式返回false时mismatch会停止。我们不能为此使用ciCharCompare，因为它返回-1、1或0，而当字符匹配时它返回0，就像strcmp。

如果我们把ciCharCompare作为判断式传给mismatch，C++会把ciCharCompare的返回类型转换为bool，而当然bool中零的等价物是false，正好和我们想要的相反！同样的，当ciCharCompare返回1或-1，那会被解释成true，因为，就像C，所有非零整数值都看作true。这再次和我们想要的相反。

要修正这个语义倒置，我们在ciCharCompare前面放上not2和ptr_fun。

**std::not1 和 std::not2**
std::not1 和 std::not2是用来把“符合某种条件的函数对象”转换为反义函数对象的函数。

`std::ptr_fun`用来把函数指针封装为符合某种条件的函数对象的函数，C++11 里可以用`std::bind`或`std::function`代替

## lexicographical_compare

可以在关联容器中用作比较函数的函数，可以把ciCharCompare修改为一个有判断式接口的字符比较函数，然后把进行字符串比较的工作交给STL中名字第二长的算法——lexicographical_compare：

```c++
// 返回在忽略大小写的情况下c1是否在c2前面；
bool ciCharLess(char c1, char c2) {
  // 条款46解释了为什么一个函数对象可能 比函数好
  tolower(static_cast<unsigned char>(c1)) <
      tolower(static_cast<unsigned char>(c2));
}

bool ciStringCompare(const string &s1, const string &s2) {
  // 算法调用的讨论在下文
  return lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(),
                                 ciCharLess);
}
```

lexicographical_compare是strcmp的泛型版本。

**strcmp只对字符数组起作用，但lexicographical_compare对所有任何类型的值的区间都起作用。**

同时，strcmp总是比较两个字符来看看它们的关系是相等、小于或大于另一个。lexicographical_compare可以传入一个决定两个值是否满足一个用户定义标准的二元判断式。


**比较过程**

在上面的调用中，lexicographical_compare用来寻找s1和s2第一个不同的位置，基于调用ciCharLess的结果。如果，使用那个位置的字符，ciCharLess返回true，lexicographical_compare也是；如果，在第一个字符不同的位置，从第一个字符串来的字符先于对应的来自第二个字符串的字符，第一个字符串就先于第二个。

就像strcmp，lexicographical_compare认为两个相等值的区间是相等的，因此它对于这样的两个区间返回false：第一个区间不在第二个之前。

也像strcmp，如果第一个区间在发现不同的对应值之前就结束了，lexicographical_compare返回true：一个先于任何区间的前缀是一个前缀。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_35/  

