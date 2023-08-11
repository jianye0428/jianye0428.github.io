# Effective STL [17] | 使用“交换技巧”来修整过剩容量(仅适用于vector和string)


> 注意: 这里的容量指的是capacity

假如有一个海选，需要从申请者中挑选一些人进入到下一轮比赛当中，我们可能会建立一个vector用来存储申请者：

```c++
class Contestant {...};
vector<Contestant> contestants;
```

`vector`会很快获得很多元素，你可能会将能够进入到下一轮的申请者放到 vector 的前端（可能通过 `partial_sort`或`partition`），如果没有参与到下一轮的申请者就会从vector 中删除（典型的方法就是调用 erase 的区间形式，具体见条款5）。

这样很好地减少了 vector 的大小，**但是没有减少它的容量**。

当然可以通过resize方法去改变，但是如果我不知道大概要多少，我只知道有一些符合我的要求，如果直接用resize可能大了很多，或者抛弃了原本符合要求的元素。所以最好有一种shrink_to_fit的方式。

```c++
vector<Contestant> contestants;
vector<Contestant>(contestants).swap(contestants);
```

<font color=Aqua>**具体工作原理：**</font>

 1. 表达式`vector<Contestant>(contestants)`建立一个临时`vector`，它是`contestants`的一份拷贝：`vector`的拷贝构造函数做了这个工作。**vector的拷贝构造函数只分配拷贝的元素需要的内存**，就是说`contestants`真实包含多少元素，就分配多少内存空间，按照`size()`的个数，而不是capicatiy个数，所以这个临时vector没有多余的容量。
 2. 让临时`vector`和`contestants`交换数据，这时contestants只有临时变量的修整过的容量(size)，而这个临时变量则持有了曾经在contestants中的发胀的容量(capacity)。在这里（这个语句结尾），临时vector被销毁，因此释放了以前contestants使用的内存。  收缩到合适。
 3. 完成收缩内存的同时，也拷贝了实际`size()`个内存，也有性能消耗。

**TEST**

```c++
int main() {
  std::vector<int> r1(100, 1); // r1 包含100个1，即100个申请者
  std::cout << "r1 size: " << r1.size() << " capacity: " << r1.capacity()
            << std::endl;
  r1.resize(50); // 只保留前50名申请者进入下一轮
  std::cout << "r1 size: " << r1.size() << " capacity: " << r1.capacity()
            << std::endl;
  std::vector<int>(r1).swap(r1); // shrink to fit
  std::cout << "r1 size: " << r1.size() << " capacity: " << r1.capacity()
            << std::endl;
  return 0;
}
```

**结果:**

```shell
r1 size: 100 capacity: 100
r1 size: 50 capacity: 100
r1 size: 50 capacity: 50
r1 size: 0 capacity: 0
```

string也同样适用：

```c++
vector<Contestant> v;
string s;
...
vector<Contest>().swap(v);	//清楚v并把它的容量变为最小
string().swap(s);			//清楚s并把它的容量变为最小
```

另外，在swap的时候，不仅仅容器内容被交换，其迭代器、指针和引用也被交换(string除外)。因此，在发生交换后，原来的迭代器、指针和引用依然有效，并指向同样的元素——但这些元素已经在另外一个容器中。

## 清空容器并减少容量到最小

**交换技巧的变体可以用于清除容器和减少它的容量到你的实现提供的最小值。**

可以简单地和一个默认构造的临时vector或string做个交换：

```c++
vector<Contestant> v;
string s;
... // 使用v和s
vector<Contestant>().swap(v); // 清除v而且最小化它的容量
string().swap(s); // 清除s而且最小化它的容量
```

测试代码：
```c++
std::vector<int> r1(100, 1); // r1 包含100个1，即100个申请者
std::cout << "r1 size: " << r1.size() << " capacity: " << r1.capacity()
          << std::endl;

std::vector<int>().swap(r1); // 清空 r1
std::cout << "r1 size: " << r1.size() << " capacity: " << r1.capacity()
          << std::endl;
```

结果:

```c++
r1 size: 100 capacity: 100
r1 size: 0 capacity: 0
```

ref:</br>
[1]. https://blog.csdn.net/u011058765/article/details/51205757?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-2-51205757-blog-18826.235%5Ev38%5Epc_relevant_anti_t3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-2-51205757-blog-18826.235%5Ev38%5Epc_relevant_anti_t3&utm_relevant_index=3



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_17/  

