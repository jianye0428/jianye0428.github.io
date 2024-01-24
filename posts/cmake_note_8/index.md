# CMake 笔记 | [8] 设置语言标准 (二)


## 三、C++ 14新特性

### 3.1 函数返回值类型推导

`c++14`对函数返回值类型推导规则做了优化：**用`auto`推导函数的返回值**

```c++
#include <iostream>
using namespace std;
auto func(int i) {
    return i;
}

int main() {
    cout << func(4) << endl;
    return 0;
}
```

上面的代码使用C++11是不能通过编译的，通过编译器输出的信息得知这个特性需要到C++14才被支持。

**返回值类型推导也可以用在模板中**

```c++
#include <iostream>
using namespace std;

template<typename T> auto func(T t) {
	return t;
}

int main() {
    cout << func(4) << endl;
    cout << func(3.4) << endl;
    return 0;
}
```

<mark>注意:</mark>
  - 函数内如果有多个return语句，它们必须返回相同的类型，否则编译失败
  - 如果return语句返回初始化列表，返回值类型推导也会失败
  - 如果函数是虚函数，不能使用返回值类型推导
  - 返回类型推导可以用在前向声明中，但是在使用它们之前，翻译单元中必须能够得到函数定义
  - 返回类型推导可以用在递归函数中，但是递归调用必须以至少一个返回语句作为先导，以便编译器推导出返回类型

**lambda参数auto:**

在C++11中，lambda表达式参数需要使用具体的类型声明：

```c++
auto f = [] (int a) { return a; }
```

在C++14中，对此进行优化，lambda表达式参数可以直接是auto：

```c++
auto f = [] (auto a) { return a; };
cout << f(1) << endl;
cout << f(2.3f) << endl;
```

### 3.2 变量模板

对变量的类型使用模板:
```c++
template<class T>
constexpr T pi = T(3.1415926535897932385L);

int main() {
	// 3
    cout << pi<int> << endl;
    // 3.14159
    cout << pi<double> << endl;
    return 0;
}
```

### 3.3 别名模板

对**别名**使用模板，并且仍然保留模板特性:

```c++
template<typename T, typename U>
struct A {
    T t;
    U u;
};

template<typename T>
using B = A<T, int>;

int main() {
    B<double> b;
    b.t = 10;
    b.u = 20;
    cout << b.t << endl;
    cout << b.u << endl;
    return 0;
}
```

### 3.4 constexptr的限制

`C++14`相较于`C++11`对`constexpr`减少了一些限制:
  - C++11中constexpr函数可以使用递归，在C++14中可以使用局部变量和循环

```c++
// C++14 和 C++11均可
constexpr int factorial(int n) {
    return n <= 1 ? 1 : (n * factorial(n - 1));
}

// C++11中不可，C++14中可以
constexpr int factorial(int n) {
    int ret = 0;
    for (int i = 0; i < n; ++i) {
        ret += i;
    }
    return ret;
}
```
  - C++11中constexpr函数必须把所有东西都放在一个单独的return语句中，而constexpr则无此限制

```c++
// C++14 和 C++11均可
constexpr int func(bool flag) {
    return 0;
}

// C++11中不可，C++14中可以
constexpr int func(bool flag) {
    if (flag) return 1;
    else return 0;
}
```

### 3.5 `deprecated`标记

`C++14`中增加了`deprecated`标记，修饰类、变、函数等，当程序中使用到了被其修饰的代码时，编译时被产生警告，用户提示开发者该标记修饰的内容将来可能会被丢弃，尽量不要使用。

### 3.6 二进制字面量与整型字面量分隔符

`C++14`引入了二进制字面量，也引入了分隔符

```c++
int a = 0b0001'0011'1010;
double b = 3.14'1234'1234'1234;
```

### 3.7 std::make_unique

C++11中有std::make_shared，却没有std::make_unique，在C++14已经改善

```c++
struct A {};
std::unique_ptr<A> ptr = std::make_unique<A>();
```

### 3.8 std::shared_time_mutex与std::shared_lock

`C++14`通过`std::shared_timed_mutex`和`std::shared_lock`来实现读写锁，保证多个线程可以同时读，但是写线程必须独立运行，写操作不可以同时和读操作一起进行。

```c++
struct ThreadSafe {
    mutable std::shared_timed_mutex mutex_;
    int value_;

    ThreadSafe() {
        value_ = 0;
    }

    int get() const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        return value_;
    }

    void increase() {
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);
        value_ += 1;
    }
};
```

### 3.9 std::integer_sequence

