# CMake 笔记 | [9] 设置语言标准 (三)


## 四、C++ 20新增特性

### 4.1 新增关键字

- concept(见下文)
- requires
  `std::require` 是一个`constexpr` 函数模板，用于在编译时检查某个表达式的真假值。如果表达式为真，则该函数返回一个无意义的类型 `void_t`；否则编译会失败，出现相应的错误信息
- consteval
  用来修饰函数时常量值的表达式，而且是强制性的。如果函数本身不是常量值的表达式的话则会编译失败
- co_await(协程相关)
- co_return(协程相关)
- co_yield(协程相关)
- char8_t

### 4.2 新增标识符

- import
- module

**模块**

**优点**

- 没有头文件
- 声明实现仍然可分离, 但非必要
- 可以显式指定那些导出(类, 函数等)
- 不需要头文件重复引入宏 (include guards)
- 模块之间名称可以相同不会冲突
- 模块只处理一次, 编译更快 (头文件每次引入都需要处理)
- 预处理宏只在模块内有效
- 模块引入顺序无关紧要

**创建模块**

```c++
// cppcon.cpp
export module cppcon;
namespace CppCon {
    auto GetWelcomeHelper() {  return "Welcome to CppCon 2019!";  }
    export auto GetWelcome() { return GetWelcomeHelper();}
}
```

**引用模块**

```c++
// main.cpp
import cppcon;
int main(){
    std::cout << CppCon::GetWelcome();
}
```

### 4.3 import头文件

- import
- 隐式地将 iostream 转换为模块
- 加速构建, 因为 iostream 只会处理一次
- 和预编译头 (PCH) 具有相似的效果

### 4.4 Ranges

`Range` 代表一串元素, 或者一串元素中的一段，类似于`begin/end` 对。

**好处**

- 简化语法和方便使用
- 防止 begin/end 不配对
- 使变换/过滤等串联操作成为可能

**相关功能**
- 视图: 延迟计算, 不持有, 不改写
- Actions: 即时处理, 改写
- Algorithms: 所有接受 begin/end 对的算法都可用
- Views 和 actions 使用管道符|串联

### 4.5 协程

**协程定义**是一个函数，具备如下关键字之一:
  - co_wait: 挂起协程, 等待其它计算完成
  - co_return: 从协程返回 (协程 return 禁止使用)
  - co_yield: 同 python yield, 弹出一个值, 挂起协程, 下一次调用继续协程的运行
  - for co_await 循环体

**应用场景**

- 简化generator
- 简化异步I/O
- 简化延迟计算
- 简化事件驱动的程序

### 4.6 Concepts

- 对模板类和函数的模板形参的约束
- 编译期断言
- 可声明多个

**定义**

```c++
template<typename T> concept Incrementable = requires(T x) {x++; ++x;};
```

**使用**

```c++
template<Incrementable T>
void Foo(T t);

template<typename T> requires Incrementable<T>
void Foo(T t);

template<typename T>
void Foo(T t) requires Incrementable<T>;

void Foo(Incrementable auto t);
```

### 4.7 Lambda表达式更新

**[=, this] 需要显式捕获this变量**

  - C++20 之前 [=] 隐式捕获this
  - C++20 开始 需要显式捕获this: [=, this]

**模板形式的 Lambda 表达式**

可以在`lambda`表达式中使用模板语法:

```c++
[]template<T>(T x) {/* ... */};
[]template<T>(T* p) {/* ... */};
[]template<T, int N>(T (&a)[N]) {/* ... */};
```

**优点**

`C++20`之前: 获取 `vector` 元素类型, 需要这么写

```c++
auto func = [](auto vec){
    using T = typename decltype(vec)::value_type;
}
```

`C++20` 可以:

```c++
auto func = []<typename T>(vector<T> vec){
    // ...
}
```

- 方便获取通用`lambda`形参类型, 访问`静态函数`:

<mark>C++20之前：</mark>

```c++
auto func = [](auto const& x){
    using T = std::decay_t<decltype(x)>;
    T copy = x; T::static_function();
    using Iterator = typename T::iterator;
}
```

