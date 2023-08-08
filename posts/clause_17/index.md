# Effective STL [17] | 使用“交换技巧”来修整过剩容量(仅适用于vector和string)


> 注意: 这里的容量指的是capacity

先举个例子，先创建个容器，然后获得了很多元素。

```c++
class Contestant {...};
vector<Contestant> contestants;
```

但是很快发现大部分元素是没有用的，那就把元素删了，但是空间仍然还是那么大，怎么办呢？</br>

当然可以通过resize方法去改变，但是如果我不知道大概要多少，我只知道有一些符合我的要求，如果直接用resize可能大了很多，或者抛弃了原本符合要求的元素。所以最好有一种shrink_to_fit的方式。

```c++
vector<Contestant> contestants;
vector<Contestant>(contestants).swap(contestants);
```
　
表达式`vector<Contestant>(contestants)`建立一个临时vector，它是contestants的一份拷贝: vector的拷贝构造函数做了这个工作。但是，vector的拷贝构造函数只分配拷贝的元素需要的内存，所以这个临时vector没有多余的容量。然后我们让临时vector和contestants交换数据，这时我们完成了，contestants只有临时变量的修整过的容量，而这个临时变量则持有了曾经在contestants中的发胀的容量。在这里（这个语句结尾），临时vector被销毁，因此释放了以前contestants使用的内存。同样的技巧可以应用于string。交换技巧的变体可以用于清除容器和减少它的容量到你的实现提供的最小值。


string也同样适用：

```c++
vector<Contestant> v;
string s;
...
vector<Contest>().swap(v);	//清楚v并把它的容量变为最小
string().swap(s);			//清楚s并把它的容量变为最小
```

另外，在swap的时候，不仅仅容器内容被交换，其迭代器、指针和引用也被交换(string除外)。因此，在发生交换后，原来的迭代器、指针和引用依然有效，并指向同样的元素——但这些元素已经在另外一个容器中。

ref:</br>
[1]. https://blog.csdn.net/u011058765/article/details/51205757?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-2-51205757-blog-18826.235%5Ev38%5Epc_relevant_anti_t3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-2-51205757-blog-18826.235%5Ev38%5Epc_relevant_anti_t3&utm_relevant_index=3



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_17/  

