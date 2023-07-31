# Effective C++ (第3版) 精读总结 [2]


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
💡 Effective C++(第3版)精读总结(三)</br>
💡 Effective C++(第3版)精读总结(四)</br>
{{< /admonition >}}

由于原书在C++11之前写成，有些现代C++特性不会提及，所以会根据本人`开发经验`新增一些个人感悟👉`By the way`环节。

## CH3.资源管理

### R13 以对象管理资源

- 为防止资源泄露，尽量使用`RAII`对象(资源获取即初始化，Resource Acquisition Is Initialization)
  如果用老式的new-delete组合手动管理资源，可能会遇到这样的场景，处理代码中有多处return/break之类的分支语句，每处都手动添加delete回收资源难免会有遗漏风险。
  1. auto_ptr
    本书提及的智能指针对象`std::auto_ptr`可以在资源A初始化化时接管对象A，对象脱离作用域，析构时释放接管的A。
    为避免重复删除资源，`std::auto_ptr特性:` 通过copy构造函数或copy-assignment操作符，操作它们，它们自身会失去资源所有权，变成NULL ！

  2. shared_ptr
    为解决上述问题，后来推出了引用计数管理资源，即`RCSP(Reference-counting smart poiner)`，std::shared_ptr 。(书中当时还是tr1::shared_ptr)特点是shared_ptr在使用copy构造函数或copy-assignment操作符时，不会失去资源所有权，而是自身引用计数加1。
  {{<admonition Note "Note">}}
  - auto_ptr目前已经被弃用，转而使用 std::unique_ptr来指涉独占资源的智能指针，不可被复制和赋值。

  - shared_ptr存在的问题是环形循环应用，互相持有对方，则无法释放，针对这一问题，需要引入std::weak_ptr来破局。

  - 这些智能指针的头文件支持：#include <memory>
  {{</admonition>}}

- 常用的RAII方式存在的问题
上述的shared_ptr是**不支持数组资源的释放**的，在析构时默认调用delete，而不是delete[] 。如果要能释放，需要手动传入析构函数。
  ```c++
  std::shared_ptr<Investment> instSpMan(new Investment[5], [](Investment* p) {delete[] p; }); // 使用lamda函数，还可以传入普通函数指针
  std::shared_ptr<Investment[]> instSp(new Investment[5]);// shared_ptr重载了[]
  std::unique_ptr<int[]> intUp(new int[5]); // unique_ptr重载了[]
  ```
  PS: 通常情况下，如果能用vector代替的场景，就不需要这样使用对象数组。

### R14 在资源管理类中小心coping行为

并非所有资源都是堆上管理(heap-based)，所以也有智能指针不适合的场景，这时需要自己实现一个资源管理类。

比如有个类CMutex ，只有2个函数lock和unlock，可以新建一个管理类CLock，来管理Mutex类，代码如下：
```c++
class CMutex {/*省略类实现*/};
void lock(CMutex *mtx) { std::cout << "has lock...\n";/*other operation...*/ }
void unlock(CMutex *mtx) { std::cout << "release lock...\n";/*other operation...*/ }

class CLock {
public:
    explicit CLock(CMutex *mtx) :mtx_(mtx) { lock(mtx_); }
    ~CLock() { unlock(mtx_); }
private: CMutex *mtx_;
};
```
如果涉及CLock的coping相关函数，那么有以下几种选择：
  - 禁止复制
  参考本书Part1-Rule06，禁止coping函数的生成。
  - 对底层资源使用“引用计数法”
  以shared_ptr<CMutex>代替裸指针，并且传入unlock函数作为该指针的“删除器”。
  - 复制底部资源
  即“深度拷贝”，复制资源管理对象时，同时复制其包裹的资源。
  - 转移底部资源所有权
  即浅复制，不拷贝包裹的资源，而是转移所有权，和前文提到的auto_ptr非常契合。

### R15 在资源管理类中提供对原始资源的访问

智能指针`auto_ptr`, `unique_ptr`, `shared_ptr`都提供了接口访问原始资源，方法名称为`get()`，对类A资源得到裸指针`A*`。

