# Effective STL [20] | 为指针的关联容器指定比较类型


## Example

假定你有一个string*指针的set，你把一些动物的名字插入进set:

```c++
set<string*> ssp;
ssp.insert(new string("Anteater"));
ssp.insert(new string("Wombat"));
ssp.insert(new string("Lemur"));
ssp.insert(new string("Penguin"));
// ssp = “set of string ptrs”
```

然后你写了下列代码打印set的内容，希望字符串按字母顺序出现。毕竟，确定set保持它们的内容有序。

```c++
for (set<string*>::const_iterator i = ssp.begin();i != ssp.end();++i) {
  cout << *i << endl;
}
```

你期望看到

```shell
Anteater
Wombat
Lemur
Penguin
```

实际运行结果:
```c++
0x5566364b5eb0
0x5566364b5f10
0x5566364b5f70
0x5566364b5fd0
```

结果是4个十六进制的数，代表指针的值。 因为set容纳指针，`*i`不是一个`string`，是一个`string`的指针。

如果你已经改为调用copy算法，

```c++
copy(ssp.begin(), ssp.end(), ostream_iterator<string>(cout, "\n"));// 把ssp中的字符串拷贝到cout（但这不能编译）
```

这个copy的调用将不能编译，因为`ostream_iterator`需要知道被打印的对象的类型，所以当你告诉它是一个string时（通过作为模板参数传递），编译器检测到那和ssp中储存的对象类型(是`string*`)之间不匹配，它们会拒绝编译代码。

把显式循环中的`*i`改为`**i`，你可能可以得到你想要的输出，但也可能不。动物名字将被打印，但它们按字母顺序出现的机会只是24份之1。ssp保持它的内容有序，但是它容纳的是指针，所以它以指针的值排序，而不以string值。对于四个指针值可能有24种排列（），所以指针被储存时有24种可能的顺序。因此你看见字符串按字母排序有24份之1的几率。

或者打印 `*string`的`char*`数组

```c++
for (set<string*>::const_iterator i = ssp.begin(); i != ssp.end(); ++i) {
  cout << (*i)->c_str() << endl;
}
```

## Solution

当我们写下`set<string*> ssp`;，其实省略了一个默认参数`set<string*, less<string*> > ssp`;，实际上还有个默认参数:

```c++
set<string*, less<string*>, allocator<string*> > ssp;
```

## 自定义仿函数

如果想要`string*`指针以字符串值确定顺序被储存在`set`中，不能使用默认比较仿函数类`less<string*>`。

**必须改为写自己的比较仿函数类，它的对象带有string*指针并按照指向的字符串值来进行排序:**
```c++
struct StringPtrLess:
  public binary_function<const string*, const string*, bool> {
    bool operator()(const string *ps1, const string *ps2) const
    {
      return *ps1 < *ps2;
    }
};
```

然后可以使用`StringPtrLess`作为`ssp`的比较类型:

```c++
typedef set<string*, StringPtrLess> StringPtrSet;
StringPtrSet ssp;
// 建立字符串的集合，
// 按照StringPtrLess定义的顺序排序
...
// 和前面一样插入
// 同样四个字符串
```

现在循环最后将做想要它做的（也就是前面你使用*i代替**i所修正的问题）:

```c++
for (StringPtrSet::const_iterator i = ssp.begin(); i != ssp.end(); ++i) {
  cout << **i << endl;
}
```

指针之前对它们解引用的函数，然后和`for_each`联用那个函数:

```c++
void print(const string *ps) {
 cout << *ps << endl;
}
for_each(ssp.begin(), ssp.end(), print);// 在ssp中的每个元素上调用print
```

或者你想象并写出了泛型的解引用仿函数类，然后让它和transform与ostream_iterator连用:

```c++
// 当本类型的仿函数被传入一个T*时，它们返回一个const T&
struct Dereference {
  template <typename T>
  const T& operator()(const T *ptr) const {
    return *ptr;
  }
};
// 通过解引用“转换” ssp中的每个元素，把结果写入cout
transform(ssp.begin(), ssp.end(), ostream_iterator<string>(cout, "\n"), Dereference());
```

要点是无论何时你建立一个指针的标准关联容器，你必须记住容器会以指针的值排序。这基本上不是你想要的，所以你几乎总是需要建立自己的仿函数类作为比较类型。

**比较类型 vs 比较函数**

注意到这里写的是“比较类型”。

你可能奇怪为什么必须特意创造一个仿函数类而不是简单地为set写一个比较函数:

```c++
bool stringPtrLess(const string* ps1, const string* ps2) { // 将成为用于按字符串值排序的string*指针的比较函数
 return *ps1 < *ps2;
}
set<string*, stringPtrLess> ssp;// 假设使用stringPtrLess 作为ssp的比较函数；
// 这不能编译
```

这里的问题是每个set模板的第三个参数都是一种类型，而`stringPtrLess`不是一种类型，它是一个函数。这就是为什么尝试使用`stringPtrLess`作为set的比较函数不能编译的原因，set不要一个函数，它要的是能在内部用实例化建立函数的一种类型。

**无论何时你建立指针的关联容器，注意你也得指定容器的比较类型。**

大多数时候，你的比较类型只是解引用指针并比较所指向的对象（就像上面的`StringPtrLess`做的那样）。

鉴于这种情况，你手头最好也能有一个用于那种比较的仿函数模板。像这样:

```c++
struct DereferenceLess {
  template <typename PtrType>
  bool operator()(PtrType pT1, PtrType pT2) const { // 参数是值传递的因为我们希望它们是（或行为像）指针
    return *pT1 < *pT2;
  }
};
```

这样的模板消除了写像`StringPtrLess`那样的类的需要，因为我们可以改为使用`DereferenceLess`:

```c++
set<string*, DereferenceLess> ssp;
// 行为就像 set<string*, StringPtrLess>
```

## 总结

本条款是关于指针的关联容器，但它也可以应用于表现为指针的对象的容器，例如，智能指针和迭代器。

如果你有一个智能指针或迭代器的关联容器，那也得为它指定比较类型。幸运的是，指针的这个解决方案也可以用于类似指针的对象。正如DereferenceLess适合作为T*的关联容器的比较类型一样，它也 可以作为T对象的迭代器和智能指针容器的比较类型。

实际上，这24种排列很可能不是平等的，所以“24份之1”的陈述有点使人误解。确实，有24个不同的顺序，而且你可能得到它们中的任何一个。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_20/  

