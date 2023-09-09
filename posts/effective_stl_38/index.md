# Effective STL [38] | 把仿函数类设计为用于值传递


## 函数指针按值传递

C和C++都不允许你真的把函数作为参数传递给其他函数。取而代之的是，你必须传指针给函数。

比如，这里有一个标准库函数qsort的声明:

```c++
void qsort(void *base, size_t nmemb, size_t size, int (*cmpfcn)(const void*, const void*));
```

一旦你忽略了所有的星号，就可以清楚地看出作为cmpfcn传递的实参，一个指向函数的指针，是从调用端拷贝（也就是，值传递）给qsort。这是C和C++标准库都遵循的一般准则:**函数指针是值传递**。

STL中的习惯是当传给函数和从函数返回时函数对象也是值传递的（也就是拷贝）。

最好的证据是标准的for_each声明，这个算法通过值传递获取和返回函数对象:

```c++
template<class InputIterator, class Function> Function
// 注意值返回， 注意值传递
for_each(InputIterator first, InputIterator last, Function f);
```

实际上，值传递的情况并不是完全打不破的，因为for_each的调用者在调用点可以显式指定参数类型。比如，下面的代码可以使for_each通过引用传递和返回它的仿函数:

```c++
class DoSomething : public unary_function<int, void> {
 public:
  void operator()(int x){...}
  ...
};

// 方便的typedef
typedef deque<int>::iterator DequeIntIter;

deque<int> di;
...
    // 建立一个函数对象
    DoSomething d;

...
    // 调用for_each，参数类型是DequeIntIter和DoSomething&；
    // 这迫使d按引用传递和返回
    for_each<DequeIntIter, DoSomething &>(di.begin(), di.end(), d);
```

## 保证拷贝传递行为正常

因为函数对象以值传递和返回，你的任务就是确保当拷贝传递时你的函数对象行为良好。

这暗示了2个东西:
 1. **你的函数对象应该很小。否则它们的拷贝会很昂贵。**
 2. **你的函数对象必须单态（也就是，非多态）——它们不能用虚函数。那是因为派生类对象以值传递代入基类类型的参数会造成切割问题: 在拷贝时，它们的派生部分被删除。**

当然效率很重要，避免切割问题也是，但不是所有的仿函数都是小的、单态的。函数对象比真的函数优越的的原因之一是仿函数可以包含你需要的所有状态。

有些函数对象自然会很重，保持传这样的仿函数给STL算法和传它们的函数版本一样容易是很重要的。

## 多态仿函数实现

禁止多态仿函数是不切实际的。C++支持继承层次和动态绑定，这些特性在设计仿函数类和其他东西的时候一样有用。仿函数类如果缺少继承就像C++缺少“++”。

带着你要放进你的仿函数类的数据和/或多态，把它们移到另一个类中。然后给你的仿函数一个指向这个新类的指针。

比如，如果你想要建立一个包含很多数据的多态仿函数类。

```c++
// BPFC = “Big Polymorphic Functor Class”
template <typename T>
class BPFC : public unary_function<T, void> {
  // 条款40解释了这个基类
 private:
  Widget w;
  Int x;
  // 这个类有很多数据，所以用值传递

  ...
      // 会影响效率
      public : virtual void
        operator()(const T& val) const;  // 这是一个虚函数，所以切割时会出问题
  ...
};
```

建立一个包含一个指向实现类的指针的小而单态的类，然后把所有数据和虚函数放到实现类:

```c++
// 用于修改的BPFC的新实现类
template <typename T>
class BPFCImpl : public unary_function<T, void> {
 private:
  // 以前在BPFC里的所有数据现在在这里
  Widget w;

  int x;
  ...
 // 多态类需要虚析构函数
 virtual ~BPFCImpl();

  virtual void operator()(const T& val) const;
  friend class BPFC<T>;  // 让BPFC可以访问这些数据
};

// 小的，单态版的BPFC
template <typename T>
class BPFC :

    public unary_function<T, void> {
 private:
  BPFCImpl<T>* pImpl;  // 这是BPFC唯一的数据

 public:
  // 现在非虚；
  void operator()(const T& val) const  {
    // 调用BPFCImpl的真的虚函数
    pImpl->operator()(val);
  }
  ...
};
```

`BPFC::operator()`的实现例证了BPFC所有的虚函数是怎么实现的: 它们调用了在BPFCImpl中它们真的虚函数。

结果是仿函数类（BPFC）是小而单态的，但可以访问大量状态而且行为多态。

顺便说一句，这种实现方法在《Effective C++》的条款34中有。在Gamma等的《设计模式》中，这叫做“Bridge模式”。Sutter在他的《Exceptional C++》中叫它“Pimpl惯用法”。

## 总结

从STL的视角看来，要记住的最重要的东西是**使用这种技术的仿函数类必须支持合理方式的拷贝**。

唯一你必须担心的是BPFC的拷贝构造函数的行为，因为当在STL中被传递或从一个函数返回时，函数对象总是被拷贝——值传递。

记得吗？那意味着两2件事: **让它们小，而且让它们单态**。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/effective_stl_38/  

