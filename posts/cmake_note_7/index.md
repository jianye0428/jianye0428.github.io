# CMake 笔记 | [7] 设置语言标准（一）


## 一、C ++ 标准历史

- 1998 年，C++ 标准委员会发布了第一版 C++ 标准，并将其命名为 C++ 98 标准
- 2011 年，新的 C++ 11 标准诞生，用于取代 C++ 98 标准。此标准还有一个别名，为C++ 0x
- 2014 年，C++ 14 标准发布，该标准库对 C++ 11 标准库做了更优的修改和更新
- 2017 年底，C++ 17 标准正式颁布

## 二、C++ 11版本特性介绍

在 `C++ 11` 标准之前，`C++` 标准委员会还在 2003 年对 C++ 98 标准做了一次修改（称为 `C++ 03` 标准），但由于仅仅修复了一些 `C++ 98` 标准中存在的漏洞，并未修改核心语法，因此人们习惯将这次修订和 C++ 98 合称为 `C++98/03` 标准。

以上 3 个标准中，`C++ 11` 标准无疑是颠覆性的，该标准在 C++ 98 的基础上修正了约 600 个 C++ 语言中存在的缺陷，同时添加了约 140 个新特性，这些更新使得 C++ 语言焕然一新。

### 2.1 类型推导之auto和decltype

在 `C++11` 之前的版本中，定义变量或者声明变量之前都必须指明它的类型，比如 int、char 等。`C++11` 使用 `auto` 关键字来支持自动类型推导。

在之前的 `C++` 版本中，`auto` 用来指明变量的存储类型，它和 `static` 是相对的。`auto` 表示变量是自动存储的，这也是编译器的默认规则，所以写不写都一样，这使得 `auto` 的存在变得非常鸡肋。

`C++ 11` 赋予 `auto` 新的含义，用它来做自动类型推导。即，使用 `auto` 关键字后，编译器会在编译期间自动推导出变量的类型。

{{<admonition quote "注意" false>}}
 - `auto` 仅仅是一个占位符，在编译器期间它会被真正的类型所替代。C++ 中的变量必须是有明确类型的，只是这个类型是由编译器自己推导出来的。
 - 使用 `auto` 类型推导的变量必须马上初始化，因为 `auto` 在 `C++11` 中只是占位符，并非如 int 一样的真正的类型声明。
{{</admonition>}}

**auto与const的结合使用**

```c++
int x = 0;
// n 为const int,auto 被推导为int
const auto n = x;
// f为const int， auto 被推导为int（const属性被抛弃）
auto f = n;
// r1为const int &类型，auto被推导为int
const auto &r1 = x;
// r1为const int&类型，auto 被推导为const int 类型
auto &r2 = r1;
```

`auto` 与 `const` 结合的用法：

- 当类型不为引用时，`auto` 的推导结果将不保留表达式的 `const` 属性；
- 当类型为引用时，`auto` 的推导结果将保留表达式的 `const` 属性。
- `auto`的限制：
  - 使用`auto`时必须对变量进行初始化
  - `auto`不能作为函数的形参
  - `auto` 不能作用于类的
- 非静态成员变量中
  - `auto` 关键字不能定义数组
  - auto 不能作用于模板参数

`decltype` 是 `C++11` 新增的一个关键字，它和 `auto` 的功能一样，都用来在编译时期进行自动类型推导。`decltype` 是declare type的缩写，译为声明类型。

`auto` 并不适用于所有的自动类型推导场景，在某些特殊情况下 `auto` 用起来非常不方便，甚至压根无法使用，所以 `decltype` 关键字也被引入到 `C++11` 中。

```c++
auto var_name = value;
decltype(exp) var_name = value;
```

其中，`var_name` 表示变量名，`value` 表示赋给变量的值，`exp` 表示一个表达式。

`auto` 根据=右边的初始值 `value` 推导出变量的类型，而 `decltype` 根据 `exp` 表达式推导出变量的类型，跟=右边的 `value` 没有关系。

`auto` 要求变量必须初始化，而 `decltype` 不要求。

`exp` 就是一个普通的表达式，它可以是任意复杂的形式，但是必须要保证 `exp` 的结果是有类型的，不能是 `void`；例如，当 `exp` 调用一个返回值类型为 `void` 的函数时，`exp` 的结果也是 `void` 类型，此时就会导致编译错误。

```c++
int a = 0;
// b 被推导成了 int
decltype(a) b = 1;
// x 被推导成了 double
decltype(10.8) x = 5.5;
// y 被推导成了 double
decltype(x + 100) y;
```

**decltype 推导规则**
- 如果 `exp` 是一个不被括号( )包围的表达式，或者是一个类成员访问表达式，或者是一个单独的变量，那么 `decltype(exp)` 的类型就和 `exp` 一致。
- 如果 `exp` 是函数调用，那么 `decltype(exp)` 的类型就和函数返回值的类型一致。
- 如果 `exp` 是一个左值，或者被`括号( )`包围，那么 `decltype(exp)`的类型就是 `exp` 的引用；假设 `exp` 的类型为 T，那么 `decltype(exp)` 的类型就是 T&。

{{<admonition quote "注意" false>}}
左值是指那些在表达式执行结束后依然存在的数据，也就是持久性的数据；
右值是指那些在表达式执行结束后不再存在的数据，也就是临时性的数据。
有一种很简单的方法来区分左值和右值，对表达式取地址，如果编译器不报错就为左值，否则为右值。
{{</admonition>}}

**auto与decltype对 cv 限定符的处理:**

cv 限定符是 const 和 volatile 关键字的统称:
  - const 关键字用来表示数据是只读的，也就是不能被修改
  - volatile 和 const 是相反的，它用来表示数据是可变的、易变的，目的是不让 CPU 将数据缓存到寄存器，而是从原始的内存中读取

在推导变量类型时，auto 和 decltype 对 cv 限制符的处理是不一样的。decltype 会保留 cv 限定符，而 auto 有可能会去掉 cv 限定符。其原理见auto与const的结合使用。

**auto与decltype对引用(&)的处理:**

当表达式的类型为引用时，auto 和 decltype 的推导规则也不一样；decltype 会保留引用类型，而 auto 会抛弃引用类型，直接推导出它的原始类型。

### 2.2 C++ 返回值类型后置

在泛型编程中，如果需要通过参数的运算来得到返回值的类型：

```c++
template <typename R, typename T, typename U>
R Add(T t, U u)
{
    return t+u;
}

int main() {
  int a = 1;
  float b = 2.0f;
  auto c = Add<decltype(a + b)>(a + b);
  return 0;
}
```

以上代码是因为我们并不关心a + b的类型是什么，因此只需要通过decltype(a + b)直接得到返回值类型即可。

上述使用过程十分不方便，因为外部其实并不知道参数之间应该如何运算，只有Add函数知道返回值应该如何推导。

在函数定义上直接通过decltype获取返回值：

```c++
template <typename T, typename U>
decltype(T() + U()) add(T t, U u) {
    return t + u;
}
```

考虑到 T、U 可能是没有无参构造函数的类，正确的写法如下：

```c++
template <typename T, typename U>
decltype((*(T*)0) + (*(U*)0)) add(T t, U u) {
    return t + u;
}
```
上述代码虽然成功地使用 `decltype` 完成了返回值的推导，但写法过于晦涩，会大大增加`decltype`在返回值类型推导上的使用难度并降低代码的可读性。

因此，在 C++11 中增加了**返回类型后置语法**，将 `decltype` 和 `auto` 结合起来完成返回值类型的推导。

```c++
template <typename T, typename U>
auto add(T t, U u) -> decltype(t + u){
    return t + u;
}
```

