# Effective C++ (第3版) 精读总结 [4]


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->
# 前言

> Effective-C++总结系列分为四部分，本文为第二部分，涉及原书第3~4章，内容范围Rule13~25。为方便书写，Rule13简写为R13。

{{< admonition Note "Effective-C++系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective C++(第3版)精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 [Effective C++(第3版)精读总结(二)](https://jianye0428.github.io/posts/parttwo/)</br>
💡 [Effective C++(第3版)精读总结(三)](https://jianye0428.github.io/posts/partthree/)</br>
💡 [Effective C++(第3版)精读总结(四)](https://jianye0428.github.io/posts/partfour/)</br>
{{< /admonition >}}

由于原书在C++11之前写成，有些现代C++特性不会提及，所以会根据本人`开发经验`新增一些个人感悟👉`By the way`环节。

## CH7 模板与泛型编程

### R41 了解隐式接口和编译期多态

面向对象编程的通常做法：
- **显式**定义接口（参数和返回值类型都是确定的，函数签名 就是确定的）
- 虚函数实现**运行时多态**

那么编译期多态呢？在面向对象编程中，我们的答案通常是函数重载。

在Templates 及泛型编程中，更为注重的是**隐式接口**和**编译期多态**：
- `隐式接口`:
  Template的接口属于隐式接口，因为它类型不确定，所以是基于有效表达式来区分。再加上运算符重载等特性，隐式接口的自由度非常大。
- `编译期多态`:
  Template的多态是通过Template具现化和函数重载决议发生于编译期。

上述描述看起来有点抽象，分析下方Template函数代码：
```c++
template<typename T>
void doProcessing(T& w){
  if(w.size() > 10 && w != someNastyWidget){
    ...
  }
}
```

doProcessing属于隐式接口，类型不定，传入不同类型T，就得到不同的函数。

由于 `操作符重载(operator overloading)` 的存在，隐式接口对传入的 T 类型的要求比较宽松，列举如下：

  1. T类型或其父类型具有 size() 接口，size() 接口返回类型S为整型或者S类型有 `bool operator>(int)` 接口；
  2. T类型有 `bool operator != (const T&)`接口；或T类型可以隐式转换为某S类型，S可以与someNastyWidget进行 != 比较，或者干脆someNastyWidget就是T类型；
  3. 上述1 、 2 点的描述没有将 `operator &&` 考虑进来，否则情况会更为复杂，这里不赘述了。

### R42 了解typename的双重意义

- `typename` 和 `class` 关键字在声明 Template 参数的时候，没有任何区别
如下示例，效果是一致的：
```c++
template<class T> class Widget ; //早期C++使用这个声明Template参数
template<typename T> class Widget; //现在更推荐用这个，暗示参数不一定是个class类型
```
- 使用关键字 `typename` 标识 **嵌套从属类型名称**
  模板内的从属类型，是指在模板函数/模板类中，需要**依赖传入的 template 参数间接表示**的类型，如果呈嵌套状，就叫嵌套从属名称/类型。 如下示例的 `C::const_iterator` 就是嵌套从属类型。
  ```c++
  template<typename C>
  void printContainer2ndVal(const C& container) {
    if (container.size() >= 2) {
      typename C::const_iterator iter(container.begin());
      std::cout << "this container's second value is: " << *++iter << std::endl;
    }
  }
  ```
  此时，typename 的作用就是告诉编译器 C::const_iterator 是个类型，否则会编译报错。此时可以用 class 关键字替代，但是不建议。

  typename 使用范围不只是 template 函数内部，也包括了函数入参 ：
  ```c++
  template<typename C>
  void foo(const C& container,    // 不允许使用 "typename"
      typename C::iterator iter); // 必须使用 "typename"
  ```
  ⚠️ 其他特殊情况：不要在 基类列表base class list 或 成员初始化列member initialization list 使用 typename
  ```c++
  template<typename T>
  class Derived: public Base<T>::Nested{ //基类列表不允许使用typename
  public:
    explicit Derived(int x):Base<T>::Nested(x){// 成员初值列也不允许typename
      typedef typename Base<T>::Nested NestedType;//结合typedef定义，常规用法
      NestedType temp;
      ...// 其他操作
    }
  };
  ```

### R43 学习处理模板化基类内的名称

**模板化基类**：指定义 template 模板类时，继承于某基类 Base<T>，这个 Base<T> 类就是模板化基类。

🤔 那么 Base<T> 类有关的函数等名称该如何处理，在子类中直接使用吗？还是有注意事项？这，就是本节要讨论的问题。示例如下：
```c++
template<typename Company>
class MsgSender{
public:
    void SendPlain(){ std::cout << "send plain\n";}
    void SendEncrypted(){ std::cout << "send encrypted\n";}
};  // 基类
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>{//子类
public:
    void SendPlainMsg(){
        SendPlain();//  ==> 这样直接调用是不行的，想想为什么？
    }
};
```
上述注释中问题的原因：因为当全特化时可随意去除 MsgSender 内的函数，那么编译器无法确定基类是否仍有 SendPlain 这个方法。所以，编译器干脆**拒绝在编译时**去模板化基类中**主动寻找**继承而来的名称。

解决方法有三个：
  1. 使用 `this->` 调用SendPlain()，即可转化为运行时的问题，推荐这个方法；
  2. 使用 `using MsgSender::SendPlain;` 这样的 using 声明式，我们在 Rule 33: 避免遮掩由继承得来的名称 中用过这种方法，显式地告诉编译器扩大名称搜索范围，尝试去基类找找；
  3. 使用显式调用，即 `MsgSender::SendPlain();` ，但这样做的坏处是直接断绝了 virtual 函数的灵活性和可能，不太推荐

### R44 将与参数无关的代码抽离templates

通常，在非模板代码中，我们很容易看出重复的代码或者类，一般采用抽取公共部分为单独函数，或者采用类的继承/聚合等面向对象设计方式。

但是， template 有关的代码，有时代码重复是隐晦的。 这一节是为了解决 template 的 `代码膨胀(code bloat)` 问题，主要原则为 `共性与差异性分析(commonality and variability analysis)` 。

**某些不恰当的设计，会导致无畏的代码膨胀。**先看下方这个反面教材：
```c++
template<typename T, std::size_t n>
class SquareMatrix {
public:
    void invert() { std::cout << "size is " << n << std::endl; };
};
```

应用代码如下：

```c++
//下面这2句，模板类实例化 1 次。 invert函数有 1 份实现
SquareMatrix<int, 5> mat5;
SquareMatrix<int, 5> mat5New;
//换成下面这2句，模板类实例化 2 次。invert函数有 2 份实现(问题暴露)
SquareMatrix<int, 5> mat5;
SquareMatrix<int, 10> mat10;
```

**膨胀原因**：在编译器看来，SquareMatrix<int, 5> 和 SquareMatrix<int, 10> 是2个**完全不同的 C++ 类**，实例化了2次，生成了这2个类。试想，如果代码中还需要其他 size 的SquareMatrix ，都会再生成一个新类，那么最后二进制文件就多了许多重复的目标代码，也就是膨胀。

如果能**抽离参数相关代码**，做到尺寸无关，便可解决这个问题。

**解决思路**：新建一个基类 BaseMatrix ，将尺寸参数传入给到基类作为成员变量保管，具体的数据可以用数组指针形式托管。不论有多少个与尺寸有关的派生类，一个 T 类型就**只有一个基类**，所以基类 `baseInvert` **函数实现只有一份**。

<mark>基类实现</mark>代码如下：

```c++
template<typename T>
class BaseSqaureMatrix {
public:
    BaseSqaureMatrix(std::size_t n,T* dataPtr) :n_(n), dataPtr_(dataPtr) {}
protected:
    void baseInvert() { std::cout << "invert: pData = " << dataPtr_ << " , size = " << n_  << std::endl; }
    void SetDataPtr(T* dataPtr) { dataPtr_ = dataPtr; };
    std::size_t n_ = 0;
    T* dataPtr_ = nullptr;
};
```

<mark>目标派生类</mark>实现如下：

```c++
template<typename T,std::size_t n>
class SquareMatrix : public BaseSqaureMatrix<T> {
public:
    SquareMatrix() :BaseSqaureMatrix<T>(n,nullptr), data_(std::shared_ptr<T>(new T[n*n])) {
        this->SetDataPtr(data_.get());
    }
    void invert() { this->baseInvert(); }// 参考Rule43
private:
    std::shared_ptr<T> data_ ;// RAII管理资源
};
```

最终效果：相同 T 类型的template实例类(比如SquareMatrix<float,5>和SquareMatrix<float,6>)，<mark>共享实现码</mark>，祛除代码膨胀。

### R45 运用模板成员函数接受所有兼容类型

众所周知，`Base *pBase = pDerived`; 这样普通类型裸指针的隐式转换是被 C++ 所认可的。

那么如果换成智能指针模板类 SmartPtr<Base> 和 SmartPtr <Derived> 呢，还能这样**不需额外配置**就随意转换吗，答案显然**是不行**的（纵然 Base 和 Derived 是兼容类型也不行）。

本节，就采用<mark>模板成员函数</mark>来解决这一问题。

- **使用模板成员函数，接受所有兼容类型的“泛化构造”/“泛化赋值”**
  `泛化(generalized)Copy构造`，是指接受其他兼容类型进行模板函数的Copy构造。举例一个粗糙的 RAII 智能指针实现，代码如下👇:
  ```c++
  template<class T>
  class SmartPtr {
  public:
    SmartPtr(T* origPtr) :originPtr_(origPtr) {}
    SmartPtr(const SmartPtr& smt) :originPtr_(smt.get()) {}// 正常Copy构造函数
    //正常copy操作符函数: 简单演示，就浅复制吧
    SmartPtr& operator=(const SmartPtr& smt) { originPtr_ = smt.get(); return *this; }

    template<class U> //泛化构造函数
    SmartPtr(const SmartPtr<U>& other):originPtr_(other.get()) { } //为了隐式转换不加 explict

    template<class U> //泛化赋值操作符
    SmartPtr& operator=(const SmartPtr<U>& other) {
        originPtr_ = other.get();
        return *this;
    }//简单演示，浅复制

    T* get() const { return originPtr_; }
    ~SmartPtr() { delete originPtr_;}
  private:
    T* originPtr_ = nullptr;
  };
  ```
  有上述的代码支持，我们就能写出如下的应用代码：
  ```c++
  class BaseObj{};
  class DerivedObj:public BaseObj {};//DerivedObj是BaseObj兼容类型
  SmartPtr<DerivedObj> smDerived(new DerivedObj());
  SmartPtr<BaseObj> smBase(smDerived);
  SmartPtr<BaseObj> smBase2 = smDerived;
  ```
- **即使有 “泛化构造/ 赋值”，也需要正常的 Copy构造/赋值**
  注意，模板成员函数不会改变语言规则，也就是说“泛化构造”不能代替正常的 copy 构造函数。所以，需要同时声明正常的 Copy 构造函数和 Copy 赋值操作符函数。

### R46 需要类型转换时请为模板定义非成员函数

有时我们希望 class 支持类型转换，类似本书的另外一节👉 [Rule24:若所有参数皆需类型转换，请采用non-member](https://jianye0428.github.io/posts/parttwo/#r24-%E8%8B%A5%E6%89%80%E6%9C%89%E5%8F%82%E6%95%B0%E7%9A%86%E9%9C%80%E7%B1%BB%E5%9E%8B%E8%BD%AC%E6%8D%A2%E8%AF%B7%E9%87%87%E7%94%A8non-member) 。

如果 Rule24 中的 Rational 类是 template 模板类，直接仿照 Rule24 利用 non-member 函数做混合运算是行不通的，原因是 **template 实参推导**是不采纳 “通过构造函数而发生的”隐式类型转换。

解决办法如下:
  - **使用 “template class 内部的friend函数” 完成参数隐式转换**
    将 Rule24 的 non-member 的 **operator *函数**改为 `inline-friend` 函数，因为 template 实例化的时候需要找到该 friend 函数的定义实现，类外定义实现是会链接错误的，所以需要 inline 。
    参考代码实现如下：
    ```c++
    template<typename T>
    class RationalNew {
    public:
      RationalNew(T numerator = 0, T denominator = 1) :numerator_(numerator), denominator_(denominator) {}

      T numerator()const { return numerator_; }
      T denominator()const { return denominator_; }
      friend const RationalNew<T> operator*(const RationalNew<T>&lhs, const RationalNew<T>&rhs) {
          return RationalNew<T>(lhs.numerator()*rhs.numerator(),lhs.denominator()*rhs.denominator());
      }
    private:
      T numerator_ = 0;
      T denominator_ = 1;
    };
    ```
    那么，对如下的应用代码，就使用自如了：
    ```c++
    RationalNew<float> oneHalf(1.0,2.0);
    RationalNew<float> res = oneHalf * 2;
    RationalNew<float> res2 = 3 * oneHalf;
    ```
  - **若inline-friend函数体太大，可以抽离出类外辅助函数供inline调用**
    上述的 operator* 函数体只有一行，实现简单，但如果实现过程复杂代码量大，考虑 inline 带来的冲击以及代码可读性，可以抽取个函数出来供 inline-friend 调用。

    修改方式如下：
    ```c++
    template<typename T>
    class RationalNew {
    public:
    //  其他部分略。注意 operator* 函数体实现，改为调用doMultiply
      friend const RationalNew<T> operator*(const RationalNew<T>&lhs, const RationalNew<T>&rhs) {
        return doMultiply(lhs,rhs);
      }
    };
    // 类外函数  doMultiply
    template<typename T>
    const RationalNew<T> doMultiply(const RationalNew<T>&lhs, const RationalNew<T>&rhs) {
      return RationalNew<T>(lhs.numerator()*rhs.numerator(), lhs.denominator()*rhs.denominator());
    }
    ```

### R47 请使用traits classes表现类型信息

C++ 中通常把 Traits 称为**类型萃取**技术，即：在 template 编程中，获取模板参数的类型信息，并在编译阶段针对不同的类型响应不同的处理。同时，这个技术要求对C++**内置类型built-in** 和 **用户自定义user-defined** 类型的表现必须一样好。

本节讨论此议题，并且以一个 C++ 标准的模板函数作为切入口进行讨论(std::advance，作用是兼容不同类型迭代器，用于后续取用指定偏移量的元素 )。

下方 “By the way” 环节简单展示了 std::advance 标准C++的声明式和基本用法： 👇（单击展开）

{{<admonition Note "By the way">}}
std::advance 需要的头文件包含为 `#include<iterator>` ，再看下它的函数原型声明：

```c++
template< class InputIt, class Distance >
void advance( InputIt& it, Distance n );       // C++17 之前
template< class InputIt, class Distance >
constexpr void advance( InputIt& it, Distance n ); // 自C++17 开始
```

再看下示例应用代码：

```c++
std::vector<int> nums{1,2,3,4,5};
auto vIter = nums.begin();
std::advance(vIter, 3);
std::cout << "after advance  3 offset: *vIter = " << *vIter << std::endl;

std::deque<float> fNums{0.1,0.2,0.3,0.4,0.5,0.6};
auto dIter = fNums.end();
std::advance(dIter,-2);
std::cout << "after advance -2 offset: *dIter = " << *dIter << std::endl;

std::list<char> chList{10,20,30,40};
auto cIter = chList.begin();
std::advance(cIter,2);
std::cout << "after advance  2 offset: *cIter = " << int(*cIter) << std::endl;
```
最终输出信息如下：
```shell
after advance  3 offset: *vIter = 4
after advance -2 offset: *dIter = 0.5
after advance  2 offset: *cIter = 30
```
{{</admonition>}}
背景知识 👉 STL 迭代器按照功能分为 5 类，如下方表格所述：

|描述	|特点	|应用|
|---|---|---|---|
|input 迭代器|	只向前移动，一次一步，只读	|istream_iterators|
|output迭代器	|只向前移动，一次一步，只写	|ostream_iterators|
|forward迭代器	|只向前移动，一次一步，可读写	|single-list iterator|
|Bidirectional迭代器	|双向移动，一次一步，可读写|	set/multiset/map/multimap|
|random-access迭代器|	双向移动，一次多步，可读写	|vetor/deque/string|

<mark>如何设计一个 trait_class 并运用起来呢？</mark>

  - **Step1： 确认若干个希望获取的类型信息** (本例只有一个 iterator 类别信息)
    针对5 种迭代器分类，C++ 提供了专属的 **卷标结构tag struct** 加以区分(可以理解为编译期的枚举作用)，继承关系如下：
    ```c++
    struct input_iterator_tag{};
    struct output_iterato_tag{};
    struct forward_iterator_tag:public input_iterator_tag {};
    struct bidirectional_iterator_tag: public forward_iterator_tag{};
    struct random_access_iterator_tag: public bidirectional_iterator_tag{};`
    ```
  - **Step2：为该信息选一个名称** (例如 iterator_category)
    那么 vector，set之类的**目标容器类**，如何与上述 iterator_tag **联系起来**呢，需要一个名称传递出去：
    ```c++
    template< ... > //省略template 参数
    class vector {
    public:
      class iterator{
      public:   // ps: C++11 之后使用的都是 using 定义式
        typedef random_access_iterator_tag iterator_category;
      };
    };
    ```
  - **Step3：提供一个 template 类和一组特化版本**（特化版本支持某些特殊情况）
    `Traits 技术`针对于迭代器的关键模板类 iterator_traits 定义如下：
    ```c++
    template<typename IterT>  //  注意：使用的是 struct
    struct iterator_traits { //这里的 IterT 就可以传入vector等容器类
      typedef typename IterT::iterator_category  iterator_category;
    }
    ```
    注意，这里的 IterT 类型不能是指针类型，因为 pointer 不能后续嵌套。那么就需要一个特化版本了，代码如下：
    ```c++
    template<typename IterT>
    struct iterator_traits<IterT*>{ // 指针类型和 random 迭代器类似
      typedef random_access_iterator_tag iterator_category;
    }
    ```

至此，iterator_traits 的基本实现就完成了，也就是说 `iterator_traits<IterT>::iterator_category` 可以在编译期确定，接下来看看advance 函数如何使用它。

💗 正确做法：利用 **函数重载overloading** 技术，使得 **trait classes 在编译期对类型执行 if … else 测试**。

回顾我们的 advance 函数，只有迭代器类型 IterT 和 偏移量 DistT 。可以重载其子函数 doAdvance ，完成类型萃取后的自适应：
```c++
template<typename IterT, typename DistT>
void doAdvance(IterT& iter, DistT d, std::random_access_iterator_tag) {
    iter += d;
}
template<typename IterT, typename DistT>
void doAdvance(IterT& iter, DistT d, std::bidirectional_iterator_tag) {
    if (d >= 0) { while (d--) ++iter; }
    else { while (d++) --iter; }
}
template<typename IterT, typename DistT>
void doAdvance(IterT& iter, DistT d, std::input_iterator_tag) {
    if (d < 0) {throw std::out_of_range("Negative distance");}//ps: msvc中的C++实现是采用编译期间的asset判定
    while (d--) ++iter;
}
```
自然地，advance 函数的实现如下所示：
```c++
template<typename IterT, typename DistT>
void  advance(IterT& iter, DistT d){
    doAdvance(iter,d, /*不论 category 的tag类型是什么，重载能找到匹配函数。即编译期的 if...else 测试*/
              typename std::iterator_traits<IterT>::iterator_category());
}
```
至此，一个完整的 (以 iterator_traits 为例)Traits 技术实现与运用的过程就完成了。

{{<admonition Note "By the way">}}
- 关于 iterator_traits ，不止有 iterator_category，还有 difference_type, value_type, pointer, reference 等4个成员，[详细可参考 cpp参考手册：iterator_traits](https://zh.cppreference.com/w/cpp/iterator/iterator_traits) 。
- C++ 标准库中类似 iterator_traits 应用了 Traits 技术的模板有十几个。

  举例常用的数值类型萃取 `numeric_limits` ，需要注意2个细节：
  1. 头文件是 #include<limits>;
  2. 成员函数 lowest/min/max 按照顺序，分别代表给定类型的 最低有限值、最小非负值、最大有限值。尤其是 float/double，取最小值是 lowest() ，不是 min().
{{</admonition>}}

### R48 认识Template模板元编程

**TMP，模板元编程template metaprogramming** ，是编写 template-based C++ 程序并执行于编译期的过程。TMP 过程结束后，若干 C++ 源码会被 templates 具现化出来，便会一如往常地被编译。

TMP 有 2 个强大的作用：
  1. 可以完成非 TMP 的常规编程做不到的事情
      比如代码生成，类型适配等。
  2. 可以将某些工作从运行期转移到编译期
      可以将运行期的错误提前暴露在编译期，可以获得更小的可执行文件，更快地运行，更少地内存需求，缺点是明显增加编译时间。


TMP 已被证明是个“**图灵完备**”的机器，意思是它强大到可以计算任何事物。使用 TMP 可以声明变量、执行循环、编写及调用函数…等等。

比较特别的是，TMP 实现上述各类功能的方式不同于常规 C++ 程序。比如上一节 Rule47 中使用重载完成了编译期的 if…else 条件分支。TMP 循环功能也通常会使用 “**递归具现化**” 来完成的。

下方代码示范如何使用 TMP 的方式来计算阶乘：
```c++
template<unsigned n>
struct Factorial
{   // 递归的形式体现: f(n) = n * f(n -1)
    enum {value = n* Factorial<n-1>::value};
};
template<>
struct Factorial<0>
{  // 模板全特化: 实际是初始化 f(0) = 1
    enum {value = 1};
};
```
那么阶乘计算在编译期就完成了，运行时就是直接取用了：
```c++
std::cout << "Factorial(" << 5 << ") = " << Factorial<5>::value << std::endl;
std::cout << "Factorial(" << 7 << ") = " << Factorial<7>::value << std::endl;
```

## CH8 定制new和delete

###  R49 了解new-handler的行为
当 `operator new` 无法满足某个内存分配需求时，一般会抛出 `std::bad_alloc` 异常。

> 如果用 std::nothrow 修饰 new 操作符，使得内存分配阶段不会抛异常，失败了就返回 null 指针。举例 :
> ```c++
> int *pArr = new (std::nothrow) int[0x1fffffff]; //即使分配失败，也不抛异常，而是得到null指针.
> //  nothroow-new 不能保证 class 后续的构造函数不抛异常
> ```
new-hanlder 的使用示例如下：
```c++
//先定义一个函数
void OutOfMemTip() {
    std::cerr << "new memory fail,out of memory!\n";
    std::abort();//终止程序，若调试模式会弹窗提示
}
// 故意制造new失败的情形，程序会调用 OutOfMemTip，触发std::abort()
std::set_new_handler(OutOfMemTip);
int *pArr =  new int[0x1fffffff];//约2GB,如果扛的住，调大这个数
```
可以看出这样的函数指针给了使用者非常大的设计弹性，可以做到以下事情：
- 提前申请内存，让 new_handler 触发下一次分配使用；
- 安装另一个 new_handler ，或许新的 handler 可以申请到内存；
- 卸载 new_handler ，只要传 null 指针即可；
- 手动抛 std::bad_alloc 的异常；
- 不返回，通常调用 std::abort() 或 std::exit() ;

🤔 我们思考另外一个问题：是否可以 让不同的 **C++ 类拥有自己的 new-handler** 呢？ C++ 标准机制是不支持的，我们可以自己实现。有两个实现途径，列举如下：

1. **针对某个特定类 ，类内重载 static 类型的 `operator new` 以及 `set_new_handler`方法**
  具体实现略，只想指出这样做法有个明显弊端，就是每个类都得这么做，比较麻烦，也容易代码冗余。
2. **使用 CRTP 方法（即 `怪异的循环模板模式curiously recurring template pattern` ）将上述方法 1 塞进 template 类**
  这样做的好处是使用模板类赋予上述 operator new 和 set_new_hanler 的操作，使用起来方便。
  CRTP 方法中的基类 NewHandlerSupport 实现如下 👇(点击打开折叠) ：
  ```c++
  template<typename T>
  class NewHandlerSupport {
  public:
      static std::new_handler set_new_handler(std::new_handler p)noexcept {
          std::new_handler oldHandler = currentHandler_;
          currentHandler_ = p;
          return oldHandler;
      }
      static void* operator new(std::size_t size) throw(std::bad_alloc) {
          std::new_handler oldHandle = std::set_new_handler(currentHandler_);
          void *res = ::operator new(size); //new完后复原global-new-handler
          std::set_new_handler(oldHandle);//原书使用RAII手法在还原这个handler，这里作用类似
          return res;
      }
  private:
      static std::new_handler currentHandler_;//初始化动作放到类外cpp文件里去
  };
  ```
  那么目标类 `TestNewHandler` 只要基于 `CRTP` 方法继承于基类就可以了，实现如下：
  ```c++
  class TestNewHandler : public NewHandlerSupport<TestNewHandler> {
    //不必声明 set_new_handler 或 operator new
  };
  //最终应用时也非常简单
  TestNewHandler::set_new_handler(OutOfMemTip);
  TestNewHandler* pTestHandle = new TestNewHandler();
  ```

### R50 了解new和delete的合理替换时机

替换编译器提供的 `operator new` 和 `operator delete` 的 常见理由 如下：

1. **检测运用上的错误**
 自定义的new/delete可以很方便地校验实际控制的区段内存，比如可以在目标区块前后额外空间添加内存签名(比如写入特定 int 值)，监测越界的问题。
 越界分2种：underruns (区块内存起点之前) 和 overruns (区块内存末尾之后)。

2. **时间或内存使用的优化**
 通用的 new/delete 需要适用各种分配形态和场景，对于内存碎片或时间性能都是中庸水平。
 对特定的需求和场景，定制化地内存管理会有很好的优化效果。

3. **收集内存使用的统计数据**
 对内存分配细节的把控，例如分配区块的大小分布、存续周期、FIFO/LIFO次序分配回收、内存峰值等情况。

4. **弥补默认内存分配器的 非最佳对齐位suboptimal alignment**
 例如x86体系结构CPU上访问double都是8bytes对齐，如果能在内存分配时就做好内存对齐，可提升访问效率。

5. **将相关对象成簇集中**
 比如已知某个数据结构往往一起使用，那么分配的时候应该尽量让所有数据的内存集中一些，避免频繁触发 换页中断page faults ，提升访问效率。

6. **其他的非传统行为**
  想完成一些系统编译器办不到的事情。比如希望分配释放共享内存的区块，但是只有 C-API 能做到，那就需要定制版的 new/delete 去包裹封装这样的API。

### R51 编写new和delete时需固守常规

上一个条款讲了重写 new/delete 的原因，这一节将讲述具体需要遵守的几个规则。

1. **正确处理 new 失败的情况**
  如果分配正常，直接返回区块对应的指针即可。可如果失败，就必须得正确调用 new-handler 函数，参考 R49 了解new-handler的行为。

2. **如果要求分配 0 byte 空间**
  C++规定，如果客户要求分配 0 byte 内存申请，就返回 1byte 空间申请，并返回该有效地址。

3. **理解 operator new 内部的无穷循环**
  operator new 有个 while(true) 循环，分配成功可以return，或由 new-handler 为 nullptr 时抛出 std::bad_alloc 异常。伪代码pseudocode 如下：
  ```c++
  void* operator new(std::size_t size) throw(std::bad_alloc){
    using namespace std;
    if (size == 0){
        size = 1;
    }
    while (true){
      尝试分配 size bytes;
      if (分配成功)
          return  target_pointer;
      new_handler globalHandler = set_new_handler(0);
      set_new_handler(globalHandler);//分配失败了
      if(globalHandler) (*globalHanler)();
      else throw std::bad_alloc();
    }
  }
  ```
4. **当基类的 operator new 被子类继承时**
  当基类被继承时，成员 operator new 也一起被继承了，要注意的是基类和子类的 size 通常是不一样的。推荐实现如下：
  ```c++
  class Base{
  public:
    static void* operator new(std::size_t size)throw (std::bad_alloc){
      if (size != sizeof(Base))
        return ::operator new(size);//子类走这里
    }
  };
  ```
  值得注意的是，**operator new[] 不能这样在基类中区分**。因为即使在Base类，也无法假定每个元素是 sizeof(Base)，通常还有额外内存空间来保存元素个数。

5. operator delete 的注意事项
  C++ 需要保证 “`删除NULL指针永远安全`”，所以必须兑现这个规则。针对null指针，就什么也不做，直接return。

### R52 写了placement-new 也要写placement-delete

placement-new，是指“`除size参数以外，接受一个额外参数参与构造`”的 特定 operator-new。

其中，“`接受一个指针指向对象该被构造之处`”是**最常使用**的 placement-new，即“`一个特定位置上的new`”，形式如下为：

```c++
//这个特殊的也是最常涉及的 placement-new 已被纳入C++标准程序库
void * operator new(std::size_t size,void* pMemory) noexcept;
```

先暂时考虑一个调用了placement-new的正常构造过程：

```c++
// 有这样一个placement-new,接收一个ostream来log分配时的相关信息
void* operator new(std::size_t size,std::ostream& logStream) throw (std::bad_alloc);
Widget* pw = new (std::cerr) Widget;//传入ostream
```
对于任何的new对象构造过程，至少可分为下述2个过程：
  1. operator new 分配对象需要的内存空间；
  2. 执行对应的构造函数
如果上述过程 1 成功了，过程 2 抛异常，已经申请的内存就需要及时回收避免memory-leak，运行期系统就会尝试寻找并调用“**额外参数个数和类型都与operator new 一致的operator delete**”，完成内存回收。

那么上述事实，就是 placement-new 和 placement-delete 需要**成对实现的理由**。

针对上例额外参数是 std::ostream 的operator new，operator delete，类声明形式如下:

```c++
class Widget{
public:
    static void* operator new(std::size_t size,std::ostream& logStream) throw (std::bad_alloc);
    //不抛异常时，最后对象析构时正常调用这个
    static void operator delete(void *pMemory) noexcept;
    //new抛异常时，调用这个 额外参数个数和类型都一致的 placement-delete
    static void operator delete(void *pMemory,std::ostream& logStream);
};
```
⚠️ **One More Thing** : C++ 在global 作用域提供以下形式的 operator new:

```c++
void * operator new(std::size_t size) throw (std::bad_alloc);
void * operator new(std::size_t size,void*) noexcept;
void * operator new(std::size_t size,const std::nothrow_t &) noexcept;//见条款49 new-Handler的行为
```
😎 所以，如果在class 内声明了上述 operator new， 则global作用域的 operator new 会被`名称遮掩`。

🤔 `解决办法`：在基类对global作用域的 ::operator new 进行封装调用，然后在子类中使用using 声明式破除名称遮掩。

## CH9 杂项讨论

### R53 不要轻易忽略编译器的警告
- **严肃对待编译器发出的警告信息**
  编译器发出的警告信息，经常会被忽略。No-Warning是值得追崇的，除非你对编译 warning 信息是充分了解并确信是无关紧要的。

  下面举一个较为常见的例子：
  ```c++
  class BaseWarn {
  public:
    virtual std::string GetWarnInfo() const {
        return "Base"; }
  };
  class DerivedWarn :public BaseWarn {
  public:
    virtual std::string GetWarnInfo()  {//缺了const
        return "Derived"; }
  };
  ```
  如上代码所示，没有成功实现虚函数重写，而是造成了“**名称遮掩**”。
  这样的错误较为隐蔽，有些编译器会给出警告信息，有些甚至连警告信息都没有（Ps：我自行测试了VS2017，没有warning信息）。
  `如果编译器给出了警告信息，请认真对待`。
- **不要过度依赖编译器的报警能力**
  还是上面的例子，不同编译器处理态度不同，警告信息甚至可能换个编译器就消失了。
### R54 熟悉包括TR1在内的标准程序库

**C++ Technical Report 1 (TR1)** 并非标准，而是一份草稿文件，对C++标准库的第一次扩展，它提出了对C++标准函式库的追加项目。

这份文件的目标在于「为扩充的C++标准函式库建立更为广泛的现实作品」。

> `我的理解`：200x 年发布的 C++ 称为 “C++0x”，持续修改中，所有修改将合并于 TR1，最终绝大部分都定版、收录发布于 **C++11**。

TR1详细叙述了许多新特性，都放在 `std::tr1` 命名空间内（以下简称 `tr1::` ），列举如下
 1. `智能指针`：tr1::shared_ptr 和 tr1::weak_ptr ，RAII 基础，不赘述；
 2. `tr1::function`：表示 可调用物callable entity，即任何函数或函数对象，只要签名一致即可；
 3. `tr1::bind`：对函数调用的封装，将函数和其参数绑定一起；
 4. `Hash tables`：采哈希表形式参与构成，名称以 unordered_ 开头的 set/multiset/map/multimap；
 5. `正则表达式`：头文件在 <regrex> ；
 6. `Tuple 元组(或叫变量组)`：不定长变量组，是 std::pair 的一种泛化；
 7. `tr1::array`: 和 C 语言数组一样，是个定长数组，包裹了 STL 用法；
 8. `tr1::mem_fn`: 传入一个函数指针（支持对成员函数取址）作为入参，构造一个函数对象，进而调用，类似地还有 mem_fn_ref;
 9. `tr1::reference_wrapper`：“封装引用为一个对象”，通常用于对引用进行封装然后装入标准容器(直接往容器塞引用是不行的)；
 10. `随机数生成工具`：random_device，可以直接生成或者使用不同的 随机数引擎 和 随机分布算法进行生成，头文件是 <random>；
 11. `数学特殊函数`：包括Laguerre多项式、Bessel 函数、完全椭圆积分等特殊数学函数，注意，这些 在 C++17 才引入C++标准，可参考cppreference: special math ，头文件在 <cmath>；
 12. `C99兼容扩充` ：C99标准是C语言的官方标准第二版，1999年发布，TR1对其进行了兼容；
 13. `Type traits 类型萃取`：template编程的精华之一，参考 Rule47:使用trait表现类型信息，头文件为 <type_traits>，功能十分丰富，可参考cppreference: type_traits；
 14. `tr1::result_of` ：可以对函数返回值做推断，得到返回值类型，头文件为 <type_traits> ，示例用法如下：
    ```c++
    // 假设有个函数 double calcDaySale(int);
    std::tr1::result_of<calcDaySale(int)>::type x = 3.14;//x就是double类型. C++11中直接 std::result_of
    ```
更详细的定版TR1信息可以参考[Effective-C++：TR1 information](https://aristeia.com/EC3E/TR1_info.html)。

### R55 让自己熟悉Boost
Boost是一个C++开发者集结的社群，也是个可自由下载的程序库集，网址是 `http://boost.org`。

**其特殊性**：和C++标准委员会有着独一无二的密切关系，且具有很深影响力；接纳程序库非常严谨，需要一次以上的同行专家评审。

Boost 程序库集可处理的场景有许多（且**囊括了TR1**的实现），可区分出数十个类别，并且还在持续增加，**列举一小部分**如下：
 - **字符串与文本处理**
 - **容器**
 - **函数对象与高级编程**
 - **泛型编程**：覆盖一大组 traits classes
 - **模板元编程**：覆盖一个针对编译器 assertions 而写的程序库，以及 Boost MPL程序库
 - **数学和数值**:包括有理数、八元数、四元数、公约数、多重运算、随机数等等
 - **正确性与测试性**
 - **数据结构**
 - **语言间的支持**：允许 C++ 和 Python 之间的无缝互联
 - **内存**：覆盖Pool程序库和智能指针等
 - **杂项**：包括 CRC 校验、日期和时间的处理、文件系统等内容

总的来说，Boost 是一个社群，也是个网站。致力于免费、源码开放、同行复审的 C++ 程序库开发，非常值得经常访问与学习。


ref:</br>
[1]. https://blog.csdn.net/cltcj/category_12098441.html</br>
[2]. https://kissingfire123.github.io/2022/05/17_effective-c-%e4%b9%8b%e9%98%85%e8%af%bb%e6%80%bb%e7%bb%93%e5%9b%9b/</br>


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/effective_cpp_part_four/  