智能指针也重载了操作符`operator ->` 和 `operator *` ，访问类成员函数都可以像普通指针那样使用。

如果自行设计资源管理类，也要像上述的智能指针那样，做到能够轻松访问原始资源。

某些情况，也可以不使用get成员函数**显式转换**，转而使用**隐式类型转换**，方便客户调用:

```c++
class Font{
public:
     // 隐式转换
  operator FontHandle() const{ return f;}
     // 显式转换
  FontHandle get() const { return f;}
private:
    FontHandle f;
}
```

### R16 成对使用new和delete时要采取相同形式

先看一段错误代码：
```c++
std::string *stringArray = new std::string[100];
delete stringArray;// undefined behavior,未定义行为；有可能只删除了第一个元素
```

{{<admonition Note "注意">}}
- new 和 delete对应
- new[] 和 delete[]对应
不可遗漏或者交叉错配！
{{</admonition>}}

简单探究 `delete[]`的原理：实际上，在编译器的实现中，对象数组的起始内存会存放“数组长度”这一变量，以便告知`delete[]`应该调用多少次析构，删除多少资源。

👉 谨慎对数组使用typedef，容易产生new-delete的匹配误解，示例如下：

```c++
typedef std::string AddressLines[4];
std::string *pal = new AddressLines;//注意：这里其实就是 new string[4]
delete pal;     // 行为未定义！！
delete [] pal;  // 正解！
// 这里建议的替代方法：vector<string>
```

### R17 以独立语句将newed对象置入智能指针

**不要将申请资源的new语句直接当作函数入参**，而是应该先以单独语句申请后传入。

考虑下述问题代码：
```c++
int priority();
void processWidget(std::shared_ptr<Widget> pw,int pri);
//函数调用
processWidget(std::shared_ptr<Widget>(new Widget),priority());
```
上述代码第4行，在processWidget函数体执行之前，至少有3个步骤(并非严格次序)：

1. 执行"new Widget"
2. 调用std::shared_ptr的构造函数
3. 调用priority函数

C++编译器能保证1->2的次序(2需要1做入参)，但是无法保证3的次序不在1，2之间，如果步骤3抛出异常，将会直接导致内存泄漏。正确示例如下：
```c++
std::shared_ptr<Widget>  pw(new Widget);
processWidget(pw,priority());
```

## CH4.设计与声明

### R18 让接口容易被正确使用，不易被误用

要想设计一个不容易误用的接口，**就要先考虑使用者可能犯什么错误**。

   - `避免“接口误用”，可以导入新类型进行限制`
      比如设计一个Date类，参数传递就可能出错，如下所示：
      ```c++
      class Date{
      public:
          Date(int month,int day,int year);
      };
      ```
      实际使用时，使用者可能写出`Date date(30,4,2021)`;这样的错误代码，如何防范呢？
      方法之一：封装出Month,Day,Year这3个Struct/Class，作为参数入参，并**添加月份限制**，代码：
      ```c++
      class Month{
      public:
          static Month Jan() { return Month(1);} // ...省略其他11个月份
      private:
          int val;
          explicit Month(int m):val(m){} //可被class-static调用，不能被外部调用
      };
      struct Day {explicit Day(int d) :day(d){}    int day;
      };  //省略 struct Year定义，和Day类似
      class Date{
      public:
          Date(const Month& month,const Day& day,const Year& year){}
      };
      ```
      最终客户代码使用的情况如下：
      ```c++
      Date date(Month::Jan(),Day(13),Year(2022));
      ```
      {{<admonition Note "By the way">}}
      C++11 支持对枚举的强化，即类型安全的`enum class`，它不能隐式地转换为整数；也无法与整数数值做比较。此处可以考虑定义一个MonthEm来代替Month：
      ```c++
      enum  class MonthEm {
      Jan = 1,
      Feb = 2, //... 省略其它的月份定义
      };
      // 修改Date的构造函数，MonthEm来代替Month
      Date(const MonthEm& monthEm, const Day& day, const Year& year) {}
      // 构造对象
      Date date2(MonthEm::Feb, Day(23), Year(2021));
      ```
      {{</admonition>}}
   - `限制类型内什么事是可不可做的`
      常见操作是加上const限制，比如“以const 修饰operator * 的返回类型”可以阻止这个错误：
      ```c++
      //本意是想做比较，写成了赋值，但是赋给const，报错！
      if (a * b = c)
      ```
   - `智能指针传入删除器可避免"Cross-dll-problem"`
      std::shared_ptr管理资源时，传入删除器，可避免在A-Dll中new，结果在另一个B-Dll中delete的情况，这会导致Runtime-Error。

