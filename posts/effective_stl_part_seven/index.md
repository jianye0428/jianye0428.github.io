# Effective STL 精读总结 [7] | 在程序中使用STL



# 前言

> Effective-STL总结系列分为七部分，本文为第七部分，涉及原书第七章，内容范围Rule43~50。为方便书写，Rule43简写为R43。

{{< admonition Note "Effective-STL系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective STL(第3版)精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 [Effective STL(第3版)精读总结(二)](https://jianye0428.github.io/posts/parttwo/)</br>
💡 [Effective STL(第3版)精读总结(三)](https://jianye0428.github.io/posts/partthree/)</br>
💡 [Effective STL(第3版)精读总结(四)](https://jianye0428.github.io/posts/partfour/)</br>
{{< /admonition >}}


## R43 算法调用优先于手写的循环

调用算法优于手写循环：
  - 效率：**算法**比手写的循环**效率更高**。
  - 正确性：手写循环比使用算法容易出错。
  - 可维护性：使用算法的代码更加简洁明了。

例子：P155，

算法的名称表明了它的功能，而 for、while、do 循环不能。

手写循环需要维护迭代器的有效性。

## R44 容器的成员函数优先于同名的算法

原因:
  1. 成员函数往往速度快。
  2. 成员函数通常与容器结合得更加紧密。（同样的名称做不同的事情）

对于 map 和 multimap 而言:
  1. 成员函数可以获得对数时间的性能。
  2. 成员函数的相同是等价，而算法是相等。
  3. 它们的成员函数只统计检查每个 pair 对象的键部分。而算法同时检查键和值/(key,value)对。

对于 list 而言，list 成员函数只是简单地维护指针，可以提供更好的性能。list 的 remove、remove_if、unqiue 则实实在在的删除了元素。sort 算法不能直接应用于 list，因为 sort 需要随机访问迭代器，而 list 的迭代器是双向迭代器。


## R45 正确区分count、find、binary_search、lower_bound、upper_bound和equal_range

1. 假设你要在容器中查找一些信息，标题列出的几个函数应该怎么选择呢？首先应该考虑区间是否是排序的，如果是排序的，则binary_search、lower_bound、upper_bound和equal_range具有更快的查找速度，如果不是排序的，那么你只能选择count、count_if、find、find_if，这些算法仅提供线性时间的查找效率。但是这些函数还是有区别的，count、count_if、find、find_if使用相等性进行查找，binary_search、lower_bound、upper_bound和equal_range使用等价性进行查找。
2. 考虑count和find的区别，count表示区间是否存在待查找的值，如果存在有多少个？find表示区间是否存在待查找的值，如果存在它在哪里？假设你只想知道区间中是否存在待查找的值，如果是使用count会更方便一些，因为使用find还需要比较find返回的指针是否是容器的end()，但是因为find找到第一个符合查找的值就会返回，count一定会遍历到容器的末尾，所以find的效率更高。
3. 当你的区间是排序的，那么你就可以使用对数时间查找的四种方法。与标准C/C++函数库中的bsearch不同的是，binary_search仅判断区间是否存在待查找的值（返回值是bool），如果你还想知道待查找值得位置，可以使用其他三种方法，先考虑lower_bound，lower_bound查找特定值会返回一个迭代器，同样你需要判断这个迭代器的结果，除了要判断这个迭代器是否是end()，还要判断其指向的值是否是待查找的值，所以很多人会这么写：

```C++
vector<Widget>::iterator i = lower_bound(vw.begin(), vw.end(), w);
if(i != vw.end() && *i == w) { ... }//这里有一个错误
```
这里`*i == w`是一个相等性测试，但是lower_bound使用等价性进行搜索的，虽然大多数情况下等价性测试和相等性测试的结果相同，但是条款19也说明了违背这个情况也很常见，所以正确也更方便的方式是使用equal_range，equal_range会返回一对迭代器，第一个迭代器等于lower_bound返回的迭代器，第二个迭代器等于upper_bound返回的迭代器，equal_range返回了一个子区间，其中的值与待查找的值等价，如果返回的两个迭代器相同，等说明查找的区间没有待查找的值，而子区间的长度就是等价值的个数，可以使用distance得到。

4. 再考虑lower_bound和upper_bound的使用场景，这次我不是希望查找某个元素，而是想找到第一个比特定值大的元素的位置，这个特定值可能不在容器中，那么可以使用lower_bound，如果希望找到第一个大于或等于特定值的元素的位置，那么可以使用upper_bound。


## R46 考虑使用函数对象而不是函数作为 STL 算法的参数
1. 第一个原因是因为效率。随着抽象程度的提高，所生成代码的效率是在降低的，比如几乎在所有情况下，操作包含一个double的对象比直接操作double都是要慢的，但是可能令你感到惊讶的是，将函数对象传递给STL算法往往比传递函数更加高效。比如说你想生成一个降序排列的vector<double>，你可以使用函数对象，即greater<double>，也可以使用一个自定义的比较函数，而第一种情况往往是更快的，这是因为如果一个函数对象的operator()函数被声明是内联的（可以通过inline显式声明，也可以定义在类中隐式声明），那么它的函数体可被编译器直接优化，但是传递函数指针则不行，编译器不能优化。
   ```C++
   sort(v.begin(), v.end(), doubleGreater);
   sort(v.begin(), v.end(), greater<double>());
   ```
  使用`greater<double>()`的sort调用比使用`doubleGreater`的 sort 调用快得多。原因：**函数内联，sort 不包含函数调用**。
  抽象性利益：C++ 的 sort 算法性能总是优于 C 的 qsort。在运行时，sort 算法以内联方式调用它的比较函数，而 qsort 则通过函数指针调用它的比较函数。
  使用函数对象，可以让你的程序正确地通过编译，避免语言本身的缺陷。
2. 第二个原因是正确性，有的时候STL平台会拒绝一些完全合法的代码，如下：
  ```c++
  //完全合法的代码但是不能通过编译
  set<string> s;
  transform(s.begin(), s.end(), ostream_iterator<string::size_type>(cout, "\n"), mem_fun_ref(&string::size));

  //可以改用函数对象的形式
  struct stringSize : public unary_function<string, string::size_type>{
  string::size_type operator()(const string& s) const {
      return s.size();
  }
  };
  ```

## R47 避免产生 “直写型” （write only）的代码

所谓”直写型“的代码是指一行代码中有过于复杂的嵌套函数调用，可能对于编写代码的人，这行代码看似非常直接和简单，但是对于阅读代码的人则显得难以理解。所以在遇到这种写出“直写型”代码时，应该将其拆分成多行代码，或者使用typedef起别名的形式，让代码更易于阅读。

## R48 总是包含（#include）正确的头文件

有的时候即使漏掉了必要的头文件，程序同样可以编译，这是因为C++标准并没有规定标准库中头文件之间的相互包含关系，这就导致了某个头文件包含了其他头文件，如<vector>包含了<string>。但是这种程序往往是不可移植的，即使在你的平台上可以编译，但是在其他平台上就可能会编译不过，所以解决此类问题的一条原则就是总是include必要的头文件

## R49 学会分析与 STL 相关的编译器诊断信息
在程序编译或者运行出错时，有时编译器给出的诊断信息非常混乱和难以阅读。对于这些信息可以使用同义词替换的方法进行简化，比如说将std::basic_string<>替换成string，将std::map<>替换成map，将看不懂的STL内部模板std::_Tree替换成something

## R50 熟悉与 STL 相关的 web 站点
[SGI STL 站点](http://www.sgi.com/tech/stl)
[STLport 站点](http://www.stlport.org/)
[Boost 站点](http://www.boost.org/)

Ref:</br>
[1]. htTps://www.cnblogs.com/Sherry4869/p/15162253.html</br>
[2]. https://blog.csdn.net/zhuikefeng/article/details/108164117#t35</br>
[3]. https://zhuanlan.zhihu.com/p/458156007</br>

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/effective_stl_part_seven/  