### 2.3 对模板实例化中连续尖括号>>改进

在 C++98/03 的泛型编程中，模板实例化过程中，连续两个右尖括号（>>）会被编译器解释成右移操作符，而不是模板参数表的结束。

```c++
template <typename T>
struct Foo{
  typedef T type;
};

template <typename T>
class A{
  // ...
};

int main(){
  //编译出错
  Foo<A<int>>::type xx;
  return 0;
}
```

上述代码使用 gcc 编译时，会得到如下错误提示：

```shell
error: ‘>>’ should be ‘>>’ within a nested template argument list Foo<A>::type xx;
```

意思就是，Foo<A<int>>这种写法是不被支持的，要写成这样`Foo<A<int> >`(注意两个右尖括号之间的空格)。

这种限制是**很没有必要**的。因为在 C++ 的各种成对括号中，目前只有右尖括号连续写两个会出现这种二义性。static_cast、reinterpret_cast 等 C++ 标准转换运算符，都是使用<>来获得待转换类型（type-id）的。若这个 type-id 本身是一个模板，用起来会很不方便。

在 C++11 标准中，要求编译器对模板的右尖括号做单独处理，使编译器能够正确判断出>>是一个右移操作符还是模板参数表的结束标记。

<mark>注意：</mark>上述这种自动化的处理在某些时候会与老标准不兼容：

```c++
template <int N>
struct Foo{
  // ...
};

int main() {
  // 解决方案：
  // Foo<(100 >> 2)> xx;
  Foo<100 >> 2> xx;
  return 0;
}
```

在 C++98/03 的编译器中编译是没问题的，但 C++11 的编译器会显示:

```shell
error: expected unqualif?ied-id before ‘>’ token Foo<100 >> 2> xx;
```
### 2.4 使用using定义别名（替代typedef）

C++可以使用typedef重定义一个类型，被重定义的类型不一定是一个新的类型，也有可能仅仅是原有类型取了一个新的名字。使用typedef重定义类型是很方便的，但它也有一些限制，如无法重定义一个模板等。

```c++
template<typename T>
using str_map_t = std::map<std::string, T>;
// ...
str_map_t<int> map_1;
```

实际上，`using`的别名语法覆盖了`typedef`的全部功能。

```c++
// 重定义unsigned int
typedef unsigned int uint_t;
using uint_t = unsigned int;
// 重定义std::map
typedef std::map<std::string, int> map_int_t;
using map_int_t = std::map<std::string, int>;

// 重定义模板
// C++98/03
template <typename T>
struct func_t{
    typedef void (*type)(T, T);
};
// 使用 func_t 模板
func_t<int>::type xx_1;
// C++11
template <typename T>
using func_t = void (*)(T, T);
// 使用 func_t 模板
func_t<int> xx_2;
```

从示例中可以看出，通过 `using` 定义模板别名的语法，只是在普通类型别名语法的基础上增加 `template` 的参数列表。使用 `using` 可以轻松地创建一个新的模板别名，而不需要像 `C++98/03` 那样使用烦琐的外敷模板。

### 2.5 支持函数模板的默认参数

在 `C++98/03` 标准中，**类模板**可以有默认的模板参数:

```c++
template <typename T, typename U = int, U N = 0>
struct Foo{
    // ...
};
```

但是不支持函数的默认模板参数：

```c++
// error in C++98/03: default template arguments
template <typename T = int>
void func(){
    // ...
}
```

现在这一限制在 C++11 中被解除了。上面的 func 函数在 C++11 中可以直接使用:

```c++
int main(void){
  //T = int
    func();
    return 0;
}
```

函数模板的默认模板参数在使用规则上和其他的默认参数也有一些不同，它没有必须写在参数表最后的限制。甚至于，根据实际场景中函数模板被调用的情形，编译器还可以自行推导出部分模板参数的类型。即当默认模板参数和编译器自行推导出模板参数类型的能力一起结合使用时，代码的书写将变得异常灵活。我们可以指定函数中的一部分模板参数采用默认参数，而另一部分使用自动推导：

```c++
template <typename R = int, typename U>
R func(U val){
    return val;
}

int main(){
  // R=int, U=int
    func(97);
  // R=char, U=int
    func<char>(97);
  // R=double, U=int
    func<double, int>(97);
    return 0;
}
```

当默认模板参数和自行推导的模板参数同时使用时，若无法推导出函数模板参数的类型，编译器会选择使用默认模板参数；如果模板参数无法推导出来，又未设置其默认值，则编译器直接报错。

```c++
template <typename T, typename U = double>
void func(T val1 = 0, U val2 = 0) {
    //...
}

int main() {
  // T=char, U=double
    func('c');
  // 编译报错
    func();
    return 0;
}
```

### 2.6 在函数模板和类模板中使用可变参数

**<font color=red>可变参数</font>**，指的是参数的个数和类型都可以是任意的。

对于函数参数而言，C++ 一直都支持为函数设置可变参数，最典型的代表就是 printf() 函数，它的语法格式为:

```c++
int printf ( const char * format, ... );
```

`...`就表示的是可变参数，即 printf() 函数可以接收任意个参数，且各个参数的类型可以不同。

```c++
printf("%d", 10);
printf("%d %c",10, 'A');
printf("%d %c %f",10, 'A', 1.23);
```

通常将容纳多个参数的可变参数称为**参数包**。借助 `format` 字符串，`printf()` 函数可以轻松判断出参数包中的参数个数和类型。

```c++
#include <iostream>
#include <cstdarg>

//可变参数的函数
void vair_fun(int count, ...) {
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; ++i) {
		int arg = va_arg(args, int);
		std::cout << arg << " ";
	}
	va_end(args);
}

int main() {
	//可变参数有 4 个，分别为 10、20、30、40
	vair_fun(4, 10, 20, 30,40);
	return 0;
}
```

想使用参数包中的参数，需要借助`<cstdarg>`头文件中的 `va_start`、`va_arg` 以及 `va_end` 这 3 个带参数的宏：
  - `va_start(args, count)`：args 是 va_list 类型的变量，可以简单的将其视为 char * 类型。借助 count 参数，找到可变参数的起始位置并赋值给 args；
  - `va_arg(args, int)`：调用 va_start找到可变参数起始位置的前提下，通过指明参数类型为 int，va_arg 就可以将可变参数中的第一个参数返回;
  - `va_end(args)`：不再使用 args 变量后，应及时调用 va_end 宏清理 args 变量。

使用`…`可变参数的过程中，需注意以下几点：
  - `…` 可变参数必须作为函数的最后一个参数，且一个函数最多只能拥有 1 个可变参数;
  - 可变参数的前面至少要有 1 个有名参数;
  - 当可变参数中包含 `char` 类型的参数时，`va_arg` 宏要以 `int` 类型的方式读取；当可变参数中包含 `short` 类型的参数时，`va_arg` 宏要以 `double` 类型的方式读取。

<mark>需要注意的是,</mark> `…`可变参数的方法仅适用于函数参数，并不适用于模板参数。

### 2.7 可变参数模板

C++ 11 标准发布之前，**函数模板**和**类模板**只能设定固定数量的模板参数。<u>C++11 标准对模板的功能进行了扩展，允许模板中包含任意数量的模板参数，这样的模板又称可变参数模板。</u>

**可变参数函数模板**

```c++
template<typename... T>
void vair_fun(T...args) {
    //函数体
}
```

模板参数中， `typename`（或者 class）后跟 `…` 就表明 T 是一个 <font color=red>可变模板参数</font> ，它可以接收多种数据类型，又称 <font color=red>模板参数包</font> 。vair_fun() 函数中，args 参数的类型用 `T…` 表示，表示 `args` 参数可以接收任意个参数，又称 <font color=red>函数参数包</font> 。即此函数模板最终实例化出的 vair_fun() 函数可以指定任意类型、任意数量的参数。