### R19 设计Class犹如设计Type

设计一个优秀的Class并不容易，很多C++书籍都像本书一样提到“Check-Table”，设计Class/Type前，问自己一些重要问题：
- 新type的对象应该如何被创建和销毁？
- 对象的初始化和赋值该有什么差别？
- 新type的对象如果值传递(pass-by-value)，意味着什么？
  copy构造函数用来定义一个type的值传递具体实现。
- 什么是新type的“合法值”？
  想清楚约束条件，特别是构造函数、赋值操作符以及**setter**函数，涉及的错误检查与非法值拦截。
- 新type需要配合某个继承图谱吗？
  如果继承自某Base类，那么就会收到virtual,non-virtual函数的约束；如果要做后续类的基类，则该type就要注意**虚析构**的设计。
- 新type涉及什么样的转换？
  与其他type之间如有转换需求，则需要自行实现相关函数，可参考`Rule15`。
- 新type需要怎样的操作符和函数？
  确定哪些是member函数，哪些不是。参考`Rule23`，`Rule24`，`Rule26`。
- 是否有需要立即驳回的标准函数？
  比如**是否禁止**copy构造，copy-assignment操作符等函数，可以声明为private；或者使用C++新特性"=delete"。参考`Rule6`。
- 新type成员的访问属性控制？
  成员变量都应为private，考虑其他member函数该为private/public/protected。以及启用友元friend的考虑。
- 什么是新type的“未声明接口”?
  参考`Rule29`。
- 新type有多么一般化？
  如果是一般化的问题处理，该考虑是否该定义Class-Template。
- 真的需要一个新type吗？

### R20 以pass-by-reference-to-const代替pass-by-value

- 尽量以pass-by-reference-to-const代替pass-by-value
  前者通常更**高效**，并可**避免“对象切割”问题**。
  如果对象入参以值传递，就会在入参时创建临时对象，函数完成后临时对象析构，涉及构造函数和析构函数的调用，这些都可能是不小的开销！
  `如果是以const reference形式，则不会有任何构造/析构的开销`。const的作用是让使用者放心，不会改变入参的值。
  “对象切割"问题：
    如果是值传递，Derived-Obj传递给Base-Param，会丢失Derived独有的特性，只保留Base的那部分。
- 对于C++内置类型，值传递往往更高效
  从C++编译器的底层实现角度来看，references引用往往是以指针的形式实现。所以如果是内置数据类型，比如int类型 ，直接值传递反而效率更高。
  > 此规则还适用于STL迭代器和函数对象。

### R21 必须返回对象时，不要随意返回reference

**不要返回指针或引用指向以下对象**:
- 局部栈对象(local stack)
  函数内部新建的栈变量对象，出了函数就面临消亡，仍持有它的引用/指针，是典型的未定义行为。
- 堆分配对象(heap allocated)
  内部new，然后return出去使用，一来会增加外部delete的负担，二来可能delete的机会都没有。代码如下：
  ```c++
  const Rational& operator*(const Rational& lhs, const Rational& rhs){
    Rational *result = new Rational(lhs.n * rhs.n,lhs.d * rhs.d);
    return *result;
  }
  // 如下使用连乘，则没有delete的机会
  Rational w,x,y,z;
  w = x * y * z; // 内存泄漏！！！
  ```
  上述代码返回的是引用，返回指针也是一样的负作用。
