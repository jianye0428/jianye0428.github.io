# Effective STL [3] | 使容器里对象的拷贝操作轻量而正确


## 拷贝对象是STL的方式

- 当一个对象进入一个容器，它已经不是你添加（`insert`或`push_back`等）的那个对象了，进入容器的是你指定的对象的拷贝；

- 当从容器中取出一个对象时，所得到的也不是容器里的对象；

- 如果从`vector`、`string`或`deque`中插入或删除了什么，现有的容器元素会移动（拷贝）

- 如果使用了任何排序算法：`next_permutation`或者`previous_permutation`；

- `remove`、`unique`或它们的同类；

- `rotate`或`reverse`等，对象会移动（拷贝）

**拷进去，拷出来**。这就是STL的方式.

因为拷贝，还解决了一个 double free 的 bug[点击查看](https://mp.weixin.qq.com/s?__biz=MzUyMDc2MDMxNg==&mid=2247490628&idx=1&sn=43650727bc93d8064fd2969733873fdc&chksm=f9e422d7ce93abc1d2784bcb5772536f55aa79902bbb783003c3314df7ed6461a014ed3cce25&token=235869638&lang=zh_CN&scene=21#wechat_redirect)

## How Copy? 如何完成拷贝

{{< admonition info "Notice" true >}}

通过拷贝构造函数和拷贝复制操作符完成！

{{< /admonition >}}

一个对象通过使用它的拷贝成员函数来拷贝，特别是它的拷贝构造函数和它的拷贝赋值操作符。

对于用户自定义类，比如Widget，这些函数传统上是这么声明的：

```c++
class Widget {
public:
...
Widget(const Widget&); // 拷贝构造函数
Widget& operator=(const Widget&); // 拷贝赋值操作符
...
};
```

**如果你自己没有声明这些函数，你的编译器始终会为你声明它们。**

拷贝内建类型（比如int、指针等）也始终是通过简单地拷贝他们的内在比特来完成的。（请参考《Effective C++》中，条款11和27专注于这些函数的行为。）


## 拷贝带来的问题

**性能瓶颈**

拷贝会导致把对象放进容器也会被证明为是一个性能瓶颈。

容器中移动越多的东西，你就会在拷贝上浪费越多的内存和时钟周期。

**切片分割**

当然由于继承的存在，拷贝会导致分割。

如果以基类对象建立一个容器，而你试图插入派生类对象，那么当对象（通过基类的拷贝构造函数）拷入容器的时候对象的派生部分会被删除：

```c++
vector<Widget> randy;
class SpecialWidget: public Widget {...}; // SpecialWidget从上面的Widget派生
SpecialWidget sw;
randy.push_back(sw); // sw被当作基类对象拷入randy，当拷贝时它的特殊部分丢失了
```

分割问题暗示了把一个派生类对象插入基类对象的容器几乎总是错的。

如果你希望结果对象表现为派生类对象，比如，调用派生类的虚函数等，总是错的。

## 解决

一个使拷贝更高效、正确而且对分割问题免疫的简单的方式是**建立指针的容器而不是对象的容器**。

也就是说，不是建立一个Widget的容器，建立一个Widget*的容器。

**拷贝指针很快，它总是严密地做你希望的（指针拷贝比特），而且当指针拷贝时没有分割，就是int类型的地址。**

<font color=red>但是一定要记得在销毁容器的时候，使用delete 销毁里面保存的每个指针。而且一定要定义对象的深拷贝构造函数和深拷贝拷贝赋值操作符，否则delete 的时候会报错。</font>

## 和数组对比，STL容器更文明

**STL容器只建立（通过拷贝）你需要的个数的对象，而且它们只在你指定的时候做。**

STL进行了大量拷贝，但它通常设计为避免不必要的对象拷贝，实际上，它也被实现为避免不必要的对象拷贝。

1. 数组在声明的时候，会默认先构造好每个元素；STL容器可以实现动态扩展

```c++
Widget randy[maxNumWidgets]; // 建立一个大小为maxNumWidgets的Widgets数组
// 默认构造每个元素
```
即使只使用其中的一些或者我们立刻使用从某个地方获取（比如，一个文件）的值覆盖每个默认构造的值，这也得构造maxNumWidgets个Widget对象。

使用STL来代替数组，你可以使用一个可以在需要的时候增长的vector，就是动态数组的概念：

```c++
vector<Widget> randy; // 建立一个0个Widget对象的vector
// 需要的时候可以扩展
```

2. 建立一个可以足够包含maxNumWidgets个Widget的空vector，但不去构造Widget，需要时再构造：

```c++
vector<Widget> randy;
randy.reserve(maxNumWidgets); // reserve的详细信息请参见条款14
```
即便需要知道STL容器使用了拷贝，但是别忘了一个事实：比起数组它们仍然是一个进步。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_3/  

