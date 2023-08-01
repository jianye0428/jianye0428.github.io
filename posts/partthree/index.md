# Effective C++ (第3版) 精读总结 [3]


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

## CH5. 实现

### R26 尽可能延后变量定义式的出现时间
- 尽可能延后变量定义式的出现，可增加程序清晰度和效率
  1. 定义后，在使用前就遭遇抛异常
    这种情况，如果是定义了对象`ObjectA a`，便白白地浪费了对象a的构造和析构成本。
  2. 不只是延后变量定义到使用时，而是尽量延后到能给它初值时
    ```c++
    std::string encryptPassword(const std::string& password){
      if(password.length() < 8){
        throw std::logic_error("Password is too short");
      }// 考虑1：在异常之后定义变量
      std::string encrypted(password);//考虑2：定义延后至变量能赋初值的时机
      encrypt(encrypted);
      return encrypted;
    }
    ```
  3. 思考变量定义**是否**该在循环内
    - 方法A：定义于循环外
      ```c++
      Widget w;
      for(int i = 0 ; i< n; ++i) {
          w = foo(i);
         // other...
      }
      ```
    - 方法B：定义于循环内
      ```c++
      for (int i = 0; i < n; ++i) {
        Widget w(foo(i));
        // other...
      }
      ```
**究竟是A还是B方法好**，取决于 `一个赋值成本`和`一组构造+析构`成本，这2者之间，如果是赋值成本低，那么A更好，否则B更好。

### R27 尽量少做转型动作
通常的转型是可能会这样写：函数风格的`int(expression)`或者C风格的(int)`expression`，这都被成为“旧式转型”。

  - 在C++中，有4种新式转型操作符：
    1. **const_cast (expression)**
      作用是移除变量的常量性(cast away the constness)，是唯一有此能力的操作符。
    2. **dynamic_cast(expression)**
      作用是“安全向下转型”(safe downcasting)，决定某个对象是否属于某继承体系。耗费重大运行成本（原因：需要查询RTTI信息，而且不同编译器实现的方法和效率有所不同）。
    3. **reinterpret_cast(expression)**
      执行低级转型，实际结果取决于编译器，移植性差。（比如int* 转为int）要清楚自己在做什么，慎用。
    4. **static_cast(expression)**
      强迫隐式类型转换，代替C风格的"旧式转换"。也可以给变量加上const特性。

    新式转换的好处：很容易在代码找到“类型系统在何处转变或破坏”；对const特性的严控，让类型系统更健壮。

  - 派生类里直接调用基类成员函数时，不要用转型
    ```C++
    class SpecialWindow:public Window{
    public:
      virtual void onResize(){
        Window::onResize();//不要使用 static_cast<Window>(*this).onResize();
      }
    };
    ```
  - 关于dynamic_cast需要注意的
    1. 如何替代和避免
      一般是持有一个`Base *pBase`，但是指向的是DerivedObj，于是转型为pDerived
      - 可以这样修改：
      修改设计，窄化类型，持有一个pDerived即可；或者将想做的事放到虚函数中，利用多态去完成。
    2. 避免串联
      避免下方这样的代码，一连串的dynamic_cast：
      ```C++
      class Window{ ... };
      // 定义子类 SpecialWindow1,SpecalWindow2,SpecialWindow3
      Window* winPtr;
      // 省略winPtr的其他操作 ...
      if(SpecialWindow1 *psw1 = dynamic_cast<SpecialWindow1*>(winPtr)){ ... }
      else  if(SpecialWindow2 *psw2 = dynamic_cast<SpecialWindow2*>(winPtr)){ ... }
      else  if(SpecialWindow3 *psw3 = dynamic_cast<SpecialWindow3*>(winPtr)){ ... }
      ```
      这样的代码又大又慢，每次继承体系有所改变，代码就需要重新检阅判断。这样的代码应该用“基于virtual函数调用”取代它。

{{<admonition Note "By the way">}}
自 **C++11** 起，针对智能指针 shared_ptr 的转型，推出了另外 4 个模版函数：
- `std::static_pointer_cast`
  函数的原型声明为：
  ```c++
  template< class T, class U >
  std::shared_ptr<T> static_pointer_cast( const std::shared_ptr<U>& r ) noexcept;
  ```
  含义以及应用场景与 `static_cast` 类似，比如子类型指针转为父类型：
  ```c++
  auto basePtr = std::make_shared<Base>();
  auto derivedPtr = std::make_shared<Derived>();
  basePtr = std::static_pointer_cast<Base>(derivedPtr);
  ```
  或许我们会有疑问，是否有必要用这个函数进行转型呢，如下实现不是一样的吗?
  ```c++
  basePtr = std::shared_ptr<Base>(static_cast<Base*>(derivedPtr.get()));
  ```
  🤔 当然不一样，static_cast<T*>((U*)nullptr)是未定义行为，而且就语法描述上来看，哪个更简洁不言自明。
  另外，自 C++20 起支持右值引用，也就是如下形式：
  ```c++
  template< class T, class U >
  std::shared_ptr<T> static_pointer_cast( std::shared_ptr<U>&& r ) noexcept;
  ```
- `std::dynamic_pointer_cast`
含义以及应用场景与 dynamic_cast 类似，用法传参与 std::static_pointer_cast 类似，且自 C++20 起支持右值引用，不赘述。
- `std::const_pointer_cast：`与前 2 者类似，不赘述。
- `std::reinterpret_pointer_cast：`与前 3 者类似，不赘述。
{{</admonition>}}

### R28 避免返回handles指向对象内部成分

这里的**handles（号码牌）包括**指向对象内部的**指针**、**迭代器**、**引用**。

