# Effective Modern C++ 阅读笔记

## Effective Modern C++ 笔记

### CH01 Deducing Types

#### [Item 1: Understand template type deduction.](https://blog.csdn.net/Dong_HFUT/article/details/122727237)

大部分简单场景下，我们根据模板调用可能一眼就能推导出来模板类型。但是，对于一些复杂场景，模板类型就没那么明显了，这就需要遵循一些基本原则。

```c++
template<typename T>
void f(ParamType param);
//.....
f(expr);  // call f with some expression
```
考虑以上代码片段，编译器在编译阶段根据调用点 expr 来推导出 T 和 ParamType 的类型。这其中 T 的推导不仅依赖 expr 的类型，也依赖 ParamType 的形式，有3种场景：
  - ParamType 是引用或者指针类型，但不是万能引用。
  - ParamType 是万能引用。
  - ParamType 不是引用也不是指针。

**Case 1: ParamType is a Reference or Pointer, but not a Universal Reference**

对于 ParamType 是引用或者指针类型，但不是万能引用的场景，类型推导方式如下：
  1. 如果 expr 是一个引用，忽略其引用部分。
  2. 然后对 expr 的类型和 ParamType 进行模式匹配来决定 T。

考虑下面的例子：
```c++
template<typename T>
void f(T& param);     // param is a reference
```

我们的变量申明如下：
```c++
int x = 27;          // x is an int
const int cx = x;    // cx is a const int
const int& rx = x;   // rx is a reference to x as a const int
```

调用点和推导结果如下：
```c++
f(x);      // T is int, param's type is int&
           // param's type is int&
f(cx);     // T is const int,
           // param's type is const int&
f(rx);     // T is const int,
           // param's type is const int&
```

第一个调用，函数调用非常简单，我们很快能得到 T 是 int，param类型是 int&。

第二个调用，cx 是 const int 类型，因此 T 被推导成 const int，param 的类型是 const int&。当我们传一个 const 对象给函数的一个引用类型参数，我们期望这个const 对象不能被修改，因此 param 被推导成常量引用（reference-to-const）。这就是为什么传递一个 const 对象给 T& 模板类型是安全的原因：对象的 constness 属性被推导成了 T 的一部分。

第三个调用，虽然 rx 的类型是一个引用，类型推导过程中将忽略 rx 的引用类型，T 被推导成一个非引用类型，即 const int，param 的类型是 const int&。

如果我们将 param 类型改成 const T&，情况略为有点不同，因为 param 的类型已经是常量引用（reference-to-const），不需要将 const 推导成 T 的一部分，如下：
```c++
template<typename T>
void f(const T& param); // param is now a ref-to-const

int x = 27;        // as before
const int cx = x;  // as before
const int& rx = x; // as before

f(x);              // T is int, param's type is const int&
f(cx);             // T is int, param's type is const int&
f(rx);             // T is int, param's type is const int&
```

如果 param 是一个指针（或者是指向常量的指针），推导方式其是引用是一样的，如下：

```c++
template<typename T>
void f(T* param);    // param is now a pointer

int x = 27;          // as before
const int *px = &x;  // px is a ptr to x as a const int

f(&x);               // T is int, param's type is int*
f(px);               // T is const int,
                     // param's type is const int*
```
Case1 场景的模板类型推导和我们设想的应该差不多，还是比较简单的。

**Case 2: ParamType is a Universal Reference**

模板类型参数是万能引用时，模板类型推导就没有那么明显了，详细介绍将会在 Item24 中展开，这里直接给出处理方式：

  - 如果 expr 是一个左值（lvalue），T 和 ParamType 都被推导成左值的引用。这是非常不寻常的。第一，这是唯一一个在模板类型推导中将 T 推导成一个引用的情况。第二，虽然 ParamType 被申明成语法上的一个右值（rvalue）引用，但它的推导类型却是一个左值引用。
  - 如果 expr 是一个右值，和正常的规则一样（比如Case1的推导方式）。

例如：

```c++
template<typename T>
void f(T&& param); // param is now a universal reference

int x = 27;
const int cx = x;
const int& rx = x;

f(x);              // x is lvalue, so T is int&,
                   // param's type is also int&
f(cx);             // cx is lvalue, so T is const int&,
                   // param's type is also const int&
f(rx);             // rx is lvalue, so T is const int&,
                   // param's type is also const int&
f(27);             // 27 is rvalue, so T is int,
                   // param's type is therefore int&&
```
我们将在 Item24 中解释这样推导的原因。

**Case 3: ParamType is Neither a Pointer nor a Reference**

当 ParamType 不是引用也不是指针，则是通过值传递的方式处理：

```c++
template<typename T>
void f(T param);       // param is now passed by value
```

这意味着不管传递进来的是啥，param 是实参的拷贝，它将是一个新的对象。推导方式如下：
  1. 和之前一样，如果 expr 的类型是一个引用，忽略其引用部分。
  2. 在忽略 expr 的引用部分之后，如果 expr 是一个 const，也忽略它。如果 expr 是 volatile 的，也同样忽略。

因此：

```c++
template<typename T>
void f(T param);       // param is now passed by value

int x = 27;
const int cx = x;
const int& rx = x;

f(x);                // T's and param's types are both int
f(cx);               // T's and param's types are again both int
f(rx);               // T's and param's types are still both int
```

注意到，虽然 cx 和 rx 是 const 类型，param 也不是 const 的。这是可以理解的，param 是一个新的对象， 独立于 cx 和 rx —— 是 cx 和 rx 的一个拷贝。param 的修改不会影响到 cx 和 rx，这就是为什么在类型推导时 expr 的 constness（以及 volatileness 等）被忽略的原因：因为 expr 不能被修改并不意味着它的拷贝不能。

对于值传递的参数模板，const 和 volatile 是被忽略的，但是对于引用类型或者常量引用类型参数的模板，expr 的 const 在类型推导时被保留下来了。考虑 expr 是一个指向常量的常量指针，通过值传递的参数类型：

```c++
template<typename T>
void f(T param);                                    // param is still passed by value

const char* const ptr = "Fun with pointers";        // ptr is const pointer to const object
f(ptr);                                             // pass arg of type const char * const
```
这里，ptr 是常量，ptr 指向的字符串也是常量。当 ptr 传递给 f，组成指针的比特位被拷贝给 param，也就是说指针自己（ptr）是值传递，根据值传递参数模板类型推导规则，ptr 的 constness 将被忽略，param 的类型被推导为 const char*，是指向常量字符串的非常量指针。在类型推导过程中，ptr 指向对象的 constness 被保留下来，ptr 自身的 constness 在被拷贝用于创建一个新的对象 param 时被忽略了。


**Array Arguments**

以上3个 case 可以覆盖主流的模板类型推导场景了，但是还是有一个特别的场景值得考虑。数组类型和指针类型是有区别的，虽然它们经常是可以互换的，在多数场景下，一个数组可以转换成指向其第一元素的指针，如：

```c++
const char name[] = "J. P. Briggs"; // name's type is
                                    // const char[13]
const char * ptrToName = name;      // array decays to pointer
```

但是，一个数组被传递给值传递参数模板时候，将会发生什么呢？

```c++
template<typename T>
void f(T param);       // template with by-value parameter

f(name);               // what types are deduced for T and param?
```

先考虑数组作为函数参数时的现象：

```c++
void myFunc(int param[]);
```

以上代码将被等价视为：

```c++
void myFunc(int* param);  // same function as above
```

由于数组参数被视为指针参数，因此在模板推导时候，对于值传递参数的函数模板， T 将被推导成 const char*。

但是，若模板函数的参数是引用：

```c++
template<typename T>
void f(T& param);    // template with by-reference parameter
```

我们传递一个数组给它：

```c++
f(name);        // pass array to f
```

这时候，T 的类型将被推导成数组类型， T 为 const char [13]，f 的参数（数组的引用）为 const char (&)[13]，数组元素个数也可以被推导出来。

有趣的是，数组引用的这种功能可以用于在编译阶段计算数组元素个数：

```c++
// return size of an array as a compile-time constant. (The
// array parameter has no name, because we care only about
// the number of elements it contains.)
template<typename T, std::size_t N>                  // see info below on constexpr and noexcept
constexpr std::size_t arraySize(T (&)[N]) noexcept
{
  return N;
}
```

这可以让我们申明一个新的数组，它的元素个数和第一数组元素个数相同：

```c++
int keyVals[] = { 1, 3, 7, 9, 11, 22, 35 }; // keyVals has 7 elements
int mappedVals[arraySize(keyVals)];
std::array<int, arraySize(keyVals)> mappedVals;  // 更 C++ 的方式
```

**Function Arguments**

在C++中，除了数组可以转为为指针外，函数类型也可以转为函数指针，并且上面讨论的对于数组的模板参数类型推导也可以应用于函数：

```c++
void someFunc(int, double);  // someFunc is a function;
                             // type is void(int, double)

template<typename T>
void f1(T param);     // in f1, param passed by value
f1(someFunc);         // param deduced as ptr-to-func;
                      // type is void (*)(int, double)

template<typename T>
void f2(T& param);     // in f2, param passed by ref
f2(someFunc);          // param deduced as ref-to-func;
                       // type is void (&)(int, double)
                       // T为 void (int, doble)
```
对于推导结果，可以参考 Item4 中提供的方法进行验证，这里给一个例子：

```c++
#include <iostream>
#include <boost/type_index.hpp>
using namespace std;

void someFunc(int, double);

template<typename T>
void f1(T param) {
    using boost::typeindex::type_id_with_cvr;

    cout << "T = " << type_id_with_cvr<T>().pretty_name() << endl;
    cout << "param = " << type_id_with_cvr<decltype(param)>().pretty_name() << endl;
}

template<typename T>
void f2(T& param) {
    using boost::typeindex::type_id_with_cvr;

    cout << "T = " << type_id_with_cvr<T>().pretty_name() << endl;
    cout << "param = " << type_id_with_cvr<decltype(param)>().pretty_name() << endl;
}

int main () {

    f1(someFunc);
    f2(someFunc);
    return 0;
}

// 打印结果
T = void (*)(int, double)
param = void (*)(int, double)
T = void (int, double)
param = void (&)(int, double)
```

{{<admonition quote "总结" false>}}
- 在模板类型推导中，引用类型参数将被视为非引用类型处理，也就是说其引用性被忽略。
- 在万能引用参数类型推导时，左值参数被特殊处理。
- 值传递形参的类型推导时，其 const 和 volatile 被忽略。
- 在模板类型推导时，数组或者函数类型被转换为指针类型，除非它们用来初始化引用。
{{</admonition>}}


#### [Item 2: Understand auto type deduction](https://blog.csdn.net/Dong_HFUT/article/details/122740091)

这次学习 auto 类型的自动推导，在 auto 类型推导方法上，除了一种场景外，在 Item1 中学习的模板类型推导方法都可以适用。回顾下模板的类型推导：

```c++
template<typename T>
void f(ParamType param);

f(expr); // call f with some expression
```

在调用点 f，编译器使用 expr 去推导出 T 和 ParamType 的类型。

对于使用 auto 声明的变量，auto 对应 T，变量的类型描述符对应 ParamType。如下例子，rx 的类型描述符是 const auto&，把 x 理解成函数 f 调用的参数 expr。

```c++
const auto& rx = x
```

对应模板类型推导根据 ParamType 类型分3种场景进行推导，使用 auto 对变量进行申明的类型推导，也根据变量的类型描述符分3种场景进行推导：
  1. Case 1：类型描述符是一个指针或者引用，但不是万能引用。
  2. Case 2：类型说明符是一个万能引用。
  3. Case 3：类型说明符既不是指针也不是引用。

Item1 的方法适用这3种场景，下面举例说明。

**Case 1：类型描述符是一个指针或者引用，但不是万能引用。**

```c++
int x = 27;
const int cx = x;
const int& rcx = x;

auto& y = x;   // y 的类型为 int&
auto& y1 = cx;  // y1 的类型为 const int&
auto& y2 = rcx; // y2 的类型为 const int&
```

**Case 2：类型说明符是一个万能引用。**
```c++
int x = 27;
const int cx = x;
const int& rcx = x;

auto&& y = 27;    // y 的类型为 int&&
auto&& y1 = x;    // y1 的类型为 int&
auto&& y2 = cx;   // y2 的类型为 const int&
auto&& y3 = rcx;  // y3 的类型为 const int&
```

**Case 3：类型说明符既不是指针也不是引用。**

```c++
int x = 27;
const int cx = x;

auto y = 27;        // y 的类型为 int
auto y1 = x;        // y1 的类型为 int
auto y2 = cx;       // y2 的类型为 int
const auto y3 = x;  // y3 的类型为 const int
const auto y4 = cx; // y4 的类型为 const int
```

Item1 中也讨论了数组和函数名退化成指针的情况，也同样适用与 auto 的类型推导：

```c++
const char name[] = "R. N. Briggs";

auto y1 = name;       // y1 类型为 const char*
auto& y2 = name;      // y2 类型为 const char (&) [13]

void someFunc(int, double);

auto f1 = someFunc;    // f1 类型为 void (*)(double, int)
auto& f2 = someFunc;   // f2 类型为 void (&)(int, double)
```

**特殊场景：初始化列表 std::initial izer_list**

对于变量初始化，如下：

```c++
int x1 = 27;
int x2(27);
int x3 = { 27 };
int x4{ 27 };
```

x3 和 x4 使用的是初始化列表的方式进行初始化，x1~x4 的类型都是int类型。但是，Item5 中将会解释为什么使用 auto 申明特定类型的变量会具有优势，这里将 int 换成 auto：

```c++
auto x1 = 27;     // type is int, value is 27
auto x2(27);      // type is int, value is 27
auto x3 = { 27 }; // type is std::initializer_list<int>,
                  // value is { 27 }
auto x4{ 27 };    // type is std::initializer_list<int>,
                  // value is { 27 }
```

x1 和 x2 还是 int 类型，但是 x3 和 x4 却是 std::initial izer_list<int> 类型，并包含一个元素 27。这是变量申明 auto 类型推导的特殊之处：当使用 auto 申明一个变量 ，并且使用大括号的方式初始化变量，变量的类型推导为 std::initial izer_list 类型。

但是，下面的初始化方式会失败：

```c++
auto x5 = { 1, 2, 3.0 };   // error! can't deduce T for std::initializer_list<T>
```

因为这里实际上包含了两种类型推导，首先 x5 的类型被推导成 std::initializer_list，由于 std::initializer_list 是一个模板，然后必须为 std::initializer_list<T> 实例化一个 T，也就是说 T 也要被推导。这里里表里包含了两种数据类型，T 推导失败。

这是 auto 类型推导和模板类型推导的区别，传递这样的初始化列表给模板将导致推导失败：

```c++
template<typename T>
void f(T param);

f({ 11, 23, 9 }); // error! can't deduce type for T
```

但是，如果你指定了模板参数类型为 std::initializer_list<T> ，模板类型推导将推导出 T：

```c++
template<typename T>
void f(std::initializer_list<T> initList);

f({ 11, 23, 9 });   // T deduced as int, and initList's type is std::initializer_list<int>
```

在 Item3 中你将看到 C++14 允许 auto 作为函数返回值，并且可以被推导。然后那是利用了模板推导，并不是 auto 推导，因此 auto 作为函数返回值时不允许返回一个大括号初始化列表，将会编译失败：

```c++
auto createInitList()
{
   return { 1, 2, 3 };  // error: can't deduce type for { 1, 2, 3 }
}
```

对于 lambda 函数也是一样：

```c++
std::vector<int> v;
...
auto resetV = [&v](const auto& newValue) { v = newValue; };  // C++14
...
resetV({ 1, 2, 3 });   // error: can't deduce type for { 1, 2, 3 }
```

{{<admonition quote "总结" false>}}
- auto 类型推导除了大括号初始化列表方式外，和模板类型推导方法一致。模板类型推导不支持 std::initializer_list。
- 函数返回值为 auto 时，实际是使用模板推导，不是 auto 类型推导。
{{</admonition>}}

#### [Item 3: Understand decltype.](https://blog.csdn.net/Dong_HFUT/article/details/122745518)

给定一个名字或者表达式，decltype 返回名字或者表达式的类型。大多数情况下，decltype 都能返回你所期望的结果，但也有一些特殊情况。

下面给出一些典型场景下，decltype 的返回结果，正如你所期望的那样：

```c++
const int i = 0; // decltype(i) is const int

bool f(const Widget& w); // decltype(w) is const Widget&
                         // decltype(f) is bool(const Widget&)

struct Point {
int x, y;   // decltype(Point::x) is int
};          // decltype(Point::y) is int

Widget w; // decltype(w) is Widget

if (f(w)) ... // decltype(f(w)) is bool

template<typename T>    // simplified version of std::vector
class vector {
public:
...
T& operator[](std::size_t index);
...
};

vector<int> v;  // decltype(v) is vector<int>
...
if (v[0] == 0) ...   // decltype(v[0]) is int&
```

在C++11中，可能 decltype 基本的应用就是申明函数模板，它的返回值类型取决于其参数类型。例如，我们有这样一个函数，传入一个容器和下标，期望在函数末尾返回下标指向的值，并且保留返回值的类型。由于容器的 operatpr[] 通常返回一个 T& （但是，std::vector<bool>，opertor[] 返回的不是一个 bool&，这个我们在 Item6 中再讨论），我们期望在函数末尾返回的也是引用类型，以便于我们对其进一步赋值。使用 decltype 达成这样的目标：

```c++
template<typename Container, typename Index>  // works, but requires refinement
auto authAndAccess(Container& c, Index i)
-> decltype(c[i])
{
  authenticateUser();
  return c[i];
}
```

这里使用了拖尾返回值类型的语法，也即在箭头后声明返回值类型，这里可以返回我们期望的引用类型。

C++14可以省略拖尾返回值的声明，这意味着 auto 需要通过编译器根据函数的实现进行类型推导得到：

```c++
template<typename Container, typename Index>   // C++14; not quite correct
auto authAndAccess(Container& c, Index i)
{
  authenticateUser();
  return c[i];    // return type deduced from c[i]
}
```

但是 Item1 和 Item2 的推导规则告诉我们，在模板类型推导中，表达式初始化的引用属性将被忽略。考虑下面的代码片段：

```c++
std::deque<int> d;
...
authAndAccess(d, 5) = 10;   // authenticate user, return d[5], then assign 10 to it; this won't compile!
```

这里，d[5] 是一个 int&，但是根据 auto的类型推导，authAndAccess 函数将会返回一个 int 类型，作为函数返回值，即一个 int 类型的值，这是一个右值，那么对其进行赋值是非法的。

为了得到我们期望的结果，也即返回引用类型，我们需要使用 decltype 的类型推导：decltype(auto)，auto 表示类型需要推导，decltype 表示使用 decltype 规则进行推导，我们修改函数实现如下：

```c++
template<typename Container, typename Index>   // C++14; works, but still requires refinement
decltype(auto) authAndAccess(Container& c, Index i)
{
  authenticateUser();
  return c[i];
}
```

这里，authAndAccess 将真正返回 c[i] 的类型，也即 T&。

decltype(auto) 不限于用在函数返回值，也可以用在变量申明，例如：

```c++
Widget w;
const Widget& cw = w;
auto myWidget1 = cw;    // auto type deduction: myWidget1's type is Widget

decltype(auto) myWidget2 = cw;   // decltype type deduction: myWidget2's type is const Widget&
```

还有两点需要进一步讨论下，第一点是上面提到的 authAndAccess 还可以进一步优化的事情。让我们再看下 authAndAccess 的声明：

```c++
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container& c, Index i);
```
这里，Container 传参是一个 lvalue-reference-to-non-const，返回值的类型是可以被修改的，这也就意味着我们不能传右值 containers，右值不能绑定到左值引用（除非是 lvalue-references-to-const ）。

一个比较好的解决方案是万能引用！如下：
```c++
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container&& c, Index i);    // c is now a universal reference
```

在这个模板中，既可以传左值，也可以传右值。但是这里有一个不好地方上是：值传递会造成不必要的拷贝。

为了遵循 Item25 中的警告，我们需要修改下如下：

```c++
template<typename Container, typename Index>  // final C++14 version
decltype(auto)
authAndAccess(Container&& c, Index i)
{
  authenticateUser();
  return std::forward<Container>(c)[i];
}
```

以上需要 C++14 的编译器，C++11 版本的写法：

```c++
template<typename Container, typename Index>   // final C++11 version
auto authAndAccess(Container&& c, Index i)
-> decltype(std::forward<Container>(c)[i])
{
  authenticateUser();
  return std::forward<Container>(c)[i];
}
```

另一问题是开头说的少数场景下，decltype 可能返回不是你所预期的结果。对于一个左值表达式 expr，如果 expr 的类型是 T，decltype(expr) 返回的是 T&。

```c++
int x = 0;
```

decltype(x) 是 int。但是对于 (x) 是一个左值表达式，则decltype((x)) 得到 int&。

```c++
decltype(auto) f1()
{
  int x = 0;
  ...
  return x;    // decltype(x) is int, so f1 returns int
}

decltype(auto) f2()
{
  int x = 0;
  ...
  return (x);    // decltype((x)) is int&, so f2 returns int&
}
```

f2 和 f1 的返回值不同，f2 返回了一个局部变量的引用，这是相当危险的。

因此，在使用 decltype 的时候，建议使用 Item4 中的方法进行检查，确保得到你期望的类型推导。

{{<admonition quote "总结" false>}}
- decltype几乎总是能够得出变量或者表达式的类型。
- 对于类型为 T 的左值表达式，而不是名字，decltype 基本上总是输出 T&。
- C++14支持 delctype(auto)，像是 auto，能够自动从初始化列表中推断出类型，但它使用的是decltype 的推断规则。
{{</admonition>}}


#### [Item 4: Know how to view deduced types.](https://blog.csdn.net/Dong_HFUT/article/details/122774872)

从前面的介绍我们知道，类型推导的结果有时候并不是非常明显就能得到，为了安全起见，我们最好能够自己亲自验证下类型推导是否符合我们的预期，在以下3个阶段我们能够获得一些类型推导的信息：
  - 代码编辑阶段
  - 编译阶段
  - 运行阶段

**IDE Editors**

一般地，在编辑代码时候，你的IDE经常可以展示一些变量、函数、参数等类型。比如如下代码，当你使用鼠标指向 y1 和 y2 时候，会显示 y1 为 int 类型、y2 为 const int* 类型。

```c++
#include <iostream>

int main() {
    const int x = 2;
    auto y1 = x;
    auto y2 = &x;
    return 0;
}
```

IDE 之所以能够知道类型推导的结果，是因为 IDE 背后运行了 C++ 的编译器（至少是编译器的前端），这就要求你的代码或多或少是可编译状态。

对于简单的类型，IDE 的显示的信息通常是有用的，但是对于复杂类型，可能它的推导类型就不准了。

**Compiler Diagnostics**

可以通过导致编译错误的方式获取类型推导的信息，编译报错信息是很好的提示信息：

```c++
#include <iostream>

template<typename T>
class TD;

int main() {
    const int x = 2;
    auto y1 = x;
    auto y2 = &x;

    TD<decltype(y1)> y1Type;
    TD<decltype(y2)> y2Type;

    return 0;
}
```
上面的代码片段，实例化模板将导致编译报错，报错信息如下：
```shell
main.cpp:11:22: error: aggregate ‘TD<int> y1Type’ has incomplete type and cannot be defined
     TD<decltype(y1)> y1Type;
                      ^~~~~~
main.cpp:12:22: error: aggregate ‘TD<const int*> y2Type’ has incomplete type and cannot be defined
     TD<decltype(y2)> y2Type;
```

从上面的报错信息可以获取到类型推导信息。

**Runtime Output**

通过允许下时打印类型是比较准确和靠谱的。第一种方式借助 typeid 控制输出：

```c++
#include <iostream>

int main() {
    const int x = 2;
    auto y1 = x;
    auto y2 = &x;

    std::cout << "y1: " << typeid(y1).name() << std::endl;
    std::cout << "y1: " << typeid(y2).name() << std::endl;
    return 0;
}
```

打印信息：

```shell
y1: i
y1: PKi
```

i 代表 int 类型， PKi 代码 pointor to const int。

这看起来还好，但是考虑下面的代码：

```c++
template<typename T>                // template function to be called
void f(const T& param);

std::vector<Widget> createVec();    // factory function

const auto vw = createVec();        // init vw w/factory return

if (!vw.empty()) {
  f(&vw[0]);                        // call f
  ...
}
```

使用 typeid 来输出 T 和 param 的类型，不论是哪个编译器，你都会得到错误的结果。例如微软的编译器会告诉你：二者的类型都是 class Widget const*。但是，param 的类型比 T 多一个 const &。这是由于我们这里 typeid 是值传递的，在用模板规则推导类型时，由 Item1 可知会丢失 const T& 中的 const 和引用属性。

幸好还有另一种方法，使用 Boost 库提供的方法，可以得到可靠的类型推导信息：

```c++
#include <boost/type_index.hpp>
template <typename T>
void f(const T& param)
{
    using std::cout;
    using boost::typeindex::type_id_with_cvr;
    cout << "T =     "
         << type_id_with_cvr<T>().pretty_name()
         << '\n';
                        `
    cout << "param = "
         << type_id_with_cvr<decltype(param)>().pretty_name();
         << '\n';
    ...`
}
```

模板函数 boost::typeindex::type_id_with_cvr 使用我们传递的类型实例化，而且它不会删去该类型已有的 const、volatile 和引用语义（所以后缀带有 “with_cvr”）。这个模板函数的返回结果是个boost::typeindex::type_index 对象，该对象的成员函数 pretty_name 会生成非常友好的 std::string来表示这个类型。

在 GNU 和 Clang 的编译器下运行结果为：

```text
T =         Widget const*
param = Widget const* const&
```
在微软的编译器下运行的结果一致：

```text
T =         class Widget const *
param = class Widget const * const &
```

{{<admonition quote "总结" false>}}
- 通常可以使用 IDE 编辑器、编译器报错信息和 Boost TypeIndex 库来查看已推断的类型。
- 一些工具的结果可能没有帮助或者不准确，还是要理解透彻 C++ 的类型推断规则。
{{</admonition>}}

### CH02: auto

#### [Item 5: Prefer auto to explicit type declarations.](https://blog.csdn.net/Dong_HFUT/article/details/122785263)

C++ 显式声明可能会产生例如变量未初始化、申明类型冗长、无法形成闭包、截断等问题，通过使用 auto 可以很好解决这些问题。

**避免变量未初始化**

```c++
int x1; // potentially uninitialized
auto x2; // error! initializer required
auto x3 = 0; // fine, x's value is well-defined
```
x1 未初始化，其值可能是未定义的，这导致程序中可能隐藏着 bug。使用 auto 声明的变量未初始化将导致编译报错。

**简化变量申明**

考虑下面的代码，通过解引用迭代器初始化局部变量：
```c++
template<typename It>       // algorithm to dwim ("do what I mean")
void dwim(It b, It e)       // for all elements in range from
{                           // b to e
  while (b != e) {
	typename std::iterator_traits<It>::value_type
	  currValue = *b;
	…
 }
}
```
使用 auto 简化上述代码：

```c++
template<typename It>    // as before
void dwim(It b, It e)
{
  while (b != e) {
	auto currValue = *b;
	…
  }
}
```

**申明闭包类型**

在 Item 2 中介绍过 auto 可以通过类型推到得到实际的类型，考虑下面的代码片段：

```c++
auto derefUPLess =                             // comparison func.
	[](const std::unique_ptr<Widget>& p1,      // for Widgets
	   const std::unique_ptr<Widget>& p2)      // pointed to by
	{ return *p1 < *p2; };                     // std::unique_ptrs
```
C++14 lambda 函数参数也可以使用 auto，代码简化如下：

```c++
auto derefLess =            // C++14 comparison
	[](const auto& p1,      // function for
	   const auto& p2)      // values pointed
	{ return *p1 < *p2; };  // to by anything
	                        // pointer-like
```
使用 auto 的 derefLess 是一个闭包类型。也可以使用 std::function 得到闭包类型：

```c++
  std::function<bool(const std::unique_ptr<Widgey>&,
                     const std::unique_ptr<Widget>&)>
    derefLess = [] (const std::unique_ptr<Widget>& p1,
                    const std::unique_ptr<Widget>& p2)
                    { return *p1 < *p2; };
```


auto 申明的持有闭包的变量和闭包有相同的类型，并且仅使用闭包需要的内存大小。而 std::function 声明持有闭包的变量有一个固定大小内存，一旦内存大小不足，则需要申请堆内存来存储闭包。因此，std::function 申明的对象要比 auto 申明的对象占更多的内存，由于约束内嵌的使用和提供间接函数的调用，通过 std::function 对象来调用一个封装体比通过 auto 对象要慢。也就是说，std::function 方法通常体积比 auto 大且慢，还有可能导致内存不足的异常。

**避免类型截断**

auto 还有一个避免内存截断的优点，考虑下面的代码片段：
```c++
std::vector<int> v;
…
unsigned sz = v.size();
```
v.size() 返回类型是 std::vector::size_type ，一个无符号整数类型，很多程序员可能会写出上面的代码。std::vector::size_type 在 32 位机器上是 4个字节，但在 64 位机器上则为 8 字节，但是 unsigned 固定为 4 字节。上面的代码在 32 位机器上运行没有什么问题，但移植到 64 位机器上则会导致类型截断的问题。

**避免类型不匹配**

auto 还具有一个非常隐蔽的效果，看下面的代码：

```c++
std::unordered_map<std::string, int> m;
...
for (std::pair<std::string, int>& p : m)
{
  ... // do something with p
}
```

看上去没有什么问题？我们很有可能写出以上的迭代代码，但是 unordered_map 的 key 是 const的，即 hash map 中 std::pair 的类型是 std::pair<const std::string, int>。下面的代码将产生编译报错：

```c++
#include <unordered_map>
#include <iostream>

int main() {
  int p;
  std::unordered_map<std::string,int> m;
  m["key"]  = 10;
  for(std::pair<std::string,int>&p : m) {
    p.second  = 2;
  }
  return 0;
}

// 编译报错
main.cpp:8:38: error: invalid initialization of reference of type 'std::pair<std::__cxx11::basic_string<char>, int>&' from expression of type 'std::pair<const std::__cxx11::basic_string<char>, int>'
    8 |   for(std::pair<std::string,int>&p : m) {
```

不知道这个隐蔽的背景知识的情况下，使用 auto 替代则会避免上述问题：

```c++
for (auto& p : m)
{
  … // as before
}
```

上面介绍了几条使用 auto 的优势， 使用 auto 也有 Item 2 和 Item 6 介绍的使用陷阱，但是可以通过 Item 4 介绍的一些方法可视化其类型推导结果，auto 还是非常值得使用的。

{{<admonition quote "总结" false>}}
- auto 变量必须初始化，不受类型不匹配导致移植和效率问题。
- auto 类型也受 Item2 和 Item6 中介绍的陷阱困扰。
{{</admonition>}}


#### [Item 6: Use the explicitly typed initializer idiom when auto deduces undesired types.](https://blog.csdn.net/Dong_HFUT/article/details/122800470)

在 Item 5 中介绍了使用 auto 申明类型的优势，也在 Item 2 介绍了 auto 类型推导的方式和 auto 类型推导有时候并非如我们所愿的情况，本文继续分析使用 auto 存在的问题。

看下面的例子，函数 features 入参为 Widget 类型，返回一个 std::vector<bool>，每一个 bool 代表 Widget 是否提供一个特殊的特性：

```c++
std::vector<bool> features(const Widget& w);
```

假设 bit 5 代表是否有高优先级，可能编码如下：

```c++
Widget w;
…
bool highPriority = features(w)[5];  // is w high priority?
…
processWidget(w, highPriority);     // process w in accord
                                    // with its priority
```

如果修改 highPriority 显示申明为 auto ：

```c++
auto highPriority = features(w)[5];  // is w high priority?
```

将导致 processWidget 出现不可预测的行为，这是为什么呢？对 vector <T> 的 operator [] 操作，一般我们期望得到 T& 类型。但是对于 vector <bool> 的 operator [] 操作，得到的是 std::vector<bool>::reference 类型，却不是 bool& 类型。

为什么会有std::vector<bool>::reference 类型呢？主要是以下几个原因：

  1. 为了节省空间，使用 1 个 bit 代替 1 个字节的 bool 类型。
  2. std::vector<T> 的 operator [] 操作应该返回的是 T&， 但标准库无法返对 bit 的引用。
  3. 为了得到接近 bool& 的类型，std::vector<bool>::reference 对象能够使用在 bool& 可以使用的场景。

由于以上几点，再看下这段代码：

```c++
bool highPriority = features(w)[5];  // is w high priority?
```

这里，features 返回的是 std::vector<bool> 对象，然后施加 operator [] 操作得到 std::vector<bool>::reference 对象， 最后被隐式转化为 bool 类型来初始化 highPriority，highPriority 得到 std::vector<bool> 中 bit 5 的值。

```c++
auto highPriority = features(w)[5]; // deduce highPriority's type
```

将 bool 换成 auto，由于 auto 的自动类型推导，最后无法得到 std::vector<bool> 中 bit 5 的值。具体是什么值取决于 std::vector<bool>::reference 的实现。std::vector<bool>::reference 的一种实现是这样的：这个对象包含一个指针，这个指针指向一个 word（word 存储了引用的 bit ，加上 word 中 bit 的 offset。在这里，features 返回的是一个 std::vector<bool> 临时对象 temp，operator [] 操作后，得到 std::vector<bool>::reference ，它包含一个指向 temp 中 word 的指针，加上 bit 5 的偏移。因此，在语句结尾，临时的 temp 被销毁，highPriority 包含了一个野指针，这将导致随后的 processWidget 函数的不可预测的行为。

这里可以使用显示类型初始化方式使用 auto ：

```c++
auto highPriority = static_cast<bool>(features(w)[5]);
```

std::vector<bool>::reference 是代理类（proxy class）的一个例子，代理类的目的是为了模拟和扩展某些其他类型行为，具有广泛的使用，比如标准库的智能指针类型。

在表达式模板中也使用代理类的技术，能够提高数值计算的效率。比如给定一个类 Matrix 和 Matrix 的对象 m1, m2, m3 和 m4，以下表达式：

```c++
Matrix sum = m1 + m2 + m3 + m4;
```

operator + 返回一个代理类对象将更加高效。operator + 两个 Matrix 对象得到代理 Sum<Matrix, Matrix> 对象，而不是Matrix 对象，上面的表达式将得到 Sum<Sum<Sum<Matrix, Matrix>,Matrix>, Matrix> 对象，最后隐式转化为为 Matrix 对象。