```c++
vair_fun();
vair_fun(1, "abc");
vair_fun(1, "abc", 1.23);
```

在模板函数内部“解”参数包方法：

<mark>方法一：递归方式解包</mark>

```C++
#include <iostream>
using namespace std;

//模板函数递归的出口
void vir_fun() {
}

template <typename T, typename... args>
void vir_fun(T argc, args... argv)
{
    cout << argc << endl;
    //开始递归，将第一个参数外的 argv 参数包重新传递给 vir_fun
    vir_fun(argv...);
}

int main()
{
    vir_fun(1, "http://www.biancheng.net", 2.34);
    return 0;
}
```

结果：

```shell
1
http://www.biancheng.net
2.34
```

程序的执行流程
  - 首先，`main()` 函数调用 `vir_fun()` 模板函数时，根据所传实参的值，可以很轻易地判断出模板参数 `T` 的类型为 in`T`，函数参数 `argc` 的值为 1，剩余的模板参数和函数参数都分别位于 args 和 `argv` 中；
  - `vir_fun()` 函数中，首先输出了 `argc` 的值（为 1），然后重复调用自身，同时将函数参数包 `argv` 中的数据作为实参传递给形参 `argc` 和 `argv`；
  - 再次执行 `vir_fun()` 函数，此时模板参数 `T` 的类型为 char*，输出 `argc` 的值为 `http:www.biancheng.net`。再次调用自身，继续将 `argv` 包中的数据作为实参；
  - 再次执行 `vir_fun()` 函数，此时模板参数 `T` 的类型为 `double`，输出 `argc` 的值为 2.34。再次调用自身，将空的 `argv` 包作为实参；
  - 由于 `argv` 包没有数据，此时会调用无任何形参、函数体为空的 `vir_fun()` 函数，最终执行结束。

**注意:**以递归方式解包，一定要设置递归结束的出口。例如本例中，无形参、函数体为空的 vir_fun() 函数就是递归结束的出口。

<mark>方法二：非递归方式解包（借助逗号表达式和初始化列表，也可以解开参数包）</mark>

```c++
#include <iostream>
using namespace std;

template <typename T>
void dispaly(T t) {
    cout << t << endl;
}

template <typename... args>
void vir_fun(args... argv){
    // 逗号表达式+初始化列表
    int arr[] = { (dispaly(argv),0)... };
}

int main() {
    vir_fun(1, "http://www.biancheng.net", 2.34);
    return 0;
}
```

以`{ }`初始化列表的方式对数组 `arr` 进行了初始化， `(display(argv),0)…` 会依次展开为 `(display(1),0)`、`(display(“http://www.biancheng.net”),0)` 和 `(display(2.34),0)`。

所以，下面的语句是等价的：

```c++
int arr[] = { (dispaly(argv),0)... };
int arr[] = { (dispaly(1),0), (dispaly("http://www.biancheng.net"),0), (dispaly(2.34),0) };
```
可以看到，每个元素都是一个逗号表达式，以 `(display(1), 0)` 为例，它会先计算 `display(1)`，然后将 `0` 作为整个表达式的值返回给数组，因此 `arr` 数组最终存储的都是 `0`。`arr` 数组纯粹是为了将参数包展开，没有发挥其它作用。

**可变参数类模板**

`C++11` 标准中，类模板中的模板参数也可以是一个可变参数。`C++11` 标准提供的 `tuple` 元组类就是一个典型的可变参数模板类。

```c++
template <typename... Types>
class tuple;
```

和固定模板参数的类不同，tuple 模板类实例化时，可以接收任意数量、任意类型的模板参数:

```c++
std::tuple<> tp0;
std::tuple<int> tp1 = std::make_tuple(1);
std::tuple<int, double> tp2 = std::make_tuple(1, 2.34);
std::tuple<int, double, string> tp3 = std::make_tuple(1, 2.34, "http://www.biancheng.net");
```

### 2.8 引入tuple和Lambda

tuple 的应用场景:
  - 当需要存储多个不同类型的元素时，可以使用 tuple；
  - 当函数需要返回多个数据时，可以将这些数据存储在 tuple 中，函数只需返回一个 tuple 对象即可。
    具体使用方式请参考《C++标准库》

**Lambda语法格式**

```c++
[外部变量访问方式说明符](参数)
mutable noexcept/throw()->返回值类型
{
	函数体;
};
```

- [外部变量访问方式说明符]：[]方括号用于向编译器表明当前是一个lambda表达式，其不能被省略。在方括号内部，可以注明当前 lambda 函数的函数体中可以使用哪些外部变量(外部变量，指的是和当前 lambda 表达式位于同一作用域内的所有局部变量)。
  - |     外部变量格式    |    功能     |
    |:-----------------:|:----------:|
    |        []         | 空方括号表示当前lambda匿名函数不导入任何外部变量|
    |        [=]        | 只有一个=符合，表示以值传递的方式导入所有外部变量|
    |        [&]        | 只有一个&符号，表示以引用传递的方式导入所有外部变量|
    | [val1, val2,...]  | 表示以值传递的方式导入val1、val2等外部变量，同时多个变量之间没有前后次序 |
    | [&val1, &val2,...]| 表示以引用传递的方式导入val1、val2等指定的外部变量，多个变量之间没有前后次序|
    |  [val, &val2,...] |以上两种方式还可以混合使用|
    |  [=, &val1,...]   |表示除val1以引用传递的方式导入以外，其他外部变量都以值传递的方式导入|
    |     [this]        |表示以值传递的方式导入当前的this指针|
- **(参数)**：和普通函数的定义一样，lambda 匿名函数也可以接收外部传递的多个参数。和普通函数不同的是，如果不需要传递参数，可以连同()小括号一起省略。
- **mutable**：此关键字可以省略，如果使用之前的 () 小括号将不能省略（参数个数可以为 0）。默认情况下，对于以值传递方式引入的外部变量，不允许在 lambda 表达式内部修改它们的值（可以理解为这部分变量都是 const 常量）。而如果想修改它们，就必须使用 mutable 关键字。（注意，对于以值传递方式引入的外部变量，lambda 表达式修改的是拷贝的那一份，并不会修改真正的外部变量）。
- **noexcept/throw()**：可以省略，如果使用，在之前的 () 小括号将不能省略（参数个数可以为 0）。默认情况下，lambda 函数的函数体中可以抛出任何类型的异常。而标注 noexcept 关键字，则表示函数体内不会抛出任何异常；使用throw()可以指定 lambda 函数内部可以抛出的异常类型。（注意，如果 lambda 函数标有 noexcept 而函数体内抛出了异常，又或者使用 throw() 限定了异常类型而函数体内抛出了非指定类型的异常，这些异常无法使用 try-catch 捕获，会导致程序执行失败）。
- **`->`返回值类型**：指明 lambda 匿名函数的返回值类型。值得一提的是，如果 lambda 函数体内只有一个 return 语句，或者该函数返回 void，则编译器可以自行推断出返回值类型，此情况下可以直接省略-> 返回值类型。
- **函数体**：和普通函数一样，lambda 匿名函数包含的内部代码都放置在函数体中。该函数体内除了可以使用指定传递进来的参数之外，还可以使用指定的外部变量以及全局范围内的所有全局变量。

<mark>注意:</mark>外部变量会受到以值传递还是以引用传递方式引入的影响，而全局变量则不会。换句话说，在 lambda 表达式内可以使用任意一个全局变量，必要时还可以直接修改它们的值。

### 2.9 列表初始化

