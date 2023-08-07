# Effective STL [16] | 如何将vector和string的数据传给遗留的API


## string 和 vector 传递给C API

### vector传递指针

C风格API接受的是**数组**和`char*`指针，这样的API还会存在很长时间，如果有1个vector对象randy， 则使用`&v[0]`就可以得到一个指向randy中数据的指针。对于string对象sesame，则传递`sesame.c_str()`即可。

表达式`randy[0]`生产一个指向vector中第一个元素的引用，所以，`&randy[0]`是指向那个首元素的指针。

vector中的元素被C++标准限定为存储在连续内存中，就像是一个数组。

所以我们可能会这么传递

```c++
// C API
void doSomething(const int* pInts, size_t numInts);

// 调用
doSomething(&randy[0], randy.size());
```

唯一的问题就是，如果randy是空的，`randysize()`是0，而`&randy[0]`试图产生一个指向根本就不存在的东西的指针。

可以提前判断一下randy的大小：

```c++
if (!randy.empty()) {
  doSomething(&randy[0], randy.size());
}
```

> **randy.begin()代替&randy[0]?**

对于vector，其迭代器实际上是指针。

`begin`的返回类型是`iterator`，而不是一个指针，当你需要一个指向vector内部数据的指针时绝不该使用begin。如果你基于某些原因决定键入`randy.begin()`，就应该键入`&*randy.begin()`，因为这将会产生和`&v[0]`相同的指针。

### string 传递指针

类似从vector上获取指向内部数据的指针的方法，**对string不是可靠的**:

  1. string中的数据并没有保证被存储在独立的一块连续内存中
  2. string的内部表示形式并没承诺以一个null字符结束。这解释了string的成员函数c_str存在的原因

即使是字符串的长度为0，`c_str`将返回一个指向`null`字符的指针。

**在两种形式下，指针都被传递为指向const的指针。vector和string的数据只能传给只读取而不修改它的API。**

如果你将`randy`传给一个修改其元素的C风格API的话，典型情况都是没问题，但**被调用的函数绝不能试图改变vector中元素的个数**。否则，randy的内部状态将会变得不一致，`v.size()`将会得到一个不正确的结果。

**把一个vector传递给需要修改vector数据的API，一定要确保这些额外限制继续被满足**，比如是否需要保持原来vector中元素的顺序。


## C风格API返回的元素初始化STL容器

### 初始化vector

**利用vector和数组潜在的内存分布兼容性将存储vecotr的元素的空间传给API函数：**
```c++
// C API：此函数需要一个指向数组的指针，数组最多有arraySize个double
// 而且会对数组写入数据。它返回写入的double数，不会大于arraySize
size_t fillArray(double *pArray, size_t arraySize);

vector<double> vd(maxNumDoubles); // 建立一个vector，它的大小是maxNumDoubles
vd.resize(fillArray(&vd[0], vd.size())); // 让fillArray把数据写入vd，然后调整vd的大小为fillArray写入的元素个数
```

**这个技巧只能工作于vector，因为只有vector承诺了与数组具有相同的潜在内存分布。**

### 初始化string

只要让API将数据放入一个`vector<char>`，然后从vector中将数据拷到string：

```c++
// C API：此函数需要一个指向数组的指针，数组最多有arraySize个char
// 而且会对数组写入数据。它返回写入的char数，不会大于arraySize
size_t fillString(char *pArray, size_t arraySize);

vector<char> vc(maxNumChars); // 建立一个vector，它的大小是maxNumChars
size_t charsWritten = fillString(&vc[0], vc.size()); // 让fillString把数据写入vc
string s(vc.begin(), vc.begin() + charsWritten); // 从vc通过范围构造函数拷贝数据到s
```

### 初始化其他STL容器

**通用方法：C风格API把数据放入一个vector，然后拷到实际想要的STL容器**

```c++
size_t fillArray(double *pArray, size_t arraySize); // 同上

vector<double> vd(maxNumDoubles); // 一样同上
vd.resize(fillArray(&vd[0], vd.size()));
deque<double> d(vd.begin(), vd.end()); // 拷贝数据到deque
list<double> l(vd.begin(), vd.end()); // 拷贝数据到list
set<double> s(vd.begin(), vd.end()); // 拷贝数据到set
```

### STL容器传递给C API

STL容器将它们的数据传给C风格API，只要将容器的每个数据拷到vector，然后将vector传给API:

```c++
void doSomething(const int*pints, size_t numInts); // C API (同上)

set<int> intSet; // 保存要传递给API数据的set
...
vector<int> v(intSet.begin(), intSet.end()); // 拷贝set数据到vector
if (!v.empty()) doSomething(&v[0], v.size()); // 传递数据到API
```

**如果在编译期就知道容器的大小**，可以将数据拷进一个**数组**，然后将数组传给C风格的API，否则不得不分配动态数组。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_16/  