这种隐形的代理类遇到 auto 时候往往得不到预期的结果。因此，一般防止出现以下语句：

```c++
auto someVar = expression of "invisible" proxy class type;
```

一种方式是使用显式的类型申明，一种方式是使用 static_cast 显示类型初始化：

```c++
bool highPriority = features(w)[5];
auto highPriority = static_cast<bool>(features(w)[5]);

Matrix sum = m1 + m2 + m3 + m4;
auto sum = static_cast<Matrix>(m1 + m2 + m3 + m4);
```
这样的习惯不仅用于**代理类**的情况，在**定义一个变量的类型及其初始化表达式类型不同**时，也最好显式申明变量类型，或者显示类型初始化。例如：

```c++
double calcEpsilon(); // return tolerance value

float ep = calcEpsilon(); // impliclitly convert double → float
auto ep = static_cast<float>(calcEpsilon());
```

{{<admonition quote "总结" false>}}
- 隐式的代理类型可能导致 auto 类型推导结果不符合预期。
- 对于这种代理类型一般使用显式类型申明或者显示类型初始化。
{{</admonition>}}


### CH03: Moving to Modern C++

#### [Item 7: Distinguish between () and {} when creating objects.](https://blog.csdn.net/Dong_HFUT/article/details/122811753)

**初始化方式**
C++11开始变量初始化方式有以下几种：
```c++
int x(0);      // initializer is in parentheses
int y = 0;     // initializer follows "="
int z{ 0 };    // initializer is in braces
int z = { 0 }; // initializer uses "=" and braces
```
其中第四种使用等号和花括号方式初始化变量通常认为和第三种花括号的方式相同。使用等号进行初始化可能会被认为是赋值操作，对于内置类型（比如 int），可以忽略它们的区别，但对于用户自定义类型，则需要区别：

```c++
Widget w1;      // call default constructor
Widget w2 = w1; // not an assignment; calls copy ctor
w1 = w2;        // an assignment; calls copy operator=
```

**使用 {} 初始化被称为统一初始化（uniform initialization），期望能够统一应用在所有初始化场景**（实际上也有缺陷，后文将介绍）。

**() 和 = 初始化方式的限制**

圆括号不能用于非静态成员变量的默认初始化：

```c++
class Widget {
...
private:
  int x{ 0 };  // fine, x's default value is 0
  int y = 0;   // also fine
  int z(0);    // error!
};

// 编译报错信息
main.cpp:10:11: error: expected identifier before numeric constant
   10 |     int z(0);    // error!
```

成员变量 z 的初始化将会导致编译报错。

另外，C++中不能拷贝的对象则不能使用等号初始化：

```c++
std::atomic<int> ai1{ 0 };
std::atomic<int> ai2(0);  // fine
std::atomic<int> ai3 = 0; // error!

// 编译报错信息
main.cpp:5:24: error: use of deleted function ‘std::atomic<int>::atomic(const std::atomic<int>&)’
   std::atomic<int> a = 0;
                        ^
```

= 和 () 初始化都有使用的限制，可能就是 {} 初始化被称为统一初始化的原因吧。

**{} 初始化的优势**

统一初始化可以避免隐式窄化转换（narrowing conversions）：

```c++
double x, y, z;

int sum1{ x + y + z };    // error! sum of doubles may not be expressible as int
int sum2(x + y + z);      // okay (value of expression truncated to an int)
int sum3 = x + y + z;     // ditto
```

统一初始化另外一个好处是避免了 C++ 复杂的语法分析（most vexing parse）：

```c++
Widget w2();    // most vexing parse! declares a function named w2 that returns a Widget!
Widget w1(10);  // call Widget ctor with argument 10
Widget w3{};    // calls Widget ctor with no args
```

第一个例子的问题可以戳 [C++‘s most vexing parse](https://blog.csdn.net/Dong_HFUT/article/details/126435993?spm=1001.2014.3001.5501) 了解更多。

**{}初始化的不足**

除了 Item 2 介绍的 auto 变量类型声明使用统一初始化时候类型被推导成 std::initializer_list 的特点外，还存在统一初始化和其他初始化行为不一致的情况。

在没有 std::initializer_list 参数类型的构造函数时：

```c++
class Widget {
public:
  Widget(int i, bool b);    // ctors not declaring
  Widget(int i, double d);  // std::initializer_list params
  ...
};

Widget w1(10, true);  // calls first ctor
Widget w2{10, true};  // also calls first ctor
Widget w3(10, 5.0);   // calls second ctor
Widget w4{10, 5.0};   // also calls second ctor
```

在增加一个std::initializer_list 参数类型的构造函数时：

```c++
class Widget {
public:
  Widget(int i, bool b);                              // as before
  Widget(int i, double d);                            // as before
  Widget(std::initializer_list<long double> il);      // added
  ...
};

Widget w1(10, true);    // uses parens and, as before,
                        // calls first ctor
Widget w2{10, true};    // uses braces, but now calls
						// std::initializer_list ctor
						// (10 and true convert to long double)
Widget w3(10, 5.0);     // uses parens and, as before,
                        // calls second ctor
Widget w4{10, 5.0};     // uses braces, but now calls
                        // std::initializer_list ctor
                        // (10 and 5.0 convert to long double)
```

这里，w2 和 w4 将会使用新增的构造函数（第3个构造函数）。但是很明显， non-std::initializer_list 参数类型构造函数比std::initializer_list 参数类型构造函数更加匹配。

更有甚者，拷贝和移动构造函数也能被 std::initializer_list 构造函数绑架：

```c++
class Widget {
public:
  Widget(int i, bool b);    // as before
  Widget(int i, double d);  // as before
  Widget(std::initializer_list<long double> il);  // as before
  operator float() const;   // convert to float
  ...
};

Widget w5(w4);    // uses parens, calls copy ctor
Widget w6{w4};    // uses braces, calls std::initializer_list ctor
                  // (w4 converts to float, and float converts to long double)
Widget w7(std::move(w4));  // uses parens, calls move ctor
Widget w8{std::move(w4)};  // uses braces, calls std::initializer_list ctor
                           // (for same reason as w6)
```

编译器匹配 std::initializer_list 构造函数的决心很强，甚至导致编译报错，也没有匹配到普通的构造函数：

```c++
class Widget {
public:
  Widget(int i, bool b);   // as before
  Widget(int i, double d); // as before
  Widget(std::initializer_list<bool> il); // element type is now bool
  ...                                     // no implicit conversion funcs
};

Widget w{10, 5.0};   // error! requires narrowing conversions
```

这里，编译器直接忽略前两个构造函数，试图匹配 std::initializer_list<bool> 构造函数，但是需要将 int (10) 和 double (5.0) 转换为 bool 类型，这是窄化转化，将会失败（前面有解释），这里就导致错误。

只有花括号中参数无法转换为 std::initializer_list 中类型时，编译器才匹配普通函数：

```c++
class Widget {
public:
  Widget(int i, bool b);    // as before
  Widget(int i, double d);  // as before
  // std::initializer_list element type is now std::string
  Widget(std::initializer_list<std::string> il);
};

Widget w1(10, true); // uses parens, still calls first ctor
Widget w2{10, true}; // uses braces, now calls first ctor
Widget w3(10, 5.0);  // uses parens, still calls second ctor
Widget w4{10, 5.0};  // uses braces, now calls second ctor
```

统一初始化除了存在上述问题，还有一些边界 case 需要处理下，看下面的例子：

```c++
class Widget {
public:
  Widget();                              // default ctor
  Widget(std::initializer_list<int> il); // std::initializer_list ctor
};

Widget w1;   // calls default ctor
Widget w2{}; // also calls default ctor
Widget w3(); // most vexing parse! declares a function!
```

w2 将会调用默认构造函数，而没有调用选择调用 std::initializer_list 构造函数并将 list 设置为空，这是一个特例。如果希望调用 std::initializer_list 构造函数， 如下：

```c++
Widget w4({}); // calls std::initializer_list ctor with empty list
Widget w5{{}}; // ditto
```

了解了以上 {} 和 () 的初始化的一些不足之后，我们再看下标准库的情况，对于新手，很容易以为下面两种方式创建的对象是相同的。

```c++
std::vector<int> v1(10, 20); // use non-std::initializer_list ctor: create 10-element
                             // std::vector, all elements have value of 20
std::vector<int> v2{10, 20}; // use std::initializer_list ctor: create 2-element std::vector,
                             // element values are 10 and 20
```
再看下面的例子：
```c++
template<typename T,                // type of object to create
          typename... Ts>            // types of arguments to use
  void doSomeWork(Ts&&... params)
  {
    create local T object from params...
    ...
}
```

将上面的伪代码替换成下面两种创建对象的方式：
```c++
T localObject(std::forward<Ts>(params)...); // using parens
T localObject{std::forward<Ts>(params)...}; // using braces
```
再考虑下面的调用代码：
```c++
std::vector<int> v;
...
doSomeWork<std::vector<int>>(10, 20);
```
如果 doSomeWork 采用圆括号的实现，结果将是 10 个元素的 std::vector。如果 doSomeWork 采用花括号的实现，结果将是 2 个元素的 std::vector。

标准库中的 std::make_shared 和 std::make_unique 使用圆括号初始化，并且在代码中做了注释。这类问题没有什么好的解决方案，只能在代码和接口中添加注释来告知调用者。

{{<admonition quote "总结" false>}}
- 括号初始化是最广泛使用的初始化语法，它防止窄化转换，并且对于 C++ 最令人头疼的解析有天生的免疫性。
- 在构造函数重载决议中，括号初始化尽最大可能与 std::initializer_list 参数匹配，即便其他构造函数看起来是更好的选择。
- 对于数值类型的std::vector来说使用花括号初始化和圆括号初始化会产生巨大的不同。
- 在模板类选择使用小括号初始化或使用花括号初始化创建对象是一个挑战。
{{</admonition>}}

#### [Item 8: Prefer nullptr to 0 and NULL.](https://blog.csdn.net/Dong_HFUT/article/details/122891898)

在 C++ 中，字面值 0 是一个 int 类型，不是一个指针类型，虽然 C++ 根据上下文可以将字面值 0 解释成一个空指针，但本质上，请注意字面值 0 是一个 int 类型。

实际上，NULL 的情况也一样，它也不是一个指针类型，根据实现情况来定，可以被允许实现为 long 类型，但本质上也不是一个指针类型。

字面值 0 和 NULL 不是指针类型这一事实会导致了一些让人困惑的场景，比如在 C++98 下：

```c++
void f(int);    // three overloads of f
void f(bool);
void f(void*);
f(0);           // calls f(int), not f(void*)
f(NULL);        // might not compile, but typically calls f(int). Never calls f(void*)
```

f(0) 会匹配到第一个，不会匹配 f(void*) 。NULL 的实现是 0L，可以转换成 int，bool 和 void* ，这会导致匹配失败，除非有一个 f(long) 的重载。对于程序员来说，f(NULL) 预想的是调用 f(void*)，但 C++ 实际上却去匹配参数为整型的 f，这是违反直觉的。因此， 对于 C++98 的程序员，最好避免使用指针类型重载整型。到了 C++11，上面的这个建议依然有效，因为开发者很有可能继续使用 0 和 NULL 作为空指针。

在 C++11 后，建议大家使用 nullptr 作为空指针。nullptr 不是一个整型，它也不是一个确定的指针类型，可以把它理解为任意类型的指针，它的准确类型是 std::nullptr_t，可以隐式转换为任意指针类型的类型。用 nullptr 代替 0 和 NULL，可以使得重载函数的调用非常明确。

```c++
f(nullptr);  // calls f(void*) overload
```

使用 nullptr 的另一个优势是可以提高代码的清晰度，尤其是使用 auto 变量时：

```c++
auto result = findRecord( /* arguments */ );
  if (result == 0) {
  ...
}
```

这里，如果你不清楚 findRecord 返回值的类型时，你可能就不清楚 result 是一个指针类型还是一个整型。但如果使用 nullptr 代替 0，代码将更加清晰，result 一定是一个指针类型，就不会模棱两可了：

```c++
auto result = findRecord( /* arguments */ );
  if (result == nullptr) {
  ...
}
```

在我们使用模板时候，nullptr 的优势将更加引人注目。假设你有这样的一些函数，只有当对应的互斥量被锁定的时候，这些函数才可以被调用，每个函数的参数是不同类型的指针：

```c++
int f1(`std::shared_ptr`<Widget> spw);    // call these only when the appropriate mutex is locked
double f2(std::unique_ptr<Widget> upw);
bool f3(Widget* pw);
```

传空指针调用这些函数可能是这样的：

```c++
std::mutex f1m, f2m, f3m; // mutexes for f1, f2, and f3

using MuxGuard = std::lock_guard<std::mutex>;  // C++11 typedef; see Item 9
...

{
  MuxGuard g(f1m);      // lock mutex for f1
  auto result = f1(0);  // pass 0 as null ptr to f1 unlock mutex
}
...

{
  MuxGuard g(f2m);         // lock mutex for f2
  auto result = f2(NULL);  // pass NULL as null ptr to f2 unlock mutex
}
...

{
  MuxGuard g(f3m);            // lock mutex for f3
  auto result = f1(nullptr);  // pass nullptr as null ptr to f3 unlock mutex
}

```

虽然前面两个调用没有使用 nullptr，但是还可以工作。但是上面代码最大的问题是调用代码没有复用，引入模板来解决这个问题：

```c++
template<typename FuncType,
		 typename MuxType,
		 typename PtrType>
auto lockAndCall(FuncType func,
				 MuxType& mutex,
				 PtrType ptr) -> decltype(func(ptr))
{
  MuxGuard g(mutex);
  return func(ptr);
}
```

在 C++ 14 中可以使用 decltype(auto) 代替上面的 —> :

```c++
template<typename FuncType,
		 typename MuxType,
		 typename PtrType>
decltype(auto) lockAndCall(FuncType func,      // C++14
						   MuxType& mutex,
						   PtrType ptr)
{
  MuxGuard g(mutex);
  return func(ptr);
}

```

对于上面的两个版本，调用代码可以如下：

```c++
auto result1 = lockAndCall(f1, f1m, 0);        // error!
...
auto result2 = lockAndCall(f2, f2m, NULL);     // error!
...
auto result3 = lockAndCall(f3, f3m, nullptr);  // fine
```

前面的两种调用将会失败。在第一个调用中，将 0 传入 lockAndCall，模板类型推导将得到它是一个 int，而 f1 期望接收的是 std::share_ptr<Widget> ，匹配失败。对于第二个调用也是类似的。第三个调用传入 nullptr 是没有问题的，当 nullptr 被传入时，ptr 的类型被推导为 std::nullptr_t ，std::nullptr_t 可以隐式转化为任意类型指针，因此能够和 f3 匹配成功。

上面的例子使用 nullptr 的优势非常明显，因此使用 nullptr 代替 0 和 NULL 吧！

{{<admonition quote "总结" false>}}
- 相较于 0 和 NULL，优先使用 nullptr 。
- 避免对整数类型和指针类型的重载。
{{</admonition>}}


#### [Item 9: Prefer alias declarations to typedefs.](https://blog.csdn.net/Dong_HFUT/article/details/122847510)

C++98 提供了给类型重新命名的功能：

```c++
typedef
std::unique_ptr<std::unordered_map<std::string, std::string>>
UPtrMapSS;
```

这在 C++11 依然能够工作， 但 C++11 提供了新的方式：别名申明（alias declarations）。

```c++
using UPtrMapSS =
std::unique_ptr<std::unordered_map<std::string, std::string>>;
```

仅从以上例子，两种使用方式没有太大区别，下面将介绍别名申明(alias)的优势。

使用别名申明比使用 typedef 更容易理解，特别是申明函数指针的时候：

```c++
// FP is a synonym for a pointer to a function taking an int and
// a const std::string& and returning nothing
typedef void (*FP)(int, const std::string&);  // typedef

// same meaning as above
using FP = void (*)(int, const std::string&);  // alias declaration
```

这个例子只能说使用别名申明的代码略为清晰易懂一点，并不是一个不可抗拒的诱惑。但是，当在使用别名模板（alias templates）时，别名申明可以直接作用在模板申明的作用域。但是，typedef 却不行， 它需要嵌套在自定义结构体的作用域内。例如：

```c++
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>; // MyAllocList<T> is synonym for std::list<T, MyAlloc<T>>
MyAllocList<Widget> lw;  // client code
```

使用 typedef ，你需要定义一个结构体，并将 typedef 放到结构体的作用域中，如下：

```c++
template<typename T>  // MyAllocList<T>::type is synonym for
struct MyAllocList {
  typedef std::list<T, MyAlloc<T>> type; // std::list<T, std::list<T, MyAlloc<T>>
};
MyAllocList<Widget>::type lw; // client code
```

显然，使用 typedef 已经非常吃力了。并且，如果你在模板中使用 typedef 的类型定义变量时，你还必须在前面加上 typename。

```c++
template<typename T>  // MyAllocList<T>::type is synonym for
struct MyAllocList {
  typedef std::list<T, MyAlloc<T>> type; // std::list<T, std::list<T, MyAlloc<T>>
};

template<typename T>
class Widget {       // Widget<T> contains Widget<T> contains as a data member
private:
  typename MyAllocList<T>::type list;
...
};
```

这里，MyAllocList<T>::type 表示的类型依赖于模板的类型 T。因此， MyAllocList<T>::type 是一个依赖类型(dependent type)。在 C++ 中，一个依赖类型的名称之前必须冠以 typename。而使用别名申明则没有这个问题：

```c++
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>; // MyAllocList<T> is synonym for std::list<T, MyAlloc<T>>

template<typename T>
class Widget {
private:
  MyAllocList<T> list;
  ...
};
```

看上去 MyAllocList<T> （使用模板别名） 也依赖于模板参数 T ，但当编译器在处理 Widget 时，MyAllocList<T> （使用模板别名） 使用了模板别名，它一定是一个类型，因此无需也不能冠以 typename。

但是，对于使用内嵌的 typename 的 MyAllocList<T> ，编译器在处理 Widget 时，并不知道 MyAllocList<T> 是一个类型，可能是一个特殊化的 MyAllocList ，在这个特殊化的 MyAllocList 中， MyAllocList<T>::type 表示的并不是一个类型。看下面的例子：

```c++
class Wine { ... };

template<>
class MyAllocList<Wine> {  // MyAllocList specialization for when T is Wine
private:
  enum class WineType
    { White, Red, Rose };
  WineType type;      // in this class, type is a data member!
  ...
};
```

这里，MyAllocList<Wine>::type 并不是指一个类型，而是一个数据成员。如果 Widget 被使用 Wine 初始化，Widget 模板中的 MyAllocList<T>::type 指的是一个数据成员，而不是一个类型。

虽然，尽管别名申明有以上多个优点，但是 C++11中的 <type_trais> 却大量用了 typedef，C++ 标准委员会意识到使用别名什么更加合适，因此，对于 C++11 中的每个类型转换 std::transformation<T>::type ，有一个对应的 C++14 的模板别名 std::transformation_t ：

```c++
std::remove_const<T>::type  // C++11: const T → T
std::remove_const_t<T>      // C++14 equivalent

std::remove_reference<T>::type  // C++11: T&/T&& → T
std::remove_reference_t<T>      // C++14 equivalent

std::add_lvalue_reference<T>::type  // C++11: T → T&
std::add_lvalue_reference_t<T>      // C++14 equivalent
```

虽然以上两种方式在 C++14 中都存在，但我们没有理由再使用 typedef 的形式。

{{<admonition quote "总结" false>}}
- typedef 不支持模板化，但是别名声明支持。
- 模板别名没有 ::type 后缀，在模板中，typedef 还经常要求使用 typename 前缀。
- C++14 为 C++11 中的类型特征转换提供了模板别名。
{{</admonition>}}

#### [Item 10: Prefer scoped enums to unscoped enums.](https://blog.csdn.net/Dong_HFUT/article/details/122914289)

一般来说，定义在花括号中的对象，其作用域也在花括号内，但是 C++98 风格的枚举（enums）类型却不遵从这一规则：
```c++
enum Color { black, white, red }; // black, white, red are in same scope as Color
auto white = false;               // error! white already declared in this scope
```
上面这种将枚举成员暴露在其花括号作用域外面的方式，官方称为无作用域限制的枚举（unscoped enums）。C++11 提供不暴露枚举成员的枚举类型：有作用域限制的枚举（scoped enums）。

```c++
enum class Color { black, white, red }; // black, white, red are scoped to Color
auto white = false;                     // fine, no other
```

**减少名称污染**
从上面的两个例子，可以看到 scoped enums 可以减少因作用域限制导致的名称污染，这是 scoped enums 的第一个优点。

**有强类型**
scoped enums 的第二个优点是其成员有强类型，不会发生隐式类型转换。看个例子：

```c++
enum Color { black, white, red }; // unscoped enum
std::vector<std::size_t>  primeFactors(std::size_t x);
Color c = red;
…
if (c < 14.5) {       // compare Color to double (!)
  auto factors = primeFactors(c); // compute prime factors of a Color (!)
  …
}
```

scoped enums 则不会发生隐式类型转换，上面的例子的 Color 修改成 scoped enums 方式，则会产生编译报错：

```c++
enum class Color { black, white, red }; // enum is now scoped
Color c = Color::red;
…
if (c < 14.5) {  // error! can't compare Color and double
auto factors = primeFactors(c);  // error! can't pass Color to function expecting std::size_t
…
}
```

如果你一定要将 Color 转换为其他类型，可以使用显示类型转换：

```c++
if (static_cast<double>(c) < 14.5) { // odd code, but it's valid
auto factors = primeFactors(static_cast<std::size_t>(c)); // suspect, but it compiles
…
}
```

**前向声明**

scoped enums 的第三个优点是其可以前向申明，也即枚举名可以在没有指定其成员前进行申明。

```c++
enum Color; // error!
enum class Color; // fine
```

其实，说 unscoped enums 不支持前向申明是不严谨的，其实在 C++11 中，它也可以前向申明，只不过不支持上面例子这样的前向申明，要想让它可以前向申明，还需要进行一点修改而已。所谓的前向申明，其本质就是告诉编译器目标是什么类型。

这里有一个事实，那就是unscoped enums 类型其实不是一个 enums，它的实际类型是编译器进行选择决定的。 scoped enums 之所以能够前向申明，是因为它的默认潜在类型（underlying type）是 int。

```c++
enum Color { black, white, red };
```

对于上面代码，编译可以将 Color 类型选择成 char 就足够了。

```c++
enum Status { good = 0,
              failed = 1,
              incomplete = 100,
              corrupt = 200,
              indeterminate = 0xFFFFFFFF
};
```

但是，对于上面这段代码，编译器可能需要将 Status 类型选择成 int。

总之，对于有作用域的枚举体的潜在类型是已知的，对于没有作用域的枚举体，你可以指定它。

对于 scoped enums ，默认的潜在类型为 int，当然你也可以进行更改：

```c++
enum class Status; // underlying type is int

enum class Status: std::uint32_t; // underlying type for Status is std::uint32_t (from <cstdint>)
```

对于 unscoped enums，指定了潜在类型，则可以前向申明：

```c++
enum Color: std::uint8_t; // fwd decl for unscoped enum; underlying type is std::uint8_t
```

**unscoped enums 的优势**

上面介绍了 scoped enums 比 unscoped enums 的几个优点。其实，unscoped enums 也有它自己的优势。看下面的例子：

```c++
using UserInfo = // type alias; see Item 9
std::tuple<std::string, // name
	       std::string, // email
	       std::size_t> ; // reputation
```

虽然注释说明了元组每个部分代表的含义，但是，当你遇到下面的代码时候，你可能还是记不清楚元组的第一个元素代表的是啥：

```c++
UserInfo uInfo; // object of tuple type
…
auto val = std::get<1>(uInfo); // get value of field 1
```

如果使用 unscoped enums 修改上面的代码，可以避免这个问题：

```c++
enum UserInfoFields { uiName, uiEmail, uiReputation };
UserInfo uInfo; // as before
…
auto val = std::get<uiEmail>(uInfo); // ah, get value of email field
```

这里，UserInfoFields 到 std::get() 要求的 std::size_t 的隐式类型转换。要是使用 scoped enums 则代码要冗余的多：

```c++
enum class UserInfoFields { uiName, uiEmail, uiReputation };
UserInfo uInfo; // as before
…
auto val =
std::get<static_cast<std::size_t>(UserInfoFields::uiEmail)>(uInfo);
```

{{<admonition quote "总结" false>}}
- C++98 风格的 enum 是 unscoped enum 。
- scoped enums 的枚举成员仅仅对枚举体内部可见。只能通过类型转换（ cast ）转换为其他类型。
- scopded enums 和 unscoped enum 都支持指定潜在类型。scoped enum 默认潜在类型是 int 。unscoped enum 没有默认的潜在类型。
- scoped enum 总是可以前置声明的。unscoped enum 只有当指定潜在类型时才可以前置声明。
{{</admonition>}}

#### [Item 11: Prefer deleted functions to private undefined ones.](https://blog.csdn.net/Dong_HFUT/article/details/123005509)

如果你的代码里有一些特殊的函数不想被别人调用，一般来说你不申明它即可。但是，有些特殊的成员函数，C++ 会自动申明，比如拷贝构造函数、拷贝赋值操作。

**C++98 的做法**
C++98 的做法是将它们申明为私有并且不定义它们。以 IO 流为例，IO 流的基础类是 basic_ios，输入输出流都是继承与它。IO 流的拷贝是被阻止的（例如输入流 istream对象，表示输入数值，一些可能已经读入到内存中，一些可能还未读入，如果要复制一个输入流，是复制哪一部分？为了避免这样的问题，直接阻止 IO 流的拷贝）。

为了阻止 IO 流的复制，C++98 处理如下：
```c++
template <class charT, class traits = char_traits<charT> >
class basic_ios : public ios_base {
public:
...
private:
  basic_ios(const basic_ios& ); // not defined
  basic_ios& operator=(const basic_ios&); // not defined
};
```

将这些函数申明为私有并不定义，若这些函数被调用，在链接阶段将会因为没有定义而失败。

**C++11 的做法**

C++11 中有更好的处理方法，给这些函数标记为 “= delete” ，表明它们是被删除的函数：

```c++
template <class charT, class traits = char_traits<charT> >
class basic_ios : public ios_base {
public:
  ...
  basic_ios(const basic_ios& ) = delete;
  basic_ios& operator=(const basic_ios&) = delete;
  ...
};
```

删除函数不可以任何方式使用，即使是成员函数和友元函数。使用这些函数将导致编译报错。这比 C++98 中的方法诊断出错误的时间提前到编译阶段。

将 delete 函数申明为 public 的原因是：C++ 先检查访问权限，再检查 delete 状态。如果申明为 private，当使用删除的私有函数时，有些编译器只会报出这些函数是私有的，但其实更明确的含义是这些是删除函数，不期望被使用。

**delete 函数优势**
delete 函数的优势是它可以应用于任何函数，而不仅仅是成员函数。例如你有一个函数，只接受输入为 int 类型：

```c++
bool isLucky(int number);
```
但是，C++ 中很多类型都可以隐式转换为 int 类型。如下调用都可以通过：

```c++
isLucky('a');
isLucky(true);
isLucky(3.5);
```
C++11 的处理方法是将他们标记成 delete：

```c++
bool isLucky(int number); // original function
bool isLucky(char) = delete; // reject chars
bool isLucky(bool) = delete; // reject bools
bool isLucky(double) = delete; // reject doubles and floats
```

上面将参数为 double 的函数标记成 delete，可以阻止 float 和 double 两种参数的调用：C++总是倾向于将 float 转换为 double。

delete 函数的另一个优势体现在使用模板时候。如下面例子：

```c++
template<typename T>
void processPointer(T* ptr);
```

假设你想阻止 void* 和 char* 类型的特例，将这些函数的实例化标记成 delete 即可：

```c++
template<>
void processPointer<void>(void*) = delete;
template<>
void processPointer<const void>(const void*) = delete;
template<>
void processPointer<char>(char*) = delete;
template<>
void processPointer<const char>(const char*) = delete;
```

如果你在一个类内部有一个函数模板，你想通过声明它们为私有来禁止某些实现，是行不通的：因为模板的特例化无法在类的作用域内定义：

```c++
#include <iostream>

class A {
public:
  A();
  template<typename T>
  void processPointer(T* ptr) {
    std::cout << ptr << std::endl;
  }
private:
  template<>
  void processPointer<void>(void*);
};

int main()
{
    return 0;
}

// 报错信息：
main.cpp:12:12: error: explicit specialization in non-namespace scope 'class A'
   12 |   template<>
      |            ^
main.cpp:13:8: error: template-id 'processPointer<void>' in declaration of primary template
   13 |   void processPointer<void>(void*);
      |
```

使用 public 和 delete 方式则没有问题：
```c++
class A {
public:
  A();
  template<typename T>
  void processPointer(T* ptr) {
    std::cout << ptr << std::endl;
  }
};

template<>
void A::processPointer<void>(void*) = delete;
```

因此，选择使用 delete 函数吧。

{{<admonition quote "总结" false>}}
- 比起声明函数为 private 但不定义，使用 delete 函数更好。
- 任何函数都能 delete ，包括非成员函数和模板实例。
{{</admonition>}}



#### [Item 12: Declare overriding functions override.](https://blog.csdn.net/Dong_HFUT/article/details/123008755)

面向对象的 C++ 主要特性是类、继承、虚函数，而这些特性的基础是继承类中重写基类的虚函数。重写（overriding）和重载（overloading）看着比较相似，但其实完全不同。重写的一个例子：

```c++
class Base {
public:
  virtual void doWork(); // base class virtual function
  ...
};

class Derived: public Base {
public:
  virtual void doWork();  // overrides Base::doWork ("virtual" is optional here)
  ...
};

std::unique_ptr<Base> upb = std::make_unique<Derived>();
...
upb->doWork(); // call doWork through base class ptr; derived class function is invoked
```

重写需要满足的条件：
  - 基类的重写函数必须是虚函数。
  - 基类和继承类的重写函数的函数名必须相同。
  - 基类和继承类的重写函数的参数类型必须相同。
  - 基类和继承类的重写函数的常量属性必须相同。
  - 基类和继承类的重写的函数的返回值类型和异常规格说明要兼容。

除了 C++98 的这些限制条件，C++11 还增加了一条：
  - 基类和继承类的重写函数的引用修饰符必须相同。

引用修饰符包括左值和右值两种，左值引用成员函数可以被 *this 为左值调用，右值引用成员函数可以被 *this 为右值调用。看一个例子：

```c++
#include <iostream>

class Widget {
public:
  void doWork() & {   // this version of doWork applies only when *this is an lvalue
    std::cout << "doWork() &" << std::endl;
  }
  void doWork() && {  // this version of doWork applies only when *this is an rvalue
    std::cout << "doWork() &&" << std::endl;
  }
};

Widget makeWidget()  { // factory function (returns rvalue)
  return Widget();
}

int main() {
  Widget w;
  w.doWork();
  makeWidget().doWork();
}

// 打印信息
doWork() &
doWork() &&
```

这里，w 是个左值，所以调用的是 doWork() & 函数，而 makeWidget() 返回的是一个右值，所以调用的是 doWork() && 函数。

重写的条件是非常严格的，细小的差错将会导致完全不同的结果。看个例子：

```c++
class Base {
public:
  virtual void mf1() const;
  virtual void mf2(int x);
  virtual void mf3() &;
  void mf4() const;
};

class Derived: public Base {
public:
  virtual void mf1();  // 缺少 const
  virtual void mf2(unsigned int x); // 参数类型不同
  virtual void mf3() &&;  // 引用修饰符不同
  void mf4() const;  // 基类没有加 virtual 修饰
};
```

以上代码，有的编译器会给出警告，有的编译器给的警告不全，还有的编译器不会给警告。所以保险的做法是自己将重写函数标记为 override，这样的话，不符合重写条件的话，编译器一定会报错。

所以，当你在继承类中要重写函数的话，加上 override 吧！

{{<admonition quote "总结" false>}}
- 为重载函数加上override。
- 成员函数引用限定让我们可以区别对待左值对象和右值对象（即*this)。
{{</admonition>}}


#### [Item13: Prefer const_iterators to iterators.](https://blog.csdn.net/Dong_HFUT/article/details/123030976)

STL 中 const_iterators 类似于 pointers-to-const，当你不修改 iterator 指向的对象时，最好使用 const_iterators，这样更加安全。

但是在 C++98 对 const_iterators 的支持并不完善。看下面的例子：

```c++
std::vector<int> values;
…
std::vector<int>::iterator it =
  std::find(values.begin(),values.end(), 1983);
values.insert(it, 1998);
```

上面的代码并不修改 iterator 指向的内容，显然 const_iterator 是更好的选择。C++98 中一种可能的修改如下，但却无法通过编译：

```c++
typedef std::vector<int>::iterator IterT; // typedefs
typedef std::vector<int>::const_iterator ConstIterT;
std::vector<int> values;
…
ConstIterT ci =
std::find(static_cast<ConstIterT>(values.begin()), // cast
          static_cast<ConstIterT>(values.end()),   // cast
          1983);
values.insert(static_cast<IterT>(ci), 1998); // may not compile; see below
```

因为 values 是非 const 的容器，为了使用 const_iterator，这里使用了 static_cast 强制转换一下。在 C++98 中，insert 和 erase 操作必须使用 iterator，但是在 C++98（包括 C++11）没办法将 iterator 转换为 const_iterator。

不过，这在 C++11 中可以得到解决，容器的成员函数 cbegin、cend 可以得到 const_iterator，即使是非 const 容器，并且 insert 和 erase 等操作可以使用 const_iterator，上述代码可以修改如下：

```c++
std::vector<int> values; // as before
…
auto it = std::find(values.cbegin(),values.cend(), 1983); // use cbegin and cend
values.insert(it, 1998);
```

但是，对于泛型编程，上述代码还是不够，因为除了标准容器，还有一些类似容器的数据结构，比如数组，没有成员函数 begin 和 end，只有非成员的 begin 和 end。因此，使用非成员函数的 begin 和 end 的代码更加通用：

```c++
template<typename C, typename V>
void findAndInsert(C& container,        // in container,
                   const V& targetVal, // find first occurrence of targetVal,
                   const V& insertVal) // then insert insertVal there
{
  using std::cbegin;
  using std::cend;
  auto it = std::find(cbegin(container), // non-member cbegin
                      cend(container),   // non-member cend
                      targetVal);
  container.insert(it, insertVal);
}
```