<mark>C++20以后：</mark>

```c++
auto func = []<typename T>(const T& x){
    T copy = x; T::static_function();
    using Iterator = typename T::iterator;
}
```

- 完美转发

<mark>C++20之前：</mark>

```c++
auto func = [](auto&& ...args) {
    return foo(std::forward<decltype(args)>(args)...);
}
```

<mark>C++20以后：</mark>

```c++
auto func = []<typename …T>(T&& …args){
    return foo(std::forward(args)...);
}
```

- Lambda 表达式捕获支持打包展开(Pack Expansion)

<mark>C++20之前：</mark>

```c++
template<class F, class... Args>
auto delay_invoke(F f, Args... args){
    return [f, args...]{
        return std::invoke(f, args...);
    }
}
```

<mark>C++20以后：</mark>

```c++
template<class F, class... Args>
auto delay_invoke(F f, Args... args){
    // Pack Expansion:  args = std::move(args)...
    return [f = std::move(f), args = std::move(args)...](){
        return std::invoke(f, args...);
    }
}
```

### 4.8 常量表达式(constexpr)的更新

- `constexpr` 虚函数
- `constexpr` 的虚函数可以重写非 `constexpr` 的虚函数
- 非 `constexpr` 虚函数可以重写 `constexpr` 的虚函数
- 使用 `dynamic_cast()` 和 `typeid`
- 动态内存分配
- 更改`union`成员的值
- 包含 `try/catch`
- 不允许`throw`语句
- 在触发常量求值的时候 `try/catch` 不发生作用
- 需要开启 `constexpr std::vector`

**constexpr string &vector**

- `std::string` 和 `std::vector` 类型现在可以作为 `constexpr`
- 未来需要支持 `constexpr` 反射

### 4.9 原子智能指针

`智能指针`(shared_ptr)`线程安全`问题：

- 安全: 引用计数控制单元线程安全, 保证对象只被释放一次
- 不安全：对于数据的读写没有线程安全

将`智能指针`变成`线程安全`：

- 使用 `mutex` 控制智能指针的访问
- 使用全局非成员原子操作函数访问, 诸如: `std::atomic_load()`, `atomic_store()`, …

`C++20`: `atomic<shared_ptr<T>>`, `atomic<weak_ptr<T>>`：

- 内部原理可能使用了`mutex`
- 全局非成员原子操作函数标记为不推荐使用(`deprecated`)

### 4.10 自动合流(Joining),可中断的线程

**自动合流**

```c++
void DoWorkPreCpp20() {
    std::thread job([] { /* ... */ });
    try {
        // ... Do something else ...
    } catch (...) {
        job.join();
        throw; // rethrow
    }
    job.join();
}

void DoWork() {
    std::jthread job([] { /* ... */ });
    // ... Do something else ...
} // jthread destructor automatically calls join()
```

**中断**

```c++
std::jthread job([](std::stop_token token) {
    while (!token.stop_requested()) {
        //...
    }
});
//... job.request_stop();
// auto source = job.get_stop_source()
// auto token = job.get_stop_token()
```

### 4.11 同步(Synchronization)库

在传统的`多线程`（`进程`）的编程中，处理数据共享是一个重中之重。目前流行的多核（多CPU）编程中，虽然采用了更多的`分布式`的算法，但最终细分到一个处理单元中，仍然是处理线程间数据的拆分。即，通过`数据结构`的设计和`算法`的分拆，实现最小的数据冲突结果。

解决`多线程`编程中的一个重要的问题就是 <mark>如何处理数据的同步</mark>问题，如有`mutex`,`event`,`condition`等等。也有的会提到`c++11`后的`lock`等。

在`c++20`中增加了以下几类`同步` `数据结构`:

**信号量(Semaphore)**
轻量级的同步原语，可以实现 `mutex`, `latches`, `barriers`, …等同步数据结构。

两种表现类型:

  - `多元信号量`(counting semaphore): 建模非负值资源计数
  - `二元信号量`(binary semaphore): 只有两个状态的信号量

