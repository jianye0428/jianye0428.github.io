# Effective STL [24] | 当关乎效率时应该在map::operator[]和map-insert之间仔细选择


## Example map插入

假设有一个支持默认构造函数以及从一个double构造和赋值的Widget类:

```C++
class Widget {
public:
Widget();
Widget(double weight);
Widget& operator=(double weight);
...
}
```

假设我们想建立一个从int到Widget的map，而且我们想有初始化有特定值的映射:

```C++
map<int, Widget> m;
m[1] = 1.50;
m[2] = 3.67;
m[3] = 10.5;
m[4] = 45.8;
m[5] = 0.0003;
```

**map operator[] 工作原理**

map的`operator[]`函数是个奇怪的东西。它与vector、deque和string的`operator[]`函数无关，也和内建的数组operator[]无关。

`map::operator[]`被设计为简化“添加或更新”功能。即，给定`map<K, V> m`;这个表达式

```C++
m[k] = v;
```

**检查键k是否已经在map里。如果不，就添加上，以v作为它的对应值。如果k已经在map里，它的关联值被更新成v。**

这项工作的原理是`operator[]`返回一个与k关联的值对象的引用。然后v赋值给所引用（`从operator[]`返回的）的对象。

当要更新一个已存在的键的关联值时很直接，因为已经有`operator[]`可以用来返回引用的值对象。

但是如果k还不在map里，`operator[]`就没有可以引用的值对象。那样的话，它使用值类型的默认构造函数从头开始建立一个，然后`operator[]`返回这个新建立对象的引用。

让我们再次地看看原先例子的第一部分：

```C++
map<int, Widget> m;
m[1] = 1.50;
```

表达式`m[1]`是`m.operator[](1)`的简化，所以这是一个`map::operator[]`的调用。

这个函数必须返回一个Widget的引用，因为m 的映射类型是Widget。

在这里，m里面还没有任何东西，所以键1在map里没有入口。因此`operator[]`默认构造一个Widget来作为关联到1的值，然后返回到那个Widget的引用。

最后，Widget成为赋值目标：被赋值的值是1.50。

所以，下面这个语句

```C++
m[1] = 1.50;
```

**功能上等价于这个:**

```C++
// 方便的 typedef
typedef map<int, Widget> IntWidgetMap; 
// 用键1建立新映射入口和一个默认构造的值对象；
pair<IntWidgetMap::iterator, bool> result =
 m.insert(IntWidgetMap::value_type(1, Widget()));

// 赋值给值类型
result.first->second = 1.50;
```

**现在已经很清楚为什么这种方法可能降低性能了：先默认构造一个Widget，然后我们立即赋给它新值。**

**`map insert`更高效**

如果**用想要的值构造Widget比默认构造Widget然后进行赋值显然更高效**，就应该用直截了当的insert调用来替换operator[]的使用（包括它的构造加赋值）:

```C++
m.insert(IntWidgetMap::value_type(1, 1.50));
```

这与上面的那些代码有相同的最终效果，除了它**通常节省了3次函数调用**：
1. 一个建立临时的默认构造Widget对象
2. 一个销毁那个临时的对象
3. 一个对Widget的赋值操作。

这些函数调用越昂贵，你通过使用map-insert代替`map::operator[]`就能节省越多。

上面的代码利用了每个标准容器都提供的value_type typedef。这typedef没有什么特别重要的，但对于map和multimap（以及非标准容器的hash_map和hash_multimap——参见条款25），记住它是很重要的，容器元素的类型总是某种pair。

之前谈及的`operator[]`被设计为简化“添加或更新”功能，而且现在我们理解了当“增加”被执行时，`insert`比`operator[]`更高效。当做更新时，情形正好相反，也就是，当一个等价的键这已经在map里时。

```c++
// 使用operator[]来把k的值更新为v
m[k] = v;

// 来把k的值更新为v使用insert
m.insert(IntWidgetMap::value_type(k, v)).first->second = v;
```

## operator[] 与 insert 权衡

insert的调用需要`IntWidgetMap::value_type`类型的实参（即`pair<int, Widget>`），所以当调用insert时，我们必须构造和析构一个那种类型的对象。那耗费了一对构造函数和析构函数，也会造成一个Widget的构造和析构，因为`pair<int, Widget>`本身包含了一个Widget对象，**operator[]没有使用pair对象，所以没有构造和析构pair和Widget。**

