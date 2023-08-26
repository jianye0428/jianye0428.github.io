# Effective STL [31] | 了解你的排序选择


## 稳定排序 VS 不稳定排序

有些排序算法是稳定的。在稳定排序中，如果一个区间中的两个元素有等价的值，它们的相对位置在排序后不改变。不稳定的算法没做这个保证。

例如，在（未排序的）widgets vector中Widget A在Widget B之前，而且两者都有相同的质量等级，那么稳定排序算法会保证在这个vector排序后，Widget A仍然在Widget B之前。

## `partial_sort`

sort是个令人称赞的算法，如果不需要完全排序时，比如有一个存有Widget 的vector，你想选择20个质量最高的Widget发送给客户，20个之外的Widget可以保持无序，也就是你需要的是部分排序，这时就可以用partial_sort。

```c++
bool qualityCompare(const Widget& lhs, const Widget& rhs) {
    // 返回lhs的质量是不是比rhs的质量好
}
...
// 把最好的20个元素（按顺序）放在widgets的前端
partial_sort(widgets.begin(),widgets.begin() + 20, widgets.end(), qualityCompare);
// 使用widgets...
...
```

调用完partial_sort后，widgets的前20个元素是容器中最好的而且它们按顺序排列，质量最高的Widget是widgets[0]，第二高的是widgets[1]等。

partial_sort是不稳定的。

## nth_element

如果你不关心哪个Widget给哪个客户，你需要的只是任意顺序的20个最好的Widget。STL中nth_element可以精确地完成了你需要的。

nth_element排序一个区间，在n位置（你指定的）的元素是如果区间被完全排序后会出现在那儿的元素。

**Example**

```c++
randy[n];
//默认求第m大的元素
std::nth_element(randy, randy+m, randy+n);

//定义cmp可求第m小的元素
bool cmp(int a, int b){
 return a>b;
}
std::nth_element(randy, randy+m, randy+n, cmp);
```

函数是将第 m 大的元素放在 arr 数组数组中适当位置，其他元素按照第 m 元素的大小划分。 在[ 0, n ]这个范围内，在第 m 个元素之前的元素都小于或等于第 m 个元素，而且第 m 个元素后面的每个元素都会比它大。

`nth_element()`函数仅将第 m 大/小的数在 randy 数组中排好了位置，并不返回值。输出 randy[m] 即是第 m 大/小的数。

**排序Widget数组**

使用nth_element来保证最好的20个Widget在widgets vector的前端：

```c++
nth_element(widgets.begin(), widgets.begin() + 19, widgets.end(), qualityCompare);
// 把最好的20个元素放在widgets前端，但不用担心它们的顺序
```

调用nth_element本质上等价于调用partial_sort，两个算法都把20个质量最高的Widget移动到vector前端。

它们结果的唯一区别是partial_sort排序了在位置1-20的元素，而nth_element不排序。

**其他用法**

除了能帮你找到区间顶部的n个元素，它也可以用于找到区间的中值或者找到在指定百分点的元素：

```c++
// 迭代器的变量方便地表示widgets的起点和终点
vector<Widget>::iterator begin(widgets.begin());
vector<Widget>::iterator end(widgets.end());

// 这个迭代器指示了下面代码要找的中等质量等级的Widget的位置
vector<Widget>::iterator goalPosition;

// 兴趣的Widget会是有序的vector的中间
goalPosition = begin + widgets.size() / 2;

// 找到widgets中中等质量等级的值
nth_element(begin, goalPosition, end, qualityCompare);
// goalPosition现在指向中等质量等级的Widget
...

// 下面的代码能找到质量等级为75%的Widget
// 指出兴趣的Widget离开始有多远
vector<Widget>::size_type goalOffset = 0.25 * widgets.size();
// 找到质量值为75%的Widget
nth_element(begin, begin + goalOffset, end, qualityCompare);
// begin + goalOffset现在指向质量等级为75%的Widget
```

## 元素有同样质量

假设有12个元素质量是1级（可能是最好的），15个元素质量是2级（第二好的）。在这种情况下，选择20个最好的Widget就是选择12个1级的和15个中的8个2级的。partial_sort和nth_element怎么判断15个中的哪些要放到最好的20个中？对于这个问题，当多个元素有等价的值时sort怎么判断元素的顺序？

partial_sort和nth_element以任何它们喜欢的方式排序值等价的元素，而且你不能控制它们在这方面行为。

nth_element、sort也没有提供稳定性。

## stable_sort

stable_sort 是稳定排序，STL并不包含partial_sort和nth_element的稳定版本。

当指定范围内包含多个相等的元素时，sort() 排序函数无法保证不改变它们的相对位置。那么，如果既要完成排序又要保证相等元素的相对位置，可以使用stable_sort()函数

stable_sort() 函数完全可以看作是 sort() 函数在功能方面的升级版。stable_sort() 和 sort() 具有相同的使用场景，就连语法格式也是相同的（后续会讲），只不过前者在功能上除了可以实现排序，还可以保证不改变相等元素的相对位置。