```c++
template<typename T, T... ints>
void print_sequence(std::integer_sequence<T, ints...> int_seq)
{
    std::cout << "The sequence of size " << int_seq.size() << ": ";
    ((std::cout << ints << ' '), ...);
    std::cout << '\n';
}

int main() {
    print_sequence(std::integer_sequence<int, 9, 2, 5, 1, 9, 1, 6>{});
    return 0;
}
```

### 3.10 std::exchange

```c++
int main() {
    std::vector<int> v;
    std::exchange(v, {1,2,3,4});
    cout << v.size() << endl;
    for (int a : v) {
        cout << a << " ";
    }
    return 0;
}
```

看样子貌似和`std::swap`作用相同，但是实际上`std::exchange`将数组{1,2,3,4}赋值给了数组v,但是没有对数组v进行赋值。

### 3.11 std::quoted

`C++14`引入`std::quoted`用于给字符串添加双引号

```c++
int main() {
    string str = "hello world";
    cout << str << endl;
    cout << std::quoted(str) << endl;
    return 0;
}
```

输出:

```shell
"hello world"
```

## 四、C++ 17新特性

### 4.1 构造函数模板推导

在`C++17`前构造一个模板类对象需要指明类型:

```c++
std::pair<int, double> p(1, 2.2);
```

`C++17`就不需要特殊指定，直接可以推导出类型:

```c++
// c++17 自动推导
std::pair p(1, 2.2);
// c++17
std::vector v = {1, 2, 3};
```

### 4.2 结构化绑定

通过`结构化绑定`，对于`tuple`、`map`等类型，获取相应值:

```c++
std::tuple<int, double> func() {
    return std::tuple(1, 2.2);
}

int main() {
	// C++17
    auto[i, d] = func();
    cout << i << endl;
    cout << d << endl;
}

// -------------***-------------//
void f() {
    map<int, string> m = {
      {0, "a"},
      {1, "b"},
    };
    for (const auto &[i, s] : m) {
        cout << i << " " << s << endl;
    }
}

int main() {
    std::pair a(1, 2.3f);
    auto[i, f] = a;
    cout << i << endl; // 1
    cout << f << endl; // 2.3f
    return 0;
}
```

`结构化绑定`还可以改变对象的值，使用引用即可:

```c++
// 通过结构化绑定改变对象的值
int main() {
    std::pair a(1, 2.3f);
    auto& [i, f] = a;
    i = 2;
    // 2
    cout << a.first << endl;
}
```

<mark>注意:</mark> 结构化绑定不能应用于constexpr。

```c++
// compile error, C++20可以
constexpr auto[x, y] = std::pair(1, 2.3f);
```

`结构化绑定`不止可以绑定`pair`和`tuple`，还可以绑定`数组`和`结构体`等

```c++
int array[3] = {1, 2, 3};
auto [a, b, c] = array;
cout << a << " " << b << " " << c << endl;

// 注意这里的struct的成员一定要是public的
struct Point {
    int x;
    int y;
};
Point func() {
    return {1, 2};
}
const auto [x, y] = func();
```

### 4.3 if-switch语句初始化

`C++17`之后可以这样:

```c++
if (init; condition)
```

```c++
if (int a = GetValue()); a < 101) {
    cout << a;
}

string str = "Hi World";
if (auto [pos, size] = pair(str.find("Hi"), str.size()); pos != string::npos) {
    std::cout << pos << " Hello, size is " << size;
}
```

### 4.4 内敛变量

`C++17`前只有`内联函数`，现在有了`内联变量`，我们印象中`C++`类的`静态成员变量`在头文件中是不能初始化的，但是有了`内联变量`，就可以达到此目的:

```c++
// header file
struct A {
    static const int value;
};
inline int const A::value = 10;

struct A {
    inline static const int value = 10;
}
```

### 4.5 折叠表达式

`C++17`引入了`折叠表达式`使可变参数模板编程更方便：

```c++
template <typename ... Ts>
auto sum(Ts ... ts) {
    return (ts + ...);
}
int a {sum(1, 2, 3, 4, 5)}; // 15
std::string a{"hello "};
std::string b{"world"};
cout << sum(a, b) << endl; // hello world
```

### 4.6 constexptr lambda表达式

`C++17`前lambda表达式只能在`运行时`使用，`C++17`引入了`constexpr lambda`表达式，可以用于在编译期进行计算:

```c++
int main() { // c++17可编译
    constexpr auto lamb = [] (int n) { return n * n; };
    static_assert(lamb(3) == 9, "a");
}
```
<mark>constexpr函数有如下限制：</mark>