- 局部static对象(local static)
  函数内部的static对象，只初始化一次，且只有一份，有记忆功能，可以被更新。那么下述代码就会出现问题：
  ```c++
  const Rational& operator*(const Rational& lhs,const Rational& rhs){
    static Rational result;
  	result = Rational(lhs.n * rhs.n,lhs.d * rhs.d)
    return result; //在调用者看来，因为是引用，永远只看到独一份的result的“最新值 ”
  }
  // 下方的比较永远都是true，永远走if分支  ！！
  Rational a,b,c,d;
  if((a*b) == (c*d)){
    //乘积相等，走if分支
  }else{
    //乘积不相等，走else分支
  }
  ```
{{<admonition Note "Tips">}}
> 那是不是永远不能以reference作为返回值呢 ？
> 当然不是，*this可以返回其引用，参考Rule10；或者类成员作为返回值时，可以返回其引用，但最好是const。
{{</admonition>}}

### R22 将成员变量声明为private

- 成员变量为private获得的好处

  1. 客户访问数据的一致性。都是用getter函数（需要加括号），而不是混用函数获取和直接访问；
  2. 细化成员变量的访问权限。根据是否有getter、setter函数可以精准控制read/write/none这样的细化。
  3. 保持实现弹性。比如获取某系统指标，可以在getter函数中修改各种灵活的策略和算法，而调用者感知不到。
  4. 封装性。封装性是指☞ `越多函数可以访问该数据，封装性越低`。
    客户调用端感知不到class内部的实现，这些确实也是不必暴露的。并且，如果是要修改某个变量，使用setter函数也更方便调试，不至于被随意修改。

- protected和public差不多，都不具备封装性
  如果某个public/protected变量权限被取消（比如改为private），将会导致大面积的编译错误，能访问的变成unaccessible，需要对应修改许多代码。

### R23 宁可用non-member,non-friend代替member函数

- 实际上，non-member/non-friend函数具有更大的封装性
  考虑下述一个类WebBrowser，member函数和non-member函数，实际上，**member函数**WebBrowser::clearEverything具有其他private数据的访问权限，**封装性是更差的**！
  ```c++
  class WebBrowser {
  public:
      void clearCache() {/*to clear cache*/}
      void clearHistory() {/*to clear history*/}
      void removeCookies() {/*to clear cookies*/}
      void clearEverything() {
          clearCache();
          clearHistory();
          removeCookies();}
  };
  void clearBrowser(WebBrowser &wb) {
      wb.clearCache();
      wb.clearHistory();
      wb.removeCookies();
  }
  ```
- 工程结构安排
  对于上述WebBrowser的例子，non-member函数放哪里呢，除了可以是其他class的member函数，更适用自然的答案是用`namespace包裹起来`。代码如下：
  ```c++
  namespace WebBrowserStuff{
    class WebBrowser {};
    void clearBrowser(WebBrowser &wb);
  }
  ```
  注意：`namespace可以跨越多个源码文件，而class不能`。
  **通常的组织方式**：多个.h头文件都使用同一个namespace，管理不同的功能模块。除了让模块更清晰，还能降低不必要的编译依赖关系。
- 对于private数据，member和friend函数具有相同的访问权限
  基于此，这里讨论的封装性不在于member与否，而是member和non-member ，non-friend函数之间。

### R24 若所有参数皆需类型转换，请采用non-member

标题需明晰：所有参数，是指**包含了被this指针所指向的那个隐含参数**（non-explicit构造函数调用隐式类型转换时）。

**应对具体场景**：构造函数允许隐式类型转换，且需要这个构造好的对象立即调用某member函数foo(比如operator函数)，此时这个foo函数应该改为non-member的foo函数。

考虑一个有理数类Rational，允许隐式类型转换构造（即non-explicit），如果`operator*`是member函数则会出现问题。Rational类实现代码示例如下：

```c++
class Rational {
public:
    Rational(int numerator = 0, int denominator = 1)
        :numerator_(numerator), denominator_(denominator) {}
    const Rational operator* (const Rational& rhs)const {
        return Rational(numerator_*rhs.numerator_, denominator_*rhs.denominator_);
    }
    int numerator()const  { return numerator_; }
    int denominator()const { return denominator_; }
private:
    int numerator_ = 0;
    int denominator_ = 1;
};
```
上述代码存在一个问题，就是不能混合运算，举例如下：
```c++
Rational rat(2, 5);
Rational rat3Tm = rat * 3;   //能成功调用member-operator*
Rational rat3Tm2 = 3 * rat;  //编译报错！！！ 本节的场景就在于此.
```