很遗憾，上述代码只能在 C++14 中才能用，因为 C++11 只提供了非成员的 begin 和 end，C++14 才提供非成员的 cbegin、cend、rbegin、rend、crbegin 和 crend。

如果你那的编译器只支持 C++11，那么你可以使用如下代码来实现 cbegin：
```C++
template <typename C>
auto cbegin(const C& container)->decltype(std::begin(container))
{
  return std::begin(container);
}
```

这里利用了 Item 1 中类型推导得到常量版本迭代器。

最后总结一下：在 C++98 中有场景可能无法使用 const_iterator。并且在 C++11 中没有提供非成员的 cbegin 和 cend。但是是鼓励大家尽可能使用 const_iterator，毕竟这样最安全。

{{<admonition quote "总结" false>}}
- 优先考虑 const_iterator 而非 iterator。
- 在最大程度通用的代码中，优先考虑非成员函数版本的 begin、end、rbegin 等，而非同名成员函数。
{{</admonition>}}

#### [Item14: Declare functions noexcept if they won‘t emit exception.](https://blog.csdn.net/Dong_HFUT/article/details/123163671)

**背景**

在 C++98 中，异常规范（exception specifications）是一个比较鸡肋的设计。一个函数的实现一旦发生修改，其异常规范也可能发生改变，进而会影响到客户代码，因为客户代码可能依赖原先的异常规范。

C++11 对异常规范提出了有意义的改进：那就是指明函数是否可能抛出异常，不可能抛出异常的函数可以使用 noexcept 标识。

**影响调用代码的安全和性能**

在接口设计中，接口是否应该申明为 noexcept 是非常差重要的，也是客户非常关心的。接口是否为 noexcept 的状态会影响到调用者代码的安全和性能。因此如果你的函数确定不会抛出异常，就把它申明为 noexcept。

**影响编译器优化**

对于一个 noexcept 函数，编译器将会有更多优化的机会。我们看下 C++98 和 C++11 中，对于一个函数不会抛出异常的不同解释。考虑一个函数 f，它保证调用者永远不会收到异常：

```c++
int f(int x) throw();   // no exceptions from f: C++98 style
int f(int x) noexcept;  // no exceptions from f: C++11 style
```

假设在运行阶段，函数 f 产生了异常，对于 C++98 风格：函数 f 的调用栈将被解开，然后经过一些动作后，程序停止。但是对于 C++11 风格：调用栈只在程序终止前才有可能被解开。

解开调用栈的时机和可能性的不同，对于代码的性能产生有很大的影响。对于一个 noexcept 函数，如果一个异常能传到函数外面去，优化器不需要保持运行期栈为解开的状态，也不需要确保noexcept 函数中的对象销毁的顺序和构造的顺序相反。

总的来说，noexcept 让函数有了更多被优化的机会：

```c++
RetType function(params) noexcept; // most optimizable
RetType function(params) throw();  // less optimizable
RetType function(params);          // less optimizable
```

**影响移动语义的使用**

对于 std::vector，当 size 和 capacity 相等时候，再为其添加一个元素（push_back）时候，std::vector 需要需要申请一个新的更大的内存块，再把原来内存块中的元素转移新的内存块中。

C++98 的做法是：通过拷贝来转移数据，它先将旧内存块中数据拷贝到新内存块中，再销毁旧的内存块。这种做法确保了 push_pack 能够提供异常安全保证：即使拷贝过程发生异常，std::vector 还能保持旧的数据。

C++11 中很容易想到的一个优化：通过 move 语义来完成数据转移。但是这可能违反 push_pack 提供的异常安全保证，假设移动过程中发生异常，std::vector 的部分数据已经移动出去了，反向再把数据移动回去是困难的，本身也会产生异常，std::vector 的数据无法恢复。

因此，C++11 中 std::vector::push_back 能否使用 move 语义优化，要看 move 操作是否可能抛出异常，如果 move 操作不会抛出异常，则可以用 move 进行优化。是否抛出异常当然就是检查是否为 noexcept。除了 std::vector::push_back 外，std::vector::reserve 和 std::deque::insert 等操作也是采用相同的策略。

**条件 noexcept**

swap 函数是很多 STL 算法的关键部分，并且经常被拷贝和赋值操作调用，noexcept 的优化对它相当有价值。值得注意的是：标准库的 swap 是否为 noexcept 往往取决于用户自定义的 swap 是否为 noexcept。标准库中，array 和 std::pair 的 swap 声明如下：

```c++
template <class T, size_t N>
void swap(T (&a)[N],
          T (&b)[N]) noexcept(noexcept(swap(*a, *b)));

template <class T1, class T2>
struct pair {
  …
  void swap(pair& p) noexcept(noexcept(swap(first, p.first)) &&
                              noexcept(swap(second, p.second)));
  …
};
```

这些 swap 函数是条件 noexcept（conditionally noexcept）：它们是否为 noexcept 取决于noexcept 中的表达式是否为 noexcept。举个例子，两个 Widget 的数组，只有用 Widget 调用的swap 是 noexcept 时，用数组调用的 swap才是 noexcept。因此，只有低层次数据结构的 swap 是noexcept，才能使得高层次数据结构的 swap 是noexcept。这就鼓励你尽量提供 noexcept swap 函数。

**异常中立函数**

从上面介绍，我们可以知道，对于不会抛出异常的函数，最好声明为 noexcept。

事实上，很多函数是异常中立的（exception-neutral），这些函数本身不抛出异常，但是他们调用的函数可能会抛出异常。这种场景下，异常中立函数允许将异常通过调用链传给处理程序。异常中立的函数永远不是 noexcept，因为他们可能抛出“我只经过一下”的异常。因此，大部分函数都不适合设计为成 noexcept。

然而，一些函数天生就不抛出异常，并且对这些函数（特别是 move 操作和 swap 函数）成为noexcept 能获得很大的回报，只要有任何可能，将他们申明为 noexcept 是值得的。

**默认 noexcept 函数**

在 C++11 中，默认情况下，所有的内存释放函数和析构函数（包括用户自定义的和编译器自动生成的）都隐式成为 noexcept。因此，我们不需要把它们声明成 noexcept（可以但没必要）。

只有一种情况的析构函数不是隐式 noexcept ，就是当类中的一个成员变量的析构函数声明了它可能会抛出异常（比如，声明这个析构函数为“noexcept(false)”）。这样的声明是不寻常的，标准库中没有这样的情况。

**wide contract and narrow contract**

一些库的接口设计区分了宽接口（wide contract）和窄接口（narrow contract）。一个带宽接口的函数是没有前提条件的。这样的函数被调用时不需要关注程序的状态，传入的参数方面没有限制，宽接口的函数永远不会出现未定义的行为。不带宽接口条件的函数就是窄接口函数。对这些函数来说，如果传入的参数违反了前提条件，结果将是未定义的。

写一个宽接口的函数，并且你知道不会抛出一个异常，那就遵循本 Item 的建议，把它声明为noexcept。但对于窄接口的函数，情况将变得比较棘手。举个例子，假设一个函数 f，这个函数被传入一个 std::string 参数。假设有一个前提条件：std::string 参数的数据长度不会超过32个字节。如果用一个超过32字节的 std::string 传入 f，f 的行为将是未定义的。f 没有义务去检查这个前提条件，调用者有责任确保前提条件得到满足。因此，把 f 声明为 noexcept 似乎是合理的：

```c++
void f(const std::string& s) noexcept; // precondition: s.length() <= 32
```
但是，当前提条件被违反了（例如系统测试时候），调用者捕捉一个抛出的异常总是比找出未定义行为的原因要简单很多。但是函数被申明成了 noexcept，无法抛出类似“前提条件被违反”的异常。因此，通常只为宽接口函数提供 noexcept 声明。

{{<admonition quote "总结" false>}}
- noexcept 是函数接口的一部分，并且调用者可能会依赖这个接口。
- 相较于 non-noexcept 函数，noexcept 函数有被更好优化的机会。
- noexcept 对于 move 操作、swap、内存释放函数和析构函数是非常有价值的。
- 大部分函数是异常中立的而不是 noexcept。
{{</admonition>}}



#### [Item 15: Use constexpr whenever possible.](https://blog.csdn.net/Dong_HFUT/article/details/123172189)

constexpr 是 C++11 中令人非常非常困惑的一个新特性。从概念上讲，它不止表明一个对象是 const 的，而且是在编译时被知道的。把它用在对象上，可以理解成 const 的加强版。把它用在函数时，将拥有完全不同的涵义。

**constexpr 对象**

先从 constexpr 对象开始，这个对象是 const 的，也是在编译时被知道（准确的说，应该是在翻译阶段被知道，翻译阶段包括编译和链接两个阶段）。

对象的值在编译时知道是非常有用的。它能被存储在只读内存中，尤其是对一些嵌入式系统来说，这是一个相当重要的特性。更广泛的应用包括用于指定数组的大小、整形模板参数、枚举成员值、对齐说明符等。如果你希望用变量做这些事情，那么将他们申明为 constexpr，编译器会保证它们是在编译时可以被知道的值：

```c++
int sz;                             // non-constexpr variable
…
constexpr auto arraySize1 = sz;     // error! sz's value not known at compilation
std::array<int, sz> data1;          // error! same problem
constexpr auto arraySize2 = 10;     // fine, 10 is a compile-time constant
std::array<int, arraySize2> data2;  // fine, arraySize2 is constexpr
```
但是 const 无法像 constexpr 一样保证值可以在编译时被知道，因为 const 对象不需要用一个编译时知道的值去初始化：

```c++
int sz;                            // as before
…
const auto arraySize = sz;        // fine, arraySize is const copy of sz
std::array<int, arraySize> data;  // error! arraySize's value not known at compilation
```

简单来说，所有的 constexpr 对象都是 const 的，但并非所有的 const 对象都是 constexpr 的。如果你希望编译器保证变量的值能够用在需要在编译时常量的上下文中时，那么你应该使用 constexpr 而不是 const。

**constexpr 函数**

当涉及到 constexpr 函数时，constexpr 对象的使用场景变得更加有趣。当使用编译时常量来调用 constexpr 函数时，它们产生编译时常量。当用来调用函数的值不能在运行时前得知时，它们产生运行时的值。完整的理解如下：

- constexpr 函数可以被使用在要求编译时常量的上下文中。如果所有传入 constexpr 函数的参数都能在编译时知道，那么结果将在编译时计算出来。如果有任何一个参数的值不能在编译时知道，你的代码将不能在编译时执行。
- 当 constexpr 函数调用参数有一个或多个不能在编译时知道值时，它就像一个正常的函数一样，在运行时计算它的值。这意味着你不需要两个函数来完成相同的操作，一个为编译时常量服务，一个为所有的其它值服务。constexpr 函数做了所有的事情。

看下面的例子：

```c++
constexpr int pow(int base, int exp) noexcept // pow's a constexpr func that never throws
{
  … // impl is below
}
constexpr auto numConds = 5;                // # of conditions
std::array<int, pow(3, numConds)> results;  // results has 3^numConds elements
```

pow 前面的 constexpr 意味着如果 base 和 exp 是编译时常量，pow 的返回结果可以用作编译时常量。如果 base 或 exp 不是编译时常量，pow 的结果将在运行时计算。这意味着 pow 不仅能在编译时计算std::array的大小，它也可以在运行时这么调用：

```c++
auto base = readFromDB("base");    // get these values at runtime
auto exp = readFromDB("exponent");
auto baseToExp = pow(base, exp);  // call pow function at runtime
```

当用编译时值调用 constexpr 函数，则要求该函数能够返回一个编译时值。一些限制会影响 constexpr 函数的实现，C++11 和 C++14 的限制是不同的。

C++11中，constexpr 函数只能有一条 return 语句。可以通过一些手段来扩展 constexpr 的 return 语句。一个问号表达式，一个是递归：
```c++
constexpr int pow(int base, int exp) noexcept
{
  return (exp == 0 ? 1 : base * pow(base, exp - 1));
}
```

C++14 的限制则大幅减小，可以实现如下：

```c++
constexpr int pow(int base, int exp) noexcept // C++14
{
  auto result = 1;
  for (int i = 0; i < exp; ++i) result *= base;
    return result;
}
```

constexpr 函数被限制只能接受和返回 literal 类型，本质上这个类型的值可以在编译时确定。在 C++11 中，除了 void 类型外的所有内置类型都是 literal 类型，用户自定义类型也可能是 literal 类型。因为构造函数和其他函数也可能是 constexpr 的：

```c++
class Point {
public:
  constexpr Point(double xVal = 0, double yVal = 0) noexcept
  : x(xVal), y(yVal)
  {}
  constexpr double xValue() const noexcept { return x; }
  constexpr double yValue() const noexcept { return y; }
  void setX(double newX) noexcept { x = newX; }
  void setY(double newY) noexcept { y = newY; }
private:
  double x, y;
};
```
在这里， Point 的构造函数被申明为 constexpr，如果传入的值是在编译时知道的值，则 Point 的成员变量的值也能在编译时知道。因此，Point也能被初始化为 constexpr：

```c++
constexpr Point p1(9.4, 27.7); // fine, "runs" constexpr ctor during compilation
constexpr Point p2(28.8, 5.3); // also fine
```

类似的，getX 和 getY 也是 constexpr 的，他们的返回值也可以被用来构造 constexpr 对象：

```c++
constexpr
Point midpoint(const Point& p1, const Point& p2) noexcept
{
 return { (p1.xValue() + p2.xValue()) / 2,    // call constexpr member funcs
          (p1.yValue() + p2.yValue()) / 2 };
}
constexpr auto mid = midpoint(p1, p2); // init constexpr object w/result of constexpr function
```

这意味着以前运行时能做的工作和编译时能做的工作之间的界限变得模糊了，一些以前只能在运行时执行的运算现在可以提前到编译是来执行了。提前的代码越多，软件的性能越好。当然编译时间也会相应增加。

C++11中，有两个限制阻止 Point 的成员函数 setX 和 setY 无法成为 constexpr。
- 第一，它们修改了操作的对象。但是在C++11中，constexpr 成员函数被隐式声明为 const。
- 第二，它们的返回值类型是 void，void 类型在C++11中不是 literal 类型。

在 C++14 中，去掉了这两个限制。因此，C++14 的 Point类中的 setX 和 setY 可以成为 constexpr：

```c++
class Point {
public:
  …
  constexpr void setX(double newX) noexcept // C++14
  { x = newX; }
  constexpr void setY(double newY) noexcept // C++14
  { y = newY; }
  …
};
```

这让下面的代码成为可能：

```c++
// return reflection of p with respect to the origin (C++14)
constexpr Point reflection(const Point& p) noexcept
{
  Point result; // create non-const Point
  result.setX(-p.xValue()); // set its x and y values
  result.setY(-p.yValue());
  return result; // return copy of it
}
```

调用代码可能是这样的：

```c++
constexpr Point p1(9.4, 27.7);
constexpr Point p2(28.8, 5.3);
constexpr auto mid = midpoint(p1, p2);
// reflectedMid's value is (-19.1 -16.5) and known during compilation
constexpr auto reflectedMid = reflection(mid);
```

因此，通过上面的介绍，我们了解了为什么要尽可能地使用 constexpr 了。

{{<admonition quote "总结" false>}}
- onstexpr 对象是 const 的，给它初始化的值需要在编译时知道。
- 如果使用在编译时就知道的参数来调用 constexpr 函数，它就能产生编译时的结果。
- 相较于 non-constexpr 对象和函数，constexpr 对象很函数能被用在更广泛的上下文中。
- constexpr 是对象接口或函数接口的一部分。
{{</admonition>}}



#### [Item 16: Make const member functions thread safe.](https://blog.csdn.net/Dong_HFUT/article/details/123316263)

**const 成员的好处**

一般地，对于不修改成员变量的函数申明为 const 类型，它隐含了是被传入的 this 指针为 const 类型。const 成员函数有两个好处：
- 不会修改成员变量（有特例，下面会介绍），比较安全。
- 可以被 const 和 非 const 对象调用。非 const 成员变量无法被 const 对象调用。

由于 const 成员函数一般不会修改成员变量，因此 const 成员函数一般也是线程安全的。

**const 成员函数的线程安全问题**

凡是皆有例外。

考虑这样一个场景：用一个类来表示多项式，并求解其根。求根的过程不会修改多项式本身，因此我们可以设计如下：

```c++
class Polynomial {
public:
  using RootsType = std::vector<double>;
  …
  RootsType roots() const; // 此处省略其实现
  …
};
```

从功能角度来说，上面的设计没有问题。但是，求解根的代价一般都比较高，我们一般不希望重复计算多次。因此，我们可以将多项式的根缓存起来，只在必要的时候才计算，可以修改如下：

```c++
class Polynomial {
public:
  using RootsType = std::vector<double>;
  RootsType roots() const
  {
    if (!rootsAreValid) { // if cache not valid compute roots, store them in rootVals
      …
      rootsAreValid = true;
    }
    return rootVals;
  }
private:
  mutable bool rootsAreValid{ false };
  mutable RootsType rootVals{};
};
```

从概念上说，求解根不会修改多项式本身。但是，缓存的行为又会修改多项式类的成员变量。经典的做法就是将成员变量申明为 mutable，成员函数申明为 const。

如果 const 成员函数不会被多个线程调用，那上述代码没有任何问题。

但是，当存在两以上的对象同时调用 const 成员变量，则会发生线程安全问题：这两个线程中的一个或两个都可能尝试去修改成员变量 rootsAreValid 和 rootVals。这意味着在没有同步的情况下，两个不同的线程读写同一段内存，这段代码会会有未定义的行为。

**const 成员函数线程安全保证**

很容易想到的解决办法是加 mutex：
```c++
class Polynomial {
public:
  using RootsType = std::vector<double>;
  RootsType roots() const
  {
    std::lock_guard<std::mutex> g(m); // lock mutex
    if (!rootsAreValid) { // if cache not valid, compute/store roots
      …
      rootsAreValid = true;
    }
    return rootVals;
  } // unlock mutex
private:
  mutable std::mutex m;
  mutable bool rootsAreValid{ false };
  mutable RootsType rootVals{};
};
```

值得注意的是，由于 std::mutex 是 move-only 类型，所以包含了 std::mutex 变量的 Polynomial 对象也是 move-only 类型，失去了 copy 的能力。

在有些情况下，使用 std::mutex 的代价可能是比昂贵的。如果你只需要同步单一变量或者内存单元，使用 std::atomic 是相对比较廉价的。举个例子，你只需要统计一个成员函数被调用的次数：

```c++
class Point { // 2D point
public:
  …
  double distanceFromOrigin() const noexcept // see Item 14 for noexcept
  {
    ++callCount; // atomic increment
    return std::sqrt((x * x) + (y * y));
  }
private:
  mutable std::atomic<unsigned> callCount{ 0 };
  double x, y;
};
```
和 std::mutex 一样， std::atomic 也是 move-only 的。

虽然 std::atomic 的开销一般比 std::mutex 高，但对于多于一个变量或内存块需要同步，还是应该使用 std::mutex。因此，看你的需求，选择使用 std::mutex 或者 std::atomic 。

{{<admonition quote "总结" false>}}
- 让 const 成员函数做到线程安全，除非你确保它们永远不会用在多线程的环境下。
- 相比于 std::mutex，使用 std::atomic 变量能提供更好的性能，但是它只适合处理单一的变量或内存单元。
- std::mutex 和 std::atomic 都是 move-only 的。
{{</admonition>}}

#### [Item 17: Understand special member function generation.](https://blog.csdn.net/Dong_HFUT/article/details/123433559)

在 C++ 中，对于某些特殊成员函数，如果我们没有申明，编译器会自动生成它们。

在 C++98 中，这些特殊成员函数包括默认构造函数、析构函数、拷贝构造函数、拷贝赋值操作。编译器生成这些构造函数的原则是：当你没有申明它们，并且需要使用到它们。编译器生成的这些特殊函数，并且一般是 public 、 inline 和 nonvirtual 的。派生类中的析构函数有点特殊，当这个派生类继承自 virtual 析构函数的基类，在这种情况下，派生类中，编译器生成的析构函数也是 virtual 的。

两个 copy 操作是独立的：声明一个不会阻止编译器生成另外一个。所以如果你声明了一个拷贝构造函数，但是没有声明拷贝赋值操作 ，然后你需要用到拷贝赋值，编译器将帮你生成一个拷贝赋值操作。类似的，如果你声明了一个拷贝赋值操作，但没有声明拷贝构造函数，然后你的代码需要拷贝构造，编译器将帮你生成一个拷贝构造函数。

你也许听过三法则（The Rule of Three）：如果你声明了任何一个拷贝构造函数，拷贝赋值操作或析构函数，那么你应该声明所有的这三个函数。三法则的建议来源于这样的：当拷贝构造、拷贝赋值或者析构函数不能胜任时候，往往是类要管理某些资源（通常是内存资源），当需要在拷贝中对资源进行管理，那么也需要在析构函数中对资源也进行管理（通常是释放内存），反之亦然。

在 C++11 中，多了两个特殊成员函数：移动构造函数和移动赋值操作：

```c++
class Widget {
public:
  ...
  Widget(Widget&& rhs);             // move constructor
  Widget& operator=(Widget&& rhs);  // move assignment operator
  ...
};
```

新增的两个 move 函数生成的规则也是类似的：当你没有申明，并且代码需要使用，编译器就会替你生成。看个例子：

```c++
#include <iostream>
#include <string>

class strA {
 public:
  strA () {
    str = "=====hello====";
  }
  std::string str;
};

int main() {
  strA s1;
  strA s2(std::move(s1));
  std::cout << "=== s1.str = " << s1.str << std::endl;
  std::cout << "=== s2.str = " << s2.str << std::endl;
  return 0;
}

// 结果：
=== s1.str =
=== s2.str = =====hello====
```

这个例子，编译器就会自动生成 move 构造函数。

但是，实际上当我们对一个类使用移动构造的时候，编译器并不保证一定是移动构造，因为这个类并不一定具备移动语义，这时候会用拷贝来代替。看这个例子：

```c++
class A {
 public:
  A () {
    p = new int(5);
    std::cout << "====construction====" << std::endl;
  }

  int *p = nullptr;
};

int main() {
  A a;
  A b(std::move(a));
  std::cout << "==== a.p = " << a.p << std::endl;
  std::cout << "==== b.p = " << b.p << std::endl;
  std::cout << "==== *(a.p) = " << *(a.p) << std::endl;
  std::cout << "==== *(b.p) = " << *(b.p) << std::endl;
  return 0;
}

// 结果：
====construction====
==== a.p = 0x1742c20
==== b.p = 0x1742c20
==== *(a.p) = 5
==== *(b.p) = 5
```

这里的 A b(std::move(a)); 实际上是调用的编译器生成的拷贝构造函数。

两个 move 函数不是独立的：如果你声明了任何一个，那就阻止了编译器生成另外一个。也就是说，如果你声明了一个 move 构造函数，那就表明默认的 memberwise move 对这个类来说是不胜任的，那么编译器会认为 默认的 memberwise move 赋值函数也是不能胜任的。所以声明一个 move 构造函数会阻止一个move 复制操作函数被自动生成，声明一个 move 赋值操作函数会阻止一个 move 构造函数被自动生成。

另外，如果任何类显式地声明了一个 copy 操作，两个move 操作就不会被自动生成。这样做的理由是：声明一个 copy 操作表明了用默认的 memberwise copy 来拷贝对象对于这个类来说是不合适的，然后编译器认为，如果对于 copy 操作来说memberwise copy 不合适，那么对于 move 操作来说 memberwise move 很有可能也是不合适的。将上面第一个例子添加一个拷贝构造函数：

```c++
#include <iostream>
#include <string>

class strA {
 public:
  strA () {
    str = "=====hello====";
  }

  strA (const strA& a) {
    std::cout << "== copy construction ==" << std::endl;
  }

  std::string str;
};

int main() {
  strA s1;
  strA s2(std::move(s1));
  std::cout << "=== s1.str = " << s1.str << std::endl;
  std::cout << "=== s2.str = " << s2.str << std::endl;
  return 0;
}

// 结果：
== copy construction ==
=== s1.str = =====hello====
=== s2.str =
```

这个例子可以看到：定义了 copy 构造函数，阻止了编译器生成 move 构造函数。

三法则的一个结论是：当类中出现一个用户自定义的析构函数，则表示简单的 memberwise copy 可能不太适合 copy 操作。这反过来就建议：如果一个类声明了一个析构函数，copy 操作可能不应该被自动生成，因为它们可能将作出一些不正确的事。在 C++98 被采用的时候，这个原因的重要性没有被发现，所以在C++98中，用户自定义的析构函数的存在不会影响编译器生成 copy 操作。这种情况在 C++11 中还是存在的，但是这只是因为条件的限制（如果阻止 copy 操作的生成会破坏太多的遗留代码）。

但是，三法则背后的考量还是有效的，并且，结合之前的观察：copy 操作的声明阻止隐式 move 操作的生成。这使得在C++11 中，一个类中有一个用户自定义的析构函数时，则编译器不会生成 move 操作。

因此：只在下面三个条件为真的时候，编译器才为类生成 move 操作（当需要的时候）：

- 没有 copy 操作在类中被声明。
- 没有 move 操作在类中被声明。
- 没有析构函数在类中被声明。

并且，当用户自定义了 move 操作时候，编译器也会认为默认的 memberwise copy 也是不适合的，也会阻止两个 copy 函数被自动生成。

如果你想突破这些限制：用户自定义的 copy 构造函数阻止了编译器生成 move 构造函数，但是如果编译器生成的函数提供的行为是正确的，也就是说如果 memberwise move 就是你想要的，C++11的 “=default” 可以帮助你：

```c++
#include <iostream>
#include <string>

class strA {
 public:
  strA () {
    str = "=====hello====";
  }

  strA (const strA& a) {
    std::cout << "== copy construction ==" << std::endl;
  }

  strA (strA&& a) = default;

  std::string str;
};

int main() {
  strA s1;
  strA s2(std::move(s1));
  std::cout << "=== s1.str = " << s1.str << std::endl;
  std::cout << "=== s2.str = " << s2.str << std::endl;
  return 0;
}

// 结果：
=== s1.str =
=== s2.str = =====hello====
```

总结 C++11 特殊成员函数生成的规则：
因此C++11对特殊成员函数的控制规则是这样的：

**默认构造函数：**
- 和C++98中的规则一样，只在类中没有用户自定义的构造函数时生成。

**析构函数：**
- 本质上和 C++98 的规则一样;
- 唯一的不同就是析构函数默认声明为 noexcept（看Item 14）。
- 和 C++98 一样，只有基类的析构函数是 virtual 时，析构函数才会是 virtual。

**拷贝构造函数：**
- 和 C++98 一样的运行期行为：memberwise 拷贝构造 non-static 成员变量。
- 只在类中没有用户自定义拷贝构造函数时被生成。
- 如果类中声明了一个move操作，它就会被删除（声明为 delete ）。
- 在有用户自定义拷贝 operator= 或析构函数时，这个函数能被生成，但是这种生成方法是被弃用的。

**拷贝赋值 operator= ：**
和 C++98 一样的运行期行为：memberwise 拷贝赋值 on-static 成员变量。
只在类中没有用户自定义拷贝 operator= 时被生成。
如果类中声明了一个 move 操作，它就会被删除（声明为 delete ）。
在有用户自定义拷贝构造函数或析构函数时，这个函数能被生成，但是这种生成方法是被弃用的。

**move构造函数和move operator=：**
- 每个都对 non-static 成员变量执行 memberwise move。
- 只有类中没有用户自定义拷贝操作、move 操作或析构函数时被生成。

需要注意的是：关于成员函数模板的存在，没有规则规定它会阻止编译器生成特殊成员函数。

```c++
class Widget {
  ...
  template<typename T>
  Widget(const T& rhs);

  template<typename T>
  Widget& operator=(const T& rhs);
  ...
};
```

即使这些 template 能实例化出拷贝构造函数和拷贝 operator= 的函数签名（就是 T 是 Widget 的情况），编译器仍然会为Widget 生成 copy 和 move 操作。Item 26 会解释这个边缘情况。

{{<admonition quote "总结" false>}}
- 特殊成员函数是那些编译器可能自己帮我们生成的函数：默认构造函数，析构函数，copy 操作，move 操作。
- 只有在类中没有显式声明的 move 操作，copy 操作和析构函数时，move 操作才被自动生成。
- 只有在类中没有显式声明的拷贝构造函数的时候，拷贝构造函数才被自动生成。只要存在 move 操作的声明，拷贝构造函数就会被删除（delete）。拷贝 operator= 和拷贝构造函数的情况类似。在有显式声明的 copy 操作或析构函数时，另一个 copy 操作能被生成，但是这种生成方法是被弃用的。
- 成员函数模板永远不会抑制特殊成员函数的生成。
{{</admonition>}}

### CH04: Smart Pointers

#### [Item 18: Use std::unique_ptr for exclusive-ownership resource management.](https://blog.csdn.net/Dong_HFUT/article/details/123465058)

原始指针非常灵活，但是使用陷阱多，容易出错，智能指针则更容易使用。本文介绍的智能指针是 std::unique_ptr。

**独占所有权**

std::unique_ptr 表现出独占所有权的语义。一个非空的 std::unique_ptr 总是对它指向的资源拥有独占所有权，它不共享它指向的资源给其他指针。因此，无法通过值传递 std::unique_ptr 给函数，也不允许复制 std::unique_ptr。看下面的例子，注意 std::make_unique 在 C++14 才开始支持，从报错信息也可以看到拷贝构造函数是 delete 的。

```c++
#include<iostream>
#include<memory>

int main()
{
    std::unique_ptr<int> pInt(new int(5));
    // std::unique_ptr<int> pInt = std::make_unique<int>(5);  // C++14 才支持
    std::unique_ptr<int> pInt1(pInt);    // 报错
}
// 报错信息：
main.cpp: In function 'int main()':
main.cpp:8:36: error: use of deleted function 'std::unique_ptr<_Tp, _Dp>::unique_ptr(const std::unique_ptr<_Tp, _Dp>&) [with _Tp = int; _Dp = std::default_delete<int>]'
    8 |     std::unique_ptr<int> pInt1(pInt);    // 报错
      |                                    ^
In file included from /usr/local/include/c++/11.2.0/memory:76,
                 from main.cpp:2:
/usr/local/include/c++/11.2.0/bits/unique_ptr.h:468:7: note: declared here
  468 |       unique_ptr(const unique_ptr&) = delete;
      |
```

std::unique_ptr 是 move-only 类型，可以 move 它的控制权，原 std::unique_ptr 则变为空指针。看下面的例子：

```c++
#include<iostream>
#include<memory>

int main()
{
    std::unique_ptr<int> pInt(new int(5));
    std::unique_ptr<int> pInt2 = std::move(pInt);    // 转移所有权
    // std::cout << *pInt << std::endl; // Segmentation fault (core dumped) ./a.out
    std::cout << *pInt2 << std::endl;
    std::unique_ptr<int> pInt3(std::move(pInt2));
}
```

std::unique_ptr 虽然不支持复制，但有个例外：可以从函数返回一个 std::unique_ptr。

```c++
#include<iostream>
#include<memory>

std::unique_ptr<int> func(int x)
{
    std::unique_ptr<int> pInt(new int(x));
    return pInt;
}

int main() {
    int x = 5;
    std::unique_ptr<int> p = func(x);
    std::cout << *p << std::endl;
}
```

**占用内存的大小**

相较于其他智能指针，std::unique_ptr 有一个优势：在不自定义删除器的情况下，std::unique_ptr 的内存占用几乎和原始指针一致。

```c++
#include<iostream>
#include<memory>

int main() {
  int *p = new int(5);
  std::unique_ptr<int> pu(new int(6));
  std::cout << sizeof(p) << ":" << sizeof(pu) << std::endl;
  return 0;
}
// 输出：8:8
```

std::unique_ptr 内部几乎不用维护其他信息（`std::shared_ptr` 需要维护引用计数），当它离开作用域，是通过 delete 删除指向的资源。但是，如果自定义了删除器，则会增加内存占用。

```c++
#include<iostream>
#include<memory>

int main() {
  int c = 2;
  int d = 3;
  // 带参数捕捉的lambda表达式，会导致unique_ptr占用内存变大
  auto delint = [&](int *p) {
    std::cout << "c = " << c << std::endl;
    std::cout << "d = " << d << std::endl;
    std::cout << "deleter" << std::endl;
    delete p;
  };
  std::unique_ptr<int, decltype(delint)> p(new int(10), delint);
  std::cout << sizeof(p) << std::endl;
  return 0;
}
// 输出：
24
c = 2
d = 3
deleter
```

**一个典型应用**

std::unique_ptr 的一个典型应用是作为一个工厂函数的返回类型（指向类层次中的对象）。这里直接使用这里的代码作为例子：

```c++
#include<iostream>
#include<memory>

using namespace std;

/*!
 * \brief The Investment class 基类
 */
class Investment
{
public:
    virtual ~Investment() = default;

public:
    virtual void doWork() = 0;
};

/*!
 * \brief The Stock class 派生类
 */
class Stock : public Investment
{
public:
    virtual void doWork() override {
        cout << "Stock doWork....\n";
    }
};

/*!
 * \brief The Stock class 派生类
 */
class Bond : public Investment
{
public:
    virtual void doWork() override {
        cout << "Bond doWork....\n";
    }
};

enum class InvestType {
    INVEST_TYPE_STOCK,
    INVEST_TYPE_BOND,
};

auto makeInvestment(InvestType type)
{
    // 自定义析构器, 这里以lambda表达式的形式给出
    auto delInvmt = [](Investment *pInvestment) {
        // TODO 自定义析构时想干的事
        cout << "delInvmt called...." << endl;
        delete pInvestment;
    };

    // 待返回的指针, 初始化为空指针，并指定自定义析构器
    // decltype(delInvmt) 用于获取自定义析构器的类型
    unique_ptr<Investment, decltype(delInvmt)> pInv(nullptr, delInvmt);

    // 注意这里用reset来指定pInv获取从new产生的对象的所有权, 不能用=赋值
    switch (type)
    {
    case InvestType::INVEST_TYPE_STOCK:
        //pInv = new Stock; // error!! c++11禁止从裸指针到智能指针的隐式转换
        pInv.reset(new Stock);
        break;
    case InvestType::INVEST_TYPE_BOND:
        pInv.reset(new Bond);
        break;
    }

    // 返回智能指针
    return pInv;
}

void test()
{
    // 测试工厂函数
    {
        // pInv出作用域后会自己析构
        auto pInv = makeInvestment(InvestType::INVEST_TYPE_STOCK);
        if (pInv)
        {
            pInv->doWork();
        }
    }

    cout << "----------------\n";

    // 测试move效果
    {
        auto pInv = makeInvestment(InvestType::INVEST_TYPE_BOND);
        auto pInv2 = move(pInv);
        cout << "after move pInv to pInv2 \n";
        if (!pInv)
        {
            cout << "pInv is empty \n";
        }
        if (pInv2)
        {
            cout << "pInv2 is valid \n";
            pInv2->doWork();
        }
    }

    cout << "----------------\n";

    // 测试unique_ptr向shared_ptr转换
    {
        shared_ptr<Investment> pInv = makeInvestment(InvestType::INVEST_TYPE_BOND);
        pInv->doWork();
    }
}

int main () {
    test();
    return 0;
}

// 输出：
Stock doWork....
delInvmt called....
----------------
after move pInv to pInv2
pInv is empty
pInv2 is valid
Bond doWork....
delInvmt called....
----------------
Bond doWork....
delInvmt called....
```