函数体不能包含`汇编语句`、`goto语句`、`label`、`try块`、`静态变量`、`线程局部存储`、`没有初始化的普通变量`，不能`动态分配内存`，不能有`new delete`等，不能有`虚函数`。

### 4.7 namespace嵌套

```c++
namespace A {
    namespace B {
        namespace C {
            void func();
        }
    }
}

// c++17，更方便
namespace A::B::C {
    void func();)
}
```

### 4.8 __has_include预处理表达式

可以判断是否有某个头文件，代码可能会在不同编译器下工作，不同编译器的可用头文件有可能不同，所以可以使用此来判断：

```c++
#if defined __has_include
#if __has_include(<charconv>)
#define has_charconv 1
#include <charconv>
#endif
#endif

std::optional<int> ConvertToInt(const std::string& str) {
    int value{};
#ifdef has_charconv
    const auto last = str.data() + str.size();
    const auto res = std::from_chars(str.data(), last, value);
    if (res.ec == std::errc{} && res.ptr == last) {
		return value;
	}
#else
    // alternative implementation...
    // 其它方式实现
#endif
    return std::nullopt;
}
```

### 4.9 在lambda表达式用*this捕获对象副本

正常情况下，`lambda表达式`中访问类的对象`成员变量`需要`捕获this`，但是这里捕获的是`this指针`，指向的是对象的引用，正常情况下可能没问题，但是如果多线程情况下，函数的作用域超过了对象的作用域，对象已经被析构了，还访问了成员变量，就会有问题。

```c++
struct A {
    int a;
    void func() {
        auto f = [this] {
            cout << a << endl;
        };
        f();
    }
};
int main() {
    A a;
    a.func();
    return 0;
}
```

C++17增加了新特性，捕获*this，不持有this指针，而是持有对象的拷贝，这样生命周期就与对象的生命周期不相关:

```c++
struct A {
    int a;
    void func() {
        auto f = [*this] {
            cout << a << endl;
        };
        f();
    }
};
int main() {
    A a;
    a.func();
    return 0;
}
```

### 4.10 新增Attribute

平时在项目中见过`__declspec`, `attribute` , `#pragma`指示符，使用它们来给编译器提供一些额外的信息，来产生一些优化或特定的代码，也可以给其它开发者一些提示信息。

```c++
struct A { short f[3]; } __attribute__((aligned(8)));

void fatal() __attribute__((noreturn));
```

在`C++11`和`C++14`中有更方便的方法：

```shell
[[carries_dependency]] 让编译期跳过不必要的内存栅栏指令
[[noreturn]] 函数不会返回
[[deprecated]] 函数将弃用的警告
```
```shell
[[noreturn]] void terminate() noexcept;
[[deprecated("use new func instead")]] void func() {}
```

C++17又新增了三个：
`[[fallthrough]]`，用在switch中提示可以直接落下去，不需要break，让编译期忽略警告

```c++
switch (i) {}
    case 1:
		// warning
        xxx;
    case 2:
        xxx;
        // 警告消除
        [[fallthrough]];
    case 3:
        xxx;
       break;
}
```

`[[nodiscard]]`: 表示修饰的内容不能被忽略，可用于修饰函数，标明返回值一定要被处理

```c++
[[nodiscard]] int func();
void F() {
	// warning 没有处理函数返回值
    func();
}
```

`[[maybe_unused]]`: 提示编译器修饰的内容可能暂时没有使用，避免产生警告

```c++
void func1() {}
// 警告消除
[[maybe_unused]] void func2() {}
void func3() {
    int x = 1;
    // 警告消除
    [[maybe_unused]] int y = 2;
}
```

### 4.11 std::variant

新增`from_chars`函数和`to_chars`函数，直接看代码：

```c++
#include <charconv>

int main() {
    const std::string str{"123456098"};
    int value = 0;
    const auto res = std::from_chars(str.data(), str.data() + 4, value);
    if (res.ec == std::errc()) {
        cout << value << ", distance " << res.ptr - str.data() << endl;
    } else if (res.ec == std::errc::invalid_argument) {
        cout << "invalid" << endl;
    }
    str = std::string("12.34);
    double val = 0;
    const auto format = std::chars_format::general;
    res = std::from_chars(str.data(), str.data() + str.size(), value, format);

    str = std::string("xxxxxxxx");
    const int v = 1234;
    res = std::to_chars(str.data(), str.data() + str.size(), v);
    cout << str << ", filled " << res.ptr - str.data() << " characters \n";
    // 1234xxxx, filled 4 characters
}
```