{{<admonition Note "Note">}}
情形选择:
1. 因此出于对效率的考虑，当给`map`添加一个元素时，我们断定`insert`比`operator[]`好；
2. 从效率和美学考虑，当更新已经在`map`里的元素值时`operator[]`更好。
{{</admonition>}}

如果STL提供一个两全其美的函数，即，在句法上吸引人的包中的高效的“添加或更新”功能:

```c++
// 如果键k不再map m中；高效地把pair(k, v)添加到m中；
// 否则高效地把和k关联的值更新为v。
// 返回一个指向添加或修改的pair的迭代器
iterator affectedPair =
 efficientAddOrUpdate(m, k, v);
```

但是，在STL内没有像这样的函数，正如下面的代码所演示的，自己写一个并不难。那些注释总结了正在做什么，而且随后的段落也提供了一些附加的解释。

```c++
// map的类型KeyArgType和ValueArgtype是类型参数的原因请看下面
template<typename MapType, typename KeyArgType, typename ValueArgtype>
typename MapType::iterator efficientAddOrUpdate(MapType& m, const KeyArgType& k, const ValueArgtype& v) {
 // 找到k在或应该在哪里；
 typename MapType::iterator Ib = m.lower_bound(k);
    // 如果Ib指向一个pair, 它的键等价于k...
 if(Ib != m.end() && !(m.key_comp()(k, Ib->first))) {
      // 更新这个pair的值
      Ib->second = v;
      // 并返回指向pair的迭代器
      return Ib;
 } else{
      typedef typename MapType::value_type MVT;
       // 把pair(k, v)添加到m并// 返回指向新map元素的迭代器
      return m.insert(Ib, MVT(k, v));
 }
}
```

执行一个高效的增加或更新，我们需要能**找出k的值是否在map中; 如果是这样，那它在哪里; 如果不是，它该被插入哪里。**

这个工作是为`low_bound`量身定做的，所以在这里我们调用那个函数。确定lower_bound是否用我们要寻找的键找到了一个元素，我们对后半部分进行一个等价测试，一定要对map使用正确的比较函数: 通过`map::key_comp`提供的比较函数。等价测试的结果告诉我们应该进行增加还是更新。

如果是更新，代码很直截了当。插入分支更有趣，因为它使用了insert的“提示”形式。结构`m.insert(Ib，MVT(k，v))`“提示”了**`Ib`鉴别出了键等价于k的新元素正确的插入位置，而且保证如果提示正确，那么插入将在分摊的常数时间内发生，而不是对数时间**。在`efficientAddOrUpdate`里，我们知道Ib鉴别出了适当的插入位置，因此insert的调用被保证为是一次常数时间操作。

**KeyArgType和ValueArgType**

这个实现的一个有趣方面是`KeyArgType`和`ValueArgType`不必是储存在map里的类型。它们只需要可以转换到储存在map里的类型。

一个可选的方法是去掉类型参数`KeyArgType`和`ValueArgType`，改为使用`MapType::key_type`和`MapType::mapped_type`。

但是，如果我们那么做，**在调用时我们可能强迫发生不必要的类型转换**:

```c++
map<int, Widget> m;
// 别忘了Widget接受从一个double赋值：
class Widget {
public:
    ...
    Widget& operator=(double weight);
    ...
};
```

现在考虑`efficientAddOrUpdate`的调用:

```c++
efficientAddOrUpdate(m, 10, 1.5);
```

假设是一次更新操作，即，m已经包含键是10的元素。那样的话，上面的模板推断出`ValueArgType`是`double`，**函数体直接把1.5作为double赋给与10相关的那个Widget**。那是通过调用`Widget::operator(double)`完成的。

如果我们用了`MapType::mapped_type`作为`efficientAddOrUpdate`的第3个参数的类型，在调用时我们得把1.5转化成一个Widget，那样的话我们就得**花费本来不需要的一次Widget构造（以及随后的析构）**。

## 结论

当关乎效率时应该在map::operator[]和map-insert之间仔细选择。

<font color=red>如果你要更新已存在的map元素，operator[]更好，但如果你要增加一个新元素，insert则有优势。</font>

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_24/  