**杂项**

std::unique_ptr 通过 std::unique_ptr<T[]> 形式支持指向数组，并通过 delete [] 释放资源。

```c++
#include<iostream>
#include<memory>

int main()
{
    std::unique_ptr<int[]> p(new int[5] {1, 2, 3, 4, 5});
    p[0] = 0;   // 重载了operator[]
}
```

std::unique_ptr 可以直接隐式转换为 `std::shared_ptr`。

```c++
`std::shared_ptr`<Investment> sp =   // converts std::unique_ptr
  makeInvestment( arguments );     // to `std::shared_ptr`
```

{{<admonition quote "总结" false>}}
- std::unique_ptr 是一个小的、快的、move-only 的智能指针，它能用来管理资源，并且独占资源的所有权。
- 默认情况下，std::unique_ptr 资源的销毁是用 delete 进行的，但也可以用户自定义 deleter。用带状态的 deleter 和函数指针作为 deleter 会增加 std::unique_ptr 对象的大小。
- 很容易将 std::unique_ptr 转换为 `std::shared_ptr`。
{{</admonition>}}


#### [Item 19: Use `std::shared_ptr` for shared-ownership resource management.](https://blog.csdn.net/Dong_HFUT/article/details/123599599)

上文中介绍了 std::unique_ptr ，它对指向的资源拥有独占所有权。本文介绍一种新的智能指针：`std::shared_ptr`，它和其他指向该资源的指针有共享所有权，它可以拷贝和传递，并且通过引用计数来管理资源的生命周期。

![](images/item_19_01.png)

``std::shared_ptr`` 的模型如上图所示：它包含两个指针，一个指向对象的原始指针和一个指向控制块的原始指针。所以 ``std::shared_ptr`` 的内存占用总是原始指针的两倍。

**引用计数**

``std::shared_ptr`` 是通过共享所有权的语义来管理对象的生命周期。对于指向该对象的所有 ``std::shared_ptr``，它们都不独占这个对象，它们合作来管理这个对象的生命周期：当最后一个指向对象的 ``std::shared_ptr`` 不再指向这个对象（比如，``std::shared_ptr`` 被销毁了或者指向了别的对象），``std::shared_ptr`` 会销毁它指向的对象。

``std::shared_ptr`` 实际是通控制块的引用计数（reference counter）来管理对象的生命周期。一个 ``std::shared_ptr`` 可以通过查看引用计数知道有多少个 ``std::shared_ptr`` 指向该对象。引用计数更新如下：
  - ``std::shared_ptr`` 的构造函数会通常增加引用计数。但是对于 move 构造函数：从一个``std::shared_ptr`` 移动构造一个``std::shared_ptr`` 会将源 ``std::shared_ptr`` 设置为 nullptr，源 ``std::shared_ptr`` 不再指向资源，并且新的 ``std::shared_ptr`` 开始指向资源。所以，它不需要维护引用计数。
  - ``std::shared_ptr`` 的析构函数会减少引用计数。
  - 拷贝 operator= 既增加也减少引用计数：如果 sp1 和 sp2 是指向不同对象的 ``std::shared_ptr``，赋值操作 “sp1 = sp2” 会修改 sp1 来让它指向 sp2 指向的对象。这个赋值操作的效果就是：原本被 sp1 指向的对象的引用计数减一，同时被 sp2 指向的对象的引用计数加一。

如果一个``std::shared_ptr`` 查询到一个引用计数在一次自减后变成 0 了，这就意味着没有别的 ``std::shared_ptr`` 指向这个资源了，所以 ``std::shared_ptr`` 就会销毁这个资源。

```c++
#include <iostream>
#include <memory>

int main() {
  `std::shared_ptr`<int> p1 = std::make_shared<int>(3);
  `std::shared_ptr`<int> p2 = std::move(p1);
  std::cout << "==== p1.use_count() = " << p1.use_count() << std::endl;
  std::cout << "==== p2.use_count() = " << p2.use_count() << std::endl;
  `std::shared_ptr`<int> p3 = std::make_shared<int>(4);
  `std::shared_ptr`<int> p4(p2);
  std::cout << "==== p3.use_count() = " << p3.use_count() << std::endl;
  std::cout << "==== p2.use_count() = " << p2.use_count() << std::endl;
  p4 = p3;
  std::cout << "==== p3.use_count() = " << p3.use_count() << std::endl;
  std::cout << "==== p2.use_count() = " << p2.use_count() << std::endl;
  return 0;
}

// output
==== p1.use_count() = 0
==== p2.use_count() = 1
==== p3.use_count() = 1
==== p2.use_count() = 2
==== p3.use_count() = 2
==== p2.use_count() = 1
```

另外，为了保证多线程安全，引用计数的增加和减少操作必须是原子操作。

**自定义deleter**

上一篇文章介绍过 `std::unique_ptr` 可以自定义 `deleter`，并且会增加 `std::unique_ptr` 占用内存大小。``std::shared_ptr`` 默认也使用 delete 来销毁资源，也支持自定义 `deleter`，但是其实现机制和 `std::unique_ptr` 不同。`std::unique_ptr` 的 `deleter` 是智能指针的一部分，但是对于 ``std::shared_ptr`` 并非如此，它的 `deleter` 是属于 control block，因此 ``std::shared_ptr`` 占用内存大小不会因为自定义 `deleter` 而改变。

```c++
auto loggingDel = [](Widget *pw)
				  {
					makeLogEntry(pw);
					delete pw;
				  };
std::unique_ptr<Widget, decltype(loggingDel)> // deleter type is
  upw(new Widget, loggingDel);                // part of ptr type
`std::shared_ptr`<Widget>         // deleter type is not
  spw(new Widget, loggingDel);  // part of ptr type
```

``std::shared_ptr`` 这样的设计更加灵活。看下面的例子：

```c++
auto customDeleter1 = [](Widget *pw) { … }; // custom deleters,
auto customDeleter2 = [](Widget *pw) { … }; // each with a different type
`std::shared_ptr`<Widget> pw1(new Widget, customDeleter1);
`std::shared_ptr`<Widget> pw2(new Widget, customDeleter2);

std::vector<`std::shared_ptr`<Widget>> vpw{ pw1, pw2 };
```

pw1 和 pw2 具有相同的类型，可以放到一个容器中。它们能互相赋值，并且它们都能被传给一个函数作为参数，只要这个函数的参数是`std::shared_ptr`类型。但是对于有自定义 `deleter` 的两个 `std::unique_ptr`，因为类型不同，无法做到这些功能。

**控制块**

上面介绍的引用计数和自定义 deleter 都是存在 `std::shared_ptr` 指向的控制块中。一个对象的控制块应该被指向这个对象的第一个 `std::shared_ptr`创建。通常，一个创建 `std::shared_ptr` 的函数不可能知道是否有其他 `std::shared_ptr` 已经指向这个对象，所以控制块的创建需要遵循以下规则：
- std::make_shared 总是创建一个控制块，它制造一个新对象，所以当 std::make_shared 被调用的时，这个对象没有控制块。
当一个 `std::shared_ptr` 的构造来自一个独占所有权的智能指针（`std::unique_ptr` 或 `std::auto_ptr`）时，创造一个控制块。独占所有权的指针不使用控制块，所以原来的被指向的对象没有控制块。
- 当使用一个原始指针构造 `std::shared_ptr` 时，它创造一个控制块。如果你想使用一个已有控制块的对象来创建一个`std::shared_ptr` 的话，你可以传入一个 `std::shared_ptr` 或一个 `std::weak_ptr` 作为构造函数的参数，但不能传入一个原始指针。使用 `std::shared_ptr` 或 `std::weak_ptr` 作为构造函数的参数不会创建一个新的控制块，因为它们能依赖传入的智能指针来指向必要的控制块。

这些规则产生一个结果：用一个原始指针来构造超过一个的 `std::shared_ptr` 的对象时，会让这个对象拥有多个控制块。

```c++
auto pw = new int;
`std::shared_ptr`<int> spw1(pw);
`std::shared_ptr`<int> spw2(pw);
```

使用原始指针变量作为 `std::shared_ptr` 构造函数的参数时，有一个特别让人惊奇的方式（涉及到 this 指针）会产生多个控制块。

```c++
std::vector<`std::shared_ptr`<Widget> processedWidget;
class Widget {
  public:
    ...
    void process();
    ...
};
void Widget::process() {
  processedWidget.emplace_back(this);   // add it to list of processed Widgets; this is wrong!
}
```

这段代码能编译，但是它传入一个原始指针（this）给一个 `std::shared_ptr` 的容器。因此 `std::shared_ptr` 的构造函数将为它指向的 Widget（*this）创建一个新的控制块。但是，如果在成员函数外面已经有 `std::shared_ptr` 指向这个 Widget，则会导致资源的 double free。例如如下代码：

```c++
`std::shared_ptr`<Widget> w(new Widget, loggingDel);
w->process();
```

如果你的类被 `std::shared_ptr` 管理，你可以继承 std::enable_shared_from_this，这样就能用this指针安全地创建一个`std::shared_ptr`。

```c++
class Widget;
std::vector<`std::shared_ptr`<Widget>> processedWidget;
auto loggingDel = [](Widget *pw) {
  delete pw;
};
class Widget : public std::enable_shared_from_this<Widget> {
  public:
    void process();
};
void Widget::process()  {
  processedWidget.emplace_back(shared_from_this());
}
int main() {
  {
    `std::shared_ptr`<Widget> w(new Widget, loggingDel);
    w->process();
  }
  return 0;
}
```

在使用 shared_from_this 返回 this 指针的 `std::shared_ptr` 的时候 shared_from_this 会先搜索当前对象的控制块，如果有就不会再创建控制块了。所以以上代码就不会产生 double free 的问题了。

但是，这个设计依赖于当前的对象已经有一个相关联的控制块了。也就是说，必须已经有一个 `std::shared_ptr` 指向当前的对象。如果没有，shared_from_this 也会抛出异常，它的行为还将是未定义的。

为了防止用户在一个 `std::shared_ptr` 指向这个对象前，调用成员函数（这个成员函数调用了 shared_from_this），继承自std::enable_shared_from_this 的类通常将它们的构造函数为申明为 private，并且让用户通过调用一个返回 `std::shared_ptr` 的工厂函数来创建对象。

```c++
class Widget: public std::enable_shared_from_this<Widget> {
public:
  // factory function that perfect-forwards args
  // to a private ctor
  template<typename... Ts>
  static `std::shared_ptr`<Widget> create(Ts&&... params);
  …
  void process(); // as before
  …
private:
  …     // ctors
};
```

此外，`std::shared_ptr` 另外一个和 std::unique_ptr 不同的地方是：`std::shared_ptr` 的 API 被设计为只能作为单一对象的指针。没有 `std::shared_ptr`<T[]>，但是使用 std::array，std::vector 和 std::string 可以满足这样的需求。

{{<admonition quote "总结" false>}}
- `std::shared_ptr` 为任意共享所有权的资源提供一种自动垃圾回收的便捷方式。
- 较之于 std::unique_ptr，`std::shared_ptr` 对象占用的内存通常大两倍，控制块会产生开销，需要原子引用计数修改操作。
- 默认资源销毁是通过 delete，但是也支持自定义 deleter。自定义 deleter 的类型对 `std::shared_ptr` 的类型没有影响。
- 避免从原始指针变量上创建 `std::shared_ptr`。
{{</admonition>}}


#### [Item 20: Use std::weak_ptr for `std::shared_ptr` like pointers that can dangle.](https://blog.csdn.net/Dong_HFUT/article/details/123612236)

**std::weak_ptr 的特点**

std::weak_ptr 通常不会单独使用，一般是与 `std::shared_ptr` 搭配使用，可以将 std::weak_ptr 类型指针视为 `std::shared_ptr` 指针的一种辅助工具，借用 std::weak_ptr 类型指针， 可以获取 `std::shared_ptr` 指针的一些状态信息，例如有多少 `std::shared_ptr` 指针指向相同的资源、`std::shared_ptr` 指针指向的内存是否已经被释放等。

std::weak_ptr 常常是通过 `std::shared_ptr` 构造而来，它和 std::shard_ptr 指向的相同的位置。但是，std::weak_ptr 不会影响对象的引用计数，也就是说，std::weak_ptr 被创建时，引用计数不会增加，当它被释放时，引用计数也不会减少。

```c++
auto spw =                     // after spw is constructed, the pointed-to Widget's
  std::make_shared<Widget>();  // ref count (RC) is 1.
  …
  std::weak_ptr<Widget> wpw(spw); // wpw points to same Widget as spw. RC remains 1
  …
  spw = nullptr;  // RC goes to 0, and the Widget is destroyed. wpw now dangles

  if (wpw.expired()) … // if wpw doesn't point to an object…
```

std::weak_ptr 没有解引用操作，但可以将它转换为 `std::shared_ptr`，使用 lock 可以保证线程安全。

```c++
`std::shared_ptr`<Widget> spw1 = wpw.lock(); // if wpw's expired, spw1 is null
auto spw2 = wpw.lock(); // same as above, but uses auto

`std::shared_ptr`<Widget> spw3(wpw); // if wpw's expired, throw std::bad_weak_ptr
```

**std::weak_ptr 的典型应用**

下面介绍 std::weak_ptr 的两个典型应用，其实本质上就是利用了 std::weak_ptr 的特点：**共享资源所有权，但又不增加其引用计数。**

<font color=red>循环引用</font>

std::weak_ptr 的一个典型应用是解决 `std::shared_ptr` 的内存泄露问题----循环引用。看下面的代码：

```c++
 #include <iostream>
 #include <memory>
 using namespace std;

class B;
class A {
  public:
    A() { cout << "A constructor" << endl; }
    ~A() { cout << "A destructor" << endl; }

    shared_ptr<B> b;
};

class B {
  public:
    B() { cout << "B constructor" << endl; }
    ~B() { cout << "B's destructor" << endl; }

    shared_ptr<A> a;
};

int main() {
  `std::shared_ptr`<A> aa = make_shared<A>(); // aa 引用计数为 1
  `std::shared_ptr`<B> bb = make_shared<B>(); // bb 引用计数为 1

  aa->b = bb;// bb 引用计数为 2
  bb->a = aa;// aa 引用计数为 2

  return 0;
}

// output
A constructor
B constructor
```

从运行结果可以看到 A 和 B 都调用了构造函数，却没有调用析构函数，导致了资源泄露。原因是 main 函数结束后，两个对象的引用计数都为 1 ，导致 `std::shared_ptr` 没有调用析构函数。解决办法是将 A 和 B 对象中 shared_ptr 换成 weak_ptr 即可。

<font color=red>带缓存的工厂方法</font>

当调用工厂方法的代价比较高时，可以通过增加缓存来优化。但是把所有对象都缓存下来会造成效率问题，当对象不再使用时，可以销毁其缓存。

示例代码参考这里:

```c++
#include <iostream>
#include <unordered_map>
#include <memory>


using namespace std;

/*!
 * \brief The Investment class 基类
 */
class Investment
{
public:
    virtual ~Investment() = default;

public:
    virtual void doWork() = 0;
};

/*!
 * \brief The Stock class 派生类
 */
class Stock : public Investment
{
public:
    ~Stock() {
        cout << "~Stock() called....\n";
    }
public:
    virtual void doWork() override {
        cout << "Stock doWork....\n";
    }
};

/*!
 * \brief The Stock class 派生类
 */
class Bond : public Investment
{
public:
    ~Bond() {
        cout << "~Bond() called....\n";
    }

public:
    virtual void doWork() override {
        cout << "Bond doWork....\n";
    }
};

enum class InvestType {
    INVEST_TYPE_STOCK,
    INVEST_TYPE_BOND,
};

// 工厂函数
auto makeInvestment(InvestType type)
{
    // 自定义析构器, 这里以lambda表达式的形式给出
    auto delInvmt = [](Investment *pInvestment) {
        cout << "custom delInvmt called...." << pInvestment << endl;

        // 注意：pInvestment可能为空指针，比如默认为空，然后调用reset赋值时，会先调用一遍析构
        if (pInvestment)
        {
            // TODO 自定义析构时想干的事
            delete pInvestment;
        }
    };

    // 待返回的指针, 初始化为空指针
    shared_ptr<Investment> pInv(nullptr);

    // 注意这里用reset来指定pInv获取从new产生的对象的所有权, 不能用=赋值
    switch (type)
    {
    case InvestType::INVEST_TYPE_STOCK:
        // 注意：自定义析构器是随对象一起指定的，这里区别于unique_ptr
        pInv.reset(new Stock, delInvmt);
        break;
    case InvestType::INVEST_TYPE_BOND:
        // 如果不指定自定义析构器的话，则不会调用
        pInv.reset(new Bond);
        break;
    }

    // 返回智能指针
    return pInv;
}

// 带缓存的工厂函数
// 使用场景：当调用工厂函数makeInvestment成本高昂(e.g. 会执行一些文件或数据块的I/O操作), 并且type会频繁的重复调用
shared_ptr<Investment> fastLoadInvestment(InvestType type)
{
    // 定义一个做缓存的容器，注意这里存的内容是weak_ptr
    // 使用weak_ptr的好处是，它不会影响所指涉对象的引用计数
    // 如果这里改为shared_ptr的话，则函数外边永远不会析构掉这个对象, 因为缓存中至少保证其引用计数为1。这就背离的我们的设计
    static unordered_map<InvestType, weak_ptr<Investment>> s_cache;


    // 将weak_ptr生成shared_ptr
    auto pInv = s_cache[type].lock();

    // 如果缓存中没有的话，则调用工厂函数创建一个新对象，并且加入到缓存中去
    if (!pInv)
    {
        cout << "create new investment..\n";
        pInv = makeInvestment(type);
        s_cache[type] = pInv;
    }

    return pInv;
}

int main () {
    {
        auto pInv = fastLoadInvestment(InvestType::INVEST_TYPE_BOND);
        pInv->doWork();
    }

    cout << "-------------------------------\n";

    {
        auto pInv = fastLoadInvestment(InvestType::INVEST_TYPE_BOND);
        pInv->doWork();
    }

    cout << "-------------------------------\n";

    {
        auto pInv = fastLoadInvestment(InvestType::INVEST_TYPE_STOCK);
        pInv->doWork();
        auto pInv2 = fastLoadInvestment(InvestType::INVEST_TYPE_STOCK);
        pInv2->doWork();
    }

    return 0;
}

// output
create new investment..
Bond doWork....
~Bond() called....
-------------------------------
create new investment..
Bond doWork....
~Bond() called....
-------------------------------
create new investment..
Stock doWork....
Stock doWork....
custom delInvmt called....0x1258cd0
~Stock() called....
```

对象的缓存管理器需要一个类似 `std::shared_ptr` 的指针，但又想这些对象的生存期可以由调用者来管理来管理，因而使用 std::weak_ptr 可以满足这种需求。


{{<admonition quote "总结" false>}}
- 对类似 `std::shared_ptr` 可能悬空的指针，使用 std::weak_ptr。
- std::weak_ptr 的潜在使用场景包括：caching、observer lists、避免 `std::shared_ptr` 的循环引用。
{{</admonition>}}



#### [Item 21: Prefer std::make_unique and std::make_shared to direct use of new.](https://blog.csdn.net/Dong_HFUT/article/details/123622543)

std::make_shared 是 C++11 开始支持的，但是 std::make_unique 是 C++14 才开始支持。如果你的编译器只支持 C++11，你可以实现自己的 make_unique。

```c++
template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}
```

std::make_unique 和 std::make_shared 是三个 make 函数中的两个，第三个 make 函数是 std::allocate_shared。它的行为和std::make_shared 一样，唯一的不同是它的第一个参数是一个分配器（allocator）对象，这个对象是用来动态申请内存的。make 函数能传入任意集合的参数，然后完美转发给构造函数，并动态创建一个对象，然后返回指向这个对象的智能指针。

创建智能指针有两种方式，一种是使用 make 函数，另一种是使用 new 直接创建。下面介绍二者的优缺点，并建议尽可能使用 make 函数。

**make 函数的优点**

<font color=red>支持 auto</font>

```c++
auto upw1(std::make_unique<Widget>());       // with make func
std::unique_ptr<Widget> upw2(new Widget);    // without make func
auto spw1(std::make_shared<Widget>());       // with make func
`std::shared_ptr`<Widget> spw2(new Widget);    // without make func
```
使用 make 函数的第一个优点是支持 auto，避免重复代码，使得代码更加清晰好维护。


<font color=red>避免异常</font>

使用 make 函数的第二个优点跟异常安全有关。先看下面这个例子：

```c++
void processWidget(`std::shared_ptr`<Widget> spw, int priority);  // declare

processWidget(`std::shared_ptr`<Widget>(new Widget), computePriority());  // potential resource leak!
processWidget(std::make_shared<Widget>(), computePriority());           // no potential resource leak
```

如果使用 new，processWidget 调用时，产生如下步骤：
- 执行 new Widget
- 执行 `std::shared_ptr` 的构造
- 执行 computePriority()

但是，编译器可能不一定产生上述代码顺序。new Widget 肯定时要在 `std::shared_ptr` 的构造函数之前执行，但 computePriority() 可能在这两个步骤的前、中或后产生，可能时这样：
- 执行 new Widget
- 执行 computePriority()
- 执行 `std::shared_ptr` 的构造

如果 computePriority() 产生异常，第一步 new 的 Widget 还未被 `std::shared_ptr` 接管，会产生内存泄漏。使用 make 函数则不会有这样的问题。


<font color=red>效率更高</font>

使用 make 函数的第三个优点是可以避免多次内存分配、效率更高。

```c++
`std::shared_ptr`<Widget> spw(new Widget);
auto spw = std::make_shared<Widget>();
```

使用 new，需要分配两次内存，一次分配 Widget 的内存，一次分配控制块的内存。若使用 make 函数，则只需要分配一次内存块，make 函数（`std::shared_ptr` 和 std::allocate_shared）会申请一块内存同时存储 Widget 和控制块。

**make函数的缺陷**

上面介绍了 make 函数的优点，下面介绍 make 函数的缺陷。

<font color=red>无法自定义 deleter</font>

使用 new，可以自定义 deleter，但是 make 函数无法做到。

```c++
auto widgetDeleter = [](Widget* pw) { … };

std::unique_ptr<Widget, decltype(widgetDeleter)>  upw(new Widget, widgetDeleter);
`std::shared_ptr`<Widget> spw(new Widget, widgetDeleter);
```

<font color=red>语义歧义</font>

对于 std::vector，支持使用圆括号和花括号两种初始化方法：

```c++
std::vector<int> p(10, 20);   // 10 elements, every element is 20
std::vector<int> p2{10, 20};  // two elements: 10 and 20
```

但是，make 函数不支持花括号的形式。原因是圆括号支持完美转发，花括号不支持完美转发，使用 make 函数可以完美转发圆括号。如果你想使用花括号进行初始化，只能使用 new。

```c++
auto sp1 = std::make_shared<std::vector<int>>(10, 20);
`std::shared_ptr`<std::vector<int>> sp2(new std::vector{10,20});
```

但是，Item 30 将会给出一个变通方案：使用auto类型推导来从初始化列表创建一个 std::initializer_list 对象，然后传入 auto 创建的对象给 make 函数：

```c++
// create std::initializer_list
auto initList = { 10, 20 };
// create std::vector using std::initializer_list ctor
auto spv = std::make_shared<std::vector<int>>(initList);
```

<font color=red>延长对象销毁时间</font>

对于 make_shared_ptr ，它是申请一块内存块，用于储存对象和控制块。我们知道，创建 shared_ptr 时候会附属产生 weak_ptr, 它也有一个引用计数（weak 计数）存储在控制块中。

std::weak_ptr 是通过检查控制块中的引用计数（非 weak counter）判断自己是否失效。如果引用计数为 0，则 weak_ptr 失效，否则未失效。但是，只有 weak counter 不为 0，整个控制块就必须存在，那么 shared_ptr 指向的对象也不能释放。如果对象类型很大，并且最后一个 std::shared_pt r和最后一个 std::weak_ptr 销毁的间隔很大，那么一个对象销毁将延迟到最后才能释放。

```c++
class ReallyBigType { … };
auto pBigObj = std::make_shared<ReallyBigType>();  // create very large object via std::make_shared
…    // create `std::shared_ptr`s and std::weak_ptrs to large object, use them to work with it

…    // final `std::shared_ptr` to object destroyed here, but std::weak_ptrs to it remain

…    // during this period, memory formerly occupied by large object remains allocated

…    // final std::weak_ptr to object destroyed here;  memory for control block and object is released
```

如果使用 new，因为是两块内存块，只要最后一个指向 ReallyBigType 对象的 `std::shared_ptr` 销毁了，这个对象的内存就能被释放：

```c++
class ReallyBigType { … }; // as before
`std::shared_ptr`<ReallyBigType> pBigObj(new ReallyBigType);  // create very large object via new

… // as before, create `std::shared_ptr`s and std::weak_ptrs to object, use them with it

… // final `std::shared_ptr` to object destroyed here, but std::weak_ptrs to it remain; memory for object is deallocated

… // during this period, only memory for the control block remains allocated

… // final std::weak_ptr to object destroyed here; memory for control block is released
```

**一个 trick**
讲完 make 的优缺点，我们回顾下上面说过的一个使用 new 可能导致内存泄漏的问题：

```c++
void processWidget(`std::shared_ptr`<Widget> spw, int priority);  // as before
void cusDel(Widget *ptr); // custom deleter

processWidget(`std::shared_ptr`<Widget>(new Widget, cusDel),    // potential resource leak!
              computePriority());
```

如果修改如下：

```c++
`std::shared_ptr`<Widget> spw(new Widget, cusDel);
processWidget(spw, computePriority());  // correct, but not optimal; see below
```

这样可以避免内存泄漏，但是效率不高。可能存在异常泄漏的版本，我们传递给 processWidget 的是一个右值，而上面这个安全版本传递的是左值。传递右值只需要 move，而传递左值必须要拷贝，拷贝一个 `std::shared_ptr` 要求对它的引用计数进行一个原子的自增操作，但是 move 一个 `std::shared_ptr` 不需要修改引用计数。因此，上面的安全版本可以通过 move 来优化：

```c++
`std::shared_ptr`<Widget> spw(new Widget, cusDel);
processWidget(std::move(spw),  computePriority()); // both efficient and exception safe
```

这样，使用 new，既安全又没有性能损失，并且还支持自定义 deleter。

最后，还是建议优先使用 make 函数，除非你有特殊的原因。

{{<admonition quote "总结" false>}}
- 和直接使用 new 相比，make 函数消除了代码重复、提高了异常安全性。对于 std::make_shared和 std::allocate_shared，生成的代码更小更快。
- 不适合使用 make 函数的情况包括需要指定自定义删除器和希望用大括号初始化。
- 对于`std::shared_ptr`s, make函数可能不被建议的其他情况包括 (1)有自定义内存管理的类和 (2)特别关注内存的系统、非常大的对象，以及 std::weak_ptrs 比对应的 `std::shared_ptr`s 存在的时间更长。
{{</admonition>}}

#### [Item 22: When using the Pimpl Idiom, define special member functions in the implementation file.](https://blog.csdn.net/Dong_HFUT/article/details/123704824)

这部分介绍一个智能指针的应用场景：PImpl技术，俗称编译防火墙。

**PImpl 技术（编译防火墙）**

PImpl（Pointer to implementation）是一种 C++ 惯用技术，它是通过将类的具体实现放在另一个单独的类（或结构体）中，并通过不透明的指针进行访问。该技术能够将具体的实现细节从对象中去除，能够有效地减少编译依赖。也被称为“编译防火墙（Compilation Firewalls）”。看一个例子：

```c++
class Widget {  // in header "widget.h"
public:
  Widget();
  ...
private:
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;        // Gadget is some user-defined type
};
```

这里， Widget 中包含 std::string、std::vector 和 Gadget 类型成员变量，对于 Widget 的客户，必须包含 <string> 、<vector> 和 gadget.h 这些头文件。一旦这些头文件的内容发生改变（当然，string 和 vector 两个头文件的内容很少被修改），使用 Widget 的客户代码必须重新编译。

PImpl 技术可以很好的解决这个问题。将 Widget 类中的数据成员变量封装成一个类（或结构体），然后将数据成员替换成这个类（或结构体）的指针。

```c++
class Widget { // still in header "widget.h"
public:
  Widget();
  ~Widget();  // dtor is needed—see below
  ...
private:
  struct Impl;  // declare implementation struct
  Impl *pImpl;  // and pointer to it
};
```

Widget 类中不在提及 std::string、std::vector 和 Gadget 类型，因而无需包含相应的头文件。即使这些头文件的内容发生改变， Widget 客户代码也无需重新编译。

一个只声明不定义的类型被成为不完整类型（incomplete type），Widget::Impl 就是这样的一个类型，声明一个指向它的指针是可以编译的，PImpl 技术就是利用了这一点。PImpl 技术的典型方式如下：
 - 前向声明一个类型，然后申明一个指向这个类型的指针。
 - 在原始类的实现文件中定义这个类型，并实现这个指针的动态内存分配和回收。

```c++
#include "widget.h"   // in impl. file "widget.cpp"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {
  std::string name;            // definition of Widget::Impl
  std::vector<double> data;    // with data members formerly in Widget
  Gadget g1, g2, g3;
};

Widget::Widget()    // allocate data members for
: pImpl(new Impl)   // this Widget object
{}

Widget::~Widget()   // destroy data members for
{ delete pImpl; }   // this object
```

这样，就把 <string> 、<vector> 和 gadget.h 这些头文件的依赖从 widget.h （Widget 的客户可见）中转移到了 widget.cpp （Widget 的实现者可见）中。

以上就是 Pimpl 技术的基本原理，这里都是直接使用原始指针，完全是 C++98 风格的实现，C++11 之后，我们更倾向使用智能指针来代替原始指针。

**PImpl 技术的智能指针版本**

这里，std::unique_ptr 是比较合适用来替换原始指针的，我们修改上面的代码：

```c++
// in "widget.h"
#include <memory>
class Widget {
public:
  Widget();
  ...
private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;  // use smart pointer instead of raw pointer
};

//==================================================================================//
// in "widget.cpp"
#include "widget.h"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {  // as before
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()
: pImpl(std::make_unique<Impl>())
{}
```

由于智能指针的自动析构指向的资源，这里无需析构函数。

```c++
#include "widget.h"
int main() {
  Widget w;
  return 0;
}
```

但是，以上代码会编译报错，报错信息如下：

```shell
In file included from /usr/include/c++/9/memory:80,
                 from widget.h:3,
                 from hello.cpp:1:
/usr/include/c++/9/bits/unique_ptr.h: In instantiation of ‘void std::default_delete<_Tp>::operator()(_Tp*) const [with _Tp = Widget::Impl]’:
/usr/include/c++/9/bits/unique_ptr.h:292:17:   required from ‘std::unique_ptr<_Tp, _Dp>::~unique_ptr() [with _Tp = Widget::Impl; _Dp = std::default_delete<Widget::Impl>]’
widget.h:5:7:   required from here
/usr/include/c++/9/bits/unique_ptr.h:79:16: error: invalid application of ‘sizeof’ to incomplete type ‘Widget::Impl’
   79 |  static_assert(sizeof(_Tp)>0,
      |
```

编译报错分析如下：

> 在 main 函数结束时候，离开了 w 的作用域，w 将被销毁，也即会调用 Widget 的析构函数。我们的代码虽然没有定义 Widget 的析构函数，但是根据 Item > 17 的介绍可以知道，编译器会生成默认的析构函数（inline 的）。析构函数中会释放 pImpl，pImpl 的是 std::unique_ptr<Impl> 类型。而 std::unique_ptr 默认使用 delete 析构，默认的 delete 代码中会使用 static_assert 确保指针不会指向一个不完整类型，这里用到了 sizeof，而一个不完整类型无法进行 sizeof。

```c++
  /// Primary template of default_delete, used by unique_ptr
  template<typename _Tp>
  struct default_delete
  {
      /// Default constructor
      constexpr default_delete() noexcept = default;

      /** @brief Converting constructor.
       *
       * Allows conversion from a deleter for arrays of another type, @p _Up,
       * only if @p _Up* is convertible to @p _Tp*.
       */
      template<typename _Up, typename = typename
         enable_if<is_convertible<_Up*, _Tp*>::value>::type>
        default_delete(const default_delete<_Up>&) noexcept { }

      /// Calls @c delete @p __ptr
      void operator()(_Tp* __ptr) const
      {
		  static_assert(!is_void<_Tp>::value,
		          "can't delete pointer to incomplete type");
		  static_assert(sizeof(_Tp)>0,
		          "can't delete pointer to incomplete type");
		  delete __ptr;
      }
  };
```

所以，要解决这个问题，只需要保证在销毁 `std::unique_ptr<Impl>` 时，Impl 是一个完整类型即可，也即有定义。Impl 的定义在 Wigdet.cpp 文件中，因此我们只需让编译器在 Wigdet.cpp 生成析构函数即可。

```c++
// in "widget.h"
#include <memory>
class Widget {
public:
  Widget();
  ~Widget();   // declaration only
  ...
private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;  // use smart pointer instead of raw pointer
};

//==================================================================================//
// in "widget.cpp"
#include "widget.h"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {  // as before
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()
: pImpl(std::make_unique<Impl>())
{}

Widget::~Widget() {}  // ~Widget definition
// Widget::~Widget() = default;  // same effect as above
```

由于智能指针的自动析构指向的资源，这里无需析构函数。

```c++
#include "widget.h"
int main() {
  Widget w;
  return 0;
}
```