```C++
//对 [first, last) 区域内的元素做默认的升序排序
void stable_sort ( RandomAccessIterator first, RandomAccessIterator last );
//按照指定的 comp 排序规则，对 [first, last) 区域内的元素进行排序
void stable_sort ( RandomAccessIterator first, RandomAccessIterator last, Compare comp );
```

```C++
#include <iostream>     // std::cout
#include <algorithm>    // std::stable_sort
#include <vector>       // std::vector
//以普通函数的方式实现自定义排序规则
bool randy_comp(int i, int j) {
    return (i < j);
}
//以函数对象的方式实现自定义排序规则
class kim_comp {
public:
    bool operator() (int i, int j) {
        return (i < j);
    }
};
int main() {
  std::vector<int> jeff_num{88, 13, 2, 22, 1, 30, 3, 33};

  std::stable_sort(jeff_num.begin(),  jeff_num.begin() + 4);  // 2 13 22 88 1 30 3 33

  //利用STL标准库提供greater<T>进行排序
  std::stable_sort(jeff_num.begin(), jeff_num.begin() + 4, std::greater<int>());  // 88 22 13 2 1 30 3 33

  //通过自定义比较规则进行排序,这里也可以换成 kim_comp()
  std::stable_sort(jeff_num.begin(), jeff_num.end(), randy_comp);  // 1 2 3 13 22 30 33 88

  for (std::vector<int>::iterator it = jeff_num.begin(); it != jeff_num.end(); ++it) {
    std::cout << *it << ' ';
  }
  std::cout << std::endl;
  return 0;
}
```

## partition

但是完全排序需要很多工作，而且对于这个任务做了很多不必要的工作。一个更好的策略是使用partition算法，它重排区间中的元素以使所有满足某个标准的元素都在区间的开头。

比如，移动所有质量等级为2或更好的Widget到widgets前端：

```c++
bool hasAcceptableQuality(const Widget& w)
{
 // 返回w质量等级是否是2或更高;
}

// 把所有满足hasAcceptableQuality的widgets移动到widgets前端，
// 并且返回一个指向第一个不满足的widget的迭代器
vector<Widget>::iterator goodEnd = partition(widgets.begin(), widgets.end(), hasAcceptableQuality);
```

此调用完成后，从widgets.begin()到goodEnd的区间容纳了所有质量是1或2的Widget，从goodEnd到widgets.end()的区间包含了所有质量等级更低的Widget。

如果在分割时保持同样质量等级的Widget的相对位置很重要，我们自然会用stable_partition来代替partition。

**list排序**

唯一我们可能会但不能使用`sort`、`stable_sort`、`partial_sort`或`nth_element`的容器是`list`，`list`通过提供`sort`成员函数做了一些补偿。（有趣的是，list::sort提供了<u>稳定排序</u>。）

如果你想要对list中的对象进行partial_sort或nth_element，你必须间接完成：

1. 把元素拷贝到一个支持随机访问迭代器的容器中，然后对它应用需要的算法;
2. 建立一个list::iterator的容器，对那个容器使用算法，然后通过迭代器访问list元素;
3. 使用有序的迭代器容器的信息来迭代地把list的元素接合到你想让它们所处的位置。

## 总结

**算法sort、stable_sort、partial_sort和nth_element需要随机访问迭代器，所以它们可能只能用于vector、string、deque和数组。**

对标准关联容器排序元素没有意义，因为这样的容器使用它们的比较函数来在任何时候保持有序。

partition和stable_partition与sort、stable_sort、partial_sort和nth_element不同，它们只需要双向迭代器。因此你可以在任何标准序列迭代器上使用partition和stable_partition。

1. 如果需要在vector、string、deque或数组上进行**完全排序**，你可以使用`sort`或`stable_sort`。
2. 如果你有一个vector、string、deque或数组，**只需要排序前n个元素**，应该用`partial_sort`。
3. 如果你有一个vector、string、deque或数组，**需要鉴别出第n个元素或你需要鉴别出最前的n个元素，而不用知道它们的顺序**，nth_element是你应该注意和调用的。
4. 如果你需要**把标准序列容器的元素或数组分隔为满足和不满足某个标准**，你大概就要找partition或stable_partition。
5. 如果你的数据是在list中，你可以直接使用partition和stable_partition，你可以使用list的sort来代替sort和stable_sort。如果你需要partial_sort或nth_element提供的效果，你就必须间接完成这个任务。

你可以通过把数据放在标准关联容器中的方法以**保持在任何时候东西都有序**。你也可能会考虑标准非STL容器priority_queue，它也可以总是保持它的元素有序。

一般来说，<font color=blue>做更多工作的算法比做得少的要花更长时间，而必须稳定排序的算法比忽略稳定性的算法要花更长时间</font>。

本节讨论的算法需要更少资源（时间和空间）的算法排序：

1. partition
2. partial_sort
3. stable_partition
4. sort
5. nth_element
6. stable_sort


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_31/  

