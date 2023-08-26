# Effective STL [6] | 仿函数、仿函数类、函数等


# 前言

> Effective-STL总结系列分为七部分，本文为第六部分，涉及原书第六章，内容范围Rule38~42。为方便书写，Rule38简写为R38。

{{< admonition Note "Effective-STL系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective STL(第3版)精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 [Effective STL(第3版)精读总结(二)](https://jianye0428.github.io/posts/parttwo/)</br>
💡 [Effective STL(第3版)精读总结(三)](https://jianye0428.github.io/posts/partthree/)</br>
💡 [Effective STL(第3版)精读总结(四)](https://jianye0428.github.io/posts/partfour/)</br>
{{< /admonition >}}


## R38: 遵循按值传递的原则来设计函数子类

函数指针是按值传递的。

函数对象往往按值传递和返回。所以，编写的函数对象必须尽可能地小巧，否则复制的开销大；函数对象必须是**单态**的（不是多态），不得使用虚函数。

如果你希望创建一个包含大量数据并且使用了多态性的函数子类，该怎么办呢？

```c++
template<typename T>
class BPFCImpl:
	public unary_function<T, void> {
private:
	Widget w;
	int x;
	...
	virtual ~BPFCImpl();
	virtual void operator() (const T& val) const;
friend class BPFC<T>;					// 允许BPFC访问内部数据。
}

template<typename T>
class BPFC:								// 新的BPFC类：短小、单态
	public unary_function<T, void> {
private:
	BPFCImpl<T> *pImpl;					// BPFC唯一的数据成员
public:
	void operator() (const T& val) const	// 现在这是一个非虚函数，将调用转到BPFCImpl中
    {
        pImpl->operator()(val);
    }
}
```

那么你应该创建一个小巧、单态的类，其中包含一个指针，指向另一个实现类，并且将所有的数据和虚函数都放在实现类中（“Pimpl Idiom”）。

```c++
template<typename T>
class BPFCImpl:
	public unary_function<T, void> {
private:
	Widget w;
	int x;
	...
	virtual ~BPFCImpl();
	virtual void operator() (const T& val) const;
friend class BPFC<T>;					// 允许BPFC访问内部数据。
}

template<typename T>
class BPFC:								// 新的BPFC类：短小、单态
	public unary_function<T, void> {
private:
	BPFCImpl<T> *pImpl;					// BPFC唯一的数据成员
public:
	void operator() (const T& val) const	// 现在这是一个非虚函数，将调用转到BPFCImpl中
    {
        pImpl->operator()(val);
    }
}
```

## R39 确保判别式是 “纯函数”

**判别式（predicate）：一个返回值为 bool 类型的函数。**

**纯函数：指返回值仅仅依赖于其参数的函数。**

判别式类（predicate class）：一个函数子类，它的 operator() 函数是一个判别式（返回 true 或 false）。

STL 中凡是可以接受一个判别式类对象的地方，也就可以接受一个判别式函数。

判别式应该是一个纯函数，而纯函数应该没有状态。

## R40 使仿函数类可适配

对函数指针，要先应用`ptr_fun`之后再应用`not1`之后才可以工作。

4 个标准的函数配接器（`not1`、`not2`、`bind1st`、`bind2nd`）都要求一些特殊的类型定义，提供这些必要类型定义（`argument_type`、`first_argument_type`、`second_argument_type`、`result_type`）的函数对象被称为可配接(可适配)（`adaptable`）的函数对象。

提供这些类型定义最简单的方法：让函数子从一个基结构继承。
  - 对于 unary_function，必须指定函数子类 operator() 所带的参数类型，以及 operator() 返回类型。
  - 对于 binary_function，必须指定 3 个类型：operator() 第一个和第二个参数类型，以及 operator() 返回类型。

```c++
template<typename T>
class MeetsThreshold: public std::unary_function<Widget, bool> {
private:
    const T threshold;						// 包含状态信息，使用类封装。
public:
    MeetsThreshold(const T& threshold);
    bool operator()(const Widget&) const;
    ...
}

struct WidgetNameCompare:					// STL中所有无状态函数子类一般都被定义成结构。
	public std::binary_function<Widget, Widget, bool> {
	bool operator()(const Widget& lhs, const Widget& rhs) const;
}
```

注意，一般情况下，传递给 binary_function 或 unary_function 的非指针类型需要去掉 const 和应用（&）部分。


## R41 理解 ptr_fun、mem_fun 和 mem_fun_ref 的来由

STL语法惯例：函数或者函数对象被调用时，总是使用非成员函数的语法形式。

```c++
for_each(vw.begin(), vw.end(), test);					// 调用1：f(x)，f为非成员函数
for_each(vw.begin(), vw.end(), &Widget::test);			// 调用2：x.f()，f为成员函数
														// x是一个对象或对象的引用
list<Widget *> lpw;
for_each(lpw.begin(), lpw.end(), &Widgettest);			// 调用3：p->f()，f为成员函数
														// p是一个指向对象x的指针。
```
mem_fun、mem_fun_t：mem_fun 将语法 3 调整为语法 1。

```c++
template<typename R, typename C>		//该mem_fun声明针对不带参数的非const成员函数
mem_fun_t<R,C>							//C是类，R是所指向的成员函数返回的类型。
mem_fun(R(C::*pmf));
```

mem_fun 带一个指向某个成员函数的指针参数 pmf，并且返回一个 mem_fun_t 类型的对象。

mem_fun_t 是一个函数子类，它拥有该成员函数的指针，并提供了 operator() 函数，在 operator() 中调用了通过参数传递进来的对象上的该成员函数。

类似地，mem_fun_ref 将语法 2 调整为语法 1。

**总结:**
- std::ptr_fun：将函数指针转换为函数对象。
- std::mem_fun：将成员函数转换为函数对象(指针版本)。
- std::mem_fun_ref：将成员函数转换为函数对象(引用版本)。

## R42

Ref:

[1]. https://www.cnblogs.com/Sherry4869/p/15162253.html
[2]. https://blog.csdn.net/zhuikefeng/article/details/108164117#t35
[3]. https://zhuanlan.zhihu.com/p/458156007

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partsix/  