具体使用方式请参考《C++ Primer Plus》《C++标准库》

### 2.10 非受限联合体

**POD (Plain Old Data) 类型介绍:**
`POD`类型一般具有以下几种特征：
  - 没有用户自定义的构造函数，析构函数、拷贝构造函数和移动构造函数
  - 不能包含虚函数和虚基类
  - 非静态成员必须声明为public
  - 类中的第一个非静态成员的类型与基类不同
  - 在类或者结构体继承时，满足以下两种情况之一：
  - 派生类中有非静态成员，且只有一个包含静态成员的基类
  - 基类有非静态成员，而派生类没有非静态成员
  - 所有非静态数据成员均和其基类也符合上述规则（递归定义），也就是说`POD`类型不能包含非`POD`类型的数据。
  - 所有建通C语言的数据类型都是`POD`类型(`struct`、`union`等不能违背上述规则)


**非受限联合体**

在 `C/C++` 中，**<font color=red>联合体</font>**是一种构造数据类型。在一个联合体内，可以定义多个不同类型的成员，这些成员将<u>会共享同一块内存空间</u>。老版本的 C++ 为了和C语言保持兼容，对联合体的数据成员的类型进行了很大程度的限制，这些限制在今天看来并没有必要，因此 C++11 取消了这些限制。

C++11 标准规定，任何非引用类型都可以成为联合体的数据成员，这种联合体也被称为**<font color=red>非受限联合体</font>**。

```c++
class Student{
public:
    Student(bool g, int a): gender(g), age(a) {}
private:
    bool gender;
    int age;
};

union T{
    Student s;  // 含有非POD类型的成员，gcc-5.1.0  版本报错
    char name[10];
};
```

上述的代码中，因为 Student 类带有自定义的构造函数，所以是一个非 POD 类型的，这导致编译器报错。

<table><tr><td bgcolor=yellow>C++ 11改进1:</td></tr></table>

  - C++11允许非POD类型
  - C++11允许联合体又静态成员(静态成员变量智能在联合体内定义，却不能在联合体外使用)

<table><tr><td bgcolor=yellow>非受限联合体的赋值注意事项：</td></tr></table>

  - C++11规定，如果非受限联合体内有一个非 POD 的成员，而该成员拥有自定义的构造函数，那么这个非受限联合体的默认构造函数将被编译器删除；其他的特殊成员函数，例如默认拷贝构造函数、拷贝赋值操作符以及析构函数等，也将被删除。
    ```c++
    #include <string>
    using namespace std;
    union U {
        string s;
        int n;
    };
    int main() {
      // 构造失败，因为 U 的构造函数被删除
        U u;
        return 0;
    }
    ```
    在上面的例子中，因为 `string` 类拥有自定义的构造函数，所以 `U` 的构造函数被删除；定义 `U` 的类型变量 `u` 需要调用默认构造函数，所以 `u` 也就无法定义成功。

  - 解决上面问题的一般需要用到 `placement new`:
    `placement new` 是 `new` 关键字的一种进阶用法，既可以在栈`（stack）`上生成对象，也可以在堆`（heap）`上生成对象。相对应地，把常见的` new `的用法称为 `operator new`，它只能在 `heap` 上生成对象。

    placement new 的语法格式:
    ```c++
    new(address) ClassConstruct(…)
    ```

    address 表示已有内存的地址，该内存可以在栈上，也可以在堆上;
    ClassConstruct(…) 表示调用类的构造函数，如果构造函数没有参数，也可以省略括号。

    placement new 利用已经申请好的内存来生成对象，它不再为对象分配新的内存，而是将对象数据放在 address 指定的内存中。

    ```c++
    #include <string>
    using namespace std;
    union U {
        string s;
        int n;
    public:
        U() { new(&s) string; }
        ~U() { s.~string(); }
    };

    int main() {
        U u;
        return 0;
    }
    ```
    构造时，采用 `placement new` 将 `s` 构造在其地址 `&s` 上，这里 `placement new` 的唯一作用只是调用了一下 `string` 类的构造函数。注意，在析构时还需要调用 `string` 类的析构函数。

### 2.11 非受限联合体的匿名声明和“枚举式类”

<mark>匿名联合体</mark>是指不具名的联合体（也即没有名字的联合体），定义如下:

```c++
union U{
	// 此联合体为匿名联合体
	union { int x; };
};
```

联合体 `U` 内定义了一个不具名的联合体，该联合体包含一个 `int` 类型的成员变量，称这个联合体为匿名联合体。

非受限联合体也可以匿名，而当非受限的匿名联合体运用于类的声明时，这样的类被称为`枚举式类`。

```c++
#include <cstring>
using namespace std;
class Student{
public:
    Student(bool g, int a): gender(g), age(a){}
    bool gender;
    int age;
};

class Singer {
public:
    enum Type { STUDENT, NATIVE, FOREIGENR };
    Singer(bool g, int a) : s(g, a) { t = STUDENT; }
    Singer(int i) : id(i) { t = NATIVE; }
    Singer(const char* n, int s) {
        int size = (s > 9) ? 9 : s;
        memcpy(name , n, size);
        name[s] = '\0';
        t = FOREIGENR;
    }
    ~Singer(){}
private:
    Type t;
    union {
        Student s;
        int id;
        char name[10];
    };
};

int main() {
    Singer(true, 13);
    Singer(310217);
    Singer("J Michael", 9);
    return 0;
}
```

### 2.12 for循环(基于范围的循环)

`C++ 11`标准之前（`C++ 98/03` 标准），如果要用 `for` 循环语句遍历一个数组或者容器，只能套用如下结构:

```c++
for(表达式 1; 表达式 2; 表达式 3){
	//循环体
}
```
举例:
```c++
#include <iostream>
#include <vector>
#include <string.h>
using namespace std;
int main() {
    char arc[] = "http://c.biancheng.net/cplus/11/";
    int i;
    // for循环遍历普通数组
    for (i = 0; i < strlen(arc); i++) {
        cout << arc[i];
    }
    cout << endl;

    vector<char>myvector(arc,arc+23);
    vector<char>::iterator iter;
    // for循环遍历 vector 容器
    for (iter = myvector.begin(); iter != myvector.end(); ++iter) {
        cout << *iter;
    }
    return 0;
}
```

`C++ 11` 标准中，除了可以沿用前面介绍的用法外，还为 for 循环添加了一种全新的语法格式:

```c++
for (declaration : expression){
	//循环体
}
```

<mark>declaration:</mark>表示此处要定义一个变量，该变量的类型为要遍历序列中存储元素的类型。需要注意的是，C++ 11 标准中，declaration参数处定义的变量类型可以用 auto 关键字表示，该关键字可以使编译器自行推导该变量的数据类型。

<mark>expression:</mark>表示要遍历的序列，常见的可以为事先定义好的普通数组或者容器，还可以是用 {} 大括号初始化的序列。

```c++
#include <iostream>
#include <vector>
using namespace std;
int main() {
    char arc[] = "http://c.biancheng.net/cplus/11/";
    // for循环遍历普通数组
    for (char ch : arc) {
        cout << ch;
    }
    cout << '!' << endl;

    vector<char>myvector(arc, arc + 23);
    // for循环遍历 vector 容器
    for (auto ch : myvector) {
        cout << ch;
    }
    cout << '!';

	// 新语法格式的 for 循环还支持遍历用{ }大括号初始化的列表
	for (int num : {1, 2, 3, 4, 5}) {
        cout << num << " ";
    }
    return 0;
}
```

