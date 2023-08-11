# Effective STL [18] | 避免使用vector<bool>


## vector<bool> 问题

作为一个STL容器，vector<bool>确实只有1个问题:
- 它不是一个STL容器；
- 它并不容纳bool。

## 剖析

一个东西要成为STL容器就必须满足所有在C++标准23.1节中列出的容器必要条件。

如果c是一个T类型对象的容器，且c支持operator[]，那么以下代码必须能够编译:

```c++
T *p = &c[0]; // 无论operator[]返回什么，都可以用这个地址初始化一个T*
```

换句话说，如果你使用`operator[]`来得到Container中的一个T对象，你可以通过取它的地址而获得指向那个对象的指针。(假设T没有倔强地重载一些操作符。) 然而如果vector是一个容器，这段代码必须能够编译：

```c++
vector<bool> v;
bool *pb = &v[0]; // 用vector<bool>::operator[]返回的东西的地址初始化一个bool*
```

但它不能编译。

```shell
test_vector.cpp:154:19: error: taking address of rvalue [-fpermissive]
  154 |   bool *pb = &r2[0];
      |                   ^
test_vector.cpp:154:14: error: cannot convert ‘std::vector<bool>::reference*’ {aka ‘std::_Bit_reference*’} to ‘bool*’ in initialization
  154 |   bool *pb = &r2[0];
      |              ^~~~~~
      |              |
      |              std::vector<bool>::reference* {aka std::_Bit_reference*}
```

因为`vector<bool>`是一个伪容器，并不保存真正的bool，而是打包bool以节省空间。在一个典型的实现中，每个保存在“vector”中的“bool”占用一个单独的比特，而一个8比特的字节将容纳8个“bool”。

在内部，`vector<bool>`使用了与位域（bitfield）等价的思想来表示它假装容纳的bool。

<font color=blue>位域也只表现为两种可能的值，但真的bool和化装成bool的位域之间有一个重要的不同：你可以创建指向真的bool的指针，但却禁止有指向单个比特的指针。</font>

## 实际实现

vector::operator[]返回一个对象，其行为类似于比特的引用，也称为代理对象。

```c++
 template<typename _Alloc>
    class vector<bool, _Alloc> : protected _Bvector_base<_Alloc>
    {
      typedef _Bvector_base<_Alloc>   _Base;
      typedef typename _Base::_Bit_pointer  _Bit_pointer;
      typedef typename _Base::_Bit_alloc_traits  _Bit_alloc_traits;

#if __cplusplus >= 201103L
      friend struct std::hash<vector>;
#endif

    public:
      typedef bool     value_type;
      typedef size_t     size_type;
      typedef ptrdiff_t     difference_type;
      typedef _Bit_reference    reference;
      typedef bool     const_reference;
      typedef _Bit_reference*    pointer;
      typedef const bool*    const_pointer;
      typedef _Bit_iterator    iterator;
      typedef _Bit_const_iterator   const_iterator;
      typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
      typedef std::reverse_iterator<iterator>  reverse_iterator;
      typedef _Alloc     allocator_type;

      allocator_type
      get_allocator() const
      { return _Base::get_allocator(); }

    protected:
      using _Base::_M_allocate;
      using _Base::_M_deallocate;
      using _Base::_S_nword;
      using _Base::_M_get_Bit_allocator;

         reference
      operator[](size_type __n)
      {
 return *iterator(this->_M_impl._M_start._M_p
    + __n / int(_S_word_bit), __n % int(_S_word_bit));
      }

      const_reference
      operator[](size_type __n) const
      {
 return *const_iterator(this->_M_impl._M_start._M_p
        + __n / int(_S_word_bit), __n % int(_S_word_bit));
      }
    }
```

上述代码不能编译的原因很明显：`bool *pb = &v[0]`右边的表达式是`vector<bool>::reference*`类型，不是`bool*`。

## 替代品

### 1. deque<bool>

`deque`提供了几乎所有`vector`所提供的（唯一值得注意的是`reserve`和`capacity`），而`deque<bool>`是一个STL容器，它保存真正的`bool`值。

`deque`内部内存不是连续的。所以不能传递`deque<bool>`中的数据给一个希望得到`bool`数组的`C API`。

条款16中用于`vector`的技术不能在`vector<bool>`上通过编译，因为它们依赖于能够取得指向容器中包含的元素类型的指针。

### 2. bitset

`bitset`不是一个STL容器，但它是C++标准库的一部分。与STL容器不同，**它的大小（元素数量）在编译期固定**，因此它不支持插入和删除元素。

因为它不是一个STL容器，它也不支持iterator。但就像`vector<bool>`，**它使用一个压缩的表示法，使得它包含的每个值只占用一比特。**

它提供`vector<bool>`特有的`flip`成员函数，还有一系列其他操作位集`(collection of bits)`所特有的成员函数。

## 总结

`vector<bool>`不满足STL容器的必要条件，你最好不要使用它；

而`deque<bool>`和`bitset`是基本能满足你对`vector<bool>`提供的性能的需要的替代数据结构。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_18/  