主要方法有：

  - release：增加内部计数器并对获取者解除阻塞
  - acquire ：减少内部计数器或阻塞到直至能获取
  - try_acquire：尝试减少内部计数器而不阻塞
  - try_acquire_for ：尝试减少内部计数器，至多阻塞一段时长
  - try_acquire_until：尝试减少内部计数器，阻塞直至一个时间点

**std::atomic 等待和通知接口**

`等待`/`阻塞`在原子对象直到其值发生改变, 然后通知函数发送通知，它比单纯的自旋锁和轮询要效率高。

主要方法有：
  - wait：阻塞线程直至被提醒且原子值更改
  - notify_one：提醒至少一个在原子对象上的等待中阻塞的线程
  - notify_all：提醒所有在原子对象上的等待中阻塞的线程

这个其实是实现`CAS`的，在以前就有，在`c++20`中又增加了相关的一些具体的实现罢了。

**锁存器(Latches)**

`latch` 是 `std::ptrdiff_t` 类型的向下计数器，它能用于`同步线程`。在创建时初始化计数器的值。线程可能在 `latch` 上阻塞直至计数器减少到零。没有可能增加或重置计数器，这使得 `latch` 为单次使用的屏障。同时调用 `latch` 的成员函数，除了`析构函数`，不引入数据竞争。

注意：它区别于下面的`Barriers`的是它只有使用一次。

**屏障(Barriers)**

`std::barrier` 提供允许至多为期待数量的线程阻塞直至期待数量的线程到达该屏障。不同于 `std::latch` ，屏障可重用：一旦到达的线程从屏障阶段的同步点除阻，则可重用同一屏障。

### 4.12 std::atomic_ref

`std::atomic_ref`类型对其引用的对象进行原子操作。

使用`std::atomic_ref` 进行多线程读写时不会造成数据争用。被引用对象的生命周期必须超过`std::atomic_ref` 。操作`std::atomic_ref` 的子对象是未定义行为。

### 4.13 其他更新

**指定初始化(Designated Initializers)**

```c++
struct Data {
    int anInt = 0;
    std::string aString;
};

Data d{ .aString = "Hello" };
```

**三路比较运算符 <=>**

```c++
// 如果 a < b 则为 true
(a <=> b) < 0
// 如果 a > b 则为 true
(a <=> b) > 0
// 如果 a 与 b 相等或者等价 则为 true
(a <=> b) == 0
```

标准库类型支持: `vector`, `string`, `map`, `set`, `sub_match`, …

**范围 for 循环语句支持初始化语句**

`switch` 语句初始化 (`C++17`):

```c++
struct Foo {
    int value;
    int result;
};

Foo GetData() {
    return Foo();
}

int main() {
    switch (auto data = GetData(); data.value) {
    case 1:
        return data.result;
    }
}
```

`if` 语句初始化 (`C++17`):

```c++
struct Foo {
    int value; int result;
};

Foo* GetData() {
  return new Foo();
}

int main() {
    if (auto data = GetData(); data) {
        // Use 'data’
    }
}
```

现在范围 `for` 循环同样支持初始化 (`C++20`):

```c++
struct Foo {
    std::vector<int> values;
};

Foo GetData() {
    return Foo();
}
int main() {
    for (auto data = GetData(); auto& value : data.values) {
        // Use 'data’
    }
}
```

**非类型模板形参支持字符串**

```c++
template<auto& s> void DoSomething() {
    std::cout << s << std::endl;
}

int main() {
    DoSomething<"CppCon">();
}
```

**[[likely]], [[unlikely]]**

<mark>先验概率指导编译器优化</mark>

```c++
switch (value) {
    case 1: break;
    [[likely]] case 2: break;
    [[unlikely]] case 3: break;
}
```

**日历(Calendar)和时区(Timezone)功能**

- 增加日历和时区的支持
- 只支持公历(Gregorian calendar)
- 其他日历也可通过扩展加入, 并能和 进行交互

具体操作和相关类型请参考其他示例。