这样就可以编译通过了。但是上面的实现还有点问题：从 Item 17 介绍我们知道，析构函数的声明会阻止编译器生成 move 操作，那么以上代码将不支持 move 操作了。解决办法也是相似的：

```c++
// in "widget.h"
#include <memory>
class Widget {
public:
  Widget();
  ~Widget();   // declaration only

  Widget(Widget&& rhs);              // declarations
  Widget& operator=(Widget&& rhs);   // only
  ...
private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;  // use smart pointer instead of raw pointer
};

//==================================================================================//
// in "widget.cpp"
#include "widget.h"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {  // as before
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()
: pImpl(std::make_unique<Impl>())
{}

Widget::~Widget() {}  // ~Widget definition
// Widget::~Widget() = default;  // same effect as above

Widget::Widget(Widget&& rhs) = default;
Widget& Widget::operator=(Widget&& rhs) = default;
```

这样 Widget 具备了 move 操作。但是还是存在一点问题：
- 根据 Item 17 知道，因为自定义了 move 操作，将会阻止编译器生成 copy 操作。
- 即使编译器生成了 copy 操作（使用 = default 进行声明），也是一个浅拷贝，std::uniqe_ptr 是一个所有权独享的对象，对它进行拷贝会转移所有权。

因此，需要我们自定义 copy 操作：

```c++
// in "widget.h"
#include <memory>
class Widget {
public:
  Widget();
  ~Widget();   // declaration only

  Widget(Widget&& rhs);              // declarations
  Widget& operator=(Widget&& rhs);   // only

  Widget(const Widget& rhs);               // declarations
  Widget& operator=(const Widget& rhs);    // only
  ...
private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;  // use smart pointer instead of raw pointer
};

//==================================================================================//
// in "widget.cpp"
#include "widget.h"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {  // as before
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()
: pImpl(std::make_unique<Impl>())
{}

Widget::~Widget() {}  // ~Widget definition
// Widget::~Widget() = default;  // same effect as above

Widget::Widget(Widget&& rhs) = default;
Widget& Widget::operator=(Widget&& rhs) = default;

Widget::Widget(const Widget& rhs)              // copy ctor
: pImpl(std::make_unique<Impl>(*rhs.pImpl))
{}
Widget& Widget::operator=(const Widget& rhs)  // copy operator=
{
  *pImpl = *rhs.pImpl;
  return *this;
}
```

到目前为止，以上代码的实现是比较完整的了。

为了实现 Pimpl 技术，std::unique_ptr 是合适的，因为 pImpl 指针对 Impl 有独有所有权。如果你使用 `std::shared_ptr` 代替 std::unique_ptr，以上出现的问题将不会出现。示例如下：

```c++
// in "widget.h"
#include <memory>
class Widget {
public:
  Widget();
  ...
private:
  struct Impl;
  `std::shared_ptr`<Impl> pImpl;
};

// in "widget.cpp"
#include "widget.h"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {  // as before
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()
: pImpl(std::make_shared<Impl>())
{}

//======================================================//
Widget w1;
auto w2(std::move(w1)); // move-construct w2
w1 = std::move(w2);     // move-assign w1
```

`std::shared_ptr` 的 deleter 不是其自身的一部分，属于控制块，我们的代码不会包含删除器的代码，因此不需要自定义析构函数，那么 move 和 copy 操作都会自定生成。而 `std::shared_ptr` 又是值语义的，拷贝也不会发生问题（通过引用计数进行内存管理）。

{{<admonition quote "总结" false>}}
- pImpl 惯用法通过减少类实现和类使用者之间的编译依赖来减少编译时间。
- 对于std::unique_ptr 类型的 pImpl 指针，需要在头文件的类里声明特殊的成员函数，但是在实现文件里面来实现他们。即使是编译器自动生成的代码可以工作，也要这么做。
- 以上的建议只适用于 std::unique_ptr，不适用于 `std::shared_ptr`。
{{</admonition>}}

### CH05: Rvalue References, Move Semantics, and Perfect Forwarding
#### [Item 23: Understand std::move and std::forward.](https://blog.csdn.net/Dong_HFUT/article/details/123765869)

std::move 和 std::forward 并不像他们名字所表达的那样，实际上 std::move 并没有移动数据，std::forward 也并没有转发数据，并且它们在运行期什么也没做。

先说 std::move，我们看下它在 C++11 中简易的实现如下：

```c++
template<typename T>    // in namespace std
typename remove_reference<T>::type&&
move(T&& param)
{
  using ReturnType =                          // alias declaration;
	typename remove_reference<T>::type&&;  // see Item 9
  return static_cast<ReturnType>(param);
}
```

std::move 只是返回了右值引用。这里使用了 remove_reference 是为了去除引用标识符。当 T 是一个引用类型的时候，根据引用折叠原理，T&& 会被折叠成一个左值引用类型。所以 remove_reference 是为了去防止 T 是一个引用类型， 它会去除引用进而保证 std::move 返回一个右值引用。因此 std::move 只是做了类型转换，并没有移动数据。由于只有右值是可以被移动的，std::move 更像是说明经过它之后对象可能会被移动（可能，而不是一定，后文会有解释）。

而 C++14 的 std::move 更加简洁：

```c++
template<typename T>             // C++14; still in
decltype(auto) move(T&& param)   // namespace std
{
  using ReturnType = remove_reference_t<T>&&;
  return static_cast<ReturnType>(param);
}
```

std::move 的目的就是让编译器把修饰的变量看做是右值，进而就可以调用其移动构造函数。事实上，右值是仅可以被移动的对象，std::move 之后不一定一定调用构造函数。看下面的例子，假如你有这样的一个类：

```c++
class Annotation {
  public:
    explicit Annotation(std::string text) : text_(text)
    std::string text_;
}

class Annotation {
 public:
  explicit Annotation(std::string text) : text_(std::move(text)) {}
  std::string text_;
};

class Annotation {
 public:
  //这里换成了带有const
  explicit Annotation(const std::string text) : text_(std::move(text)) {}
  std::string text_;
};
```

第一个实现会发生两次拷贝，第二个实现会发生一次拷贝和一次移动，那么第三个实现会发生什么呢？

由于 Annotation 的构造函数传入的是一个 const std::string text，std::move(text) 会返回一个常量右值引用，也就是 const 属性被保留了下来。而 std::string 的 move 构造函数的参数只能是一个非 const 的右值引用，这里不能去调用 move 构造。只能调用 copy 构造，因为 copy 构造函数的参数是一个 const 引用，它是可以指向一个 const 右值。因此，第三个实现也是发生两次拷贝。

也可以用下面的例子验证一下：

```c++
#include <iostream>
#include <boost/type_index.hpp>

using boost::typeindex::type_id_with_cvr;

class A {
public:
  A(){
    std::cout << "constructon" << std::endl;
  }
  A(const A& a) {
    std::cout << "copy constructon" << std::endl;
  }
  A(A&& a) {
    std::cout << "move constructon" << std::endl;
  }
};

int main() {
  const A a1;
  std::cout << type_id_with_cvr<decltype(std::move(a1))>().pretty_name() << std::endl;
  auto a2(std::move(a1));

  return 0;
}

// output
constructon
A const&&
copy constructon
```
因此，我们可以总结出两点启示：
- 第一，假如你想对象能够真正被移动，不要声明将其申明为 const，对 const 对象的移动操作会被转换成了拷贝操作。
- 第二，std::move 不仅不移动任何东西，甚至不能保证被转换的对象可以被移动。唯一可以确认的是应用 std::move 的对象结果是个右值。

再说 std::forward。std::forward 也并没有转发数据，本质上只是做类型转换，与 std::move 不同的是，std::move 是将数据无条件的转换右值，而 std::forward 的转换是有条件的：当传入的是右值的时候将其转换为右值类型。

看一个 std::forward 的典型应用：

```c++
#include<iostream>
#include<chrono>

class Widget {
};

void process(const Widget& lvalArg) {
  std::cout << "process(const Widget& lvalArg)" << std::endl;
}

void process(Widget&& rvalArg) {
  std::cout << "process(Widget&& rvalArg)" << std::endl;
}

template<typename T>
void logAndProcess(T&& param) {
  auto now = std::chrono::system_clock::now();
  process(std::forward<T>(param));
}

int main () {
  Widget w;
  logAndProcess(w);              // call with lvalue
  logAndProcess(std::move(w));   // call with rvalue
}

// output
process(const Widget& lvalArg)
process(Widget&& rvalArg)
```

当我们通过左值去调用 logAndProcess 时，自然期望这个左值可以同样作为一个左值转移到 process 函数，当我们通过右值去调用 logAndProcess 时，我们期望这个右值可以同样作为一个右值转移到 process 函数。

但是，对于 logAndProcess 的参数 param，它是个左值（可以取地址）。在 logAndProcess 内部只会调用左值的 process 函数。为了避免这个问题，当且仅当传入的用来初始化 param 的实参是个右值，我们需要 std::forward 来把 param 转换成一个右值。至于 std::forward 是如何知道它的参数是通过一个右值来初始化的，将会在 Item 28 中会解释这个问题。

{{<admonition quote "总结" false>}}
- std::move 无条件将输入转化为右值。它本身并不移动任何东西。
- std::forward 把其参数转换为右值，仅仅在参数被绑定到一个右值时。
- std::move 和 std::forward 只是做类型转换，在运行时（runtime）不做任何事。
{{</admonition>}}


#### [Item 24: Distinguish universal references from rvalue references.](https://blog.csdn.net/Dong_HFUT/article/details/123773321)

在 C++11 移动语义出现后，遇到 T&& ，你可能认为就是右值引用，其实不然，这可能是一个万能引用（universal reference），右值引用和万能引用只是形式上相似而已，二者实际上是两个概念。右值引用只能绑定到一个右值上；而万能引用既可以绑定到一个右值，也可以绑定到一个左值。另外，万能引用能绑定到 const 或非 const 对象，也能绑定到 volatile 或非 volatile 对象，甚至能绑定到 const 加 volatile 的对象。

```c++
void f(Widget&& param);           // rvalue reference
Widget&& var1 = Widget();         // rvalue reference
auto&& var2 = var1;               // not rvalue reference
template<typename T>
void f(std::vector<T>&& param);   // rvalue reference
template<typename T>
void f(T&& param);                // not rvalue reference
```

万能引用主要有两种形式，一种是作为函数模版参数：

```c++
template<typename T>
void f(T&& param);  // param is a universal reference
```

一种是使用 auto 进行类型推导：

```c++
auto&& var2 = var1;  // var2 is a universal reference
```

两种形式有一个共同点，那就是形式为 T&& 或者 auto&&，并且存在类型推导。像下面的代码就不是万能引用：

```c++
void f(Widget&& param);   // no type deduction; param is an rvalue reference
Widget&& var1 = Widget(); // no type deduction; var1 is an rvalue reference
```

万能引用是一种引用，它必须被初始化。万能引用的初始化决定了它代表一个右值还是一个左值。如果初始化为一个右值，万能引用对应右值引用。如果初始化为一个左值，万能引用对应一个左值引用。

```c++
template<typename T>
void f(T&& param); // param is a universal reference
Widget w;
f(w); // lvalue passed to f; param's type is Widget& (i.e., an lvalue reference)
f(std::move(w)); // rvalue passed to f; param's type is Widget&& (i.e., an rvalue reference)
```

要使一个引用成为万能引用，必须满足两个条件：形式为 T&& 或者 auto&& （当然 T 只是符号，也可以是 type&& 等等），并且存在类型推导。

```c++
template<typename T>
void f(std::vector<T>&& param); // param is an rvalue reference

std::vector<int> v;
f(v); // error! can't bind lvalue to rvalue reference
```

这个例子中，如果调用点没有显示指明类型，也会发生类型推导。但它的形式不是 T&&，而是 std::vector&&，只是右值引用，若传递一个左值给 f ，将发生编译报错。

T&& 的形式是要求非常严格的，哪怕加一个 const 在前面，就会变成右值引用：

```c++
template<typename T>
void f(const T&& param); // param is an rvalue reference
```

还需要注意，存在形式完全是 `T&&`，但因为不存在类型推导而不是万能引用的例子，std::vector 的 push_back 就是这样的情况：

```c++
template<class T, class Allocator = allocator<T>> // from C++ Standards
class vector {
public:
  void push_back(T&& x);
  …
};
```

push_back 的参数完全符合万能引用的形式，但是没有类型推导发生。因为 push_back 不能存在于vector 的特定实例之外，并且实例的类型就完全决定了 push_back 的声明类型。

```c++
std::vector<Widget> v;  // declare

// causes the std::vector template to be instantiated as follows:
class vector<Widget, allocator<Widget>> {
public:
  void push_back(Widget&& x); // rvalue reference
  …
};
```

std::vector 中和 push_back 概念上相似的 emplace_back 用到了类型推导：

```c++
template<class T, class Allocator = allocator<T>> // still from C++ Standards
class vector {
public:
  template <class... Args>
  void emplace_back(Args&&... args);
  …
};
```

类型参数 Args （Args 其实是一个参数包，不是一个类型参数，可以把它视为一个类型参数。）独立于 vector 的类型参数 T，所以每次 emplace_back 被调用的时，Args 必须被推导，因而这里是万能引用。

auto&& 形式的变量也发生类型推导，也是万能引用。auto 形式的万能引用在 C++14 的 lambda 表达式中非常常见：

```c++
auto timeFuncInvocation =
  [](auto&& func, auto&&... params) // C++14
  {
    start timer;
    std::forward<decltype(func)>(func)(          // invoke func
      std::forward<decltype(params)>(params)...  // on params
    );
    stop timer and record elapsed time;
  };
```

func 是一个万能引用，它能被绑定到任何调用的对象上，不管是左值还是右值。params 也是一个万能引用，它能被绑定到任何数量的任意类型的对象上去。

{{<admonition quote "总结" false>}}
- 如果一个函数模板参数有 T&& 格式，并且发生类型推导，或者一个对象使用 auto&& 来声明，那么参数或对象就是一个万能引用。
- 如果类型推导的格式不是准确的 T&&（type&&），或者如果类型推导没有发生，T&&（type&&）就是一个右值引用。
- 如果用右值来初始化，万能引用相当于右值引用。如果用左值来初始化，则相当于左值引用。
{{</admonition>}}

#### [Item 25: Use std::move on rvalue references, std::forward on universal references.](https://blog.csdn.net/Dong_HFUT/article/details/123946594)

如果函数参数为右值引用，那么这个参数只能绑定到一个右值，你应该充分利用右值的特性（可以被移动），使用 std::move 无条件将参数转换为右值。

```c++
class Widget {
public:
  Widget(Widget&& rhs)        // rhs is rvalue reference
  : name(std::move(rhs.name)),
    p(std::move(rhs.p))
    { … }
  …
private:
  std::string name;
  `std::shared_ptr`<SomeDataStructure> p;
};
```

万能引用既可以绑定到右值，也可以绑定到左值。当万能引用的参数被初始为右值时候，应该使用 std::forward 将其转换为右值。

```c++
class Widget {
public:
  template<typename T>
  void setName(T&& newName)  // newName is universal reference
  { name = std::forward<T>(newName); }
  …
};
```

总的来说，在转发右值引用参数给其他函数时候，应该使用 std::move 无条件将其转为右值。当转发万能引用参数给其他函数时候，应该使用 std::forward 有条件将其转换为右值，因为万能引用有可能绑定到右值。

虽然参数是右值引用时候，使用 std::forward 会将其转换为右值，但还是建议你使用 std::move，因为这样代码更加简洁，也更符合习惯。

如果参数是万能引用，则需要避免使用 std::move 转换为右值。看下面的例子：

```c++
class Widget {
public:
  template<typename T>
  void setName(T&& newName) // universal reference
  { name = std::move(newName); }
  …
private:
  std::string name;
  `std::shared_ptr`<SomeDataStructure> p;
};

std::string getWidgetName(); // factory function
Widget w;
auto n = getWidgetName(); // n is local variable
w.setName(n); // moves n into w!
…             // n's value now unknown
```

这里使用 std::move 将会无条件将参数转为为右值，n 会被移动给 w.name，n 会变空，这显然不是好的代码设计。为了让 setName 函数不修改入参，有人可能会想通过重载 setName 改善上面代码：

```c++
class Widget {
public:
  void setName(const std::string& newName)  // set from const lvalue
  { name = newName; }
  void setName(std::string&& newName)  // set from rvalue
  { name = std::move(newName); }
  …
};
```

这依然不是好的设计，还是有缺点。一方面，上面的代码可能比较低效，考虑这样的调用：

```c++
w.setName("Adela Novak");
```

std::string 是可以直接通过字面字符串进行构造，如果是万能引用版本，则可以直接在 setName 内部通过字面字符串直接构造 w.name。但是对于重载版本的 setName 来说，则会产生临时的 std::string 对象。

另一方面，最大的缺点是 setName 的参数若有 N 个的话，那需要写 2^N 个重载函数。更糟糕的是，像模板函数不限制个数的参数时候，这种重载的方式更难以为继了。

```c++
template<class T, class... Args>            // from C++11
shared_ptr<T> make_shared(Args&&... args);  // Standard

template<class T, class... Args>            // from C++14
unique_ptr<T> make_unique(Args&&... args);  // Standard
```

需要注意的是，当我们在一个函数中使用 std::move 转换右值引用和 std::forward 转化万能引用时候，在这个参数最后一次使用时候才应用 std::move 或 std::forward 。

```c++
template<typename T>        // text is
void setSignText(T&& text)  // univ. reference
{
  sign.setText(text);  // use text, but don't modify it
  auto now = std::chrono::system_clock::now();  // get current time
  signHistory.add(now, std::forward<T>(text));  // conditionally cast text to rvalue
}
```

如果函数的入参是一个右值引用（或万能引用），函数体中返回这个入参（by value），你应该使用 std::move (std::forward) 来返回这个引用。

```c++
Matrix   // by-value return
operator+(Matrix&& lhs, const Matrix& rhs)
{
  lhs += rhs;
  return std::move(lhs);  // move lhs into return value
}
```

使用 std::move 将 lhs 转化为右值，可以促使编译使用移动而非拷贝的方式将 lhs 移动给函数返回值。

对于万能引用，情况也是类似的。如果参数绑定到右值，使用 std::forward 可以促使编译器使用移动而非拷贝动作。

```c++
template<typename T>
Fraction   // by-value return
reduceAndCopy(T&& frac)   // universal reference param
{
  frac.reduce();
  return std::forward<T>(frac);  // move rvalue into return
}                                // value, copy lvalue
```

但是，上述的情况不能推广到函数中返回局部变量的场景。看下面的例子：

```c++
Widget makeWidget()  // "Copying" version of makeWidget
{
  Widget w;  // local variable configure w
  …
  return w;  // "copy" w into return value
}
```

你可能做如下 “优化” ：

```c++
Widget makeWidget()  // Moving version of makeWidget
{
  Widget w;
  …
  return std::move(w);  // move w into return value
}                       // (don't do this!)
```

“优化” 的版本反而会让编译器生成的代码效率更低，原因是因为编译器的返回值优化（RVO），可以查阅 C++ 返回值优化 RVO 了解更多，这里不再赘述了。

{{<admonition quote "总结" false>}}
- 对右值引用使用 std::move，对通用引用使用 std::forward。
- 对按值返回的函数返回值，无论返回右值引用还是通用引用，执行相同的操作。
- 当局部变量就是返回值是，不要使用s td::move 或者 std::forward。
{{</admonition>}}

#### [Item 26: Avoid overloading on universal references.](https://blog.csdn.net/Dong_HFUT/article/details/123965108)

这一节给出的建议是尽量不要对万能引用参数的函数进行重载，根因是重载函数的匹配规则。先从一个例子说起：

```c++
std::multiset<std::string> names;  // global data structure
void logAndAdd(const std::string& name)
{
  auto now = std::chrono::system_clock::now();  // get current time
  log(now, "logAndAdd");  // make log entry
  names.emplace(name);    // add name to global data structure; see Item 42 for info on emplace
}
```

上面的代码，我们看3个调用：

```c++
std::string petName("Darla");
logAndAdd(petName); // pass lvalue std::string
logAndAdd(std::string("Persephone")); // pass rvalue std::string
logAndAdd("Patty Dog"); // pass string literal
```

第一个调用：logAndAdd 的参数 name 被绑定到一个左值 petName 上。由于 name 是一个左值，names.emplace(name) 将发生一次拷贝。

第二个调用：std::string(“Persephone”) 首先会显示构造出一个临时的 std::string，并且是一个右值。name 被绑定到一个右值，但是 name 是一个左值，names.emplace(name) 将发生一次拷贝。

第三个调用：“Patty Dog” 传入 logAndAdd 将隐式构造出一个临时的 std::string，并且是一个右值。name 被绑定到一个右值，但是 name 是一个左值，names.emplace(name) 将发生一次拷贝。

后面两个调用点， name 都是绑定到一个右值，我们可以通过移动来代替拷贝来提高性能，我们很容易使用万能引用重写 logAndAdd 如下：

```c++
template<typename T>
void logAndAdd(T&& name)
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}

std::string petName("Darla");
logAndAdd(petName);  // as before, copy lvalue into multiset
logAndAdd(std::string("Persephone")); // move rvalue instead of copying it
logAndAdd("Patty Dog"); // create std::string in multiset instead of copying a temporary std::string
```

现在，步入本节的主题。对于上述代码，假设在 logAndAdd 内部需要根据一个索引查找 name，logAndAdd 被重载成这样：

```c++
std::string nameFromIdx(int idx); // return name corresponding to idx
void logAndAdd(int idx) // new overload
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(nameFromIdx(idx));
}

template<typename T>  // as berfore
void logAndAdd(T&& name)
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}
```

新增一个 int 类型参数的调用方式：

```c++
std::string petName("Darla");
logAndAdd(petName);                     // as before, these
logAndAdd(std::string("Persephone"));   // calls all invoke
logAndAdd("Patty Dog");                 // the T&& overload

logAndAdd(22); // calls int overload
```

以上还没什么问题，一切都还符合我们的预期。但是，考略下面的调用场景：

```c++
short nameIdx;
…  // give nameIdx a value
logAndAdd(nameIdx);  // error!
```

对于 short 类型的 nameIdx，我们期望的显示是调用 int 类型的 logAndAdd 重载。但事实却是这样：万能引用版本的 T 将被推导成 short，因而产生一个确切的匹配版本，然后在 names.emplace 时候会用 short 类型去构造 std::string，显然会报错。

在 C++ 中，以万能引用为参数的函数是最贪婪的函数，它能实例化出多数能够胜任的精确匹配版本，而这个例子中 short 需要做类型转换成 int 类型才会匹配到 int 类型的 logAndAdd。而 C++ 重载函数的匹配原则：**如果模板实例化出的函数和普通重载函数都精确匹配，则优先选择普通重载函数，其次选择模板函数实例化出来的精确版本**。因此这里会匹配到万能引用实例化出的版本。

再看万能引用构造函数的例子：

```c++
class Person {
public:
  template<typename T>
  explicit Person(T&& n)         // perfect forwarding ctor;
  : name(std::forward<T>(n)) {}  // initializes data member
  explicit Person(int idx)       // int ctor
  : name(nameFromIdx(idx)) {}
  …
private:
  std::string name;
};
```

