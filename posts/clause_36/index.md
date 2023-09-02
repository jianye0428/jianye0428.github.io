# Effective STL [36] | 了解copy_if的正确实现


## STL中copy算法

STL有很多有趣的地方，其中一个是虽然有11个名字带“copy”的算法，但没有一个是copy_if。

- copy
- copy_backward replace_copy
- reverse_copy
- replace_copy_if
- unique_copy
- remove_copy
- rotate_copy remove_copy_if
- partial_sort_copy
- unintialized_copy

如果你只是简单地想要拷贝一个区间中满足某个判断式的元素，你只能自己做。

## 自己实现

假设你有一个函数来决定一个Widget是否有缺陷的：

```c++
bool isDefective(const Widget& w);
```

而且你希望把一个vector中所有有缺陷的Widget写到cerr。如果存在copy_if，你可以简单地这么做：

```c++
vector<Widget> widgets;
...
// 这无法编译：STL中并没有copy_if
copy_if(widgets.begin(), widgets.end(), ostream_iterator<Widget>(cerr, "\n"), isDefective);
```

**一个不很正确的copy_if实现**

这里有一个合理的看待copy_if：

```c++
template<typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator begin, InputIterator end, OutputIterator destBegin, Predicate p) {
 return remove_copy_if(begin, end, destBegin, not1(p));
}
```

虽然STL并没有让你说“拷贝每个判断式为true的东西”，但它的确让你说了“拷贝除了判断式不为true以外的每个东西”。

要实现copy_if，似乎我们需要做的就只是加一个not1在我们希望传给copy_if的判断式前面，然后把这个结果判断式传给remove_copy_if，结果就是上面的代码。

如果上面的理由有效，我们就可以用这种方法写出有缺陷的Widget：

```c++
// 不会编译的善意代码
copy_if(widgets.begin(), widgets.end(), ostream_iterator<Widget>(cerr, "\n"),  isDefective);
```

你的STL平台将会敌视这段代码，因为它试图对isDefective应用not1（这个应用出现在copy_if内部）。

**`not1`不能直接应用于一个函数指针，函数指针必须先传给ptr_fun。要调用这个copy_if实现，你必须传递的不仅是一个函数对象，而且是一个可适配的函数对象。**

标准STL算法从来不要求它们的仿函数是可适配的，copy_if也不应该要求。

## 正确的实现

```c++
template<typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator begin, InputIterator end, OutputIterator destBegin, Predicate p) {
    while (begin != end) {
        if (p(*begin))
            *destBegin++ == *begin;
        ++begin;
    }
    return destBegin;
}
```

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_36/  

