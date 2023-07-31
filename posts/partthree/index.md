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















ref:</br>
[1]. https://blog.csdn.net/cltcj/category_12098441.html</br>
[2]. https://kissingfire123.github.io/2022/02/14_effective-c-%E4%B9%8B%E9%98%85%E8%AF%BB%E6%80%BB%E7%BB%93%E4%B8%89/</br>

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partthree/  