这里会有两个问题。首先，传一个除 int 外的整形类型（比如，std::size_t, short, long）将不会调用 int 版本的构造函数，而是调用万能l引用版本的构造函数，然后这将导致编译失败。然后还有一个更加糟糕的问题，根据 [Item 17: Understand special member function generation](https://blog.csdn.net/Dong_HFUT/article/details/123433559?spm=1001.2014.3001.5502). 介绍我们知道编译器将在合适的条件下生成 copy 和 move 类构造函数。Person 实际可能是下面这个样子：

```c++
class Person {
public:
  template<typename T>  // perfect forwarding ctor
  explicit Person(T&& n)
  : name(std::forward<T>(n)) {}
  explicit Person(int idx);   // int ctor

  Person(const Person& rhs);  // copy ctor (compiler-generated)
  Person(Person&& rhs);  // move ctor (compiler-generated)
  …
};
```

考虑下面的调用：

```c++
Person p("Nancy");
auto cloneOfP(p); // create new Person from p; this won't compile!
```

使用 p 去创建一个新的 Person，这里不会调用 Person 的拷贝构造函数，而会调用完美转发构造函数。这是因为 Person 的拷贝构造函数的参数是一个 const 类型的 ，而 p 是一个非 const 类型，并且完美转发构造函数会实例化出一个精确的匹配版本。当我们稍微改造下 p，就可以调用编译器生成的拷贝构造函数：

```c++
const Person cp("Nancy");  // object is now const
auto cloneOfP(cp);         // calls copy constructor!
```

虽然完美转发构造函数也能实例化出一个精确函数签名的版本，但是 C++ 重载匹配会选择普通的重载版本。

当继承介入进来之后，问题将变得更加让人无法接受，我们看下这样的代码片段：

```c++
class SpecialPerson: public Person {
public:
  SpecialPerson(const SpecialPerson& rhs) // copy ctor; calls
  : Person(rhs)                           // base class
  { … }                                   // forwarding ctor!
  SpecialPerson(SpecialPerson&& rhs)      // move ctor; calls
  : Person(std::move(rhs))                // base class
  { … }                                   // forwarding ctor!
};
```

这两个构造函数都会调用基类的完美转发构造函数，因为这两个构造函数给基类传入的都是 SpecialPerson 类型的参数，完美转发构造函数会实例化出精确匹配的版本，最后代码将无法编译通过。

总之，对万能引用参数函数进行重载是一个糟糕的设计，我们需要尽量避免。

{{<admonition quote "总结" false>}}
- 对万能引用参数的函数进行重载，调用机会将比你期望的多得多。
- 完美转发构造函数是糟糕的实现，因为对于 non-const 左值不会调用拷贝构造而是完美转发构造，而且会劫持派生类对于基类的拷贝和移动构造的调用。
{{</admonition>}}

#### [Item 27: Familiarize yourself with alternatives to overloading on universal references.](https://blog.csdn.net/Dong_HFUT/article/details/124227488)

在 [Item26](https://blog.csdn.net/Dong_HFUT/article/details/123965108?spm=1001.2014.3001.5501) 中建议大家尽量不要对万能引用进行重载，但同时也确实存在需要对万能引用进行重载的场景。今天就和大家探索下如何满足这种场景的需求，这个 Item 将沿用上个 Item的例子，阅读本文前建议先看上一个 Item。

**放弃重载**
对于 Item26 中 logAndAdd 函数，为了避免万能引用实例化匹配产生的问题，一种方式就是不使用重载，取而代之的是给这些重载函数起不同的名字。

```c++
logAndAddName(...)
logAndAddNameByIdx(...)
```

这种方法虽然在一定程度上可以解决这个问题，但是对于构造函数，就无能为力了（构造函数函数名是固定的）。

**const T& 传递**

另一种选择是不采用万能引用传参（pass-by-universal-reference），使用 const T& 传参。这也是 Item26 开始就介绍的方法，但是它的缺点是效率不高。

**值传递**

直接选择传值，这种方式将在 Item41 中继续讨论，这里只介绍这种方法的使用：

```c++
class Person {
public:
  explicit Person(std::string n) // replaces T&& ctor; see
  : name(std::move(n)) {}        // Item 41 for use of std::move
  explicit Person(int idx)       // as before
  : name(nameFromIdx(idx)) {}
  …
private:
  std::string name;
};
```

std::string 的构造函数没有传 int 的版本，所有 int 类型和类似 int 类型（int，short，size_t，long）参数的传递给构造函数都将匹配到 int 类型重载构造函数。类似的，所有 std::string 类型和类似 std::string 类型（比如字面的"Ruth"）的参数传递给构造函数都将匹配到 std::string 类型重载构造函数。

**使用 Tag 分发**

const T& 传递和值传递都不支持完美转发。如果使用万能引用的动机是为了完美转发，那还必须只能使用万能引用，并且那你也不想放弃重载，这里介绍一种使用 Tag 分发的方法。

基于 Tag 分发其实就是使用 Tag 对参数进行区分，进而分发到不同的函数实现。对于上个 Item 中的例子:

```c++
std::multiset<std::string> names; // global data structure
template<typename T>              // make log entry and add

void logAndAdd(T&& name) // name to data structure
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}
```

使用 Tag 分发的实现如下：

```c++
template<typename T>
void logAndAddImpl(T&& name, std::false_type)
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}

void logAndAddImpl(int idx, std::true_type)
{
  logAndAdd(nameFromIdx(idx));
}

template<typename T>
void logAndAdd(T&& name)
{
  logAndAddImpl(
    std::forward<T>(name),
    std::is_integral<typename std::remove_reference<T>::type>()
  );
}
```

C++11 引入的 std::is_integral<T> 可以判断参数类型是否为整形。在这个例子中，如果 logAndAdd 传入的是左值类型的 int，T将被推导成 int&，但不是 int，为了解决这个问题，使用 std::remove_reference 去除引用。

从概念上讲，logAndAdd 传递了一个布尔值给 logAndAddImpl，表示传递的实参是否为整形。但是我们知道 true 和 false 都是运行时的值，而模板匹配是编译阶段的事情。C++ 标准库提供了 std::true_type 和 std::false_type 两种类型代表 true 和 false 的含义。如果 T 是整形，那么 logAndAdd 传递给 logAndAddImpl 的参数是一个继承了 std::true_type 的对象，否则是一个继承了std::false_type 的对象。

**约束接受万能引用的模板**

使用 Tag 分发的技术，是在通用引用参数函数内部根据参数类型进行分发，它解决不了 Item26 中介绍的 Person 完美转发构造函数的问题。如果你在一个构造函数内部实现 Tag 分发，但是编译器在一些情况下会自动生成构造函数，将会绕过使用 Tag 分发的构造函数。

问题不在于编译器生成的构造函数会绕过使用 Tag 分发的完美转发构造函数，而是从来没有绕过。例如你想用一个左值的对象去初始化一个新的对象，你想调用的是编译器生成的拷贝构造函数，但是正如 Item26 介绍的那样，实际上调用完美转发的构造函数。

万能引用的匹配重载函数总是贪婪的，我们需要另外一种技术控制万能引用调用的条件，那就是 std::enable_if。

默认条件下，所有模板都是 enable 的，当使用了 std::enable_if 后，只有满足条件的模板才是 enable 的。语法规则是这样的：

```c++
class Person {
  public:
    template<typename T,
             typename = typename std::enable_if<condition>::type>
    explicit Person(T&& n);
    .....
};
```

只有满足了 condition 条件才使能。我们期望不是 Person 类型的参数，模板构造函数才使能，当然我们可以使用 is_same 来判断类型是否相同，因而我们的条件可能是 !std::is_same<Person, T>::value。但是这里会有点小问题，比如 Person 和 Person& 不是一个类型，而我们这里显然不希望 Person& 类型满足条件进而使能模板。
- 对于引用。我们期望 Person& 和 Person&& 都像 Person 一样处理，即不使能模板。
- 对于 const 和 volatile，即 CV 描述符。我们期望 const Person、volatile Person 和 volatile const Person 也能像 Person 一样处理，即不使能模板。

标准库为我们提供了 std::decay，std::decay<T>::type 的类型 和 T 的类型相同，它忽略了引用和 CV 描述符。因此我们想控制模板使能的条件是：

```c++
!std::is_same<Person, typename std::decay<T>::type>::value
```

这样就可以得到我们想要的实现：

```c++
class Person {
public:
  template<
    typename T,
    typename = typename std::enable_if<
                 !std::is_same<Person,
                               typename std::decay<T>::type
                               >::value
               >::type
  >
  explicit Person(T&& n);
  …
};
```

再看 Item26 中万能引用重载在遇到类继承的问题：

```c++
class SpecialPerson: public Person {
public:
  SpecialPerson(const SpecialPerson& rhs) // copy ctor; calls
  : Person(rhs)                           // base class
  { … }                                   // forwarding ctor!
  SpecialPerson(SpecialPerson&& rhs)      // move ctor; calls
  : Person(std::move(rhs))                // base class
  { … }                                   // forwarding ctor!
};
```

当我们拷贝或移动一个 SpecialPerson 对象，我们期望调用基类的拷贝或移动构造函数，但是我们传递给基类的是 SpecialPerson 类型的参数，会匹配到基类的完美转发构造函数。

标准库 type trait 提供了 std::is_base_of 帮我们解决这个问题。如果 T2 继承于 T1，那么 std::is_base_of<T1, T2>::value 为 true，并且 std::is_base_of<T, T>::value 也是 true。上面的代码使用 std::is_base_of 代替 is_same 得到的代码将更加合适：

```c++
class Person {
public:
  template<
    typename T,
    typename = typename std::enable_if<
                 !std::is_base_of<Person,
                                  typename std::decay<T>::type
                                  >::value
               >::type
  >
  explicit Person(T&& n);
  …
};
```

如果使用 C++14 实现将更加简洁：

```c++
class Person {
public:
  template<
    typename T,
    typename = typename std::enable_if_t<
                 !std::is_base_of<Person,
                                  std::decay_t<T>::type
                                  >::value
               >
  >
  explicit Person(T&& n);
  …
};
```

到目前为止，我们已经接近了完美解决了 Item26 中介绍的万能引用模板重载的问题。再加上处理整数参数类型的 Person 的重载，我们汇总代码如下：

```c++
class Person {
public:
  template<
    typename T,
    typename = std::enable_if_t<
      !std::is_base_of<Person, std::decay_t<T>>::value
      &&
      !std::is_integral<std::remove_reference_t<T>>::value
    >
  >
  explicit Person(T&& n)     // ctor for std::strings and
  : name(std::forward<T>(n)) // args convertible to
  { … }                      // std::strings
  explicit Person(int idx)   // ctor for integral args
  : name(nameFromIdx(idx))
  { … }
  … // copy and move ctors, etc.
private:
  std::string name;
};
```

**权衡**

本 Item 介绍的后两种技术：使用 Tag 分发和限制模板使能条件，都支持了完美转发。但使用完美转发也有缺点：

一个是有些类型不能完美转发，这个将在 Item30 中讨论。另外一个是当用户传递无效参数时，编译报错信息的可读性非常差。

例如，在创建 Person 对象的时候传递了个char16_t（C++11引进的一种以16位表示一个字符的类型）字符组成的字符串，而不是char：

```c++
Person p(u"Konrad Zuse"); // "Konrad Zuse" consists of characters of type const char16_t
```

当使用本 Item 的前三种技术时，编译器看到可执行的构造函数只接受 int 和 std::string，编译器会产生一些直观的错误信息表明：无法将 const char16_t[12] 转换到 int 或 std::string。

万能引用在接受 char16_t 类型时候没有问题，当构造函数把 char16_t 类型数组转发到 std::string 成员变量的构造中时，才发现 char16_t 数组不是 std::string 可接受的参数类型，我使用 g++ 的报错信息如下：

```shell
hello.cpp: In instantiation of ‘Person::Person(T&&) [with T = const char16_t (&)[12]; <template-parameter-1-2> = void]’:
hello.cpp:30:26:   required from here
hello.cpp:18:28: error: no matching function for call to ‘std::__cxx11::basic_string<char>::basic_string(const char16_t [12])’
   : name(std::forward<T>(n)) // args convertible to
                            ^
In file included from /usr/include/c++/7/string:52:0,
                 from /usr/include/c++/7/bits/locale_classes.h:40,
                 from /usr/include/c++/7/bits/ios_base.h:41,
                 from /usr/include/c++/7/ios:42,
                 from /usr/include/c++/7/ostream:38,
                 from /usr/include/c++/7/iostream:39,
                 from hello.cpp:1:
/usr/include/c++/7/bits/basic_string.h:604:9: note: candidate: template<class _InputIterator, class> std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(_InputIterator, _InputIterator, const _Alloc&)
         basic_string(_InputIterator __beg, _InputIterator __end,
         ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:604:9: note:   template argument deduction/substitution failed:
hello.cpp:18:28: note:   candidate expects 3 arguments, 1 provided
   : name(std::forward<T>(n)) // args convertible to
                            ^
In file included from /usr/include/c++/7/string:52:0,
                 from /usr/include/c++/7/bits/locale_classes.h:40,
                 from /usr/include/c++/7/bits/ios_base.h:41,
                 from /usr/include/c++/7/ios:42,
                 from /usr/include/c++/7/ostream:38,
                 from /usr/include/c++/7/iostream:39,
                 from hello.cpp:1:
/usr/include/c++/7/bits/basic_string.h:566:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&&, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(basic_string&& __str, const _Alloc& __a)
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:566:7: note:   candidate expects 2 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:562:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(const basic_string& __str, const _Alloc& __a)
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:562:7: note:   candidate expects 2 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:558:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(std::initializer_list<_Tp>, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(initializer_list<_CharT> __l, const _Alloc& __a = _Alloc())
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:558:7: note:   no known conversion for argument 1 from ‘const char16_t [12]’ to
std::initializer_list<char>’
/usr/include/c++/7/bits/basic_string.h:531:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(basic_string&& __str) noexcept
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:531:7: note:   no known conversion for argument 1 from ‘const char16_t [12]’ to
std::__cxx11::basic_string<char>&&’
/usr/include/c++/7/bits/basic_string.h:519:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type, _CharT, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>; std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type = long unsigned int]
       basic_string(size_type __n, _CharT __c, const _Alloc& __a = _Alloc())
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:519:7: note:   candidate expects 3 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:509:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const _CharT*, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(const _CharT* __s, const _Alloc& __a = _Alloc())
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:509:7: note:   no known conversion for argument 1 from ‘const char16_t [12]’ to
const char*’
/usr/include/c++/7/bits/basic_string.h:499:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const _CharT*, std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>; std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type = long unsigned int]
       basic_string(const _CharT* __s, size_type __n,
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:499:7: note:   candidate expects 3 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:481:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&, std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type, std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>; std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type = long unsigned int]
       basic_string(const basic_string& __str, size_type __pos,
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:481:7: note:   candidate expects 4 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:465:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&, std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type, std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>; std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type = long unsigned int]
       basic_string(const basic_string& __str, size_type __pos,
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:465:7: note:   candidate expects 3 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:450:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&, std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type, const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>; std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::size_type = long unsigned int]
       basic_string(const basic_string& __str, size_type __pos,
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:450:7: note:   candidate expects 3 arguments, 1 provided
/usr/include/c++/7/bits/basic_string.h:437:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(const basic_string& __str)
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:437:7: note:   no known conversion for argument 1 from ‘const char16_t [12]’ to
const std::__cxx11::basic_string<char>&’
/usr/include/c++/7/bits/basic_string.h:429:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string(const _Alloc&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string(const _Alloc& __a) _GLIBCXX_NOEXCEPT
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:429:7: note:   no known conversion for argument 1 from ‘const char16_t [12]’ to
const std::allocator<char>&’
/usr/include/c++/7/bits/basic_string.h:420:7: note: candidate: std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::basic_string() [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]
       basic_string()
       ^~~~~~~~~~~~
/usr/include/c++/7/bits/basic_string.h:420:7: note:   candidate expects 0 arguments, 1 provided
```

如果完美转发多次，错误信息将更加迷惑。std::is_constructible 可以在编译期间测试一个类型的对象是否能被另一个不同类型（或一些不同类型）的对象（或者另一些对象）构造，我们可以使用 static_assert 断言来实现：

```c++
class Person {
public:
  template<
    typename T,
    typename = std::enable_if_t<
      !std::is_base_of<Person, std::decay_t<T>>::value
      &&
      !std::is_integral<std::remove_reference_t<T>>::value
    >
  >
  explicit Person(T&& n)     // ctor for std::strings and
  : name(std::forward<T>(n)) // args convertible to std::strings
  {
    // assert that a std::string can be created from a T object
	static_assert(
	  std::is_constructible<std::string, T>::value,
	  "Parameter n can't be used to construct a std::string"
	);
    …
  }
  explicit Person(int idx)   // ctor for integral args
  : name(nameFromIdx(idx))
  { … }
  … // copy and move ctors, etc.
private:
  std::string name;
};
```

g++ 编译报错如下：

```shell
// ...... 此处省略
/usr/include/c++/7/bits/basic_string.h:420:7: note:   candidate expects 0 arguments, 1 provided
hello.cpp:20:6: error: static assertion failed: Parameter n can't be used to construct a std::string
      static_assert(
      ^~~~~~~~~~~~~
```

至此，我们我们已经完美解决了 Item26 中介绍的万能引用模板重载的问题。

{{<admonition quote "总结" false>}}
- 万能引用和重载的组合替代方案包括使用不同的函数名、通过 const 左值引用传参、按值传递参数，使用 tag 分发。
- 通过 std::enable_if 约束模板来允许万能引用和重载组合使用，std::enable_if 可以控制编译器什么条件才使用万能引用的实例。
- 万能引用参数通常具有高效率的优势，但通常可用性较差。
{{</admonition>}}

#### [Item 28: Understand reference collapsing.](https://blog.csdn.net/Dong_HFUT/article/details/124517043)

**引用折叠规则**

所谓引用折叠（reference collapsing）就是引用指向引用（reference to reference）会折叠（或者坍塌）成一种引用。我们知道，引用分为左值引用和右值引用，因此，引用指向引用就存在 4 种情况：
  1. 左值引用指向左值引用，记为 A& &
  2. 左值引用指向右值引用，记为 A& &&
  3. 右值引用指向左值引用，记为 A&& &
  4. 右值引用指向右值引用，记为 A&& &&

引用折叠的规则为：

> 只要两个引用中的一个为左值引用的话，则折叠为左值引用，否则为右值引用。

应用上述引用折叠规则，引用指向引用的 4 种情况的结果为：
  1. A& & —> A&
  2. A& && ----> A&
  3. A&& & ----> A&
  4. A&& && ----> A&&

根据引用折叠规则，我们看一下引用折叠应用的几种应用场景。

**万能引用的实例化**

对于万能引用：

```c++
template<typename T>
void func(T&& param);
```

我们在 [Item 24](https://blog.csdn.net/Dong_HFUT/article/details/123773321?spm=1001.2014.3001.5501) 中介绍过，万能引用的参数（param）的初始化决定了它代表一个右值还是一个左值。如果初始化为一个右值，万能引用对应右值引用。如果初始化为一个左值，万能引用对应一个左值引用。

这里其实是应用了引用折叠规则。首先类型 T 根据传递给 param 的参数是左值还是右值进行推导，推导的机制为：

> 如果一个左值传递给 param，T 被推导为一个左值引用；如果一个右值传递给 param，T 被推导成一个非引用类型。

我们先看传递一个左值的情况：

```c++
Widget w;
func(w);
```

首先，T 被推导成一个左值引用，这里为 Widget&，我们用它实例化模板，得到：

```c++
void func(Widget& && param);
```

然后应用引用折叠规则，得到：

```c++
void func(Widget& param);
```

因此，初始化为一个左值，万能引用对应一个左值引用。

再看传递一个右值的情况：

```c++
Widget widgetFactory();
func(widgetFactory());
```

首先，T 被推导成一个非引用类型，这里为 Widget，我们用它实例化模板，得到：

```c++
void func(Widget&& param);
```

这里没有引用指向引用的情况，因此，如果初始化为一个右值，万能引用对应右值引用。

以上就解释了万能引用如何根据初始化参数推导模板参数类型的。

**std::forward 机制**

引用折叠也是 std::forward 机制的关键部分。看一个 std::forward 应用于万能引用参数的例子：

```c++
template<typename T>
void f(T&& fParam)
{
  … // do some work
  someFunc(std::forward<T>(fParam)); // forward fParam to someFunc
}
```

根据上面的介绍，我们知道， T 的推导类型取决于 fParam 被初始参数为左值还是右值：如果一个左值传递给 param，T 被推导为一个左值引用；如果一个右值传递给 param，T 被推导成一个非引用类型。

```c++
template<typename T>
T&& forward(typename remove_reference<T>::type& param)
{
  return static_cast<T&&>(param);
}
```

如果 fParam 被初始化为一个左值类型 Widget，则 T 被推导为 Widget&，则 std::forward 被实例化成 std::forward<Widget&>：

```c++
Widget& && forward(typename remove_reference<Widget&>::type& param)
{
  return static_cast<Widget& &&>(param);
}
```

std::remove_reference<Widget&>::type 产生 Widget，则 std::forward 变为：

```c++
Widget& && forward(Widget& param)
{ return static_cast<Widget& &&>(param); }
```

再应用引用折叠规则，则 std::forward 变为：

```c++
Widget& forward(Widget& param)
{ return static_cast<Widget&>(param); }
```

正如你所见，当一个左值传递给模板函数 f，std::forward 的入参和返回值类型都是一个左值引用，由于 param 的类型已经是一个 Widget&，因此 std::forward 内部的 cast 啥也没干。这完全符合对 std::forward 的预期，传入左值 std::forward 则返回左值（左值引用本质上就是左值），实际上并没有做任何类型 cast。

再来看 fParam 被初始化为一个右值类型 Widget，则 T 被推导为 Widget，则 std::forward 被实例化成 std::forward<Widget>：

```c++
Widget&& forward(typename remove_reference<Widget>::type& param)
{ return static_cast<Widget&&>(param); }
```

std::remove_reference 产生 Widget，则 std::forward 变为：

```c++
Widget&& forward(Widget& param)
{ return static_cast<Widget&&>(param); }
```

这里没有引用折叠的事情，结果也是符合我们对 std::forward 的预期：传入右值 std::forward 则返回右值（把左值参数 param 转化为右值）。

**生成 auto 变量**

再看 auto 变量的情况，也是类似模板类型的：

```c++
auto&& w1 = w;
```

这其实也是一个万能引用（见 Item 24）。如果用一个左值初始化 w1，auto 类型被推导成 Widget&，则上面代码则变成一个引用指向引用的表达式：

```c++
Widget& && w1 = w;
```

应用引用折叠，则变为：

```c++
Widget& w1 = w;
```

w1 的结果为一个左值引用。

另一方面，用一个右值初始化 w2：

```c++
auto&& w2 = widgetFactory();
```

auto 被推导成非引用类型 Widget，则上述代码变为：

```c++
Widget&& w2 = widgetFactory();
```

w2 的结果为一个右值引用。

**typedef 类型别名**

若使用 typedef 时候发生引用指向引用的情况，则同样应用引用折叠规则：

```c++
template<typename T>
class Widget {
public:
  typedef T&& RvalueRefToT;
  …
};
```

假设使用左值引用实例化：

```c++
Widget<int&> w;
```

这里将产生引用指向引用的表达式：

```c++
typedef int& && RvalueRefToT;
```

应用引用折叠规则，表达式变为：

```c++
typedef int& RvalueRefToT;
```

**decltype 类型推导**

当分析 decltype 产生的类型时，存在引用指向引用的表达式，也是引用折叠的应用场景。可以参阅 [Item 3](https://blog.csdn.net/Dong_HFUT/article/details/122745518?spm=1001.2014.3001.5502)。

{{<admonition quote "总结" false>}}
- 引用折叠发生在四种情况：模板实例化，auto 类型的生成，创建和使用 typedef、别名声明和decltype。
- 当编译器生成了引用的引用时，通过引用折叠就是单个引用。其中之一为左值引用就是左值引用，否则就是右值引用。
- 在类型推导区分左值和右值以及引用折叠发生的上下文中，万能引用是右值引用。
{{</admonition>}}

#### [Item 29: Assume that move operations are not present, not cheap, and not used.](https://blog.csdn.net/Dong_HFUT/article/details/124577258)

在 C++11 新增特性中，移动语义无疑是最重要的一个，它允许编译器使用高效的 move 操作代替低效的 copy 操作。一般地，把你的 C++98 代码使用 C++11 编译器重新编译后，运行的会更快一些。

然而，凡是都不是绝对的，本 item 会介绍一些移动语义不可用、不那么高效的场景。

对于 C++ 标准库，针对 C++11 特性做了大量的修改，添加了对移动语义的支持，对 C++ 标准库使用移动操作基本上都会带来性能的提升。但对于我们自己存量的老代码，多数是不支持移动语义的。并且在 Item 17 中也介绍了，编译器只会在没有用户自定义拷贝操作和析构函数时才会生成移动操作。因此，这种情况下无法享受到移动语义带来的性能收益。

C++11 标准库已经都支持移动操作了，但不意味着一定都会带来性能的提升。例如 std::array，其实它本质上是披着标准库容器接口外衣的数组。一般的 STL 容器的对象，其数据成员是在堆上，对象中有一个指针指向这个堆。这个指针的存在，让容器内容的移动只要将目标容器的指针指向源容器的堆，然后将源容器的指针设置为空即可。

![](images/item_29_01.png)

std::array 没有这样的指针，它的内容直接存储在对象的 buffer 中，它的移动没法像一般的容器那样通过直接改变容器中指针的指向来高效完成移动。std::array 的移动需要将数据一个一个移动或拷贝。

![](images/item_29_02.png)

对于 std::string，提供了常量时间的移动和线性时间的拷贝，听起来移动比拷贝高效很多。然而，也有例外。std::string 有一种实现叫 SSO（small string optimization），对于小字符串（例如少于 15 个字符）其数据直接存储在对象中，而不存储在堆上。SSO 直接使用对象内部的 buffer 存放内容，而省去动态申请堆内存。移动基于 SSO 实现的小字符串并不会比拷贝高效。

即使对于支持移动语义的类型，看似一定使用移动的场景，却最终使用的是拷贝。Item 14 介绍了一些标准库操作提供了异常规范影响移动语义的场景。只有移动操作一定不会抛出异常的情况下，拷贝操作在内部才会被移动操作替换。如果移动操作没有被申明为 noexcept，即使是适合移动操作的场景，编译器也会依然生成拷贝操作。

此外，虽然左值可以使用 std::move 将其转换为右值进行移动操作，但会存在一些异常情况，参见 Item 25 。因而尽可能只对右值进行移动操作。

{{<admonition quote "总结" false>}}
- 假设移动操作不可用、不廉价。
- 在已知类型或支持移动语义的代码中，不需要进行此假设。
{{</admonition>}}

#### [Item 30: Familiarize yourself with perfect forwarding failure cases.](https://blog.csdn.net/Dong_HFUT/article/details/124787082)


完美转发（perfect forwarding）是 C++11 非常重要的一个特性。转发意味着一个函数将其参数传给另一函数，第二个函数的目的是接收第一个函数接收到的参数，并且二者是同一个对象。这就排除了值传递参数形式，因为值传递需要拷贝对象，拷贝后对象就不是源对象了。指针传递也可以达到转发的效果，但要求用户必须传递指针，算不上完美转发。

完美转发不仅要转发对象本身，还有附带属性，比如对象是左值还是右值、是 const 还是 volatile。根据 Item 24 的介绍，只能使用万能引用的方式，因为只有万能引用能够对传递的参数的左值或右值信息进行编码。

典型的形式如下：

```c++
template<typename T>
void fwd(T&& param)           // accept any argument
{
  f(std::forward<T>(param));  // forward it to f
}
```

fwd 还可以转发可变参数：

```c++
template<typename... Ts>
void fwd(Ts&&... params)           // accept any arguments
{
  f(std::forward<Ts>(params)...);  // forward them to f
}
```

一些类型的参数会导致完美转发失败，本 Item 接下来介绍几个导致完美转发失败的场景。

**花括号初始化（统一初始化、列表初始化）**

假设函数 f 申明如下：

```c++
void f(const std::vector<int>& v);
```

使用统一初始化方式调用 f 没有问题：

```c++
f({ 1, 2, 3 }); // fine, "{1, 2, 3}" implicitly
                // converted to std::vector<int>
```

对于 f ({ 1, 2, 3 })，在函数调用点，编译器会比较入参和函数的参数申明，看它们是否兼容。如有必要，编译器会执行隐式类型转换来让调用成功。在这个例子中，编译器会将 {1, 2, 3} 转换成一个 std::vector<int> 类型（因为 std::vector 有初始化列表的构造函数版本），这样就调用成功了。

但是将列表初始化参数传递给 fwd，则产生编译错误：

```c++
fwd({ 1, 2, 3 }); // error! doesn't compile
```

通过转发函数模板 fwd 间接调用 f，编译器将不再比较通过 fwd 调用点传入的参数和函数 f 的参数申明。取而代之的是，编译器推导通过 fwd 传递的参数类型，并将比较推导类型和函数 f 的参数申明。下面两者之一的情况发生时，将导致完美转发失败：
- 编译器无法推导 fwd 的参数类型。
- 编译器将 fwd 的参数类型推导“错误”。这里的错误可能是使用推导类型的 fwd 的实例无法编译通过，也可能是使用推导类型调用 f 和直接使用传递给 fwd 的参数调用 f 的行为不一样。

对于 fwd({ 1, 2, 3 })，由于 fwd 没有申明为一个 std::initializer_list，编译器不会将表达式 {1, 2, 3} 推导类型，因此编译失败。

```c++
#include<iostream>
#include<vector>

void f(const std::vector<int>& v) {
  // do something
}

template<typename T>
void fwd(T&& param)           // accept any argument
{
  f(std::forward<T>(param));  // forward it to f
}

int main () {
  fwd({1, 2,3});
}
// 编译报错如下：
main.cpp: In function 'int main()':
main.cpp:15:6: error: no matching function for call to 'fwd(<brace-enclosed initializer list>)'
   15 |   fwd({1, 2,3});
      |   ~~~^~~~~~~~~~
main.cpp:9:6: note: candidate: 'template<class T> void fwd(T&&)'
    9 | void fwd(T&& param)           // accept any argument
      |      ^~~
main.cpp:9:6: note:   template argument deduction/substitution failed:
main.cpp:15:6: note:   couldn't deduce template parameter 'T'
   15 |   fwd({1, 2,3});
```

这是完美转发失败的第一个例子。不过，上面的问题也可以解决：虽然模板类型推导无法推导出初始化列表的类型，但是 auto 可以:

```c++
auto il = { 1, 2, 3 }; // il's type deduced to be
                       // std::initializer_list<int>
fwd(il);               // fine, perfect-forwards il to f
```

**0 或 NULL 作为空指针**

在 Item8 中介绍过， 当使用 0 或者 NULL 作为一个空指针传给模板，会被推导成 int 类型，无法被当成指针类型进行完美转发。

```c++
#include<iostream>

void f(void*) {
  // do something
}

template<typename T>
void fwd(T&& param)           // accept any argument
{
  f(std::forward<T>(param));  // forward it to f
}

int main () {
  fwd(NULL);
  // fwd(0);
}

// 编译报错
main.cpp: In instantiation of 'void fwd(T&&) [with T = long int]':
main.cpp:14:6:   required from here
main.cpp:10:4: error: invalid conversion from 'long int' to 'void*' [-fpermissive]
   10 |   f(std::forward<T>(param));  // forward it to f
      |   ~^~~~~~~~~~~~~~~~~~~~~~~~
      |    |
      |    long int
main.cpp:3:8: note:   initializing argument 1 of 'void f(void*)'
    3 | void f(void*) {
      |        ^~~~~
```

解决办法也很简单，使用 nullptr 作为空指针。

**仅仅声明整型的静态常量（static const）数据成员**

一般地，没有必要在类中定义静态常量数据成员，仅仅申明它就够了，因为编译器会对执行常量传播，因此不会为静态常量数据成员分配实际的存储空间。例如下面的代码片段：

```c++
class Widget {
public:
  static const std::size_t MinVals = 28; // MinVals' declaration
  …
};
… // no defn. for MinVals

std::vector<int> widgetData;
widgetData.reserve(Widget::MinVals); // use of MinVals
```

虽然 Widget::MinVals 没有存储空间，但是使用它初始化 widgetData 是没有问题的，因为编译器会直接将使用到它的地方替换成 28。但是若对 Widget::MinVals 取地址，将找不到 Widget::MinVals 的定义，链接时将会失败。

```c++
void f(std::size_t val);
f(Widget::MinVals);   // fine, treated as "f(28)"
fwd(Widget::MinVals); // error! shouldn't link
```

虽然万能引用没有对 Widget::MinVals 取地址，但是万能引用的参数是引用类型，对于编译器而言，引用通常被对待成指针一样。因此，完美转发也就失败了。

上面说的是引用通常被当成指针，不排除有的编译器不是这样，也即可以对静态常量数据成员进行完美转发。但我们没必须要冒这样的险，只要增加一个定义即万事大吉。

```c++
const std::size_t Widget::MinVals; // in Widget's .cpp file
```

**函数重载和函数模板**

```c++
void f(int (*pf)(int)); // pf = "processing function"

int processVal(int value);
int processVal(int value, int priority);

f(processVal);   // fine
fwd(processVal); // error! which processVal?
```

对于重载函数 processVal，通过 fwd 转发 processVal 将会失败，因为模板类型推导无法推导 processVal 的类型。使用模板函数，也有同样的问题。

```c++
template<typename T>
T workOnVal(T param) // template for processing values
{ … }
fwd(workOnVal); // error! which workOnVal
                // instantiation?
```

解决的办法是：主动给出函数重载和函数模板的类型：

```c++
using ProcessFuncType = // make typedef; see Item 9
  int (*)(int);
ProcessFuncType processValPtr = processVal;  // specify needed signature for processVal
fwd(processValPtr); // fine
fwd(static_cast<ProcessFuncType>(workOnVal)); // also fine
```

当然上面的行为有点奇怪，完美转发需要知道转发的是哪一个。

**位域**

完美转发失败的最后一个例子是使用位域作为函数参数。

```c++
struct IPv4Header {
  std::uint32_t version:4,
                IHL:4,
                DSCP:6,
                ECN:2,
                totalLength:16;
…
};

void f(std::size_t sz); // function to call
IPv4Header h;
…
f(h.totalLength); // fine
fwd(h.totalLength); // error!
```

失败的原因是因为 fwd 的参数是一个引用，C++ 标准规定一个非 const 引用无法引用一个位域字段。这样的规定也是合理的，比特位域通常只是 int 类型的一部分，没有一个确切的地址，也就没办法通过指针指向它，而通常引用本质上是指针，因此无法引用位域。

而指向常量的引用可以绑定到位域，本质上是因为绑定到了位域的一个拷贝对象上（比如 int）。

```c++
#include<iostream>

struct IPv4Header {
  std::uint32_t version:4,
                IHL:4,
                DSCP:6,
                ECN:2,
                totalLength:16;
};

void f(const int&) {
  // do something
}

template<typename T>
void fwd(T&& param)           // accept any argument
{
  f(std::forward<T>(param));  // forward it to f
}

int main () {
  IPv4Header h;
  f(h.totalLength);  // fine
  fwd(h.totalLength); // error
}

// 编译报错
main.cpp: In function 'int main()':
main.cpp:24:9: error: cannot bind bit-field 'h.IPv4Header::totalLength' to 'unsigned int&'
   24 |   fwd(h.totalLength);
      |       ~~^~~~~~~~~~~
```

那么完美转发位域也可以通过对位域进行拷贝，然后再转发。

```c++
// copy bitfield value; see Item 6 for info on init. form
auto length = static_cast<std::uint16_t>(h.totalLength);
fwd(length); // forward the copy
```

{{<admonition quote "总结" false>}}
- 当模板类型推导失败或者推导类型是错误的时候完美转发会失败。
- 导致完美转发失败的类型有花括号初始化、空指针的 0 或者 NULL、只声明的整型 static const 数据成、，模板和重载的函数名和位域。
{{</admonition>}}

### CH06: Lambda Expressions
#### [Item 31: Avoid default capture modes.](https://blog.csdn.net/Dong_HFUT/article/details/125037605)

C++11 lambda 表达式有两种默认捕获模式：传引用捕获和传值捕获。默认传引用捕获可能导致引用悬挂的问题。默认传值捕获其实也不能避免这个问题，并且你的 lambda 闭包也不是独立的。

先看默认传引用捕获导致引用悬挂的问题。看下面的代码片段：

```c++
using FilterContainer = std::vector<std::function<bool(int)>>;
FilterContainer filters; // filtering funcs

void addDivisorFilter()
{
  auto calc1 = computeSomeValue1();
  auto calc2 = computeSomeValue2();
  auto divisor = computeDivisor(calc1, calc2);
  filters.emplace_back(
    [&](int value) { return value % divisor == 0; } // danger! ref to divisor will dangle！
  );
}
```

`filters` 存放了 `lamdba` 闭包，闭包引用了 `addDivisorFilter` 作用域内的局部变量 `divisor`，当离开 `addDivisorFilter` 作用域后，局部变量 `divisor` 将被析构，若此时使用 `filters` 则导致引用悬挂。

使用显示的传引用捕获也有同样的问题：

```c++
filters.emplace_back(
  [&divisor](int value)             // danger! ref to
  { return value % divisor == 0; }  // divisor will
);                                  // still dangle!
```

不过，显示的传引用捕获可以提醒我们 lambda 表达式的生命周期依赖于 divisor 的生命周期，也可以提醒我们需要确保 divisor 的生命周期要长于 lambda 表达式的生命周期。

解决上述问题，可以通过默认传值捕获的方式解决：

```c++
filters.emplace.back(
  [=](int value) { return value % divisor == 0; }
)
```

但是，默认传值捕获也不一定能够解决悬挂问题：例如你通过传值的方式捕获一个指针，也即你拷贝了一个指针给 lambda 闭包了，但是你无法阻止 lambda 闭包外指针被 delete，从而导致指针悬挂的问题。看下面的例子：

```c++
class Widget {
public:
  … // ctors, etc.
  void addFilter() const; // add an entry to filters
private:
  int divisor; // used in Widget's filter
};

void Widget::addFilter() const
{
  filters.emplace_back(
    [=](int value) { return value % divisor == 0; }
  );
}
```

上面代码貌似是安全的。因为你通过默认传值捕获方式，应该不会有悬挂的问题。但是，lambda 表达式只能捕获作用域内的非静态局部变量，而 divisor 是一个成员变量。那么上面的代码为什么可以编译通过呢？可以先看下面这段代码：

```c++
void Widget::addFilter() const
{
  filters.emplace_back(
    [divisor](int value) { return value % divisor == 0; }
  );
}
```

这段代码编译无法通过：

```shell
main.cpp: In member function 'void Widget::addFilter() const':
main.cpp:19:6: error: capture of non-variable 'Widget::divisor'
   19 |     [divisor](int value) { return value % divisor == 0; }
```

正如我们所设想的，lambda 表达式无法捕获非静态成员变量。前面默认传值捕获之所以能够通过编译，原因其实是这里的 lambda 表达式捕获的是 this 指针，也就是将 this 指针拷贝进了闭包。编译器在内部将 divisor 替换成了 this->divisor，等价如下：

```c++
void Widget::addFilter() const
{
  filters.emplace_back(
    [=](int value) { return value % this->divisor == 0; }
  );
}
```

这里实际捕获的不是 divisor，而是 this 指针。了解了上述真相后，就不难理解默认传值捕获也可能导致指针悬挂的问题了。

```c++
using FilterContainer = std::vector<std::function<bool(int)>>;
FilterContainer filters;

void doSomeWork() {
  auto pw = std::make_unique<Widget>();
  pw->addFilter();
}
```

`filters` 包含了 Widget 的 this 指针的拷贝。 doSomeWork 执行完成后，Widget 将被析构，导致 filters 包含了一个悬挂的指针。

上述问题可以通过使用一个局部变量拷贝成员变量来解决。如下：

```c++
void Widget::addFilter() const
{
  auto divisorCopy = divisor; // copy data member
  filters.emplace_back(
    [divisorCopy](int value) // capture the copy
    { return value % divisorCopy == 0; } // use the copy
  );
}
```

或者默认传值捕获也是一样：

```c++
void Widget::addFilter() const
{
  auto divisorCopy = divisor; // copy data member
  filters.emplace_back(
    [=](int value) // capture the copy
    { return value % divisorCopy == 0; } // use the copy
  );
}
```

c++14 的方式更加简洁：

```c++
void Widget::addFilter() const
{
  filters.emplace_back(               // C++14:
    [divisor = divisor](int value)    // copy divisor to closure
    { return value % divisor == 0; }  // use the copy
  );
}
```

`lambda` 表达式只能捕获非静态局部变量，对于 `static` 或者 `global` 变量，`lambda` 表达式不会捕获。所以这些变量发生改变会影响到 `lambda` 表达的行为。使用默认传值捕获，可能会让你以为你的 `lambda` 闭包是独立的，不依赖外部变量的变化，其实不然。

```c++
void addDivisorFilter()
{
  static auto calc1 = computeSomeValue1(); // now static
  static auto calc2 = computeSomeValue2(); // now static
  static auto divisor = // now static
    computeDivisor(calc1, calc2);
  filters.emplace_back(
    [=](int value)   // captures nothing!
    { return value % divisor == 0; } // refers to above static
  );
  ++divisor;   // modify divisor
}
```

上述代码可能会让你产生错觉：因为是使用默认传值捕获，`lambda` 闭包是将 `divisor` 拷贝进去的。但是 `static` 变量不会被 `lambda` 闭包捕获， `divisor` 的改变会影响到 `lambda` 表达式的行为。

{{<admonition quote "总结" false>}}
- 默认的按引用捕获可能会导致引用悬挂。
- 默认的按值引用对于悬挂指针很敏感（尤其是this指针），并且它会误导人认为 lambda 是独立的。
{{</admonition>}}

#### [Item 32: Use init capture to move objects into closures.](https://blog.csdn.net/Dong_HFUT/article/details/125111586)

如果你想移动一个对象都 lambda 闭包，值捕获和引用捕获都不能实现该目的。C++ 14 提供了初始化捕获（init capture）模式支持移动捕获。C++11 并不支持，但是可以使用 std::bind 间接模拟。

**C++14 使用初始化捕获模式实现移动捕获**

C++14 提供了支持移动捕获的机制，但并没有类似值捕获 [=] 或者引用捕获 [&] 的模式直接添加一个移动捕获 [&&] 模式。而是采取了一种更加灵活的机制 ----- 初始化捕获模式。移动捕获是采用初始化捕获的机制实现，除了默认捕获模式，初始化捕获模式可以做原来 C++11 支持的所有捕获模式能做的事，甚至还更多，比如 item31 中介绍的类成员变量的捕获。初始化捕获模式语法如下：

- 指定闭包类的成员名称。
- 指定一个表达式来初始化这个成员。

看下面的例子：移动一个 std::unique_ptr 对象到 lambda 闭包。

```c++
class Widget { // some useful type
public:
  …
  bool isValidated() const;
  bool isProcessed() const;
  bool isArchived() const;
private:
  …
};

auto pw = std::make_unique<Widget>(); // create Widget;
                                      // see Item 21 for info on
                                      // std::make_unique
…                                     // configure *pw
auto func = [pw = std::move(pw)]                // init data mbr
            { return pw->isValidated()          // in closure w/
                     && pw->isArchived(); };    // std::move(pw)
```

在捕获列表中，`“=”` 左边的 `pw` 是指定的 `lambda` 闭包的成员名称。`“=”` 右边的 `std::move(pw)` 是指定的用于初始化闭包成员 pw 的表达式。`“=”` 两边的作用域也不一样，左边的作用域在 `lambda` 闭包中，右边的作用域是 `lambda` 表达式被定义的作用域。

**C++11 使用 std::bind 间接实现移动捕获**

C++11 使用 std::bind 间接实现移动捕获：
- 将被捕获对象移动至 std::bind 产生的函数对象中。
- 给 lambda 表达式一个被捕获对象的引用。

看下面的例子：

```c++
std::vector<double> data;
…
auto func =
  std::bind(                            // C++11 emulation
    [](const std::vector<double>& data) // of init capture
    { /* uses of data */ },
    std::move(data)
);
```

和 lambda 表达式类似，std::bind 产生一个函数对象，称为绑定函数对象。std::bind 的第一个参数是可调用对象，紧接着的参数是传递给这个对象的值。

对于 std::bind 传递的参数，如果是左值，则拷贝到绑定函数对象中；如果是右值，则移动到绑定函数对象中。但是对于绑定函数对象而言，它的参数是一个左值引用。在这个例子中，std::bind 传递的是一个右值 std::move(data)，func 内部调用移动构造来初始化 data。

默认的，lambda 闭包内的 operator() 成员方法是一个 const 的，不能对参数进行修改，所以这里显示申明成 const。如果你希望可以对参数进行修改，则可以使用 mutable关键字进行修饰。

```c++
auto func =
  std::bind( // C++11 emulation
   [](std::vector<double>& data) mutable // of init capture
   { /* uses of data */ }, // for mutable lambda
   std::move(data)
);
```

再给一个前面 C++14 std::unique_ptr 移动到 lambda 闭包的例子。

C++14 的实现如下：

```c++
auto func = [pw = std::make_unique<Widget>()] // as before,
            { return pw->isValidated()        // create pw
                     && pw->isArchived(); };  // in closure
```

C++11 间接实现如下：

```c++
auto func = std::bind(
              [](const std::unique_ptr<Widget>& pw)
              { return pw->isValidated()
                       && pw->isArchived(); },
              std::make_unique<Widget>()
            );
```

{{<admonition quote "总结" false>}}
- C++14 使用初始化捕获模式实现移动捕获。
- C++11 使用 std::bind 间接实现移动捕获。
{{</admonition>}}

#### [Item 33: Use decltype on auto&& parameters to std::forward them.](https://blog.csdn.net/Dong_HFUT/article/details/125116613)

C++14 最令人激动的特性之一便是通用 lambda（generic lambda），使用 auto 指定参数类型。lambda 表达式在编译器内部可以看作一个实现 operator() 的类，使用 auto 参数的 lambda 表达式，其实就是对 operator() 模板化。

例如如下 lambda 表达式：

```c++
auto f = [](auto x){ return func(normalize(x)); };
```

可以看作成：

```c++
class SomeCompilerGeneratedClassName {
public:
  template<typename T>        // see Item 3 for
  auto operator()(T x) const  // auto return type
  { return func(normalize(x)); }

  …  // other closure class functionality
};
```
在上面这个例子中，lambda 表达式就是将它的参数 `x` 转发给 `normalize`。如果 `normalize` 处理左值和右值的方式不同，则上述代码写的并不合理，因为它总是传递左值给 `normalize`，算不上完美转发。

正确的做法是使用完美转发改写上面的代码。首先将参数变成万能引用，然后使用 std::forward 进行转发：

```c++
auto f = [](auto&& x)
         { return func(normalize(std::forward<???>(x))); };
```

但是，`std::forward` 的参数类型怎么写呢？如果是使用参数类型 T 的模板函数，可以直接写成 `std::forward<T>`，但是对于通用 lambda，没有这样的 T 可以被使用。

在 [item28](https://blog.csdn.net/Dong_HFUT/article/details/124517043?spm=1001.2014.3001.5502) 中介绍过：<font color=red>如果一个左值传递给万能引用参数，则万能引用参数被推导成一个左值；若一个右值传递给万能引用参数，则万能引用参数被推导成一个右值</font>。

这里我们可以借助 decltype。通过 item3 的介绍我们知道：如果 x 是一个左值，则 decltype(x) 产生一个左值引用类型；如果 x 是一个右值，则 decltype(x) 产生一个右值引用类型。通用 lambda 完美转发版本可以写成：

```c++
auto f =
  [](auto&& param)
  {
    return
      func(normalize(std::forward<decltype(param)>(param)));
  };
```

结合 C++14 完美转发的实现进行理解：

```c++
template<typename T>
T&& forward(remove_reference_t<T>& param)
{
  return static_cast<T&&>(param);
}
```

假设使用 Widget 类型的变量 x 初始化 param 。如果 x 是一个左值，则 decltype(x) 产生一个左值引用类型。而 remove_reference_t 去除类型。则完美转发为：

```c++
Widget& && forward(Widget& param)
{
  return static_cast<Widget& &&>(param);
}
```
应用引用折叠规则，则转换为：

```c++
Widget& forward(Widget& param)
{
  return static_cast<Widget&>(param);
}
```
显然，完美转发没有做类型转化。也即转发的还是一个左值的 param。

如果 x 是一个右值，则 decltype(x) 产生一个右值引用类型。则完美转发为：

```c++
Widget&& && forward(Widget& param)
{
  return static_cast<Widget&& &&>(param);
}
```

应用引用折叠规则，转换为：

```c++
Widget&& forward(Widget& param)
{
  return static_cast<Widget&&>(param);
}
```

显然，完美转发将左值 param 转换为右值。

综上，使用 decltype 可以实现通用引用参数的完美转发。

此外，C++14 的 lambda 还支持变长参数类型：

```c++
auto f =
  [](auto&&... params)
  {
    return
      func(normalize(std::forward<decltype(params)>(params)...));
  };
```

{{<admonition quote "总结" false>}}
- 对 auto&& 参数使用 decltype来转发（std::forward）。
{{</admonition>}}

#### [Item 34: Prefer lambdas to std::bind.](https://blog.csdn.net/Dong_HFUT/article/details/125130410)

C++11 的 std::bind 是对 C++98 std:bind1st 和 std::bind2nd 的继承，它在 2005 年以 TR1 文档形式非正式地成为标准库的一部分。因为，许多 C++ 程序员可能有十几年的 std::bind 使用经验，现在告诉他放弃使用 std::bind，多少可能有些不情愿。但是，本 Item 会告诉你使用 lambda 替代 std::bind 将是个更好的选择。

对于 C++11，除了个别边缘 case，lambda 表达式要比 std::bind 更有优势。而对于 C++14，lambda 则可以完全替代 std::bind。

lambda 第一个优势是代码的可读性更强。例如，我们有一个设置声音报警的函数：

```c++
// typedef for a point in time (see Item 9 for syntax)
using Time = std::chrono::steady_clock::time_point;
// see Item 10 for "enum class"
enum class Sound { Beep, Siren, Whistle };
// typedef for a length of time
using Duration = std::chrono::steady_clock::duration;
// at time t, make sound s for duration d
void setAlarm(Time t, Sound s, Duration d);
```

如果我们想在设置声音报警后 1h，关闭报警，并持续 30s。使用 lambda 表达式修正 `setAlarm`，可以实现如下：

```c++
// setSoundL ("L" for "lambda") is a function object allowing a
// sound to be specified for a 30-sec alarm to go off an hour
// after it's set
auto setSoundL =
  [](Sound s)
  {
    // make std::chrono components available w/o qualification
    using namespace std::chrono;
    setAlarm(steady_clock::now() + hours(1), // alarm to go off
             s, // in an hour for
             seconds(30)); // 30 seconds
  };
```

上述代码逻辑非常清楚。如果使用 C++14 字面值 `std::literals` 改写上面代码，可以更加简洁：

```c++
auto setSoundL =
  [](Sound s)
  {
    using namespace std::chrono;
    using namespace std::literals; // for C++14 suffixes
    setAlarm(steady_clock::now() + 1h, // C++14, but
             s, // same meaning
             30s); // as above
  };
```

如果使用 std::bind 直接替换 lambda 表达式，可以改写成如下：

```c++
using namespace std::chrono; // as above
using namespace std::literals;
using namespace std::placeholders; // needed for use of "_1"
auto setSoundB = // "B" for "bind"
  std::bind(setAlarm,
            steady_clock::now() + 1h, // incorrect! see below
            _1,
            30s);
```

首先，相较于 `lambda` 版本，使用 `std::bind`，函数调用和传参不那么明显。并且这里还有一个占位符 “_1”，使用 `setSoundB` 时候，你需要查阅 `setAlarm` 的函数申明，才知道这里的占位符的传参类型。

最重要的是这里的代码逻辑有问题。显然，我们期望的是在调用 `setAlarm` 时候计算表达式 `steady_clock::now() + 1h` 的值。但是，使用 `std::bind` 的时候，表达式 `steady_clock::now() + 1h` 是传递给 std::bind 而不是 `setAlarm`，这意味着，在调用 `std::bind` 的时候，表达式的值就被计算出来，然后保存在绑定对象内部。这就导致和在调用 setAlarm 时候计算表达式的期望不一致。可以再使用一个 `std::bind` 封装该表达式以延迟到 `setAlarm` 调用的时候才计算：

```c++
auto setSoundB =
  std::bind(setAlarm,
            std::bind(std::plus<>(), steady_clock::now(), 1h),  // C++14
            _1,
            30s);
```

注意到 `std::plus<>` 缺省了类型参数，这是 C++14 的新特性，如果是 C++11，则需要指定类型：

```c++
auto setSoundB =
  std::bind(setAlarm,
            std::bind(std::plus<steady_clock::time_point>(), // C++11
                      steady_clock::now(),
                      hours(1)),
            _1,
            seconds(30));
```

如果 `setAlarm` 增加一个重载版本：

```c++
enum class Volume { Normal, Loud, LoudPlusPlus };
void setAlarm(Time t, Sound s, Duration d, Volume v);
```

先前 lambda 版本代码依然可以正常工作。但是，std::bind 将会产生编译报错。因为编译器无法确认传递哪个版本的 `setAlarm`。需要将 setAlarm 转换为合适的函数指针：

```c++
using SetAlarm3ParamType = void(*)(Time t, Sound s, Duration d);
auto setSoundB =                                        // now
  std::bind(static_cast<SetAlarm3ParamType>(setAlarm),  // okay
            std::bind(std::plus<>(),
                      steady_clock::now(),
                      1h),
            _1,
            30s);
```

但是，这又引入了 std::bind 和 lambda 二者的不同。setSoundL 使用正常的函数调用来调用 setAlarm，编译器可以选择使用内联。

```c++
setSoundL(Sound::Siren); // body of setAlarm may
                         // well be inlined here
```

但是 std::bind 不可以，setSoundB 使用函数指针调用调用 setAlarm，这是运行期的行为，无法被内联。

```c++
setSoundB(Sound::Siren); // body of setAlarm is less
                         // likely to be inlined here
```

这就是使用 lambda 的第二个优势：代码的性能可能会更好。

使用 lambda 的第三个优势是代码更容易理解。看下面的例子：

```c++
enum class CompLevel { Low, Normal, High }; // compression level
Widget compress(const Widget& w, // make compressed
                CompLevel lev);  // copy of w
```

假设我们想创建一个函数对象，用来指定特定 `Widget` 的压缩等级。使用 std::bind 创建函数对象：

```c++
Widget w;
using namespace std::placeholders;
auto compressRateB = std::bind(compress, w, _1);
```

传递 `w` 给 `std::bind`，那么 w 如何存放在 `compressRateB` 内部的呢？是传值还是传引用？如果 `w` 在调用 `std::bind` 和 `compressRateB` 之间发生改变，传引用的方式将导致结果的不同。

`std::bind` 默认是拷贝它的参数到绑定对象内，用户可以使用 `std::ref` 指定传引用：

```c++
auto compressRateB = std::bind(compress, std::ref(w), _1);
```

这就需要你了解 std::bind 实现机制。但对于 lambda 的实现版本，w 是值捕获还是引用捕获非常明显：

```c++
auto compressRateL =            // w is captured by
  [w](CompLevel lev)            // value; lev is
  { return compress(w, lev); }; // passed by value
```

同样明显的是参数如何传递给 lambda 的。这里，很明显 lev 是值传递：

```c++
compressRateL(CompLevel::High); // arg is passed
                                // by value
```

但是，std::bind 的绑定对象的调用，参数是如何传递的？

```c++
compressRateB(CompLevel::High); // how is arg
                                // passed?
```

答案是引用传递，这就需要你了解 std::bind 的工作机制：std::bind 绑定对象的函数调用使用了完美转发机制。

通过上述比较我们可以看到，相较于使用 std::bind，使用 lambda 表达式的代码可读性更强、更容易理解、性能可能更好。对于 C++14，你没有理由不选择使用 lambda。对于 C++11，只有两种场景，std::bind 可以弥补 lambda 的不足：

第一：移动捕获。C++14 的初始化捕获模式支持移动捕获。C++11 的 lambda 不支持移动捕获，可以使用 std::bind 模拟来间接实现，参见 Item32 。

第二：多态函数对象。C++14 支持 auto 参数类型，也即通用 lambda，参见 Item33 。但是 C++11 不支持通用 lambda。而 std::bind 绑定对象的函数调用使用完美转发实现，可以接收任何类型的参数。如下例子：
————————————————

```c++
class PolyWidget {
public:
  template<typename T>
  void operator()(const T& param);
  …
};

PolyWidget pw;
auto boundPW = std::bind(pw, _1);

boundPW(1930);       // pass int to
                     // PolyWidget::operator()
boundPW(nullptr);    // pass nullptr to
                     // PolyWidget::operator()
boundPW("Rosebud");  // pass string literal to
                     // PolyWidget::operator()
```

C++11 做不到，C++14 则很容易：

```c++
auto boundPW = [pw](const auto& param) // C++14
               { pw(param); };
```

{{<admonition quote "总结" false>}}
- 相较于 std::bind，lambda 代码可读性更强、更容易理解、性能可能更好。
- C++11 的 std::bind 在实现移动捕获、模板函数对象方面可以弥补 lambda 的不足。
{{</admonition>}}


### CH07: The Concurrency API
#### [Item 35: Prefer task-based programming to thread-based.](https://blog.csdn.net/Dong_HFUT/article/details/125702349)

如果你想异步运行一个函数 donAsyncWork，你有两个基本的选择：基于线程的方法（thread-based）和基于任务的方法（task-based）。

```c++
int doAsyncWork();
std::thread t(doAsyncWork);         // thread-based
auto fut = std::async(doAsyncWork); // task-based
```

在比较二者优劣前，我们先介绍下 C++ 软件中线程的3个层次：
- 硬件线程。硬件实际执行计算的并行数。现代计算机架构中，一个硬件核对应一个或多个硬件线程。
- 软件线程。也被称为系统线程，指的是操作系统管理核调度的所有线程。软件线程运行在硬件线程之上，并且可以创建的软件线程要多于硬件线程。这样的好处是：当某些软件线程处于阻塞状态（等待IO、mutex、condition variable）时，可以执行其他线程以提高吞吐率。
- std::thread。C++ 的线程对象，作为句柄对应系统的软件线程。std::thread 也可以是空句柄而不对应系统的软件线程。例如没有执行函数、执行函数被移动其他线程、已经 join 或 detached 的 std::thread 对象。

<mark>基于任务的方法一般要优于基于线程的方法。</mark>

doAsyncWork 有返回值，可以代表任务的执行状态。基于线程的方法没有提供一个很好的机制获取返回值。而 std::async 返回的 std::future 对象提供了 get 方法可以获取到返回值。并且当 doAsyncWork 返回异常时，基于线程的方法直接抛出 std::terminate，而基于任务的方法可以根据返回值做异常处理。

系统的软件线程是有限的，当请求创建的 std::thread 多于系统提供的最大软件线程数，将抛出 std::system_error，即使 doAsyncWork 被设置成 noexcept。因而基于线程的方法需要处理这种情况，这就需要对线程进行管理。

即使你没有用尽软件线程，基于线程的方法还存在认购超额（oversubscription）的问题，即就绪态的软件线程高于硬件线程。操作系统会采用时间片轮询的方式执行所有的软件线程，而线程的上下文切换会增加线程管理的开销。并且硬件线程被切换到另一个软件线程时，其 cache 上的数据通常会失效，也会增加线程的开销。想要避免认购超额问题还比较困难，软件线程于硬件线程的合理比例取决于多种因素。例如硬件架构的特点、cache的使用方式、任务的特点等。

综上，线程的管理是比较困难的。而基于任务的方法将线程管理交给了 C++ 标准库，而 C++ 标准库可以更好地管理线程。例如，你无需担心软件线程耗尽的问题，因为默认参数的 std::async 不一定会创建线程，它可能在认购超额时将当前任务安排在当前线程上执行。另外 C++ 标准库可能比你更清楚硬件线程的资源，可以很好的避免负载不均衡的问题。

当然，基于线程的方法也有一定的优势：

需要访问实现线程的底层API。std::thread 可以获取底层线程的句柄，可以使用底层线程的API。
需要优化线程的使用。例如，如果你正在开发一个服务软件，而这个软件是这台机器上执行的唯一有意义的进程，并且你清楚这台机器的硬件配置。
需要实现一些高级的线程技术。例如线程池技术，而 C++ 标准库没有提供。
除了上述情况外，建议优先使用基于任务的编程方法。

{{<admonition quote "总结" false>}}
- std::thread API 不能直接访问异步函数执行的结果，如果执行函数有异常抛出，代码终止执行。
- 基于线程的编程方式存在资源耗尽、认购超额、负载均衡的方案移植性不佳。
- 通过 std::async 的基于任务的编程方式会默认解决上面的问题。
{{</admonition>}}

#### [Item 36: Specify std::launch::async if asynchronicity is essential.](https://blog.csdn.net/Dong_HFUT/article/details/126076160)

当你使用 `std::async()` 执行一个函数或可调用对象时，你通常期望这个函数是异步执行。但是， `std::async()` 不一定如你所愿。其实 `std::async()` 是根据执行策略决定是否会异步执行。 `std::async()` 有两种执行策略，定义在 `std::launch` 作用域中：

`std::launch::async` 函数或可执行对象必须异步执行，也即运行在其他线程上。
`std::launch::deferred` 函数或可执行对象延迟执行。仅在 `std::async()` 的返回对象 `std::future` 调用 `get` 或 `wait` 时，才在当前线程同步执行，并且调用者会阻塞直到函数执行完成。

`std::async()` 的默认策略其实是二者的组合，也即以下两者涵义完全相同：

```c++
auto fut1 = std::async(f); // run f using default launch policy

auto fut2 = std::async(std::launch::async |   // run f either
                       std::launch::deferred, // async or
                       f);                    // deferred
```

默认的策略下，f 可能是同步执行也可能是异步执行。正如 Item 35: Prefer task-based programming to thread-based. 中讨论的：标准库的线程管理模块承担了线程的创建和释放的职责，可以有效避免超额订阅、保证负载均衡。这极大地方便了 std::async 的使用。

但是，默认策略也会有如下问题：
- 无法预测 f 是否是并发执行。
- 无法预测 f 是否运行在 get 或 wait 调用时的线程上。
- 甚至无法预测 f 是否已经执行了。因为没法保证一定会调用 get 或 wait。

当 `f` 要访问本地线程存储（TLS，Thread Local Storage）时，无法预测访问的是哪个线程的本地存储。

```c++
auto fut = std::async(f); // TLS for f possibly for
                          // independent thread, but
                          // possibly for thread
                          // invoking get or wait on fut
```

std::async 的默认策略还会影响到 wait_for 超时调用写法，可能导致 bug，例如：

```c++
using namespace std::literals; // for C++14 duration suffixes; see Item 34
void f()                       // f sleeps for 1 second,  then returns
{
  std::this_thread::sleep_for(1s);
}
auto fut = std::async(f);             // run f asynchronously (conceptually)
while (fut.wait_for(100ms) !=         // loop until f has
       std::future_status::ready)     // finished running...
{                                     // which may never happen!
  …
}
```

如果 std::async 是并发执行，也即执行策略为 std::launch::async，以上代码没有问题。但是，如果执行策略为 std::launch::deferred时，fut.wait_for 总是返回 future_status::deferred，以上代码就会有问题。解决办法也很简单，先通过 wait_for 的超时时间为 0 来检测 std::async 是异步执行还是同步执行：

```c++
auto fut = std::async(f);          // as above
if (fut.wait_for(0s) ==            // if task is
    std::future_status::deferred)  // deferred...
{
             // ...use wait or get on fut
  …          // to call f synchronously

} else {     // task isn't deferred
  while (fut.wait_for(100ms) !=          // infinite loop not
         std::future_status::ready) {    // possible (assuming
                                         // f finishes)
    …                // task is neither deferred nor ready,
                     // so do concurrent work until it's ready
  }
  …                  // fut is ready
}
```

综上，如果你的使用场景不是以下几种，则需要考虑是否需要替换 std::async 的默认策略：
- 当调用 `get` 或 `wait` 时，任务不需要并发执行。
- 并不关心访问的是哪个线程的本地存储。
- 可以保证 `get` 或 `wait` 一定会被调用，或者任务不被执行也能接受。
- 使用 `wait_for` 或 `wait_until` 时，需要考虑 `std::launch::deferred` 策略。


如果不是以上场景，你可能需要指定使用 `std::launch::async` 策略，也即真正创建一个线程去并发执行任务：

```c++
auto fut = std::async(std::launch::async, f);  // launch f asynchronously
```

这里提供一个并发执行任务的封装：

```c++
template<typename F, typename... Ts>  // C++11
inline
std::future<typename std::result_of<F(Ts...)>::type>
reallyAsync(F&& f, Ts&&... params)       // return future
{                                        // for asynchronous
  return std::async(std::launch::async,  // call to f(params...)
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}
```

`reallyAsync` 接受一个可执行对象 f 和 多个参数 params，并完美转发给 std::async ，同时使用 std::launch::async 策略。C++14 版本如下：

```c++
template<typename F, typename... Ts>
inline
auto     // C++14
reallyAsync(F&& f, Ts&&... params)
{
  return std::async(std::launch::async,
  std::forward<F>(f),
  std::forward<Ts>(params)...);
}
```

{{<admonition quote "总结" false>}}
- std::async 的默认启动策略允许是异和者同步。
- 灵活性导致访问 thread_locals 的不确定性，隐含了任务可能不会被执行的含义，会影响程序基于超时的 wait 调用。
- 只有确定是异步时才指定为 std::launch::async。
{{</admonition>}}

#### [Item 37: Make std::threads unjoinable on all paths.](https://blog.csdn.net/Dong_HFUT/article/details/126195848)

每个 `std::thread` 只会处于两种状态状态之一：其一为 `joinable`，其二为 un`joinable` 。一个 `joinable` 的 `std::thread` 对应于一个正在或可能在运行的底层线程。例如，一个对应于处于阻塞或者等待调度的底层线程的 `std::thread` 是 `joinable`。对应于底层线程的 `std::thread` 已经执行完成也可以被认为是 `joinable`。

而 `unjoinable` 的线程包括：
- 默认构造的 `std::thread`。这样的 `std::thread` 没有执行函数，也就不会对应一个底层的执行线程。
- `std::thread` 对象已经被 move。其底层线程已经被绑定到其它 `std::thread`。
- `std::thread` 已经 join。已经 join 的对应 `std::thread` 的底层线程已经运行结束。
- `std::thread` 已经 detach。已经 detach 的 `std::thread` 与其对应的底层线程已经没有关系了。

std::thread 的 joinabilty 状态之所以重要的原因之一是：一个 joinable 状态的 std::thread 对象的析构函数的调用会导致正在运行程序停止运行。例如，我们有一个 doWork 函数，它接收一个过滤函数 filter 和一个最大值 MaxVal 作为参数。 doWork 检查并确定所有条件满足时，对 0 到 MaxVal 执行 filter。对于这样的场景，一般会选择基于任务的方式来实现，但是由于需要使用线程的 handle 设置任务的优先级，只能使用基于线程的方法来实现（相关讨论可以参见 [Item 35: Prefer task-based programming to thread-based.](https://blog.csdn.net/Dong_HFUT/article/details/125702349?spm=1001.2014.3001.5502)）。可能的实现如下：

```c++
constexpr auto tenMillion = 10000000; // see Item 15 for constexpr
bool doWork(std::function<bool(int)> filter, // returns whether
            int maxVal = tenMillion)         // computation was
{                                            // performed; see
                                             // Item 2 for
                                             // std::function
  std::vector<int> goodVals;  // values that
                              // satisfy filter
  std::thread t([&filter, maxVal, &goodVals]  // populate
                {                             // goodVals
                  for (auto i = 0; i <= maxVal; ++i)
                  { if (filter(i)) goodVals.push_back(i); }
                });
  auto nh = t.native_handle();      // use t's native
  …                                 // handle to set
                                    // t's priority
  if (conditionsAreSatisfied()) {
    t.join();                       // let t finish
    performComputation(goodVals);
    return true;                    // computation was
  }                                 // performed
  return false;                     // computation was
}                                   // not performed
```

对于上面的实现，如果 conditionsAreSatisfied() 返回 true，没有问题。如果 conditionsAreSatisfied() 返回 false 或抛出异常，`std::thread` 对象处于 `joinable` 状态，并且其析构函数将被调用，会导致执行程序停止运行。

你可能会疑惑为什么 `std::thread` 的析构函数会有这样的行为，那是因为其他两种选项可能更加糟糕：

隐式的 join。析构函数调用时，隐式去调用 join 等待线程结束。这听起来似乎很合理，但会导致性能异常，并且这有点反直觉，因为 conditionsAreSatisfied() 返回 false 时，也即条件不满足时，还在等待 filter 计算完成。
隐式 detach。析构函数调用时，隐式调用 detach 分离线程。doWork 可以快速返回，但可能导致 bug。因为 doWork 结束后，其内部的 goodVals 会被释放，但线程还在运行，并且访问 goodVals ，将导致程序崩溃。
由于 `joinable` 的线程会导致严重的后果，因此标准委员会决定禁止这样的事情发生（通过让程序停止运行的方式）。这就需要程序员确保 `std::thread` 对象在离开其定义的作用域的所有路径上都是 un`joinable` 。但是想要覆盖所有的路径并非易事，return、continue、goto、break 或者异常等都能跳出作用域。

无论何时，想在出作用域的路径上执行某个动作，常用的方法是将这个动作放入到一个局部对象的析构函数中。这种对象被成为 RAII（Resource Acquisition Is Initialization）对象，产生这个对象的类是 RAII 类。RAII 类在标准库中很常见，例如 STL 容器（每个容器的析构函数销毁容器中的内容并释放它的内存）中的智能指针（std::unique_ptr 析构函数调用它的 deleter 删除它指向的对象，`std::shared_ptr` 和 std::weak_ptr 的析构函数中会减少引用计数）、std::fstream 对象（析构函数关闭相应的文件）。但是 `std::thread` 对象没有标准的 RAII 类，这可能是标准委员会拒绝将 join 和 detach 作为默认选项，因为他们也不知道这个类应该有什么样的行为。

好在实现这样的一个类也并非难事。例如，你可以让用户指定 ThreadRAII 类在销毁时选择 join 还是 detach：

```c++
class ThreadRAII {
public:
  enum class DtorAction { join, detach };    // see Item 10 for
                                             // enum class info
  ThreadRAII(std::thread&& t, DtorAction a)  // in dtor, take
  : action(a), t(std::move(t)) {}            // action a on t

  ~ThreadRAII()
  {
    if (t.joinable()) {                     // see below for
                                            // joinability test
      if (action == DtorAction::join) {
        t.join();
      } else {
        t.detach();
      }
    }
  }

  std::thread& get() { return t; }         // see below

private:
  DtorAction action;
  std::thread t;
};
```

关于上面代码的几点说明：
- 构造函数只接收 `std::thread` 的右值，因为 `std::thread` 不可拷贝。
- 构造函数参数排列顺序符合调用者的直觉（std:thread 为第一个参数，DtorAction 为第二个参数），但是成员变量的初始化符合成员变量的申明顺序。在这个类中两个成员变量的前后顺序没有意义，但是通常而言，一个成员的初始化依赖另一个成员。
- `ThreadRAII` 提供了 get 函数，用于访问底层的 `std::thread` 对象。提供 get 方法访问 `std::thread`，避免了重复实现所有 `std::thread` 的接口。
- `ThreadRAII` 的析构函数首先检查 t 是否为 `joinable` 是必要的，因为对一个 un`joinable` 的线程调用 join 和 detach 将产生未定义的行为。


将 `ThreadRAII` 应用于 `doWork` 的例子上：

```c++
bool doWork(std::function<bool(int)> filter,
            int maxVal = tenMillion)
{
  std::vector<int> goodVals;

  ThreadRAII t(                           // use RAII object
    std::thread([&filter, maxVal, &goodVals]
                {
                  for (auto i = 0; i <= maxVal; ++i)
                    { if (filter(i)) goodVals.push_back(i); }
                }),
    ThreadRAII::DtorAction::join          // RAII action
  );

  auto nh = t.get().native_handle();
  ...
  if (conditionsAreSatisfied()) {
    t.get().join();
    performComputation(goodVals);
    return true;
  }
  return false;
}
```

这个例子中，我们选择 join 作为 ThreadRAII 析构函数的动作。正如前文所述，detach 可能导致程序崩溃，join 可能导致性能异常。两害取其轻，性能异常相对可以接受。

正如 [Item 17: Understand special member function generation.](https://blog.csdn.net/Dong_HFUT/article/details/123433559?spm=1001.2014.3001.5501) 所介绍的，由于 ThreadRAII 自定义了析构函数，编译器将不在自动生成移动操作，但没有理由让 ThreadRAII 对象不支持移动。因而，需要我们将移动操作标记为 default：

```c++
class ThreadRAII {
public:
  enum class DtorAction { join, detach };

  ThreadRAII(std::thread&& t, DtorAction a)
  : action(a), t(std::move(t)) {}

  ~ThreadRAII()
  {
    ...  // as before
  }

  ThreadRAII(ThreadRAII&&) = default;            // support
  ThreadRAII& operator=(ThreadRAII&&) = default; // moving

  std::thread& get() { return t; }               // as before

private:
  DtorAction action;
  std::thread t;
};
```

{{<admonition quote "总结" false>}}
- 在所有路径上保证 thread 是 unjoinable 的。
- 析构时 join 会导致难以调试的性能异常问题。
- 析构时 detach 会导致难以调试的未定义行为。
- 在成员列表的最后声明 std::thread 类型成员。
{{</admonition>}}

#### [Item 38: Be aware of varying thread handle destructor behavior.](https://blog.csdn.net/Dong_HFUT/article/details/126435546)

Item 37 介绍了 std::thread 对应一个底层的系统执行线程，一个非 deferred 任务的 future（这里包括 std::future 和 std::shared_future，下同）也对应一个底层的系统执行线程。一个 joinable 的 std::thread 对象析构时会导致程序终止，因为隐式的 join() 和隐式的 detach() 可能会更加糟糕。但是，future 的析构函数可能是隐式地执行 detach()，也可能是隐式地执行 join()，或者二者皆不是。本 Item 将和大家探讨下这个问题。

直观地观察，被调用者（callee）和调用者（caller）之间有一个通信通道（channel），callee 异步执行完成后，将结果写入（通常通过 std::promise 对象）这个通道，caller 通过 future 读取结果。类似下面这个模型：

![](images/item_38_01.png)

但是，这个模型有点问题：callee 的结果存储在哪里？首先，不能存储在 callee 的 std::promise 对象中，因为 caller 在调用 future 的 get 方法之前，callee 可能已经结束了，callee 的局部变量 std::promise 已经销毁了。再者，callee 的结果也不能存储在 caller 的 std::future 中，因为 std::future 可以用来创建 std::shared_future ，那么这个结果就需要被拷贝多次，不是所有结果的类型都是可以拷贝的。其实 callee 的结果是被存储在独立于 caller 和 callee 之外的特殊位置，被成为共享状态（shared state）的位置。模型如下：

![](images/item_38_02.png)

由于这个共享状态的存在， future 的析构函数的行为则与这个共享状态关联的 future 决定：
- 由 std::async 发起的非 deferred 的任务的返回的 future 对象，并且它是最后一个引用共享状态的，那么它的析构会一直阻塞到任务完成，也就是隐式执行 join()。
- 其他的 future 只是简单的销毁。对于异步任务，类似隐式执行 detach()，对于 deferred 策略的任务则不再运行。

反过来看，future 会隐式执行 join() 需要满足下面 3 个条件：
- future 是由 std::async 创建产生，并且引用共享状态。
- std::async 指定的任务策略是 std::launch::async。
- future 是最后一个引用共享状态的对象。

future 的 API 没有提供它是否由 std::async 产生并指向共享状态，因此，对于任意的 future，我们无法知道它的析构函数会不会阻塞到任务完成。

```c++
// this container might block in its dtor, because one or more
// contained futures could refer to a shared state for a non-
// deferred task launched via std::async
std::vector<std::future<void>> futs;   // see Item 39 for info
                                       // on std::future<void>
class Widget {                         // Widget objects might
public:                                // block in their dtors
  ...
private:
  std::shared_future<double> fut;
};
```

如果你知道给定的 future 不满足上述 3 个条件，那么其析构函数就不会阻塞住。例如，future 来自于 std::packaged_task：

```c++
int calcValue();               // func to run

std::packaged_task<int()>      // wrap calcValue so it
  pt(calcValue);               // can run asynchronously

auto fut = pt.get_future();    // get future for pt
```

一般地， std::packaged_task pt 需要运行在一个 std::thread 上， std::packaged_task 的 future 是的析构是结束程序，还是执行 join，异或执行 detach，交给了对应的 std::thread 接下来的行为了：

```c++
{                                  // begin block
  std::packaged_task<int()>
    pt(calcValue);

  auto fut = pt.get_future();

  std::thread t(std::move(pt));   // std::packaged_task 不可拷贝

  ...                              // see below
}                                  // end block
```


`“…”` 就是接下来操作 t 的代码，可能如下：
- t 什么也没做（没有 join 也没有 detach）。在 block 结束时， t 是 joinable 的，这会导致程序终止。
- t 执行 join 。这个时候， fut 的析构函数没有必要再执行 join。
- t 执行 detach 。这个时候， fut 的析构函数没有必要再执行 detach。

下面给一个测例作为验证：

```c++
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

void func() {
    std::this_thread::sleep_for(3s);
}

int main() {
    {
        std::packaged_task<void()> pt(func);
        auto fut = pt.get_future();
        std::thread t(std::move(pt));
        // t.join();
        // t.detach();
    }
    std::cout << "hello!" << std::endl;
    return 0;
}
```

执行结果如下：

```c++
terminate called without an active exception
Aborted (core dumped)
```

将 `join` 或者 `detach` 部分代码打开，则不会产生上述问题。

{{<admonition quote "总结" false>}}
- future 的正常析构行为只是销毁 future 本身的成员数据。
- 最后一个引用通过 std::async 创建的 non-deferred 任务的共享状态的 future 会阻塞到任务结束。
{{</admonition>}}

#### [Item 39: Consider void futures for one-shot event communication.](https://blog.csdn.net/Dong_HFUT/article/details/126895081)

对于两个异步任务，经常需要一个任务（检测线程）告诉另一个任务（反应线程）特定的事件已经发生了，反应线程可以继续执行了。这个事件可能是某个数据结构被初始化了，某一阶段计算完成了，或者一个传感器数据已经采集好了。需要一种机制来完成两个任务线程间的通信，有哪些比较好的方法呢？

**使用条件变量**

一个明显的方法就是使用条件变量。检测线程在特定事件发生后，通过条件变量通知反应线程。反应线程需要借助 std::mutex 和 std::unique_lock（std::unique_lock 和 std::lock_guard 都是管理锁的工具，都是 RAII 类；它们都是在定义时获得锁，在析构时释放锁。它们的主要区别在于 std::unique_lock 管理锁机制更加灵活，可以再需要的时候进行 lock 或者 unlock ，不必须得是析构或者构造时。因而为了防止线程一直占用锁，条件变量选择和 std::unique_lock 一起工作，条件变量的 wait 系列方法会在阻塞时候自动释放锁）。代码逻辑如下：

```c++
std::condition_variable cv;   // condvar for event
std::mutex m;                 // mutex for use with cv

// 检测线程
…                     // detect event
cv.notify_one();      // tell reacting task
// cv.notify_all();   // tell multiple reacting task

// 反应线程
…                                        // prepare to react
{                                        // open critical section
  std::unique_lock<std::mutex> lk(m);    // lock mutex
  cv.wait(lk);                           // wait for notify;
                                         // this isn't correct!
  …                                      // react to event
                                         // (m is locked)
}                                        // close crit. section;
                                         // unlock m via lk's dtor
…                                        // continue reacting
                                         // (m now unlocked)
```

上述代码除了使用锁使程序变得复杂以外，还存在以下问题：
- 如果检测线程在反应线程 cv.wait 前发出通知，反应线程将会错过通知而永远不会被唤醒。
- 反应线程的 cv.wait 存在被虚假唤醒的可能（由于操作系统的问题，wait 在不满足条件时，也可能被唤醒，也即虚假唤醒）。虽然可以给 wait 传谓词参数，用于判断是否为真的唤醒，但是多数情况先并没有好的判断方法。


```c++
cv.wait(lk,
        []{ return whether the event has occurred; });
```

**使用共享的flag**

大家可能会想到使用一个共享的 flag 来实现不同线程的同步。代码逻辑如下：

```c++
std::atomic<bool> flag(false);   // shared flag; see Item 40 for std::atomic

// 检测线程
…                    // detect event
flag = true;         // tell reacting task

// 反应线程
…                   // prepare to react
while (!flag);      // wait for event
…                   // react to event
```

这种方法的缺点是反应线程在等待过程中不阻塞的，而是轮询机制，一直处在运行状态，也就是仍然占用硬件资源。

**使用条件变量加共享的flag**

还可以将条件变量和共享 flag 结合使用，flag 表示是否为发生了关心的事件。通过 std::mutex 同步访问 flag，就无需使用 std::atomic 类型的 flag 了，只要简单的 bool 类型即可。

```c++
std::condition_variable cv;
std::mutex m;
bool flag(false);

// 检测线程
…                                    // detect event
{
  std::lock_guard<std::mutex> g(m);  // lock m via g's ctor
  flag = true;                       // tell reacting task
                                     // (part 1)
}                                    // unlock m via g's dtor
cv.notify_one();                     // tell reacting task
                                     // (part 2)

// 反应线程
…                                      // prepare to react
{
  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, [] { return flag; });    // use lambda to avoid
                                       // spurious wakeups
  …                                    // react to event
                                       // (m is locked)
}
…                                      // continue reacting
                                       // (m now unlocked)
```

这种方法功能上没有什么问题，就是代码稍微复杂了些。

**使用 future**

在 [Item38](https://blog.csdn.net/Dong_HFUT/article/details/126435546?spm=1001.2014.3001.5502) 中介绍了 std::future 和 std::promise 的通信方式。std::future 内部存储了一个将来会被赋值的值，并可以通过 get 方法访问。而 std::promise 在将来给这个值赋值，每个 std::promise 内部都有一个 std::future 对象，std::promise 和其内部的 std::future 共享这个值。我们并不关心这个值具体是啥，因而 std::promise 和 std::future 的模板类型使用 void 即可。代码逻辑如下：

```c++
std::promise<void> p;      // promise for communications channel

// 检测线程
…                  // detect event
p.set_value();     // tell reacting task

// 反应线程
…                       // prepare to react
p.get_future().wait();  // wait on future corresponding to p
…                       // react to event
```

使用这种方法的优点包括：避免了使用 mutex，wait 是真阻塞的，也没有条件变量的 notify 在 wait 之前执行的问题。

当然这种方法也有缺点。首先 std::future 和 std::promise 间的共享状态是动态申请的堆内存，需要堆资源的申请和释放，有一定的开销。更重要的问题是，由于 std::promise 只能设置值一次，因而这种通知机制是一次性的。

假设你想让反应线程创建后暂停执行，直到期望的事件发生后继续执行，使用基于 future 的方法是一个不错的选择。例如：

```c++
std::promise<void> p;
void react();           // func for reacting task

void detect()                             // func for detecting task
{
  std::thread t([]                        // create thread
                {
                  p.get_future().wait();  // suspend t until
                  react();                // future is set
                });
  …                                       // here, t is suspended
                                          // prior to call to react
  p.set_value();                          // unsuspend t (and thus
                                          // call react)
  …                                       // do additional work
  t.join();                               // make t unjoinable
}                                         // (see Item 37)
```

为了让 detect 的所有出口 t 都是 unjoinable 的，应该使用 Item37 中介绍的 ThreadRAII 类的，例如：

```c++
void detect()
{
  ThreadRAII tr( // use RAII object
    std::thread([]
                {
                  p.get_future().wait();
                  react();
                }),
    ThreadRAII::DtorAction::join // risky! (see below)
  );
  …              // thread inside tr
                 // is suspended here
  p.set_value(); // unsuspend thread
                 // inside tr
  …
}
```

然而，上述代码还存在问题。如果在第一个 “…” 的地方发生异常，p 的 set_value 不会被执行，那么 lambda 函数中的 wait 永远不会返回，由于 tr 的类型是 join 的，则 tr 的析构永远不会完成，代码将会挂起（见 http://scottmeyers.blogspot.com/2013/12/threadraii-thread-suspension-trouble.html 中的相关讨论）。

这里给出不使用 RAII 类 Thread 的方法使其挂起然后取消挂起，这里关键是使用 std::shared_future 代替 std::future，std::future 的 share 成员函数将共享状态所有权转移到 std::shared_future：


```c++
std::promise<void> p;
void detect()                          // now for multiple
{                                      // reacting tasks
  auto sf = p.get_future().share();    // sf's type is
                                       // std::shared_future<void>
  std::vector<std::thread> vt;         // container for
                                       // reacting threads
  for (int i = 0; i < threadsToRun; ++i) {
    vt.emplace_back([sf]{ sf.wait();   // wait on local
                          react(); }); // copy of sf; see
  }                                    // Item 42 for info
                                       // on emplace_back
  …                                    // detect hangs if
                                       // this "…" code throws!
  p.set_value();                       // unsuspend all threads
  …
  for (auto& t : vt) {                 // make all threads
    t.join();                          // unjoinable; see Item 2
  }                                    // for info on "auto&"
}
```
这样，就可以很好地使用 future 实现线程间的一次性通信。

{{<admonition quote "总结" false>}}
- 对于简单的事件通信，基于条件变量的方法需要一个多余的互斥锁、对检测和反应任务的相对进度有约束，并且需要反应任务来确认事件是否已发生。
- 基于 flag 的方法可以避免的上一条的问题，但是不是真正的阻塞任务。
- 组合条件变量和 flag 使用，上面的问题都解决了，但是逻辑让人多少有点感觉有点生硬。
- 使用 std::promise 和 future 的方案可以避免这些问题，但为共享状态使用了堆内存，并且仅限于一次性通信。
{{</admonition>}}

#### [Item 40: Use std::atomic for concurrency, volatile for special memory.](https://blog.csdn.net/Dong_HFUT/article/details/127013499)

本 Item 探讨一下 atomic 类型和 volatile 关键字在并发程序中的区别和应用。

C++11 提供了 std::atomic 类模版，可以保证操作的原子性，确保其他线程看到的肯定是操作后的结果。类似对操作加锁，而其内部使用特殊指令实现，因而开销较小。考虑下面的应用场景：

```c++
std::atomic<int> ai(0);  // initialize ai to 0
ai = 10;                 // atomically set ai to 10
std::cout << ai;         // atomically read ai's value
++ai;                    // atomically increment ai to 11
--ai;                    // atomically decrement ai to 10
```

在上面代码执行过程中，其他线程读取 ai 的值只能是 0、10、11，不会有其他的值。这里面有两个方面值得注意：
- 其一，std::cout << ai 这整个操作不是原子的，只能保证 ai 的读取是原子的，不能保证整个语句是原子的，也就是说在 ai 读取后和写到标准输出之间的时刻，ai 的值可以被其他线程修改。不过，也不影响到 ai 的输出值，因为 operator<< 是值拷贝的。
- 其二，对于最后两条语句（++ai、--ai），它们都是 read-modify-write（RMW）类型操作，都是原子执行的。

而对于使用 volatile 关键字的对应的例子：

```c++
volatile int vi(0);  // initialize vi to 0
vi = 10;             // set vi to 10
std::cout << vi;     // read vi's value
++vi;                // increment vi to 11
--vi;                // decrement vi to 10
```

在上述代码执行过程中，其他线程读取到 vi 值可能是任一值，例如 -12、68、4090727，这是一种位定义的行为。再考虑下面的场景：

```c++
std::atomic<int> ac(0);  // "atomic counter"
volatile int vc(0);      // "volatile counter"

// Thread 1
++ac;
++vc;

// Thread 2
++ac;
++vc;
```

两个线程完成后，ac 的值肯定是 2，因为 ac 的 RMW 过程是保证原子的。但 vc 的值却不一定是 2，因为 vc 的 RMW 过程可以是交替进行的，例如：
  1. 线程1 读取 vc 的值，为 0。
  2. 线程2 读取 vc 的值，仍然为 0。
  3. 线程1 将读取的 vc 值从增加到 1，然后写进 vc 的内存。
  4. 线程2 将读取的 vc 值从增加到 1，然后写进 vc 的内存。

这样， vc 的值最终为 1 。vc 最终的值是不可预测的，这是一种未定义的行为。

这种 RMW 行为的原子性并不是关键字 volatile 和 atomic 类型的唯一区别。考虑这样一个场景：当一个线程完成一个重要计算后，通知另外一个线程。Item 39: Consider void futures for one-shot event communication. 讨论这一场景的方案。这里，我们使用 atomic 变量通信。代码类似如下：

```c++
std::atomic<bool> valAvailable(false);
auto imptValue = computeImportantValue(); // compute value
valAvailable = true;                      // tell other task it's available
```

从代码顺序上看，imptValue 的赋值发生在 valAvailable 赋值之前。但事实并未一定如此，编译器可以对改变二者的执行顺序以提高性能，例如：

```c++
a = b;
x = y;
```

因为两个赋值语句不互相依赖，编译器可以重排序如下：

```c++
x = y;
a = b;
```

即使编译器不重排序，底层的硬件也可能做重排序。

但是 `std::atomic` 的使用禁止了编译器和底层硬件对这段代码的重排序，这种行为称为顺序一致性模型。而 `volatile` 无法阻止这种重排序。

`volatile` 无法保证操作的原子性和无法阻止指令的重排序，这就导致了它在并发编程中很少使用，那么 `volatile` 的使用场景是什么呢？简而言之，`volatile` 用于告诉编译器它所处理的内存表现的不太“正常”。“正常”的内存有这样的特点：将一个值写入内存，这个值保持不变，直到它被改写。例如：

```c++
auto y = x; // read x
y = x;      // read x again
```

上面的代码中，多次读取 x 的值，编译器可以这样优化：会将 x 的值放在寄存器中，再读取 x 的值时，直接从寄存器中读取即可。

对于写内存，编译器也会做优化。例如：

```c++
x = 10; // write x
x = 20; // write x again
```

编译器会进行优化：只执行了 x = 12 条语句，而删除 x = 10 这条语句。

上述的优化对于“正常”行为的内存是适用的，但对于特殊的内存并不适用。最常见的这种特殊内存用于 memory-mapped I/O，这种内存用于和外设通信：

```c++
auto y = x; // read x
y = x;      // read x again
```

这样的两次写内存都会对外设产生影响。例如外设根据该内存的值显示波形，那么上述多条写内存的操作就不是冗余的。对于这种情况来说必需使用 volatile 来告诉编译器禁止对变量的读写进行优化。例如：

```c++
volatile int x;

auto y = x; // read x
y = x;      // read x again (can't be optimized away)

x = 10;  // write x (can't be optimized away)
x = 20;  // write x again
```

而 std::atomic 无法做到这一点。例如：

```c++
std::atomic<int> x;
x = 10;  // write x
x = 20;  // write x again
```

可能被编译器优化为：

```c++
std::atomic<int> x;
x = 20;  // write x
```

而对于：

```c++
std::atomic<int> x;
auto y = x; // error!
y = x;      // error!
```

实际上无法编译的，因为 std::atomic 的拷贝操作是被 deleted 的。std::atomic 的成员函数 load 和 store 可以提供这样的功能：

```c++
td::atomic<int> y(x.load()); // read x
y.store(x.load());           // read x again
```

对于上述代码，编译器可能优化为：

```c++
register = x.load();           // read x into register
std::atomic<int> y(register);  // init y with register value
y.store(register);             // store register value into y
```

可以将二者结合起来使用。例如：

```c++
volatile std::atomic<int> vai;  // operations on vai are
                                // atomic and can't be
                                // optimized away
```

可以用于 memory-mapped I/O 内存，并被多线程访问。

{{<admonition quote "总结" false>}}
- std::atomic 用于不使用锁的多线程数据访问，用于编写并发程序。
- volatile 阻止内存的读写优化。用于特殊内存的场景。
{{</admonition>}}


### CH08: Tweaks
#### [Item 41: Consider pass by value for copyable parameters that are cheap to move and always copied.](https://blog.csdn.net/Dong_HFUT/article/details/127054642)

C++ 函数参数传递方式有值传递、指针传递、引用传递的方式。一般地，考虑到拷贝开销，建议使用引用传递的方式。例如：

```c++
class Widget {
public:
  void addName(const std::string& newName) // take lvalue;
  { names.push_back(newName); }            // copy it

  void addName(std::string&& newName)      // take rvalue;
  { names.push_back(std::move(newName)); } // move it; see
  ...                                      // Item 25 for use
                                           // of std::move
private:
  std::vector<std::string> names;
};
```

对于左值，拷贝进 Widget.names 中。对于右值，移动进 Widget.names。上面代码是有效的，但是实现和维护两个函数有点冗余。

另一种方案是使用万能引用（universal reference）传参。例如：

```c++
class Widget {
public:
  template<typename T>                            // take lvalues
  void addName(T&& newName)                       // and rvalues;
  {                                               // copy lvalues,
    names.push_back(std::forward<T>(newName));    // move rvalues;
  }                                               // see Item 25
                                                  // for use of
  ...                                             // std::forward
};
```

万能引用版本代码量减少了很多，看起来也清爽很多，但也会有其他问题。但模板的实现一般要放到头文件里，也会实例化出多个版本（左值版本、右值版本以及可以转换为 std::string 的类型版本）。于此同时，还存在诸如 [Item 30](https://blog.csdn.net/Dong_HFUT/article/details/124787082?spm=1001.2014.3001.5502) 介绍万能引用和完美转发失效的例子、[Item 27](https://blog.csdn.net/Dong_HFUT/article/details/124227488?spm=1001.2014.3001.5502) 介绍的传参错误时编译报错可读性很差的问题。

那么有没有什么完美的方案可以解决上述两种方案遇到的问题呢？我们来分析下值传递的方案。

```c++
class Widget {
public:
  void addName(std::string newName)         // take lvalue or
  { names.push_back(std::move(newName)); }  // rvalue; move it
  ...
};
```

在 addName 内对 newName 使用 std::move 可以减少一次拷贝。这里使用 std::move 考虑到两点：首先，newName 独立于传入的参数，不会影响到调用者；再者，这里是最后使用 newName 的地方，对其移动不会影响其他代码。

值传递的方案可以解决引用重载版本的源码冗余问题和万能引用版本的不适用场景、传参错误报错信息可读性等问题，那剩下的问题就是值传递方案的性能了。

在 C++98 中，对于值传递的方案，不管传入的左值还是右值，newName 都会通过拷贝构造函数来构造。而到了 C++11，newName 在传入左值时是拷贝构造，传入右值是移动构造。考虑到下面的代码：

```c++
Widget w;
...
std::string name("Bart");

w.addName(name);            // call addName with lvalue
...
w.addName(name + "Jenne");  // call addName with rvalue
                            // (see below)
```

对于第一个调用，参数 `newName` 使用左值初始化，是拷贝构造。对于第二个调用，参数 `newName` 使用右值初始化，是移动构造。

我们把上述三种方案写到一起再对比下性能：

```c++
class Widget {                             // Approach 1:overload for
public:                                    // lvalues and rvalues.
  void addName(const std::string& newName) // take lvalue;
  { names.push_back(newName); }            // copy it

  void addName(std::string&& newName)      // take rvalue;
  { names.push_back(std::move(newName)); } // move it; see
  ...                                      // Item 25 for use
                                           // of std::move
private:
  std::vector<std::string> names;
};

class Widget {                             // Approach 2: use universal reference
public:
  void addName(const std::string& newName) // take lvalue;
  { names.push_back(newName); }            // copy it

  void addName(std::string&& newName)      // take rvalue;
  { names.push_back(std::move(newName)); } // move it; see
  ...                                      // Item 25 for use
                                           // of std::move

};

class Widget {                              // Approach 3: pass by value
public:
  void addName(std::string newName)         // take lvalue or
  { names.push_back(std::move(newName)); }  // rvalue; move it
  ...
};
```

同样，考虑上面两种调用方式：

```c++
Widget w;
...
std::string name("Bart");

w.addName(name);            // call addName with lvalue
...
w.addName(name + "Jenne");  // call addName with rvalue
                            // (see below)
```

这里，我们忽略掉编译器根据上下文信息所做的编译优化的干扰，对比下三种方案的性能开销：

- **引用重载**：首先，无论是左值还是右值重载函数， 调用者的实参是被绑定到引用 newName上，没有拷贝或移动开销。再者，对于左值引用重载函数， newName 被拷贝到 Widget::names 内，而对于右值引用重载函数，newName 被移动到 Widget::names 内。总的来说，左值需要一次拷贝，右值需要一次移动。
- **万能引用**：首先，调用者的实参也是被绑定到引用 newName上，也没有拷贝或移动开销。再者，由于使用了 std::forward ，左值实参则被拷贝到 Widget::names 内，而右值实参则被移动到 Widget::names 内。总的来说，左值需要一次拷贝，右值需要一次移动。对于调用者传入的参数不是 std::string 类型，而是可以转换为 std::string 的类型，比如 char* 类型，对于引用重载版本，需要先将 char* 构造成 std::string，这会增加其开销，而万能引用版本则直接将 char* 转发给 std::string 构造函数直接构造 std::string 类型，详见 Item 25 。这里不考虑这种特殊情况。
- **值传递**：首先，对于左值，需要调用拷贝构造 newName，而对于右值，需要移动构造 newName。再者， newName 被无条件移动到 Widget::names 内。总的来说，左值需要一次拷贝加一次移动，右值需要两次移动。相较于前两种引用传参的方法，多了一次移动操作。


再回头看下本 Item 的标题： Consider pass by value for copyable parameters that are cheap to move and always copied。缘于以下四个原因：

  1. 只考虑值传递的话，只需要写一个函数，目标代码中也会生成一个函数，并且可以避免万能引用方法的问题。但是引入了一点性能开销。
  2. 只对可拷贝的参数使用值传递方法。如果参数是 move-only 的，那值传递的方法肯定会失败。对于 move-only 类型参数，也无须提供左值引用重载函数，只需要一个右值引用的重载函数即可。例如，对于传递 std::unique_ptr 类型参数：
  ```c++
  	class Widget {
	public:
	  ...
	  void setPtr(std::unique_ptr<std::string>&& ptr)
	  { p = std::move(ptr); }
	private:
	  std::unique_ptr<std::string> p;
	};
	...
	Widget w;
    ...
    w.setPtr(std::make_unique<std::string>("Modern C++"));
  ```

  上述代码，`std::make_unique<std::string>("Modern C++")` 产生一个右值，然后被移动到成员变量 p 上。因此总的开销是一次移动。如果只提供值传递的方法：
    ```c++
    class Widget {
    public:
      ...
      void setPtr(std::unique_ptr<std::string> ptr)
      { p = std::move(ptr); }
      ...
    };
    ```

  相同的调用，会隐式移动构造 `ptr`，接着移动赋值给`p`。因而总的开销则是两次移动操作。
  3. 只有当移动开销低时才考虑值传递方法。因为只有当移动开销很低时，额外的一次移动才是可接受的。否则，执行一次不必要的移动操作和执行一次不必要的拷贝操作是类似的，都一样违反了 C++98 中避免值拷贝这一规则。
  4. 只有当参数总是要被拷贝的时才考虑值传递方法。假设在将参数放入 Widget::names 内之前先对参数进行合法性检查，满足条件才放入到 Widget::names 内。例如：
  ```c++
  class Widget {
  public:
    void addName(std::string newName)
    {
      if ((newName.length() >= minLen) &&
          (newName.length() <= maxLen))
      {
        names.push_back(std::move(newName));
      }
    }
    ...
  private:
    std::vector<std::string> names;
  };
  ```
  如果不满足条件则会浪费 newName 的构造和析构的开销，想比较而言，引用传参开销更小。


即使上述条件都满足（移动开销低的可拷贝参数被无条件拷贝）时，值传递也不一定适用。函数参数的拷贝有两种方式：通过构造（拷贝构造或移动构造）和通过赋值（拷贝赋值或移动赋值）。上面例子中的 addName 使用的就是构造的方式，其参数 newName 通过拷贝构造创建了一个新的元素放在 std::vector 的尾部。这种情况比引用传参多一次移动。

当参数通过赋值拷贝，情况要复杂的多。例如，你有一个表示密码的类，由于密码可以被改变，需要同时提供 setter 和 changeTo 两个方法，值传递方法的实现如下：

```c++
class Password {
public:
  explicit Password(std::string pwd)  // pass by value
  : text(std::move(pwd)) {}           // construct text

  void changeTo(std::string newPwd)   // pass by value
  { text = std::move(newPwd); }       // assign text

  ...

private:
  std::string text;                  // text of password
};

std::string initPwd("Supercalifragilisticexpialidocious");
Password p(initPwd);
```

这里，`p.text` 通过构造函数进行了密码的初始化。通过前面的分析可知，相比较引用传递的方法，多了一次额外的移动开销。当通过下面的方式修改密码时：

```c++
std::string newPassword = "Beware the Jabberwock";
p.changeTo(newPassword);
```

changeTo 采用的是赋值构造，值传递的方法会产生性能问题。构造 newPwd 时， std::string 的构造函数会被调用，这个构造函数中会分类内存来保存 newPwd，然后， newPwd 移动赋值给 text，这将导致 text 原来指向的内存会释放掉。也就是说，修改密码的过程发生一次内存的申请和一次内存的释放。其实在这里，旧的密码（“Supercalifragilisticexpialidocious”）比新的密码（“Beware the Jabberwock”）长度更长，没有必要申请或者释放内存。如果采用下面引用重载的方法，很可能申请和释放内存都不会发生：

```c++
class Password {
public:
  ...
  void changeTo(const std::string& newPwd) // the overload
  {                                        // for lvalues
    text = newPwd;          // can reuse text's memory if
                            // text.capacity() >= newPwd.size()
  }
  ...
private:
  std::string text;
};
```

当 text 的字符串长度大于 newPwd 的时会复用已经分配的内存。因此，开销要比值传递的方式要小。如果旧密码的长度要比新密码短时，那么赋值过程中的申请和释放内存不可避免，则值传递和引用传递二者的开销一致。

上面对函数参数通过赋值来拷贝的分析要考虑多种因素，例如传递的类型、左值还是右值、类型是否使用动态内存等。例如: 对于 std::string，如果它使用了SSO 优化，那么赋值的操作会将要赋值的内容放到 SSO 的缓存中，那么情况又不一样了。SSO 优化详见 [Item 29](https://blog.csdn.net/Dong_HFUT/article/details/124577258?spm=1001.2014.3001.5502)。

如果要追求极致的性能，值传递的方式可能不再是一个可行的方法，因为避免一次廉价的移动开销也是很重要的。并且我们并不是总是知道会有多少次这样的移动操作，例如，addName 通过值传递造成了一次额外的移动操作，但是这个函数内部又调用了 validateName，并且也是值传递的方式，这将就又造成了一次额外的移动开销，validateName 内部如果再调用其他的函数，并且这个函数同样是值传递的方式呢?这就造成了累加效应，而采用引用传递的方式就不会有这样的累加效应。

最后，一个与性能无关的话题，但却值得我们关注。那就是值传递的类型切割问题（slicing problem），详见 [C++ 按值传递的切割问题（Slicing Problem）](https://blog.csdn.net/Dong_HFUT/article/details/124577258?spm=1001.2014.3001.5502)。

{{<admonition quote "总结" false>}}
- 对于可复制、移动开销低、且无条件复制的参数，按值传递效率基本与按引用传递效率一致，而且易于实现，生成更少的目标代码。
- 通过构造函数拷贝参数可能比通过赋值拷贝开销大的多。
- 按值传递会引起切片问题，不适合基类类型的参数。
{{</admonition>}}

#### [Item 42: Consider emplacement instead of insertion.](https://blog.csdn.net/Dong_HFUT/article/details/127073175)

如果你有一个容器用于保存 std::string，你可以使用插入函数（例如 insert、push_front、push_back 或 std::forward_list 的insert_after）添加元素。例如：

```c++
std::vector<std::string> vs;  // container of std::string
vs.push_back("xyzzy");        // add string literal
```

这里，std::vector 的类型是 std::string，而插入的是字面值字符串（const char[6]）。std::vector 的 push_back 重载了左值和右值引用：

```c++
template <class T,                         // from the C++11
          class Allocator = allocator<T>>  // Standard
class vector {
public:
  ...
  void push_back(const T& x);  // insert lvalue
  void push_back(T&& x);       // insert rvalue
  ...
};
```

对于下面的调用：

```c++
vs.push_back("xyzzy");
```

由于实参的类型（const char[6]）和 push_back 形参类型（std::string 引用）类型不匹配，编译器会使用字符串字面值创建一个临时的 std::string 对象，再将这个临时对象传给 push_back，类似如下语义：

```c++
vs.push_back(std::string("xyzzy")); // create temp. std::string
                                    // and pass it to push_back
```

我们再仔细分解一下编译器的行为如下：
  1. 使用字面值 "xyzzy" 创建临时的 std::string 对象（记为 temp），这里调用一次 std::string 的构造函数。并且 temp 是一个右值。
  2. temp 接着被传入右值引用重载的 push_back，也即将 temp 拷贝给 x。接着将 x 放入 vs 中，这里调用移动构造函数完成。
  3. 最后 temp 被销毁，调用 std::string 的析构函数。

我们只是将字符串字面值传给 std::string 容器，却要调用两次构造和一次析构，对于追求代码性能的程序员而言，这个性能开销可能是无法接受的。

解决方案是使用 emplace_back 代替：

```c++
vs.emplace_back("xyzzy"); // construct std::string inside
                          // vs directly from "xyzzy"
```

emplace_back 使用了完美转发机制，如果传入的是右值，将直接使用这个右值在容器内部完成元素的构造。使用 emplace_back 将不会创建临时的 std::string 对象，将使用传入的字符串字面值（"xyzzy"）直接在容器内构造 std::string 对象。只要传入的参数合法，emplace_back 可以接收任意参数，然后完美转发到容器内部直接构造容器的元素。例如：

```c++
vs.emplace_back(50, 'x'); // insert std::string consisting
                          // of 50 'x' characters
```

`emplace` 系列接口和传统插入接口不同之处在于它可以接收可变参数，并且采用了完美转发机制，可以直接使用传入参数来构造容器元素（必须匹配到容器元素的构造函数）。而传统插入接口必须要插入和容器元素类型完全相同的对象。emplace 的优势是避免了临时对象的构造和析构。如果直接插入容器元素对象，那么二者是等价的，例如：

```c++
vs.push_back(queenOfDisco);     // copy-construct queenOfDisco
                                // at end of vs

vs.emplace_back(queenOfDisco);  // ditto
```

`emplace` 接口可以实现传统插入接口能做的所有事情，并且理论上，`emplace` 接口有时更高效。但实际却情况并非完全如此，虽然多数场景下，emplace 接口要比传统插入接口更加高效。但在少数场景下，传统插入接口要比 `emplace` 接口更加高效，这样的场景并不好归类，因为这取决于多种因素，例如传入参数的类型、使用的容器、插入容器中的位置、容器元素构造函数的异常安全机制、容器是否允许插入重复值、要插入的元素是否已经在容器中等。因而，给性能调优的建议是性能实测。

当然还是有一定的办法帮你来识别，如果以下条件都满足，`emplace` 接口几乎肯定要比传统插入接口更加高效：
  - **要插入的值是通过构造函数插入容器，而非赋值**。上面插入的字符串字面值就是这种情况，但如果插入的位置已经有元素了，情况就不同了，例如：
    ```c++
    std::vector<std::string> vs;
    vs.emplace(vs.begin(), "xyzzy");  // add "xyzzy" to
                                  // beginning of vs
    ```
    很少有编译器采用构造的方法将元素插入已经存在容器中存在的问题（这里是 vs[0]），而多数采用移动赋值的方法插入到已存在的位置。移动赋值需要被移动的对象，这就意味着需要构造临时的对象。那么 emplace 不会有临时对象的构造和析构的优势也就不存在了。
  - **传入参数的类型和容器元素的类型不同**。emplace 的优势是需要构造临时的对象，如果传参的类型和容器元素的类型相同，也就不会产生临时对象了，emplace 的优势也就不存在了。
  - **容器不大可能因为元素值重复而拒绝其加入**。这就意味着要不容器允许重复值加入，要不新加入的值大多数是唯一的。这样要求的原因是因为为了检测一个新值是否已经存在， emplace 的实现通常会创建一个新值的节点，然后和容器中已存在节点的值相比较，如果新节点的值不在容器中，则链接该节点。如果新节点的值已经在容器中，新创建的节点就要被销毁，这意味着新节点的构造和销毁就浪费了。

下面的调用完全满足上面的条件，因而 `empalce_back` 比 `push_back` 要高效。

```c++
vs.emplace_back("xyzzy");  // construct new value at end of
                           // container; don't pass the type in
                           // container; don't use container
                           // rejecting duplicates
vs.emplace_back(50, 'x');  // ditto
```

在决定是否使用 `emplace` 的时候，还有另外两个因素需要注意。第一个因素就是资源管理。例如：

```c++
std::list<`std::shared_ptr`<Widget>> ptrs;
```

如果你要添加一个自定义 deleter 的 `std::shared_ptr` 对象，那么无法使用 std::make_shared_ptr 来创建（详见Item 21）。只能使用 `std::shared_ptr` 管理原始指针:

```c++
void killWidget(Widget* pWidget);

ptrs.push_back(`std::shared_ptr`<Widget>(new Widget, killWidget));
// ptrs.push_back({ new Widget, killWidget });  // ditto
```

这样会先创建一个临时的 `std::shared_ptr` 对象，然后再传给 push_back。如果使用 emplace 接口，原则上临时对象的创建是可以避免的，但是这里创建临时对象却是必要的，考虑下面的过程：
  1. 首先，临时的 `std::shared_ptr`<Widge> 对象（temp）被创建。
  2. 然后， push_back 接受 temp 的引用。在 分配节点（用于接收 temp 的拷贝）的时候发生 OOM（out-of-memory）。
  3. 最后，异常从 push_back 传出后，temp 被销毁，它所管理的 Widget 对象也通过 killWidget 进行释放。

而如果使用 empalce 接口：

```c++
ptrs.emplace_back(new Widget, killWidget);
```

  1. new Widget 创建的原始指针被完美转发到 emplace_back 内部构造器，此时发生 OOM。
  2. 异常从 push_back 传出后，原始指针是 Widget 唯一访问路径，它直接被销毁，但其管理的内存却没办法释放，就会发生内存泄漏。

对于 std::unique_ptr 也有类似的问题。出现这样问题的根本原因是 `std::shared_ptr` 和 std::unique_ptr 对资源的管理取决于它们是否立即接管了这个资源，而 emplace 的完美转发机制延迟了资源管理对象的创建，这就给资源异常留下了可能的机会。这也是为什么建议使用 std::make_shared 和 std::make_unique 创建对象的原因。其实不应该将 “new Widget” 这样的表达式直接传给传统插入和 emplace 这样的函数，而应该直接传智能指针对象，像下面这样：

```c++
`std::shared_ptr`<Widget> spw(new Widget,  // create Widget and
                            killWidget); // have spw manage it
ptrs.push_back(std::move(spw));          // add spw as rvalue
```

或者:

```c++
`std::shared_ptr`<Widget> spw(new Widget, killWidget);
ptrs.emplace_back(std::move(spw));
```

两种方式都可以避免内存泄漏的问题，同时 emplace 的性能和传统插入接口也是一致的。

使用 emplace 第二个值得注意的因素是它和显示构造函数的交互。C++11 支持了正则表达式，假设创建一个存放正则表达式的容器：

```c++
std::vector<std::regex> regexes;
```

如果不小心写出了下面的错误代码：

```c++
regexes.emplace_back(nullptr); // add nullptr to container
                               // of regexes?
```

nullptr 不是正则表达式，为什么编译不会报错？例如：

```c++
std::regex r = nullptr; // error! won't compile
```

而使用 push_back 接口就是会报错：

```c++
regexes.push_back(nullptr); // error! won't compile
```

这背后的原因是使用字符串构造 std::regex 对象比较耗时，为此 std::regex 禁止隐式构造，采用 const char* 指针的std::regex 构造函数是显式的。这也就是下面代码无法编译通过的原因了：

```c++
std::regex r = nullptr;     // error! won't compile
regexes.push_back(nullptr); // error! won't compile
```

使用 emplace 接口，由于完美转发机制，最后在容器内部直接拿到 const char* 显示构造 std::regex ，因此。下面的代码可以编译通过：

```c++
regexes.emplace_back(nullptr); // can compile
```

总而言之，使用 emplace 接口时一定要注意传入参数的正确性。

{{<admonition quote "总结" false>}}
- 原则上，emplacement 函数会比传统插入函数更高效。
- 实际上，当执行如下操作时，emplacement 函数更快：（1）值被构造到容器中，而不是直接赋值；（2）传入参数的类型与容器类型不一致；（3）容器不拒绝已经存在的重复值。
- emplacement 函数可能执行类型转化，而传统插入函数会拒绝。
{{</admonition>}}


Reference:</br>
[ebook -- Effective Modern C++](https://moodle.ufsc.br/pluginfile.php/2377667/mod_resource/content/0/Effective_Modern_C__.pdf)</br>
[1]. [《Effective Modern C++》笔记与实践](https://zhuanlan.zhihu.com/p/592921281)</br>
[2]. [Effective Modern C++ 完全解读笔记汇总](https://blog.csdn.net/Dong_HFUT/article/details/127155670)</br>
[3]. [Effective modern C++ 学习笔记](https://zhuanlan.zhihu.com/p/553706181)</br>
[4]. [Effective Modern C++42招独家技巧助你改善C++11和C++14的高效用法笔记](https://blog.csdn.net/fengbingchun/article/details/104136592)</br>

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/effective_modern_c/  