### 4.12 std::optional

`c++17`增加`std::variant`实现类似`union`的功能，但却比union更高级，举个例子union里面不能有string这种类型，但std::variant却可以，还可以支持更多复杂类型，如map等

```c++
int main() {
	// c++17可编译
    std::variant<int, std::string> var("hello");
    cout << var.index() << endl;
    var = 123;
    cout << var.index() << endl;

    try {
        var = "world";
		// 通过类型获取值
        std::string str = std::get<std::string>(var);
        var = 3;
		// 通过index获取对应值
        int i = std::get<0>(var);
        cout << str << endl;
        cout << i << endl;
    } catch(...) {
        // xxx;
    }
    return 0;
}
```

<mark>注意: </mark>一般情况下variant的第一个类型一般要有对应的构造函数，否则编译失败：

```c++
struct A {
    A(int i){}
};
int main() {
	// 编译失败
    std::variant<A, int> var;
}
```

避免这种情况呢，可以使用`std::monostate`来打个桩，模拟一个空状态。

```c++
// 可以编译成功
std::variant<std::monostate, A> var;
```

### 4.13 std::optional

有时候可能会有需求，让函数返回一个对象

```c++
struct A {};
A func() {
    if (flag) return A();
    else {
        // 异常情况下，怎么返回异常值呢，想返回个空呢
    }
}
```

有一种办法是`返回对象指针`，异常情况下就可以返回nullptr，但是这就涉及到了内存管理，也许你会使用智能指针，但这里其实有更方便的办法就是`std::optional`。

```c++
std::optional<int> StoI(const std::string &s) {
    try {
        return std::stoi(s);
    } catch(...) {
        return std::nullopt;
    }
}

void func() {
    std::string s{"123"};
    std::optional<int> o = StoI(s);
    if (o) {
        cout << *o << endl;
    } else {
        cout << "error" << endl;
    }
}
```

### 4.14 std::any

`C++17`引入了`any`可以存储任何类型的单个值

```c++
int main() {
	// c++17可编译
    std::any a = 1;
    cout << a.type().name() << " " << std::any_cast<int>(a) << endl;
    a = 2.2f;
    cout << a.type().name() << " " << std::any_cast<float>(a) << endl;
    if (a.has_value()) {
        cout << a.type().name();
    }
    a.reset();
    if (a.has_value()) {
        cout << a.type().name();
    }
    a = std::string("a");
    cout << a.type().name() << " " << std::any_cast<std::string>(a) << endl;
    return 0;
}
```

### 4.15 std::apply

使用`std::apply`可以将`tuple`展开作为函数的参数传入

```c++
int add(int first, int second) { return first + second; }

auto add_lambda = [](auto first, auto second) { return first + second; };

int main() {
    std::cout << std::apply(add, std::pair(1, 2)) << '\n';
    std::cout << add(std::pair(1, 2)) << "\n"; // error
    std::cout << std::apply(add_lambda, std::tuple(2.0f, 3.0f)) << '\n';
}
```

### 4.16 std::make_from_tuple

使用make_from_tuple可以将tuple展开作为构造函数参数

```c++
struct Foo {
    Foo(int first, float second, int third) {
        std::cout << first << ", " << second << ", " << third << "\n";
    }
};
int main() {
   auto tuple = std::make_tuple(42, 3.14f, 0);
   std::make_from_tuple<Foo>(std::move(tuple));
}
```

### 4.17 std::string_view

通常传递一个string时会触发对象的拷贝操作，大字符串的拷贝赋值操作会触发堆内存分配，很影响运行效率，有了string_view就可以**避免拷贝操作**，平时传递过程中传递string_view即可。

```c++
void func(std::string_view stv) { cout << stv << endl; }

int main(void) {
    std::string str = "Hello World";
    std::cout << str << std::endl;

    std::string_view stv(str.c_str(), str.size());
    cout << stv << endl;
    func(stv);
    return 0;
}
```

### 4.18 as_const

C++17使用as_const可以将左值转成const类型

```c++
std::string str = "str";
const std::string& constStr = std::as_const(str);
```

### 4.19 file_system

C++17正式将file_system纳入标准中，提供了关于文件的大多数功能，基本上应有尽有:

```c++
namespace fs = std::filesystem;
fs::create_directory(dir_path);
fs::copy_file(src, dst, fs::copy_options::skip_existing);
fs::exists(filename);
fs::current_path(err_code);
```

### 4.20 std::shared_mutex

C++17引入了shared_mutex，可以实现读写锁。



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_8/  