<table><tr><td bgcolor=yellow>注意:</td></tr></table>

  - 程序中在遍历 `myvector` 容器时，定义了 `auto` 类型的 `ch` 变量，当编译器编译程序时，会通过 `myvector` 容器中存储的元素类型自动推导出 `ch` 为 `char` 类型。注意，这里的 `ch` 不是迭代器类型，而表示的是 `myvector` 容器中存储的每个元素。
  - 在输出结果，其中第一行输出的字符串和 `!` 之间还输出有一个空格，因为新格式的 for 循环在遍历字符串序列时，不只是遍历到最后一个字符，还会遍历位于该字符串末尾的 \0（字符串的结束标志）。
<table><tr><td bgcolor=yellow>注意:</td></tr></table>

  - 在使用新语法格式的 `for` 循环遍历某个序列时，如果需要遍历的同时修改序列中元素的值，实现方案是在 `declaration` 参数处定义引用形式的变量。
  -
    ```c++
    #include <iostream>
    #include <vector>
    using namespace std;
    int main() {
        char arc[] = "abcde";
        vector<char>myvector(arc, arc + 5);
        // for循环遍历并修改容器中各个字符的值
        for (auto &ch : myvector) {
            ch++;
        }
        // for循环遍历输出容器中各个字符
        for (auto ch : myvector) {
            cout << ch;
        }
        return 0;
    }
    ```

### 2.13 constexpr：验证是否为常量表达式

`constexpr` 是 C++ 11 标准新引入的关键字。

常量表达式，指的就是由多个（≥1）常量组成的表达式。即如果表达式中的成员都是常量，那么该表达式就是一个常量表达式。这也意味着，常量表达式一旦确定，其值将无法修改。

以定义数组为例，数组的长度就必须是一个常量表达式：

```c++
//正确
int url[10];
// 正确
int url[6 + 4];
// 错误，length是变量
int length = 6; // 改进： const int length = 6;
int url[length]
```