以下讲述当返回对象内部的handles时，存在的2个问题：

  - 可能会破坏封装性
    考虑一个场景：public函数返回一个private成员的非const引用，就让外界有了修改private的机会，破坏了封装性。
    此时，需要将非const引用改为const引用，只读属性。
    ```c++
    struct Pixel {
      float r,g,b;
    }
    class Image {
    public:  //注意：后面这const只保证成员变量vPixPtrs_不改
        Pixel& GetThePixel(int idx) const{
            return  vPixPtrs_[i];}//隐患：其实外部调用者仍能直接修改Pixel的rgb值
        // 上一句的返回值应该改为 "const Pixel&"
    private:
        std::vector<std::shared_ptr<Pixel>> vPixPtrs_;
    }
    ```
  - 可能会引起“空悬handles”
    即使用const解决了封装性的问题，因为很容易出现**“handles比其所指对象更长寿”**，可能存在对象已析构，但handles还留存的问题。尤其是临时变量的析构，不太容易察觉。
    ```c++
    class SceneGraph{ ... };
    const Image CaptureImage(const SceneGraph& graph);
    //那么调用方可能会这样使用
    SceneGraph Grap;
    //下面这句之后，Image临时对象被销毁，pPix指向一个不存在的对象
    const Pixel *pPix = &(CaptureImage(Grap).GetThePixel(0));
    ```
    注意，CaptureImage的确能返回一个临时Image对象，能成功调用GetThePixel，但这句结束后，临时对象立马会被销毁，造成空悬现象，或叫虚吊(dangling) ！

### R29 为“异常安全”而努力是值得的

“**异常安全**”是指，当异常抛出时，代码依然能做到如下2点：
  1. 不泄露任何资源
    包括内存资源，锁资源。
  2. 不允许数据败坏
    不会因为异常而导致空悬指针等未定义行为。

考虑下方的示例代码，（如果`new Image`抛`std::bad_alloc`异常）则会同时违背了上述2条：

```c++
class PrettyMenu{
private:
    Mutex mutex_; //互斥器
    Image* bgImage_ = nullptr;
    int imageChangeCnt_ = 0;
public:
	void PrettyMenu::changeBackground(std::ifstream& imgSrc){
        lock(&mutex_);// 这个可以改为RAII的锁，来保证异常安全
        delete bgImage_;
        ++imageChangeCnt_;
        //这里new Image抛异常，导致无法解锁；且bgImage_指向资源已经释放，空悬指针
        bgImage_ = new Image(imageSrc_);
        unlock(&mutex_);
    }
};
```

#### 异常安全的3个等级

异常安全的函数，有**3个等级**的异常安全保证，会满足**三者之一**：

- **基本承诺**
  如果异常被抛出，程序内的任何事务仍然保持在有效状态下，也没有任何数据败坏。比如上例中如果抛异常，会另外添加实现，使bgImage_持有某个默认图像，或保持原值，让程序继续有效运行。

- **强烈保证**
  如果异常被抛出，程序状态不改变。这样的函数要么成功，要么退回到执行前的状态。

  上述案例则应该会被修改成如下形式：
  ```c++
  class PrettyMenu{
  private:
      std::shared_ptr<Image> bgImage_; //RAII避免了异常发生时的资源泄漏和数据败坏
      //... 省略其他成员
  public:
    void PrettyMenu::changeBackground(std::ifstream& imgSrc){
          CLock ml(&mutex_);//RAII封装的Lock类，详细可参考 阅读总结(二)-Rule14
          bgImage_.reset(new Image(imgSrc));//若new失败，则不会reset
          ++imageChangeCnt_;//把事情做完再++count
      }
  };
  ```
- **不抛异常**
  在原书中，这个“No Throw”不是绝对不抛异常，而是一旦意外抛异常，就会调用unexpected函数进而abort（例如`int doSomething() throw();//空白的异常明细`）。

{{<admonition Note "By the way">}}
原书中例子的 **throw()** 在不同编译器表现不一致，现在已经 **不推荐使用**。

