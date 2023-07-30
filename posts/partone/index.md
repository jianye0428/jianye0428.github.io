# Effective C++ (第3版) 精读总结 [1]


<!--more-->
# 序言

这本C++的经典之作，作者是大佬`Scott Meyers`👉[大佬主页](https://www.aristeia.com/books.html)，还写过其他几本影响深远的C++经典，例如`《Effective STL》`,`《More Effective C++》`,`《Effective Mordern C++》`,`《Overview of the New C++(C++11/14)》`等等。本人看的是中文版，侯捷老师翻译的，精读分析并实践推敲后，整理成博客记录下来。

> (Effective-C++总结系列分为四部分，本文为第一部分，涉及原书第1~2章，内容范围Rule01~12。为方便书写，Rule01简写为R01)。

{{< admonition Note "Effective-C++系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective C++(第3版)精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 Effective C++(第3版)精读总结(二)</br>
💡 Effective C++(第3版)精读总结(三)</br>
💡 Effective C++(第3版)精读总结(四)</br>
{{< /admonition >}}

## CH1.让自己习惯C++

### R01 视C++为一个语言联邦

如今的C++已经是个多重范式(multiparadigm)语言，同时支持面向过程形式、面向对象形式、函数形式、泛型形式、元编程形式。 要理解这么多特性，可以简单的归结为`四种次语言`(sublanguage)组成：

- **C语言：**`C++仍以C为基础`。C++是C的超集，区块 、语句、预处理、内置数据类型、数组、指针等全部来自于C语言；
  - 说到底 C++ 仍然以 C 为基础。区块、语句、预处理器、内置数据类型 、数组、指针等统统来自C，许多时候C++对问题的解决其实不过就是较高级的 C 解法，但当你C++内的 C 成分工作时，高效编程守则映照出 C 语言的局限：没有模板(template) ，没有异常(exceptions)，没有重载(overloading)……
- **Object-Oriented C++:** 面向对象特性。这部分也就是 C with classes 所诉求的：classes(包括构造函数和析构函数)，封装(encapsulation)、继承(inheritance)、多态(polymorhpism)、virtual函数(动态绑定)……等等，这一部分是面向对象设计之古典守则在C++ 上的直接实施。
- **Template C++:**C++的泛型(generic)编程的部分，也带来了黑魔法-模板元编程(TMP,Metaprogramming)；
- **STL：**STL(Standard Temlate Library)即标准模板库，它是template程序库。封装了各类容器(container)、配置器(allocator)、迭代器(iterator)、算法以及常用对象。

**总结:**
C++高效编程守则视状况而变化，取决于你使用C++的哪一部分

### R02 尽量以`const`,`enum`,`inline`替换`#define`

- **对于宏定义的常量，建议用const常量或者枚举enum替换**
  这样做的好处是方便调试，因为宏报错就是个常数值，没有符号表；并且宏不具有封装性(宏的作用域是在编译时是其定义之事)。
  ```c++
  // 举例：MAX_DATA_COUNT在预处理阶段就会被替换，编译器不会见到它，所以一旦有相关报错，给的是100这个值
  #define MAX_DATA_COUNT   100
  const  int MAX_DATA_COUNT = 100 ;//常量只有一份，宏会导致多份常量值
  class Buffer{
  public://...类其他部分省略
      static const double factor_ ;//static常量,类内声明
      static const int  times_ = 2;// int类型允许类内初始化,规范上还是建议拿到类外
  private:
      static const  int ArrLength = 5;
      int arr[ArrLength];
  };
  const double Buffer::factor_  = 0.1;//类外初始化,一般写在实现文件*.cpp,*.cc中
  ```
  如果编译器不允许声明时"in-class初值设定",如果是整形常量，可以让枚举值来替代，而且<font color=red>`枚举值不能被取地址`</font>。
- **对于宏定义的函数，建议用内联inline函数替换**
  宏函数没办法单行debug调试，而内联函数可以；
  宏的写法即使小心翼翼的加好了括号，也可能造成意想不到的<font color=red>`宏函数重复计算`</font>的问题。
  ```c++
  #define  GET_MAX(a,b)   ((a)>(b) ? (a) :(b))
  int a = 5, b = 0;
  GET_MAX(++a,b);    // a累加二次
  GET_MAX(++a,b+10); // a累加一次
  // 定义个inline函数就不会有这个问题,(a,b)作为函数入参就只会计算一次
  ```
  {{<admonition Note "By the way">}}
  上述情况，从纯C语言角度，想避免“宏函数重复计算”，其实还有个方法，就是使用GNU C 扩展的 typeof 或 GCC 的 `__auto_type` 关键字，详细可参考GCC官方文档页面。2者都适用于GCC和Clang，都不适用MSVC），示例如下：
  ```c
  #define  GET_MAX_ONCE(a,b) \
        ( {typeof(a) _a = (a);   \
           typeof(b) _b = (b);   \
           (_a) > (_b) ? (_a) : (_b); } )
  ```

  测试代码如下：
  ```C
  int a = 10,b = 20;
  int c = GET_MAX(++a, b++);
  std::cout << "a = " << a << ", b = "<< b << ", c = " << c << std::endl;
  a = 10 , b = 20;
  c = GET_MAX_ONCE(++a, b++);
  std::cout << "a = " << a << ", b = "<< b << ", c = " << c << std::endl;
  ```

  测试代码输出：
  ```shell
  a = 11, b = 22, c = 21
  a = 11, b = 21, c = 20
  ```
  > 🤔 `使用 __auto_type` 来取代时要赋初值，关键的 typeof 那行用法改为`__auto_type _a = (a);` 。
  >__auto_type 比 typeof 的优势之处在于面对变长数组(VLA)，只解析1次；以及面对嵌套宏定义时也是只严格解析一次。

  {{</admonition>}}


### R03 尽可能用const

说起const，先复习个面试高频题😁
```c++
//类型char在哪里没关系，关键看const和*的相对位置：
  //const在*左边，指针所指物为常量;
  //const在*右边，指针为常量；
const char* p; // 指针p所指的字符串为常量，但是p可以修改
char* const p; // 指针p是常量，指向的字符串可修改
```
  - const修饰函数返回值时的防御性
    const修饰函数的返回值，可以避免一些错误，如下：
    ```c++
    const Rational operator* (const Rational& lhs,const Rational& rhs);
    //存在Rational a,b,c;
    if(a*b = c) //例如手误 "=="打成了"="，编译器会直接报错
    ```
  - const成员函数的限制
    const成员函数有2个好处：(1). 明确理解函数是否修改对象内容；(2). 使“操作const对象”成为可能。
    可以通过const特性让对象自动调用正确的版本：
    ```c++
    class TextBlock{//...类其他部分省略
      const char& operator[](std::size_t index) const{ return text[index];}
      char& operator[](std::size_t index) { return text[index];}
    private:
        std::string text;
    }
    void print(const TextBlock& ctb){
        std::cout << ctb[0] ;//根据const特性，调用 const TextBlock::operator[]
    }
    ```
    思考：const成员函数不修改成员对象，那么，如果成员对象为指针`char *p`，仅修改`p`指向的内容，那它还是`const`成员函数吗？
    实测：const成员函数是允许`p[2]= 'x'`这一操作的，但是不允许p++；
    - `bitwise-constness`(又称为physical constness)理念认为不是，不能更改任何对象内的任何一个bit。这种说法也有纰漏，const函数返回一个引用就失控了,外部可改；
    - `logical-constness`理念则允许const成员函数修改成员变量的bits，但只有在客户端侦测不出的情况下;
    - `const成员函数`如果一定要修改成员变量，成员变量使用 `mutable` 修饰即可.
  - const与non-const的成员函数实现完全相同时
  这种情况，如何去除代码冗余是个问题。不要封装出一个private函数然后一起调用，多了层调用。
  正确做法👉 `使用转型,让non-const调用const成员函数`，如下为示例：
  ```c++
  class Rawdata {
  public:
      Rawdata(char *src):p_(src),length_(strlen(src)) {}
      const int& GetLength()const{
          p_[1] = 'a';//为求简便,不作检查了。p_内容修改编译器是允许的.
          return length_;
      }
      int GetLength() {
          return const_cast<int&>(
              (static_cast<const Rawdata&>(*this)).GetLength());
      }
  private:
      char *p_ = nullptr;
      int length_ = 0;
  };
  ```

### R04 确定对象使用前先初始化

分清"赋值"和"初始化"，在类的构造函数体内使用等号"="赋值并非"初始化”。成员变量的初始化是在构造函数的<u>成员初始化列表</u>实现，效率更高。

`C++初始化的次序:`
1. Base class总是早于Derived Class被初始化；
2. Class内的成员变量总是以声明次序被初始化；
   举例: 下图中的代码初始化顺序为声明顺序"age_$\rightarrow$name_$\rightarrow$gender_ $\rightarrow$isVip_"
   ```c++
    class Customer {
    public: //这里只是为了验证，如果是实际工程代码，建议初值列表尽量和声明顺序保持一致
        Customer(std::string gender, uint8_t age, std::string name, bool isVip = false)
            :isVip_(isVip), gender_(gender), name_(name), age_(age) {}
    private:
        uint8_t age_;
        std::string name_;
        std::string gender_;
        bool isVip_;
    };
   ```
   {{<admonition Note "By the way">}}
    💡：任何一个成员变量a的初始化流程：类内声明赋初值(C++11)->构造函数初始化列表->构造函数体内赋值.
   {{</admonition>}}
3. 函数体外static变量称为`non-local static`变量，这种变量可以在各自的编译单元正常工作，但C++无法保证初始化次序，当编译单元之间需要共享变量时，而该变量依赖non-local static，就可能会出问题。
**解决办法**是将这样的变量放回函数体内，成为local static，因为C++确保在函数被调用时一定会初始化这个static变量。

## CH2.构造/析构/赋值运算

### R05 了解C++默认编写并调用哪些函数

编译器可以暗自为Class创建`default构造函数`，`copy构造函数(复制构造)`，`copy assigment(赋值构造)操作符`，以及`default析构函数`。
```c++
class EmptyClass{};
```
等价于:
```c++
class EmptyClass {
public:
    EmptyClass() {}        // default构造函数
    EmptyClass(const EmptyClass& rhs) {}//copy构造函数
    ~EmptyClass() {}//default析构函数，注意是non-virtual
    EmptyClass& operator=(const EmptyClass& rhs) {}//copy assignment操作符
};
```
- 如果一个非空Class自行声明了构造函数，编译器就不再为它创建default构造函数;
- 如果一个Class内有引用变量或const变量，编译器不会为其生成copy-assignment函数，需要自己实现.

  ```c++
  class NameObject {
  public:
      NameObject(std::string& name, const int value) :nameValue_(name), objectVal_(value) {}
  private:
      std::string& nameValue_;
      const int objectVal_;
  };//考虑以下应用代码
  std::string name("Mike"), name2("Nicky");
  NameObject nObj(name,22), nObj2(name2,33);
  nObj2 = nObj;//这一句会导致编译失败，因为引用不能指向2个对象，且const不可改
  ```

### R06 不想编译器的自动生成的函数,明确拒绝

某些场景，类对象本身是认为独一无二的，比如描述人物性格的类Personality(可能不恰当)，不同的人性格不同，肯定是不希望能复制/赋值的。
如果不想编译器自动生成那几个函数，比如不想要复制构造或赋值构造，就明确在代码中禁止，也防止外部用户使用。

  - 技巧1：可以声明为private函数但是不给出实现，让编译器报错；
  - 技巧2：可以设立基类Base，让复制构造和赋值构造都为private函数，让目标类继承于Base类。

{{<admonition Note "By the way">}}
C++11针对此类情况，给予了delete关键字一个新功能，在成员函数后面新增"=delete"，即可显式地拒绝这个函数的生成和调用.举例如下:
```c++
class A {
public:
    A(const A&) = delete; // 拷贝构造
    A& operator=(const A&) = delete; // 拷贝赋值
};
```
{{</admonition>}}

### R07 为多态基类声明`virtual`析构函数

- 针对`Base* pBase= new DerivedObj()`，释放pBase时，如果Base基类的析构函数`None-Virtual`，则会导致Derived的析构函数不会被调用，造成`内存不完全释放`，即内存泄漏；
- Non-Virtual的Class不要做基类；
- 如果不做基类，就不要声明析构函数为virtual函数，避免虚表为其分配vptr造成浪费；
- 如果想要抽象类，又暂时没有合适接口，可以让先虚析构成为纯虚函数，例如
  ```c++
  class RawData{
  public:
      virtual ~RawData() = 0;
  };
  // 这里特殊的是，需要为这个纯虚函数提供定义
  ~RawData::RawData(){}
  ```
### R08 别让异常逃离析构函数

- 析构函数最好不要吐出异常</br>
对于某个Widget类，如果析构函数抛出异常，那么`vector<widget>`析构时可能连续抛出多个异常以至于无法处理，直接导致提前程序结束或者未定义行为。

- 如果有某个函数可能会抛出异常，并需要对异常做出反应，应该提供一个非析构的函数来处理</br>
注意，在析构中抛异常并吞下(catch后go-on-execute)，会掩盖错误，也不是个好办法;抛异常后catch住并std::abort()反而可以提前终结『未定义行为』。

### R09 绝不在构造和析构过程中调用`virtual`函数

- 在Derived-Class的Base-Class构造期间，对象的类型是Base-Class，而非Derived，即使`dynamic_cast`也是这样认为的，因为此时virtual函数不会下降到Derived-Class的阶层；换句话说==> “`在Base-Class构造期间，virtual函数不是virtual函数`”；
- 在析构的时候也是类似道理，也不要在析构函数中调用virtual函数，间接调用也不行(比如non-Virtual->virtual)；
- 那么替代方案是？</br>
  如果有这样一种情况，希望在子类构造时能调用父类的foo函数(下方logTransaction函数)，那么将foo改为non-Virtual函数，并且子类构造时调用父类构造函数，有参数时一并传过去；
```c++
class Transaction {
public:
    explict Transaction(const std::string& info,int id):id_(id) { logTransaction(info); }
    void logTransaction(const std::string& info) {
        std::cout << info.c_str(); }//non-Virtual函数
private:
    int id_;
};

class BuyTransaction :public Transaction {
public:  //将log信息传递给 基类Transaction构造函数； 并初始化了基类成员
    BuyTransaction(const std::string& info, int id)
        :Transaction(info,id) {/**/}
};
```
{{<admonition Note "By the way">}}
上述代码中，如果有几个Base构造函数，就得写几个Derived构造函数传参。为了简化这一步骤， 在现代C++11，推出了继承构造函数(Inheriting Constructor)，子类可以一个都不用写，直接写一句using声明即可，使用`using BaseClass::BaseClass`的形式，如下👇
```c++
class BuyTransaction :public Transaction {
public:
  using Transaction::Transaction;
  //其他
};
```
{{</admonition>}}


### R10 令`operator=`返回一个reference to *this

- 关于赋值操作符`operator=`，主流做法是:
  ```c++
  class Widget {
  public:
      Widget& operator=(const Widget &rhs) {
        //... do some thing
          return *this;
      }
  };
  ```
  上述做法不局限于operator=，还有`operator += ,-=,*=,/=`等其他赋值运算符。</br>
  该主流协议/做法不是强制性的，只是个建议，因为这样可以允许**连续赋值**($x=y=z$的形式)。</br>
  该协议被内置类型，以及STL的类型(如string,vector,complex,shared_ptr)共同遵守。</br>

## R11 在`operator=`中处理"自我赋值"

一个对象赋值给自己，这种看起来有点傻的做法，有时候会比较难以发觉。比如：`a[i] = a[j]`，如果 $i$ 和 $j$ 相等，就是“自我赋值”的经典场景了。
所以，我们需要一个尽量完备的赋值操作符函数🤔。
考虑如下类`MapWidget`，内有数据裸指针`BitMap *pb_`，在赋值时同时考虑**异常安全**和 **“自我赋值”** 安全(认同测试保障自我赋值安全)，是一个相对不错的实现。但是，认同测试会降低运行效率，根据实际工程实践情况(自我赋值概率极低 )可以酌情去掉。

```c++
class BitMap {/*省略类声明&实现*/};
class MapWidget {
private:
    BitMap * pb_ = nullptr;
public:
    explicit MapWidget(BitMap *pb) :pb_(pb) {}
    MapWidget& operator=(const MapWidget& rhs) {
        if (this == &rhs) return *this; //这一句为认同测试(identity test)
        BitMap *pOrig = pb_;//不要一上来就delete，而是保存this->pb_
        pb_ = new BitMap(*rhs.pb_);//因为这一句可能抛异常
        delete pOrig;
        return *this;
    }
};
```
- Copy and Swap技术 上文中保障**异常安全**和 **“自我赋值”**安全的技术手段，另外一个办法，就是Copy and Swap技术。这个技术的关键在于“修改对象数据的副本，然后在一个不抛异常的函数中将修改的数据和原件置换 ”。
  ```c++
  void MapWidget::Swap(MapWidget& rhs){std::swap(rhs.pb_,pb_);}
  MapWidget& MapWidget::operator=(const MapWidget& rhs) {
      MapWidget temp(rhs);
      Swap(temp);
      return *this;
  }
  ```

### R12 复制对象时别忘记其每一个成分

- 编写类的Copying函数时需要做到2点
这里的Copying函数是指有copy属性的特殊函数==> copy构造函数和copy赋值操作符。

  1. 复制所有Local成员变量
     如果成员复制的时候有遗漏，编译器并不会有怨言，这就埋下了隐患。如果新增了成员，要对应修改Copying函数。
  2. 调用所有Base-Class内部的适当的Copy函数
     具体代码的推荐实现 如下👇：
  ```c++
  class Customer {
  public:
      Customer(std::string name, float money):name_(name),money_(money) {}
  private:
      std::string name_;
      float money_;
  };

  class VipCustomer :public Customer {
  public:  //注意：派生类要复制基类那部分的成员变量
      using Customer::Customer; //参考R09
      VipCustomer(const VipCustomer& rhs)
          :priority_(rhs.priority_),Customer(rhs){}
      VipCustomer& operator=(const VipCustomer& rhs){
          Customer::operator=(rhs);
          priority_ = rhs.priority_;
          return *this;
      }
      void setPriority(int priority) { priority_ = priority; };
  private:
      int priority_;//子类独有成员可以单独set函数赋值，或者构造函数初始化列表赋值
  };
  ```

- 不要用一个Copying函数去实现另一个
令copy-assignment操作符调用copy构造函数是不合理的；反之，后者调用前者也是无意义的。 如果2者有大量的代码是相同的，可以剥离一个类内private类型的`init`函数出来，提供给上述2者调用。

ref:</br>
[1]. https://blog.csdn.net/cltcj/category_12098441.html</br>
[2]. https://kissingfire123.github.io/2021/12/06_effective-c-%e4%b9%8b%e5%ad%a6%e4%b9%a0%e6%80%bb%e7%bb%93%e4%b8%80/</br>

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partone/  