程序的执行过程为预处理、编译、汇编和链接四个阶段，具体请参考[计算机系统漫游(一)](https://mp.weixin.qq.com/s?__biz=MzkxMzI5Mjk4Mg==&mid=2247483661&idx=1&sn=0071d0fb2ab4123f54885101382b522a&scene=21#wechat_redirect)。大致又可以说由编译、链接、运行这3 个阶段。常量表达式和非常量表达式的计算时机不同，非常量表达式只能在程序运行阶段计算出结果；而常量表达式的计算往往发生在程序的编译阶段，这可以极大提高程序的执行效率，因为表达式只需要在编译阶段计算一次，节省了每次程序运行时都需要计算一次的时间。

在实际开发中，判定一个表达式是否为常量表达式方式:

  - 人为判定；
  - C++11 标准还提供有 constexpr 关键字。

**constexpr修饰普通变量**

`C++11` 标准中，定义变量时可以用 `constexpr` 修饰，从而使该变量获得在编译阶段即可计算出结果的能力。

使用 `constexpr` 修改普通变量时，变量必须经过初始化且初始值必须是一个常量表达式。

```c++
#include <iostream>
using namespace std;

int main() {
    constexpr int num = 1 + 2 + 3;
    int url[num] = {1,2,3,4,5,6};
    couts<< url[1] << endl;
    return 0;
}
```

上述代码中，如果尝试将 constexpr 删除，此时编译器会提示url[num] 定义中 num 不可用作常量。使用 constexpr 修饰 num 变量，同时将 1+2+3 这个常量表达式赋值给 num。由此，编译器就可以在编译时期对 num 这个表达式进行计算，因为 num 可以作为定义数组时的长度。

<mark>注意:</mark>当常量表达式中包含浮点数时，考虑到程序编译和运行所在的系统环境可能不同，常量表达式在编译阶段和运行阶段计算出的结果精度很可能会受到影响，因此 C++11 标准规定，浮点常量表达式在编译阶段计算的精度要至少等于（或者高于）运行阶段计算出的精度。


**constexpr修饰函数**

这样的函数又称为`常量表达式函数`。

`constexpr` 并非可以修改任意函数的返回值。必须满足如下条件:

  - 整个函数的函数体中，除了可以包含 `using` 指令、`typedef` 语句以及`static_assert`断言外，只能包含一条 `return` 返回语句。
    ```c++
    constexpr int display(int x) {
        // 可以添加 using 执行、typedef 语句以及 static_assert 断言
        return 1 + 2 + x;
    }
    ```
  - 该函数必须有返回值，即函数的返回值类型不能是 void
  - 函数在使用之前，必须有对应的定义语句。函数的使用分为“声明”和“定义”两部分，普通的函数调用只需要提前写好该函数的声明部分即可（函数的定义部分可以放在调用位置之后甚至其它文件中），但常量表达式函数在使用前，必须要有该函数的定义。
  - return 返回的表达式必须是常量表达式

<mark>注意:</mark>在常量表达式函数的 return 语句中，不能包含赋值的操作（例如 return x=1 在常量表达式函数中不允许的）。另外，用 constexpr 修改函数时，函数本身也是支持递归的。

**constexpr修饰类的构造函数**

对于 C++ 内置类型的数据，可以直接用 constexpr 修饰，但如果是自定义的数据类型（用 struct 或者 class 实现），直接用 constexpr 修饰是不行的。

自定义一个可产生常量的类型时，正确的做法是在该类型的内部添加一个常量构造函数:

```c++
#include <iostream>
using namespace std;
// 自定义类型的定义
struct MyType {
    constexpr MyType(char *name,int age):name(name),age(age){};
    const char* name;
    int age;
    //其它结构体成员
};

int main() {
    constexpr struct MyType mt { "zhangsan", 10 };
    cout << mt.name << " " << mt.age << endl;
    return 0;
}
```

<mark>注意:</mark> constexpr 修饰类的构造函数时，要求该构造函数的函数体必须为空，且采用初始化列表的方式为各个成员赋值时，必须使用常量表达式。

constexpr 可用于修饰函数，而类中的成员方法完全可以看做是位于类这个命名空间中的函数，所以 constexpr 也可以修饰类中的成员函数，只不过此函数必须满足前面提到条件。

<mark>注意:</mark> C++11 标准中，不支持用 constexpr 修饰带有 virtual 的成员方法。

**constexpr修饰模板函数**

C++11 语法中，`constexpr` 可以修饰模板函数，但由于模板中类型的不确定性，因此模板函数实例化后的函数是否符合常量表达式函数的要求也是不确定的。

针对这种情况下，C++11 标准规定，如果 `constexpr` 修饰的模板函数实例化结果不满足常量表达式函数的要求，则 `constexpr` 会被自动忽略，即该函数就等同于一个普通函数。

**constexpr与const的区别**

C++ 11标准中，为了解决 const 关键字的双重语义问题，保留了 const 表示“只读”的语义，而将“常量”的语义划分给了新添加的 constexpr 关键字。因此 C++11 标准中，建议将 const 和 constexpr 的功能区分开，即凡是表达只读语义的场景都使用 const，表达常量语义的场景都使用 constexpr。

`只读`和`不允许被修改`之间并没有必然的联系

```c++
#include <iostream>
using namespace std;

int main() {
    int a = 10;
    const int & con_b = a;
    cout << con_b << endl;
    a = 20;
    cout << con_b << endl;
}
```

程序中用 const 修饰了 con_b 变量，表示该变量只读，即无法通过变量自身去修改自己的值。但这并不意味着 con_b 的值不能借助其它变量间接改变，通过改变 a 的值就可以使 con_b 的值发生变化。

<table><tr><td bgcolor=yellow>在大部分实际场景中，const 和 constexpr 是可以混用的:</td></tr></table>

```c++
const int a = 5 + 4;
constexpr int a = 5 + 4;
```

在某些场景中，必须明确使用 constexpr

```c++
#include <iostream>
#include <array>
using namespace std;
constexpr int sqr1(int arg) {
    return arg * arg;
}

const int sqr2(int arg) {
    return arg * arg;
}

int main() {
	// 可以，因为sqr1时constexpr函数
    array<int,sqr1(10)> mylist1;
	// 不可以，因为sqr2不是constexpr函数
    array<int,sqr2(10)> mylist1;
    return 0;
}
```

### 2.14 long long超长整形

将 long long 整形写入 C++ 11 标准中，如同 long 类型整数需明确标注 L 或者 l 后缀一样，要使用 long long 类型的整数，也必须标注对应的后缀：
  - 对于有符号 long long 整形，后缀用 LL 或者 ll 标识。如，10LL 就表示有符号超长整数 10。
  - 对于无符号 long long 整形，后缀用 ULL、ull、Ull 或者 uLL 标识。如，10ULL 就表示无符号超长整数 10；

<mark>注意:</mark> 如果不添加任何标识，则所有的整数都会默认为 int 类型。

了解当前平台上 long long 整形的取值范围，可以使用<climits>头文件中与 long long 整形相关的 3 个宏，分别为 LLONG_MIN、LLONG_MAX 和 ULLONG_MIN：
   - LLONG_MIN：代表当前平台上最小的 long long 类型整数；
   - LLONG_MAX：代表当前平台上最大的 long long 类型整数；
   - ULLONG_MIN：代表当前平台上最大的 unsigned long long 类型整数（无符号超长整型的最小值为 0）；

```c++
#include <iostream>
#include <iomanip>
#include <climits>
using namespace std;
int main() {
    cout <<"long long最大值：" << LLONG_MIN <<" "<< hex << LLONG_MIN <<"\n";
    cout << dec <<"long long最小值：" << LLONG_MAX << " " << hex << LLONG_MAX << "\n";
    cout << dec << "unsigned long long最大值：" << ULLONG_MAX << " " << hex << ULLONG_MAX;
    return 0;
}
```

### 2.15 右值引用

<table><tr><td bgcolor=yellow>C++左值和右值</td></tr></table>
在 C++/C 语言中，一个表达式（可以是字面量、变量、对象、函数的返回值等）根据其使用场景不同，分为左值表达式和右值表达式。确切的说 C++ 中左值和右值的概念是从 C 语言继承过来的。

<mark>注意：</mark>左值的英文简写为lvalue，右值的英文简写为rvalue。很多人认为它们分别是left value、right value 的缩写。其实不然，lvalue 是loactor value的缩写，可意为存储在内存中、有明确存储地址（可寻址）的数据，而 rvalue 译为 read value，指的是那些可以提供数据值的数据（不一定可以寻址，例如存储于寄存器中的数据）。

通常情况下，判断某个表达式是`左值`还是`右值`，最常用的有以下 2 种方法:
  - 可位于赋值号(=)左侧的表达式就是左值；反之，只能位于赋值号右侧的表达式就是右值。
  -
  ```c++
  int a = 5;
  // 错误，5 不能为左值
  5 = a;

  // b 是一个左值
  int b = 10;
  // a、b 都是左值，只不过将 b 可以当做右值使用
  a = b;
  ```
  - 有名称的、可以获取到存储地址的表达式即为`左值`；反之则是`右值`。

以上面定义的变量 `a`、`b` 为例，`a` 和 `b` 是变量名，且通过 &`a` 和 &`b` 可以获得他们的存储地址，因此 `a` 和 `b` 都是左值；反之，字面量 5、10，它们既没有名称，也无法获取其存储地址（字面量通常存储在寄存器中，或者和代码存储在一起），因此 5、10 都是右值。

**右值引用**

`右值引用`可以从字面意思上理解，指的是以引用传递（而非值传递）的方式使用 C++ 右值。

`C++98/03` 标准中有引用，使用 `&` 表示。但此种引用方式有一个缺陷，即正常情况下只能操作 C++ 中的左值，无法对右值添加引用。
```c++
int num = 10;
// 正确
int &b = num;
// 错误
int &c = 10;
```

<mark>注意:</mark> 虽然 C++98/03 标准不支持为右值建立非常量左值引用，但允许使用常量左值引用操作右值。也就是说，常量左值引用既可以操作左值，也可以操作右值。

```c++
int num = 10;
const int &b = num;
const int &c = 10;
```

<mark>注意:</mark> `C++11` 标准中对右值做了更细致的划分，分别称为 <font color=red>纯右值（pure value，简称 pvalue）</font>和 <font color=red>将亡值（expiring value，简称 xvalue ）</font>。其中纯右值就是 C++98/03 标准中的右值，而<u>将亡值则指的是和右值引用相关的表达式（比如某函数返回的 T && 类型的表达式）</u>。对于纯右值和将亡值，都属于右值。

### 2.16 移动构造函数的功能和用法

`右值引用`主要用于实现`移动（move）语义`和`完美转发`。

**完美转发及其实现**

C++11 标准为 C++ 引入右值引用语法的同时，还解决了一个 C++ 98/03 标准长期存在的短板，即使用简单的方式即可在函数模板中实现参数的完美转发。

1. **<font color=red>完美转发</font>**
  指的是函数模板可以将自己的参数"完美"地转发给内部调用的其它函数。所谓完美，即不仅能准确地转发参数的值，还能保证被转发参数的左、右值属性不变。如：
    ```c++
    template<typename T>
    void function(T t) {
        otherdef(t);
    }
    ```
    上述示例中，function() 函数模板中调用了 otherdef() 函数。在此基础上，完美转发指的是：如果 function() 函数接收到的参数 t 为左值，那么该函数传递给 otherdef() 的参数 t 也是左值；反之如果 function() 函数接收到的参数 t 为右值，那么传递给 otherdef() 函数的参数 t 也必须为右值。
    function() 函数模板并没有实现完美转发。
    - 参数 t 为非引用类型，这意味着在调用 function() 函数时，实参将值传递给形参的过程就需要额外进行一次拷贝操作；
    - 无论调用 function() 函数模板时传递给参数 t 的是左值还是右值，对于函数内部的参数 t 来说，它有自己的名称，也可以获取它的存储地址，因此它永远都是左值，也就是说，传递给 otherdef() 函数的参数 t 永远都是左值。总之，无论从那个角度看，function() 函数的定义都不“完美”。

    如果使用 C++ 98/03 标准下的 C++ 语言，可以采用函数模板重载的方式实现完美转发，例如：
    ```c++
    #include <iostream>
    using namespace std;

    // 重载被调用函数，查看完美转发的效果
    void otherdef(int & t) {
        cout << "lvalue\n";
    }

    void otherdef(const int & t) {
        cout << "rvalue\n";
    }

    // 重载函数模板，分别接收左值和右值
    // 接收右值参数
    template <typename T>
    void function(const T& t) {
        otherdef(t);
    }

    // 接收左值参数
    template <typename T>
    void function(T& t) {
        otherdef(t);
    }
    int main()
    {
      // 5 是右值
        function(5);
        int  x = 1;
      // x 是左值
        function(x);
        return 0;
    }
    ```
    对于右值5 来说，它实际调用的参数类型为 const T& 的函数模板，由于 t 为 const 类型，所以 otherdef() 函数实际调用的也是参数用 const 修饰的函数，所以输出“rvalue”；对于左值 x 来说，2 个重载模板函数都适用，C++编译器会选择最适合的参数类型为 T& 的函数模板，进而 therdef() 函数实际调用的是参数类型为非 const 的函数，输出“lvalue”。
    使用重载的模板函数实现完美转发也是有弊端的，此方式仅适用于模板函数仅有少量参数的情况，否则就需要编写大量的重载函数模板，造成代码的冗余。为了更快速地实现完美转发，C++ 11 标准中允许在函数模板中使用右值引用来实现完美转发。

    C++11 标准中规定，通常情况下右值引用形式的参数只能接收右值，不能接收左值。但对于函数模板中使用右值引用语法定义的参数来说，不再遵守这一规定，既可以接收右值，也可以接收左值（此时的右值引用又被称为“万能引用”）。
    在 C++11 标准中实现完美转发，只需要编写如下一个模板函数即可：
    ```c++
    template <typename T>
    void function(T&& t) {
        otherdef(t);
    }
    ```
    此模板函数的参数 t 既可以接收左值，也可以接收右值。但仅仅使用右值引用作为函数模板的参数是远远不够的，还有一个问题继续解决，如果调用 function() 函数时为其传递一个左值引用或者右值引用的实参，如下所示：
    ```c++
    int n = 10;
    int & num = n;
    // T 为 int&
    function(num);
    int && num2 = 11;
    // T 为 int &&
    function(num2);
    ```
    由 function(num) 实例化的函数底层就变成了 function(int && t)，同样由 function(num2) 实例化的函数底层则变成了 function(int && && t)。C++98/03 标准是不支持这种用法的，而 C++ 11标准为了更好地实现完美转发，特意为其指定了新的类型匹配规则，又称为引用折叠规则（假设用 A 表示实际传递参数的类型）：
      - 当实参为左值或者左值引用（A&）时，函数模板中 T&& 将转变为 A&（A& && = A&）；

      - 当实参为右值或者右值引用（A&&）时，函数模板中 T&& 将转变为 A&&（A&& && = A&&）。

    <mark>注意：</mark>在实现完美转发时，只要函数模板的参数类型为 T&&，则 C++ 可以自行准确地判定出实际传入的实参是左值还是右值。

    通过将函数模板的形参类型设置为 T&&，可以很好地解决接收左、右值的问题。但除此之外，还需要解决一个问题，即无论传入的形参是左值还是右值，对于函数模板内部来说，形参既有名称又能寻址，因此它都是左值。

    将函数模板接收到的形参连同其左、右值属性，一起传递给被调用的函数方法：`引入一个模板函数 forword()`:
    ```c++
    #include <iostream>
    using namespace std;

    // 重载被调用函数，查看完美转发的效果
    void otherdef(int & t) {
        cout << "lvalue\n";
    }

    void otherdef(const int & t) {
        cout << "rvalue\n";
    }

    // 实现完美转发的函数模板
    template <typename T>
    void function(T&& t) {
        otherdef(forward<T>(t));
    }

    int main()
    {
        function(5);
        int  x = 1;
        function(x);
        return 0;
    }
    ```
2. **<font>移动语义</font>**
  指的就是以移动而非深拷贝的方式初始化含有指针成员的类对象。简单的理解，移动语义指的就是将其他对象（通常是临时对象）拥有的内存资源“移为已用”。
    ```c++
    #include <iostream>
      using namespace std;

      class demo{
      public:
          demo():num(new int(0)){
              cout<<"construct!"<<endl;
          }

          demo(const demo &d):num(new int(*d.num)){
              cout<<"copy construct!"<<endl;
          }

          // 移动构造函数
          demo(demo &&d):num(d.num){
              d.num = NULL;
              cout<<"move construct!"<<endl;
          }

          ~demo(){
              cout<<"class destruct!"<<endl;
          }
      private:
          int *num;
      };

      demo get_demo(){
          return demo();
      }

      int main(){
          demo a = get_demo();
          return 0;
      }
    ```
  demo类的其中一个构造函数(`demo(demo &&d)`)使用右值引用形式的参数，又称为移动构造函数。并且在此构造函数中，`num` 指针变量采用的是浅拷贝的复制方式，同时在函数内部重置了 `d.num`，有效避免了“同一块对空间被释放多次”情况的发生。

  `非 const 右值引用`只能操作右值，程序执行结果中产生的`临时对象`（例如函数返回值、lambda 表达式等）既`无名称`也无法获取其`存储地址`，所以属于`右值`。当类中同时包含`拷贝构造函数`和`移动构造函数`时，<mark>如果使用临时对象初始化当前类的对象，编译器会优先调用移动构造函数来完成此操作。只有当类中没有合适的移动构造函数时，编译器才会退而求其次，调用拷贝构造函数。</mark>

  <mark>注意:</mark>在实际开发中，通常在类中自定义移动构造函数的同时，会再为其自定义一个适当的拷贝构造函数，由此当用户利用右值初始化类对象时，会调用移动构造函数；使用左值（非右值）初始化类对象时，会调用拷贝构造函数。

  默认情况下，`左值`初始化同类对象只能通过拷贝构造函数完成，如果想调用移动构造函数，则`必须使用右值进行初始化`。C++11 标准中为了满足用户使用左值初始化同类对象时也通过移动构造函数完成的需求，新引入了 `std::move()` 函数，<font color=red>它可以将左值强制转换成对应的右值，由此便可以使用移动构造函数</font>。


### 2.17 `move()`函数:将左值转换为右值

C++11 标准中借助右值引用可以为指定类添加移动构造函数，这样当使用该类的右值对象（可以理解为临时对象）初始化同类对象时，编译器会优先选择移动构造函数。

移动构造函数的调用时机是：`用同类的右值对象初始化新对象`。用当前类的左值对象（有名称，能获取其存储地址的实例对象）初始化同类对象时，调用移动构造函数方法：`调用 move() 函数`。

move 的功能很简单，就是将某个左值强制转化为右值。

**引用限定符的用法**

首先，我们定义左值的类对象称为左值对象，右值的类对象称为右值对象。

默认情况下，对于类中用 public 修饰的成员函数，既可以被左值对象调用，也可以被右值对象调用:

```c++
#include <iostream>
using namespace std;
class demo {
public:
    demo(int num):num(num){}
    int get_num(){
        return this->num;
    }
private:
    int num;
};

int main() {
    demo a(10);
    cout << a.get_num() << endl;
    cout << move(a).get_num() << endl;
    return 0;
}
```

可以看到，`demo` 类中的 `get_num()` 成员函数既可以被 `a 左值对象`调用，也可以被 `move(a)` 生成的右值 `demo` 对象调用，运行程序会输出两个 `10`。

某些场景中，可能需要限制调用成员函数的对象的类型（`左值`还是`右值`），为此 C++11 新添加了`引用限定符`。所谓引用限定符，就是在成员函数的后面添加 & 或者 &&，从而限制调用者的类型（`左值`还是`右值`）。

```c++
#include <iostream>
using namespace std;
class demo {
public:
    demo(int num):num(num){}
    int get_num()&{
        return this->num;
    }
private:
    int num;
};

int main() {
    demo a(10);
	// 正确
    cout << a.get_num() << endl;
	// 错误
    cout << move(a).get_num() << endl;
    return 0;
}
```

```c++
#include <iostream>
using namespace std;
class demo {
public:
    demo(int num):num(num){}
    int get_num()&&{
        return this->num;
    }
private:
    int num;
};

int main() {
    demo a(10);
	// 错误
    cout << a.get_num() << endl;
	// 正确
    cout << move(a).get_num() << endl;
    return 0;
}
```
<mark>注意:</mark>引用限定符不适用于静态成员函数和友元函数。

**const和引用限定**

const 也可以用于修饰类的成员函数，习惯称为`常成员函数`。

```c++
class demo{
public:
    int get_num() const;
}
```

`const` 和`引用限定符`修饰类的成员函数时，都位于函数的末尾。

<mark>注意:</mark> `C++11` 标准规定，当`引用限定符`和 `const` 修饰同一个类的成员函数时，const 必须位于引用限定符前面。

当 `const &&` 修饰类的成员函数时，调用它的对象只能是`右值对象`；当 `const &` 修饰类的成员函数时，调用它的对象既可以是`左值对象`，也可以是`右值对象`。无论是 `const &&` 还是 `const &` 限定的成员函数，内部都不允许对当前对象做修改操作。

```c++
#include <iostream>
using namespace std;
class demo {
public:
    demo(int num,int num2) :num(num),num2(num2) {}
    //左值和右值对象都可以调用
    int get_num() const &{
        return this->num;
    }
    //仅供右值对象调用
    int get_num2() const && {
        return this->num2;
    }
private:
    int num;
    int num2;
};
int main() {
    demo a(10,20);
	// 正确
    cout << a.get_num() << endl;
	// 正确
    cout << move(a).get_num() << endl;

    // 错误
    cout << a.get_num2() << endl;
	// 正确
    cout << move(a).get_num2() << endl;
    return 0;
}
```

### 2.17 nullptr：初始化空指针


实际开发中，避免产生`野指针`最有效的方法，就是在<u>定义指针的同时完成初始化操作，即便该指针的指向尚未明确，也要将其初始化为空指针</u>。

`野指针`，又称`悬挂指针`，指的是没有明确指向的指针。野指针往往指向的是那些不可用的内存区域，这就意味着像操作普通指针那样使用野指针（例如 `&p`），极可能导致程序发生异常。

C++98/03 标准中，将一个指针初始化为空指针的方式：

```c++
int *p = 0;
// 推荐使用
int *p = NULL;
```

可以看到，可以将指针明确指向 `0（0x0000 0000）`这个内存空间。一方面，明确指针的指向可以避免其成为野指针；另一方面，大多数操作系统都不允许用户对地址为 0 的内存空间执行写操作，若用户在程序中尝试修改其内容，则程序运行会直接报错。

相比第一种方式，推荐将指针初始化为 NULL。NULL 并不是 C++ 的关键字，它是 C++ 事先定义好的一个**宏**，并且它的值往往就是字面量 `0（#define NULL 0）`。

`C++` 中将 `NULL` 定义为字面常量 `0`，虽然能满足大部分场景的需要，但个别情况下，它会导致程序的运行和预期不符。

```c++
#include <iostream>
using namespace std;
void isnull(void *c){
    cout << "void*c" << endl;
}

void isnull(int n){
    cout << "int n" << endl;
}

int main() {
    isnull(0);
    isnull(NULL);
    return 0;
}
```
以上代码都将输出`int n`。对于 `isnull(0)` 来说，显然它真正调用的是参数为整形的 `isnull()` 函数；而对于 `isnull(NULL)`，我们期望它实际调用的是参数为 `void*c` 的 `isnull()` 函数，但程序的执行结果并不符合预期。

`C++ 98/03` 标准中，如果想令 `isnull(NULL)` 实际调用的是 `isnull(void* c)`，就需要对 `NULL`（或者 0）进行强制类型转换：

```c++
isnull( (void*)NULL );
isnull( (void*)0 );
```

由于 `C++ 98` 标准使用期间，`NULL` 已经得到了广泛的应用，出于兼容性的考虑，`C++11` 标准并没有对 `NULL` 的宏定义做任何修改。为了修正 C++ 存在的这一 BUG，在 `C++11` 标准中引入一个新关键字，即 `nullptr`。

`nullptr` 是 `nullptr_t` `类型的右值常量，专用于初始化空类型指针。nullptr_t` 是 `C++11` 新增加的数据类型，可称为<font color=red>指针空值类型</font>。也就是说，nullpter 仅是该类型的一个实例对象（已经定义好，可以直接使用），如果需要完全定义出多个同 nullptr 完全一样的实例对象。<font color=red>nullptr 可以被隐式转换成任意的指针类型</font>。

通过将指针初始化为 nullptr，可以很好地解决 NULL 遗留的问题:

```c++
#include <iostream>
using namespace std;
void isnull(void *c){
    cout << "void*c" << endl;
}

void isnull(int n){
    cout << "int n" << endl;
}

int main() {
    isnull(NULL);
    isnull(nullptr);
    return 0;
}
```

### 2.18 智能指针

`智能指针`，可以从字面上理解为“智能”的指针。具体来讲，`智能指针`和`普通指针`的用法是相似的，不同之处在于，`智能指针`<font color=red>可以在适当时机自动释放分配的内存</font>。也就是说，<u>使用智能指针可以很好地避免“忘记释放内存而导致内存泄漏”问题出现</u>。

`C++ 智能指针`底层是采用**引用计数**的方式实现的。简单的理解，智能指针在申请堆内存空间的同时，会为其配备一个整形值（初始值为 1），每当有新对象使用此堆内存时，该整形值 +1；反之，每当使用此堆内存的对象被释放时，该整形值减 1。当堆空间对应的整形值为 0 时，即表明不再有对象使用它，该堆空间就会被释放掉。

关于智能指针的具体使用方法，请参考《C++标准库》

<table><tr><td bgcolor=yellow>shared_ptr</td></tr></table>

实际上，每种智能指针都是以类模板的方式实现的，shared_ptr 也不例外。shared_ptr（其中 T 表示指针指向的具体数据类型）的定义位于<memory>头文件，并位于 std 命名空间中。

<font color=red>和 unique_ptr、weak_ptr 不同之处在于，多个 shared_ptr 智能指针可以共同使用同一块堆内存</font>。并且，由于该类型智能指针在实现上采用的是引用计数机制，即便有一个 shared_ptr 指针放弃了堆内存的使用权（引用计数减 1），也不会影响其他指向同一堆内存的 shared_ptr 指针（只有引用计数为 0 时，堆内存才会被自动释放）。

<table><tr><td bgcolor=yellow>unique_ptr</td></tr></table>

unique_ptr 指针自然也具备“在适当时机自动释放堆内存空间”的能力。和 shared_ptr 指针最大的不同之处在于，unique_ptr 指针指向的堆内存无法同其它 unique_ptr 共享，也就是说，每个 unique_ptr 指针都独自拥有对其所指堆内存空间的所有权。

注意：每个 unique_ptr 指针指向的堆内存空间的引用计数，都只能为 1，一旦该 unique_ptr 指针放弃对所指堆内存空间的所有权，则该空间会被立即释放回收。

unique_ptr 智能指针是以模板类的形式提供的，unique_ptr（T 为指针所指数据的类型）定义在<memory>头文件，并位于 std 命名空间中。

<table><tr><td bgcolor=yellow>weak_ptr</td></tr></table>

和 shared_ptr、unique_ptr 类型指针一样，weak_ptr 智能指针也是以模板类的方式实现的。weak_ptr（ T 为指针所指数据的类型）定义在<memory>头文件，并位于 std 命名空间中。

C++11标准虽然将 weak_ptr 定位为智能指针的一种，但该类型指针通常不单独使用（没有实际用处），只能和 shared_ptr 类型指针搭配使用。甚至于，我们可以将 weak_ptr 类型指针视为 shared_ptr 指针的一种辅助工具，借助 weak_ptr 类型指针， 我们可以获取 shared_ptr 指针的一些状态信息，比如有多少指向相同的 shared_ptr 指针、shared_ptr 指针指向的堆内存是否已经被释放等等。

当 weak_ptr 类型指针的指向和某一 shared_ptr 指针相同时，weak_ptr 指针并不会使所指堆内存的引用计数加 1；同样，当 weak_ptr 指针被释放时，之前所指堆内存的引用计数也不会因此而减 1。也就是说，weak_ptr 类型指针并不会影响所指堆内存空间的引用计数。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_7/  