更详细资料可参考[A Pragmatic Look at Exception Specifications](http://gotw.ca/publications/mill22.htm)和[Should I use an exception specifier in C++](https://stackoverflow.com/questions/88573/should-i-use-an-exception-specifier-in-c)

在C++11中，有了更可靠有效的关键字noexcept，使用也很简单，有操作符和异常提示符两种作用，下方展示简单用法：
  ```c++
  void f() noexcept; // 函数 f() 不会抛出
  void (*fp)() noexcept(false); // fp 指向可能会抛出的函数
  void g(void pfa() noexcept);  // g 接收指向不会抛出的函数的指针
  // typedef int (*pf)() noexcept; // 错误
  ```
更详细的`noexcept`介绍可以访问[cppreference：noexcept操作符](https://zh.cppreference.com/w/cpp/language/noexcept)和[cppreference：noexcept异常说明符](https://zh.cppreference.com/w/cpp/language/noexcept_spec)
{{</admonition>}}

#### 使用copy-and-swap保障异常安全

`copy-and-swap`技术：先拷贝一份想修改的对象，等修改彻底完成后（过程中不抛异常），再与原对象交换。

为了更形象展示这一过程，使用pIml手法对bgImage_封装一下：
```c++
struct ImgPimpl{ //选用struct而非Class：方便；最后被private成员形式使用，封装性不用担心
   std::shared_ptr<Image> bgImage_;
   int imageChangeCnt_ = 0;
};
```

那么PrettyMenu类可以改为如下：
```c++
class PrettyMenu{
private:
    Mutex mutex_;
    std::shared_ptr<ImgPimpl> pImpl_;
public:  //构造略
	void PrettyMenu::changeBackground(std::ifstream& imgSrc){
        using std::swap;   //参考 阅读总结(二)-Rule25
        CLock ml(&mutex_);//RAII封装的Lock类，详细可参考总结(二)-Rule14
        std::shared_ptr<ImgPimpl> pNewCopy(new ImgPimpl(*pImpl_));
        pNewCopy->bgImage_.reset(new Image(imgSrc)); //修改副本
        ++pNewCopy->imageChangeCnt_;

        swap(pImpl_,pNewCopy);//改完之后swap
    }
};
```

#### 异常安全的连带影响(side effects)

函数提供的“异常安全保证等级”只**取决于**其调用的**各个子函数的“最不安全者”**。

考虑以下函数SomeFunc代码：
```c++
void SomeFunc{
    ... // 对local状态做一份副本
    f1();
    f2();
    ...  //将修改后的状态置换过来
}
```

**`分析`**：显然copy-and-swap在尽力强烈保证异常安全，但是，f1或者f2的异常安全如果比较低，那么可能需要单独对f1，f2进行copy-and-swap，来尝试保证“强烈异常安全”；即使如此，如果f1能成功做了修改，但是f2修改失败了并回退，那么**f1、f2整体**看起来还是“**改了一部分**”。

**`这也告诫我们`**，如果引入了异常不安全的旧代码，那么这种特性会波及其他代码。

就设计者而言，只能根据实际情况，尽可能保证“异常安全”，选择3个异常安全等级之一实施。

### R30 透彻了解inline的里里外外

`inline`是C++的关键字，表示内联函数。`直接在对应位置展开代码`，免去函数调用的开销，难以避免 “**代码膨胀**”问题。

使用inline时应该注意以下问题：
- **inline只是对编译器的申请**
  inline只是对编译器的申请/建议，不是强制命令，**编译器有权利** 对其认为不适合inline的函数**拒绝**inline。 （原书说：如果编译器拒绝，通常它会给出warning信息。实际本人实测VS2017没看到）
  inline有2种申请方式 ：
    1. 在函数定义 时使用关键字inline**显式强调**
    2. 实现在Class内的成员函数或friend函数，属于**隐式inline**
- **inline和Template没有任何必然联系**
  虽然有不少简短的Template函数是带有inline（例如下方的std::max），但不是必然为之，**两者没有因果关系**。
  ```c++
  template<typename T>
  inline const T& std::max(const T& a, const T& b){ //可以申请inline，但不是必须申请
      return a < b ? b : a;
  }
  ```
- **编译器拒绝复杂函数进行inline**
  复杂函数的inline会带来较严重的“代码膨胀”问题，并且可能会更慢，因为增加了运行时的“额外换页”行为，降低了指令cache命中率。
  1. inline函数内不要出现循环或递归
  2. 虚函数也不适合做inline
      因为inline是编译期间决定的事，而虚函数是运行时决定的事，两者就不是同一个场景的。
  3. 构造/析构函数也不适合做inline
      编译器可能会在构造/析构函数内部做精妙复杂的异常处理；以及在继承体系下，Base类函数体到处inline膨胀。

- **以函数指针形式的调用通常不能inline**
  对绝大多数编译器而言，是否inline是compile阶段决定的事情，少数编译器放到了link阶段。

  讨论大多数情况：需要在编译时得知inline的本体，而函数指针办不到，示例代码如下：
  ```c++
  inline void f() {...} //假设编译器有意愿inline “对f的调用”
  void (* pf)() = f;//pf 执行f
  ...
  f();     //这个调用将被inlined，因为是个正常调用
  pf();     //这个很可能不被inlined
  ```
- **过度inline对调试和发布带来困难**
  inline是代码嵌入与展开，而非函数调用，所以某些编译器不支持inline的单步Debug（就像宏展开一样不支持调试）；另外，inline只要已修改，涉及调用它的代码全都要编译，如果是non-inline则可能只需要重新link即可。

### R31 将文件间的编译依赖关系降至最低

C++的Class定义式包括了成员变量，假设某成员变量是类对象`ClassObjA a`，如果`ClassObjA`类的内部实现发生了改变（哪怕这个`ClassObjA.h`内只在某处加了一个空格 ），那么include了`ClassObjA.h`的所有`.h`文件`.cpp`文件都会**重新编译**。

👆这就是由“**编译依赖**”关系带来的问题。

使用 `PIMPL` 手法(pointer to implementation)可以很好地分离声明和定义：
```c++
//Person.h文件  不需要include "PersonImpl.h"和"BirthDay.h"
class PersonImpl;
class BirthDay;// 前置声明代替include
class Person {
public:// 构造函数这里放实现无所谓，因为是函数指针，不需知道PersonImpl本体实现
    Person(const std::shared_ptr<PersonImpl>& pImpl);//构造函数的实现也放到cpp里去
    const BirthDay& GetBirthDay();// 注意：这里只声明，把实现部分放到cpp里
    std::string GetName();//实现细节由pImpl_转发实现
private:
    std::shared_ptr<PersonImpl> pImpl_;
};
//Person.cpp文件
#include "PersonImpl.h" //在"PersonImpl.h"文件内include那个"BirthDay.h"
#include "Person.h"
```
注意：Impl用法，相关的头文件里不要放置任何函数实现代码 。
上述做法可以让使用Person类的客户不需要再关心PersonImpl以及BirthDay的实现细节了，做到了“接口与实现分离”，关键点在于把“定义的依赖性” 换成 $\Rightarrow$ “**声明的依赖性**”。
这里突出了如何**最小化编译依赖性**的本质：`让头文件尽可能自我满足`，如果不行，也要依赖于其他文件的声明式而非定义式。

具体到设计策略上，有以下几种做法：
- 如果能用object references 或 object pointers完成任务，就别用objects
  如果要定义某类型的objects，就要使用定义式，指针和应用则可以只用声明式。

- 尽量以class声明式替换class定义式
  函数声明种的Class类型可以只用声明式，即使以by-object-value形式传值也是如此。

- 为声明式和定义式提供不同的文件
  比如Date类，分为只包含声明式的"Datefwd.h"和包含定义式的"Date.h"，那么使用时用声明式头文件代替前置声明，在需要应用代码client.cpp里include定义式头文件。这种方式在标准库里采用较多，参考<iosfwd>和<sstream>,<fstream>,<streambuf>等。

{{<admonition Note "By the way">}}
- C/C++可以使用编译器预处理指令<font color=red>`#pragma message`</font>，打印出该文件是否参与此次编译，以及参与编译时被哪个文件所依赖。
  ```c++
  //比如可以在 testEffective.h 文件内加上这句，就能在编译输出信息里看到打印信息，观察到依赖关系
  #pragma message("testEffective.h 参与重新编译")
  //还可以用于测试某些宏是否真的生效，有时IDE的高亮显示不准确而令人生疑。
  ```
- 关于头文件include的其他编写规范，可以参考：[Google-C++风格指南：1.头文件](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/headers/#)
{{</admonition>}}


使用 <font color>**`Interface Class`**</font> 也能做到接口和实现的真正分离：

这种方式常见于输出动态库给到客户使用，客户能见到接口定义和使用，但无法看到内部实现。

用法较为常见，不赘述，直接**show-code**：

```c++
//VirtualPerson.h   //和 lib文件一起提供给到客户
class VirtualPerson {
public: //create的返回值还可以根据需求，换成RAII的智能指针
    static VirtualPerson * create(int level, int salary);
    virtual int Level() = 0;
    virtual int Salary() = 0;
    virtual ~VirtualPerson();// avoid memory leak
};
//VirtualPerson.cpp  //源码不提供给客户，而是编译好的二进制 lib文件
VirtualPerson* VirtualPerson::create(int level, int salary) {
    if (level == 3) { //工厂方法，还可以生成其他子类
        return new Engineer(level, salary);
    }
    return nullptr;
}
VirtualPerson::~VirtualPerson() { }
// Engineer.h  //Engineer也是参与编译到 lib文件中
class Engineer: public VirtualPerson {
public:
    Engineer(int level,int salary);
    virtual int Level() ;
    virtual int Salary() ;
private:
    int level_ = 3;
    int salary_ = 0;
};
```

## CH6. 继承与面向对象设计

### R32 确定你的public继承塑造出is-a关系

原书标题：**Make sure public inheritance models “is-a”**. 侯捷老师翻译为“塑模”，我个人更愿意称为“塑造”。

- **“public继承”意味着is-a**
  is-a，即“是一种”，就是说，适用于Base Class身上的每一件事，也一定适用于Derived Class身上。

{{<admonition Note "By the way">}}
面向设计对象的设计有著名的5大原则， SOLID 原则，每个字母分别代表一种原则：
- S–单一责任原则(SRP) –Single Responsibility Principle
- O–开放封闭原则(OCP)– Open-Closed Principle
- L–里式替换原则(LSP)– Liskov Substitution Principle
- I –- 接口分离原则(ISP)–Interface Segregation Principle
- D–-依赖倒置原则(DIP)– Dependency Inversion Principle

更详细的叙述可以参考：[腾讯云：SOLID原则](https://cloud.tencent.com/developer/article/1503629)
{{</admonition>}}

本节条款的中心思想即里氏替换原则：**一个对象出现的地方都可以由其子类代替并且不会出错。**

继承关系有时候听起来很好理解，比如 `Class Student: public Person`理所应当，但有时也会导致误解。比如企鹅属于鸟类，但企鹅不会飞，那么基类`Bird::Fly`方法又当如何处理，下方满足设计意图：

```c++
class Bird{   //... 不声明Fly()方法
};
class FylingBird:public Bird{
public: virtual void Fly();
};
class Penguin: public Bird{ //... 不声明Fly()方法
};
// 注：如果Bird类以及子类都不考虑Fly()方法，那么Penguin直接继承于Bird即可
```

还有另一种场景，父类和子类对于同一个方法的数据修改规则不同，导致了继承体系的缺陷。
比如`Class Square:public Rectangle` $\rightarrow$ 正方形继承于长方形，但是考虑这样一个**类外方法**:

```c++
void makeBigger(Rectangle& r){  //普通非成员函数
    int oldHeight = r.height();
    r.setWidth(r.width() + 10);//如果r是Squqre，可能内部自动就长宽一起变了
    assert(r.height() == oldHeight);//这个assert对于正方形就不合适了,贸然去除又违背设计本意
}
```

应对上述这样的情况，就需要修改设计或修改继承体系了。

### R33 避免遮掩由继承得来的名称
首先，什么是名称的遮掩，通俗地说，是指由于作用域不同带来的变量名覆盖。考虑下方代码：
```c++
int x = 10;
void someFunc(){
    double x = 0.1;
    std::cout<<"x is" << x << std::endl; // local作用域找到了x，直接覆盖全局的x，输出0.1
}
```
那么如果将继承体系考虑进来呢：**Derived的作用域会覆盖Base的作用域**，包括virtual和non-virtual。考虑下方代码：

```c++
class Base {
public:
    virtual void mf1() = 0;
    virtual void mf1(int x) { std::cout << "Base::mf1():x =" << x << std::endl; }
    virtual void mf2() { std::cout << "Base::mf2()\n"; }
    void mf3() { std::cout << "Base::mf3()\n"; }
    void mf3(int x) { std::cout << "Base::mf3():x =" << x << std::endl; }
    virtual ~Base() {}
};
class Derived :public Base {
public:
    virtual void mf1() { std::cout << "Derived::mf1()\n"; }
    void mf3(){ std::cout << "Derived::mf3()\n"; }
};
```
很明显存在名称遮掩的问题，Derived的mf1，mf3会遮掩子类的所有同名函数，测试结果如下：
```c++
Derived d;
d.mf1();      //OK，输出: Derived::mf1()
//d.mf1(100);  编译报错，因为名称被遮掩
d.mf2();      //OK，输出: Base::mf2()
d.mf3();      //OK，输出: Derived::mf3()
//d.mf3(300);  编译报错，同理
d.Base::mf3(300); //OK，输出: Base::mf3():x =300 . 但是不太建议这么写，丑！！
```

- 为解决上述问题，可以采用using声明式或转发函数
  1. using 声明式
     可以使用using声明式，让Derived可以忽略名称遮掩，看到Base作用域内的函数。可以让上方代码的“编译报错”消失，正常调用d.mf1(100)和d.mf3(300)。
     ```c++
      class Derived :public Base {
      public: //修改本节内容中上方代码的Derived类的声明
          using Base::mf1;
          using Base::mf3; //这2个using使得Base类作用域内所有mf1,mf3函数都可见
          // ... 其他，略
      }
     ```
  2. 转发函数(forwarding function)
    应用场景: 在private继承下，强调的是继承实现而非继承接口，如果想在子类的成员中调用父类函数，此时可以通过函数转发来实现。
    ```c++
    class Derived: private Base{ //改写本节上方代码，注意，是私有继承
    public:
        virtual void mf1(){
            Base::mf1(); //拿到了父类的函数实现
            // ... 该函数其他部分
        }
    };
    // 应用代码
    Derived d;
    d.mf1();    //调用成功，Derived::mf1
    d.mf1(100);  //编译失败
    ```
    当然，public继承也能使用转发函数，写出`d.Base::mf3(300)`; 这样的代码。但是，一来public继承理应遵循"is-a"规则，using声明拿到所有被遮掩的接口；二来明显代码不美观。

### R34 区分接口继承和实现继承

当一个子类Derived继承于父类Base，那么要时刻清楚，对于类中的成员函数，是想继承父类的接口，还是想继承父类的实现。

- **对于Public继承，接口总是会被继承**
  基于“**is-a**”的关系，作用于父类的任何事情也一定要适用于子类。
- **声明纯虚函数(pure-virtual)的目的是让子类只继承函数接口**
  对于纯虚函数，子类必须重新实现该接口。注意，`父类可以选择性给出纯虚函数的实现`，但是一般不会给。
- **隐患：从非纯虚函数(impure-virtual)同时继承接口和缺省实现**
  非纯虚函数，可以让子类选择是否重新实现该接口。那么，如果子类是有必要重写，但是**忘记写了**却默默用父类版本，便事与愿违了。
  举例：父类Airplane有子类PlaneModelA、PlaneModelB、PlaneModelC，其中C型飞机不同于AB型，是新式飞机：
  ```c++
  class Airport {...};//机场类，实现略
  class Airplane{
  public:
      virtual void fly(const Airport& destination);//父类还会给出默认的fly实现
  };
  class PlaneModelA: public Airplane{ ... }; // 不重写fly，继承父类的fly实现
  class PlaneModelB: public Airplane{ ... }; // B和A一样
  class PlaneModelC: public Airplane{ ... }; //新型飞机，本来要重写fly，结果忘了
  ```
  那么这个`隐患该如何解决`呢，也就是说，在实现C型飞机类时别忘了fly方法？
  核心思想是“**切断virtual函数接口和其默认实现之间的连接**”。
  1. 方法1：设置fly为纯虚函数，并新增一个defaultyFly方法
      注意细节：defaultFly方法要设置为protected属性的non-virtual函数，代码如下：
      ```c++
      class Airplane{
      public:
          virtual void fly(const Airport& destination) = 0;// 父类不给出实现
      protected:
          void defaultFly(const Airport& destination){ ... }//默认的fly实现
      };
      class PlaneModelA: public Airplane{
      public:   //纯虚接口,子类必须给出实现
          virtual void fly(const Airport& destination){
              defaultFly(destination); //调用父类的缺省实现
          }
      };//PlaneModelB 和 PlaneModelA 类似,略
      class PlaneModelC: public Airplane{ ...};// 重写fly方法
      ```
      这样写还有个好处：fly()和defaultFly()享有不同的保护级别。
  2. 方法2： 父类的默认实现塞到纯虚接口fly中
      这样就不需要定义defaultFly方法了，因为子类必须实现fly方法，对于A 型、B型飞机，子类fly()转发一次父类的fly()即可，C类飞机实现新式的fly()。缺点是让原本在defaultFly内的实现内容暴露在外了(指public属性)。
      {{<admonition Note "Note">}}
      个人认为，这方法2还有个缺点：它让虚基类的纯虚接口承载了接口实现，不够纯粹(比如需要输出给到客户，应该只继承接口)。
      {{</admonition>}}

- **non-virtual函数具体指定接口继承和强制性实现继承**
  如果成员函数是non-virtual，表示它不打算在子类中有不同的行为，或者说，不变性凌驾于特异性。对应地，绝不应该在子类中重写non-virtual函数。

### R35 考虑virtual函数的替代选择

假设这样一个场景：设计一款游戏，不同人物以不同方式计算生命值，那么$\Longrightarrow$ 设计继承体系，子类共同继承父类的public-virtual方法healthValue()，子类各自重新实现healthValue()接口。

😄很好，中规中矩，那么，有没有其他方式呢？

- **`NVI手法(non-virtual interface)实现Template Method模式`**
  思路就是父类定义个non-virtual的public方法healthValue()，调用virtual的private方法healthValueImpl。子类直接重写healthValueImpl，达到类似模版方法设计模式的效果。
  父类`GameCharacter`设计如下：
  ```c++
  class GameCharacter{//构造函数和虚析构均略去
  public:
      int healthValue()const{ //ps:方便展示，而写在了头文件里，成了inline
          std::cout<< "Do prepare works...\n";//事前，如加锁,写log,验证条件等
          int retVal = healthValueImpl();
          std::cout<< "\nDo post works...\n";//事后,如解锁,更新数据
          return retVal;
      }
  private:
      virtual int healthValueImpl() const{
          int val=0;
          std::cout << "default caculate process... GetValue:" << val;//随后进行计算，过程略
          return val;
      }
  };
  ```
  子类`GoodGuy`和`BadGuy`设计如下：
  ```c++
  class GoodGuy:public GameCharacter{
  private:
      virtual int healthValueImpl() const{
          int val = 60;
          std::cout << "goodGuy caculate ... GetValue: " << val << " ";//过程略
          return val;
      }
  };
  class BadGuy:public GameCharacter{
  private:
      virtual int healthValueImpl() const{
          int val = 80;
          std::cout << "badGuy caculate ... GetValue: " << val << " ";//过程略
          return val;
      }
  };
  ```
  应用端代码如下：
  ```c++
  std::shared_ptr<GameCharacter> pGood = std::make_shared<GoodGuy>();
  pGood->healthValue(); //得到60
  std::shared_ptr<GameCharacter> pBad = std::make_shared<BadGuy>();
  pBad->healthValue();//得到80
  ```
- **`用函数指针实现Strategy模式`**
  主体思想是添加一个函数指针为private成员变量pFunc，这个函数通过外部传入，从而实现不同的行为。
  ```c++
  class GameCharacter;//forward declaration
  int defaultHealthCalc(const GameCharacter& gc);//默认算法实现
  class GameCharacter{
  public:
    typedef int (*HealthCalcFunc)(const GameCharacter& gc);
    explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc):calcFunc_(hcf){}//传入函数指针,自定义实现
    int healthValue()const{
      return calcFunc_(*this);
    }
  private:
    HealthCalcFunc calcFunc_ = nullptr;
  };
  ```
  这个设计有2个有趣的**设计弹性**：
    1. 即使同一个人物类型的不同实体，允许拥有不同的生命值计算方法；
    2. 某个人物对象的生命值计算方法，在其生命期内可以任意修改，只要添加一个set方法即可；
- **`用std::function实现Strategy模式`**
  private成员变量由上文的函数指针替换成std::function对象，相当于是`指向函数的泛化指针`。就`更具设计弹性`了。std::function可以传入函数指针、仿函数、std::bind函数对象。GameCharacter的类实现修改为：
  ```c++
  class GameCharacter{
  public:
      typedef std::function<int (const GameCharacter&)>  healthCalcFunc;
      explicit GameCharacter(healthCalcFunc hcf = defaultHealthCalc):healthValueImpl_(hcf){}
      int healthValue()const{
          std::cout<< "Do prepare works...\n";
          int retVal = healthValueImpl_(*this); // 这里改了
          std::cout<< "\nDo post works...\n";
          return retVal;
      }
  private:
      healthCalcFunc healthValueImpl_ = nullptr;
  };
  ```
  1. 传入函数指针
    ```c++
     gameChashort quickHurtHealthCalc(const GameCharacter2& gc);//返回值不是int,可隐式转换;实现略去
    //应用端代码如下：
    GameCharacter quickGuy(quickHurtHealthCalc);
    quickGuy.healthValue();//内部调用quickHurtHealthCalc
    ```

  2. 传入仿函数
    仿函数：即函数对象，而且重载了operator() 。
    ```c++
    struct HealthCalculator{//
      int operator()(const GameCharacter2& gc) const{
        return 180;//省略实现
      }
    };
    //应用端代码如下：
    GameCharacter functorGuy( (HealthCalculator()) );//用括号将仿函数括起来
    functorGuy.healthValue();
    ```
  3. 传入`std::bind`函数对象
    std::bind是函数对象模板，接收一个函数指针f和若干函数入参得到fObj，调用fObj等同于调用带参数的f。本例代码如下：
    ```c++
    class GameLevel{
    public: //用类内函数作为函数指针f
        float health(const GameCharacter& gc) const{
            return -20.3;
        }
    };
    //应用端代码如下：
    GameLevel  curLevel;
    GameCharacter levelGuy(std::bind(&GameLevel::health, curLevel,std::placeholders::_1));
    levelGuy.healthValue();//内部调用等价于curLevel.health(leveGuy);
    ```
    {{<admonition Note "By the way">}}
    其实这里传入C++11新增的Lambda表达式作为std::function也是可以的，而且更方便，示例代码如下：
    ```c++
    GameCharacter  lamGuy([&](const GameCharacter& gc){
     std::cout<< "value is " << 75 << std::endl;//具体计算略
	   return 75;
	   });
    lamGuy.healthValue();
    ```
    关于Strategy设计模式更多参考：
      - [Strategy设计模式-原理讲解](https://refactoringguru.cn/design-patterns/strategy)
      - [Strategy设计模式-C++代码参考](https://refactoringguru.cn/design-patterns/strategy/cpp/example)
    {{</admonition>}}


### R36 绝不重新定义继承而来的non-virtual函数

- 由于名称遮掩，不要重新定义继承而来的non-virtual函数
  看个反例：
  ```c++
  class B{
  public:
      void mf(){
        std::cout << "B::mf()";
      }
  }
  class D: public B{
  public:
      void mf(){//重新定义mf()，违反了Rule33
          std::cout << "D::mf()";
      }
  };
  ```
  那么看这样的应用代码：
  ```c++
  D d;
  D* pD = &d;
  B* pB = &d;
  d.mf();    // 输出 D::mf()
  pD -> mf();// 输出 D::mf()
  pB -> mf();// 输出 B::mf()
  ```
  这就很诡异了！都通过对象d调用成员函数mf，调用结果应该相同。

  另外一点，出于public继承的“**is-a**”特性，这样重新定义non-virtual函数也是**对"is-a"的严重违背**。

### R37 绝不重新定义继承而来的缺省入参值

这里说的缺省入参，指的是函数入参的默认值，在重写带有缺省入参的virtual函数时，不要修改那个默认参数的默认值。

原因：virtual函数为`动态绑定`特性，而缺省参数值是`静态绑定`特性。修改后会造成一些令人费解的现象。

请看下方反面教材：
```c++
enum class Color {
    RED = 0,GREEN ,BLUE
};
class Shape {
public:  // ：父类默认入参是RED
    virtual void draw(Color col = Color::RED) {
      std::cout << "Shape:col is " << int(col) << std::endl;
    }
};
class Rectangle : public Shape{ // ：子类类修改默认入参为GREEN
    virtual void draw(Color col = Color::GREEN) {
      std::cout << "Rectangle:col is " << int(col) << std::endl;
    }
};
```
那么当出现典型应用场景`Base* pB=new Derived`时，就会造成“父类子类各出一半力”的情形：
```c++
Shape *pRec = new Rectangle();
pRec->draw(); // 输出：Rectangle:col is 0  （0是RED）
```

结果确实调用子类draw，但是默认入参取的是基类的 RED，而非子类的GREEN。
那怎么修改合适呢，都带默认参数，且子类父类相同？带来一个**耦合问题**，如果父类改了，所有子类都得改。
正如Rule35提到的NVI(non-virtual interface)手法，此处便是绝佳的应用场景$\Longrightarrow$ draw方法改为默认参数的non-virtual，把virtual函数放到private里去，代码修改如下：

```c++
class Shape {
public:   //子类继承该默认入参的non-virtual接口，别重写
    void draw(Color col = Color::RED) {
        drawImpl(col);
    }
private: //纯虚函数是强制子类重写，看具体情况，impure-virtual也行
    virtual void drawImpl(Color col) = 0;//子类重写这个drawImpl
};
```

### R38 通过复合塑造出has-a或"根据某物实现出”

原书标题：`Model “has-a” or “is-implemented-in-terms-of” through composition`，同Rule32，侯捷老师翻译为“塑模”。

复合关系（composition）是一种常见的类关系，当某种类型的对象内含有它种类型的对象时，便是此种关系。

复合关系分为2种：`"has-a"` 和 `“is-implemented-in-terms-of"`。

- “`has-a`”关系：
  指的是**应用域**部分，不参与内的具体各项实现。是一种单纯的完备对象的包含关系，比如Person类有Address、PhoneNumber、Job等类型的成员变量，又或是Image类有Buffer、Mutexx、SearchTree等类型的成员。
- “`is-implemented-in-terms-of`“关系：
  指的是**实现域**部分，参与类的各类实现，比如数据结构的设计中，想用现有的 std::list来实现Set类，这样可能效率不高(通常更具效率的实现是采用平衡查找树 )，但是可行。
  📌：此处不能让Set以public继承于std::list，因为list允许重复元素，而Set不行，不满足“is-a”关系。
  正确实现部分代码示例如下：
  ```c++
  template<class T>
  class Set {
  public:
    bool contains(const T& item)const{
        return std::find(rep_.begin(),rep_.end(),item) != rep_.end();
    }
    void insert(const T& item){
        if(!contains(item)) rep_.push_back(item);
    }
    void remove(const T& item);// 实现略
    std::size_t size() const;  // 实现略
  private:
    std::list<T> rep_; //用来表述Set的数据
  };
  ```

### R39 明智而审慎地使用private继承

“明智而审慎”的意思是👉当考虑了其他方案对比后，仍然觉得private继承是最合适的，才使用它。

首先明确private继承的2个特性：
  1. 编译器不会自动将一个derived-class对象隐式转换为base-class对象(函数入参时)；
  2. 继承而来的成员，在derived-class中都会变成private属性；

private继承的意义：意味着**implemented-in-terms-of**，在类关系设计上没有太大意义，只看重软件实现。

考虑以下使用private的2个应用场景：

- **derived-class想继承base-class的某public接口实现，但又想隐藏此接口**
  考虑如下应用场景：对于一个已知的类Widget，想用另一个已知的计时类Timer辅助性能分析，在尽量小改动已有代码的情况下，如何启用Timer？
  private继承做法：让Widget类private继承于Timer，重写父类Timer的onTick函数。
  具体代码如下：
  ```c++
  class Timer{
  public:
    explicit Timer(int tickFrequency);
    virtual void onTick() const;//定时器滴答一次，自动被调用一次
  };
  class Widget:private Timer{
  private: // private继承而来的所有成员都是private属性
    virtual void onTick() const;//查看并记录Widget数据，资源等
  };
  ```
  该问题除了上方的private继承，能不能用其他方案替代private继承呢？
  👉👉“public继承+复合”替代private继承：在Widget内部嵌套定义private属性的新类WidgetTimer:private Timer，即可同样启用Timer且隐藏了Timer。代码如下：
  ```c++
  class Widget{
  private:
    class WidgetTimer:public Timer{// 类内嵌套定义
    public:
        virtual void onTick() const;
    };
    WidgetTimer wTimer_;
  };
  ```
  ⭐⭐ WidgetTimer也可以不定义在Widget类内,类内只放WidgetTimer* 和WidgetTimer的前置声明，完全解耦合，**降低编译依赖性**。而这样的设计自由度是单纯的private继承不具备的。

- **空白基类最优化(EBO,empty base optimization)**
  ⚡值得一提：空类(Empty Class)是指不含non-static数据成员和virtual-func的类。
  空类的size会被C++强制要求至少为1，通常是用1个char占位。如果让Empty-Class作为数据成员，因为内存对齐而导致Derived-Class浪费内存。
  示例代码 👇：
  ```c++
  class Empty { // 空类，1字节. 不含non-static数据，不含virtual
    void privteFoo() { std::cout << "private non-virtual.";}
  public:
    typedef char*  pChar;
    typedef void(*pFuncReadData)(std::string url);
    enum class clolr {
        red,green,blue
    };
    void foo() { std::cout << "public non-virtual!"; }
    static int count ;//static 数据也不属于class实体
  };
  class HoldsIntsAndEmpty { //内存对齐后12字节
    int x_;   // 4字节
    Empty e_; // 1字节
    int y_;   // 4字节
  };
  class HoldsInts:private Empty {//使用EBO,类大小8字节
    int x_;//4字节
    int y_;//4字节
  };
  ```
  EBO优化可以减少Derived-Class的内存大小，注意EBO只适用于单继承。

### R40 明智而审慎地使用多重继承

多重继承(multiple inheritance)是指继承一个以上的父类。但是这些父类应该避免拥有共同的祖父类，会形成比较麻烦的“菱形继承”(或者叫钻石继承)。

- **多重继承的成本以及副作用**
  上面说“菱形继承”比较麻烦，主要原因是如果祖父类如果拥有某个成员变量x，那么2个父类分别public形式继承了x，到了目标子类就有了2份x。
  解决问题的办法是**虚继承**(virtual inheritance)，如此，上述子类只有一份x。为保证虚继承的正确性，编译器在背后需要付出更多代价，可能造成子类内存更大或运行速度更慢。
  👉如果存在菱形继承，那么祖父类尽量不要持有数据成员。
  虚继承示例代码如下：
  ```c++
  class File{...};  //祖父类最好不要持有non-static数据成员
  class InputFile: virtual public File{...};
  class OutputFile: virtual public File{...};
  class IOFile:public InputFile,public OutputFile{...};
  ```
- **应用场景：public继承接口+private继承实现**
  思考这样的应用场景，PersonBase类是虚基类，RealPerson是目标子类（需要继承接口），但是获取name和birthDate信息的函数在另一个PersonInfo类都有了现成的实现（只需要简单修改该实现）。

  两者结合后，即让RealPerson类public继承于PersonBase，private继承于PersonInfo。
  ```c++
  class PersonBase {
  public:
      virtual ~PersonBase(){}
      virtual std::string name() const = 0;
      virtual std::string birthDate() const = 0;
  };
  class PersonInfo {
  public:
      virtual  ~PersonInfo() {}
      explicit PersonInfo(int pID):id_(pID) {}
      virtual const char* theName() const{
          static char value[1024];
          static const char* exampleName = "Luka";// 计算过程略,用固定字符串替代
          std::strcpy(value, valueDelimLeft());   // 获取左界定符
          std::strcat(value, exampleName);
          std::strcat(value, valueDelimRight());// 获取右界定符
          return value;
      }
      virtual const char* theBirthDate() const { return "1990-1-1"; }
      virtual const char* valueDelimLeft() const { return "["; };
      virtual const char* valueDelimRight() const { return "]"; };
  private:
      int id_ = 0;
  };
  ```
  多重继承的代码为👇:
  ```c++
  class RealPerson : public PersonBase, private PersonInfo {//多重继承
  public:
      explicit RealPerson(int pID) :PersonInfo(pID) {} // 委托构造
      virtual std::string name() const{ //实现必要的虚基类Person的pure-virtual成员函数
          return PersonInfo::theName();
      }
      virtual std::string birthDate() const {
          return PersonInfo::theBirthDate();
      }
  private:
      virtual const char* valueDelimLeft() const { return ""; };//重写界定符函数
      virtual const char* valueDelimRight() const { return ""; };
  };
  ```
  最后应用端代码：
  ```c++
  RealPerson rPerson(613);
  std::cout << rPerson.name(); //输出Luka ,而不是[Luka]
  ```
  可以看到，多重继承体系完美解决该问题。

回到本节开头，明智和审慎的意思是👉即使多重继承可以用单继承方案替代解决，思考后，如果多重继承依然是最简洁、最易维护、最合理的做法，那就选择它。


ref:</br>
[1]. https://blog.csdn.net/cltcj/category_12098441.html</br>
[2]. https://kissingfire123.github.io/2022/02/14_effective-c-%E4%B9%8B%E9%98%85%E8%AF%BB%E6%80%BB%E7%BB%93%E4%B8%89/</br>

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partthree/  