**std::span**
定义：某段连续数据的”视图”
特性：

- 不持有数据, 不分配和销毁数据
- 拷贝非常快, 推荐复制的方式传参(类似 `string_view`)
- 不支持数据跨步(`stride`)
- 可通过运行期确定长度也可编译器确定长度

```c++
// fixed-size: 42 ints
int data[42]; span<int, 42> a {data};
// dynamic-size: 42 ints
span<int> b {data};
// compilation error
span<int, 50> c {data};
// dynamic-size: len ints
span<int> d{ ptr, len };
```

**特性测试宏**
通过它可以判断编译器是否支持某个功能。

语言特性：

- `__has_cpp_attribute(fallthrough)`
- `__cpp_binary_literals`
- `__cpp_char8_t`
- `__cpp_coroutines`

标准库特性:

- `__cpp_lib_concepts`
- `__cpp_lib_ranges`
- `__cpp_lib_scoped_lock`

包含 `C++ 标准库`版本, `发布日期`, `版权证书`, `特性宏`等。

**consteval 函数**

  - `constexpr` 函数可能编译期执行, 也可以在运行期执行
  - `consteval` 只能在编译器执行, 如果不满足要求编译不通过。

**constinit: 强制指定以常量方式初始化**

```c++
const char* GetStringDyn() {
    return "dynamic init";
}
constexpr const char* GetString(bool constInit) {
    return constInit ?
        "constant init" :
        GetStringDyn();
}
// ✔
constinit const char* a = GetString(true);
// ❌
constinit const char* b = GetString(false);
```

**用 using 引用 enum 类型**

```c++
enum class CardTypeSuit {
    Clubs,
    Diamonds,
    Hearts,
    Spades
};

std::string_view GetString(const CardTypeSuit cardTypeSuit) {
    switch (cardTypeSuit) {
    case CardTypeSuit::Clubs:
        return "Clubs";
    case CardTypeSuit::Diamonds:
        return "Diamonds";
    case CardTypeSuit::Hearts:
         return "Hearts";
    case CardTypeSuit::Spades:
         return "Spades";
    }
}

std::string_view GetString(const CardTypeSuit cardTypeSuit) {
    switch (cardTypeSuit) {
        using enum CardTypeSuit;
        case Clubs: return "Clubs";
        case Diamonds: return "Diamonds";
        case Hearts: return "Hearts";
        case Spades: return "Spades";
    }
}
```

**格式化库(std::format)**

```c++
std::string s = std::format("Hello CppCon {}!", 2019);
```

**增加数学常量**

包含 `e`, `log2e`, `log10e`, `pi`, `inv_pi`, `inv_sqrt` `ln2`, `ln10`, `sqrt2`, `sqrt3`, `inv_sqrt3`, `egamma`

**std::source_location**

用于获取代码位置, 对于日志和错误信息尤其有用

**[[nodiscard(reason)]]**

表明返回值不可抛弃, 加入理由的支持

```c++
[[nodiscard("Ignoring the return value will result in memory leaks.")]]
void* GetData() { /* ... */ }
```

**位运算**

加入循环移位, 计数`0`和`1`位等功能

**一些小更新**

  - 字符串支持 `starts_with`, `ends_with`
  - `map` 支持 `contains` 查询是否存在某个键
  - `list` 和 `forward list` 的 `remove`, `remove_if` 和 `unique` 操作返回 `size_type` 表明删除个数
  - 增加 `shift_left`, `shift_right`
  - `midpoint` 计算中位数, 可避免溢出
  - `lerp` 线性插值 `lerp( float a, float b, float t )` 返回
  - 新的向量化策略 `unsequenced_policy`(`execution::unseq`)

```c++
std::string str = "Hello world!";
// starts_with, ends_with
bool b = str.starts_with("Hello");
std::map myMap{ std::pair{1, "one"s}, {2, "two"s}, {3, "three"s} };
// contains, 再也不用  .find() == .end() 了
bool result = myMap.contains(2);
```


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/cmake_note_9/  