如果要让这种混合运算无障碍，把`operator*`函数改成non-member即可，注意要**删掉原来member内的`operator*`**，不能同时存在这2个operator*。
```c++
class Rational {
    //... 注意，要删除类内的member函数operator*
};
const Rational operator* (const Rational& lhs, const Rational& rhs) {
        return Rational(lhs.numerator() * rhs.numerator(), lhs.denominator() * rhs.denominator());
}
```

### R25 考虑写一个不抛异常的swap函数

- 当 std::swap对某个类型效率不高时，提供一个swap成员函数，且该swap不抛异常
  先看一下效率不高的std::swap实现：
  ```c++
  namespace std {
    template<typename T>  //std::swap的典型实现
    void swap(T& a,T& b) { //置换 a 和 b 的值
      T temp(a);   //若T是class，此处调用copy构造函数
      a = b;       //若T是class，此处调用copy assignment操作符
      b = temp;
	  }
  }
  ```
  考虑这样的使用场景：需要swap一个"pimpl手法”(pointer to implementation)实现的类，这种传统的std::swap必然是效率不高(深复制impl类的数据耗时较长)。
  ```c++
  class WidgetSwImpl {
  private:
      int a, b, c;
      std::vector<double> v; //很大的vector,意味着复制时间很长
  };
  class WidgetSw {
  public:
      WidgetSw(WidgetSwImpl* pImpl) :pImpl_(pImpl) {}
      WidgetSw(const WidgetSw& rhs) {/*其他copy&initial操作*/}
      WidgetSw& operator=(const WidgetSw& rhs) {
          *pImpl_ = *(rhs.pImpl_);
      }
      void swap(WidgetSw& other) { //public-member函数swap
          using std::swap;   //令std::swap在此函数可用
          swap(pImpl_,other.pImpl_);
      }
  private:
      WidgetSwImpl* pImpl_ = nullptr;
  };
  ```
  调用时代码如下：
  ```c++
  WidgetSw wSw1(new WidgetSwImpl);
  WidgetSw wSw2(new WidgetSwImpl);
  wSw1.swap(wSw2); //成功运行
  ```
- 如果提供了member swap，需提供一个non-member的swap调用前者
  注意，其实上述swap成员函数使用起来不算直观和统一，可以全特化std::swap让使用更直观方便，代码如下：
  ```c++
  namespace std {
    template<>
    void swap<WidgetSw>(WidgetSw& a, WidgetSw& b) {
        a.swap(b);
    }
  }
  // 客户端应用代码可以如下调用
  // 注意，不应该，也没必要加 std::，会限制编译器只使用std命名空间的swap
  swap(wSw1,wSw2);//编译器会优先挑中全特化版本的std::swap
  ```
- 如果是Class-Template，则不要特化std命名空间的swap，而是移到其它命名空间
  ```c++
  namespace WidgetStuff{
    template<typename T>
    class WidgetSw{ /*省略类定义*/} // 类内含有swap成员函数
    template<typename T>
    void swap(WidgetSw<T>& a, WidgetSw<T>& b) {
      a.swap(b);
    }
  }
  ```
  {{<admonition Note "Notice">}}
  - std命名空间的函数不要尝试重载，也不要增加任何新东西
  - C++只允许对`Class-Template`直接偏特化，不能对`Function-Template`偏特化(函数模板可以通过重载间接实现“偏特化”的作用)
  - **成员版swap绝对不要抛出异常**，非成员swap可以(因为成员swap的一个最好应用就是提供异常安全性)
  {{</admonition>}}

ref:</br>
[1]. https://blog.csdn.net/cltcj/category_12098441.html</br>
[2]. https://kissingfire123.github.io/2022/01/11_effective-c-%e4%b9%8b%e5%ad%a6%e4%b9%a0%e6%80%bb%e7%bb%93%e4%ba%8c/</br>


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/parttwo/  

