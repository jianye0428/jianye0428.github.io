# Effective STL [34] | 注意哪个算法需要有序区间



不是所有算法可以用于任意区间。比如，**remove需要前向迭代器和可以通过这些迭代器赋值的能力**。所以，它不能应用于由输入迭代器划分的区间，也不能是map或multimap，也不能是set和multiset的一些实现。

同样，**很多排序算法需要随机访问迭代器**，所以不可能在一个list的元素上调用这些算法。

最常见的就是一些算法**需要有序值的区间**。无论何时都应该坚持这个需求，因为冒犯它不仅会导致编译器诊断，而且会造成未定义的运行期行为。

既可以和有序又可以和无序区间合作的算法很少，但**当操作有序区间的时候它们最有用**。

## 只能操作有序数据的算法的表

- binary_search
- lower_bound
- upper_bound
- equal_range
- set_union
- set_intersection
- set_difference
- set_symmetric_difference
- merge
- inplace_merge
- includes

## 一般用于有序区间，但不强制要求

- unique
- unique_copy

## `binary_search`、`lower_bound`、`upper_bound`和`equal_range`

搜索算法binary_search、lower_bound、upper_bound和equal_range需要**有序区间**，因为它们**使用二分法查找来搜索**值。像C库中的bsearch，这些算法保证了**对数时间的查找**，但作为交换的是，你必须给它们已经排过序的值。

实际上，仅当传给它们的是随机访问迭代器时它们才能保证有那样的性能。

如果给它们威力比较小的迭代器（比如双向迭代器），它们仍然进行对数次比较，但运行是线性时间的。那是因为，缺乏进行“**迭代器算术（arithmetic）**”的能力。它们在搜索的区间中需要花费线性时间来从一个地方移动到另一个地方。

## `union`、`set_intersection`、`set_difference`和`set_symmetric_difference`

算法`set_union`、`set_intersection`、`set_difference`和`set_symmetric_difference`的四人组提供了线性时间设置它们名字所提出的操作的性能。

为什么它们需要有序区间？因为如果不是的话，它们不能以线性时间完成它们的工作。

你会发现，**需要有序区间的算法为了比它们用于可能无序区间提供更好的性能而这么做**。

## merge和inplace_merge

merge和inplace_merge执行了有效的单遍合并排序算法：它们读取两个有序区间，然后产生一个包含了两个源区间所有元素的新有序区间。

它们以线性时间执行，如果它们不知道源区间已经有序就不能完成。

## includes

includes 用来检测是否一个区间的所有对象也在另一个区间中。

因为includes可能假设它的两个区间都已经有序，所以它保证了线性时间性能。没有那个保证，一般来说它会变慢。

## unique和unique_copy

unique和unique_copy甚至在无序区间上也提供了定义良好的行为。

看看标准是怎么描述unique的行为的：

> 从每个相等元素的连续组中去除第一个以外所有的元素。

如果你要unique从一个区间去除所有重复值（也就是，让区间中所有值“唯一”），你必须先确保所有重复值一个接着一个。那是排序完成的东西之一。

实际上，unique一般用于从区间中去除所有重复值，所以你几乎总是要确保你传递给unique（或unique_copy）的区间是有序的。Unix开发者会发现STL的unique和Unix的uniq之间有惊人的相似。

顺便说说，unique从一个区间除去元素的方式和remove一样，也就是说它只是区分出不除去的元素。

## 排序

因为STL允许你指定用于排序的比较函数，**不同的区间可能以不同的方式排序**。

比如，给定两个int的区间，一个可能以默认方式排序（也就是升序），而另一个使用`greater<int>`排序，因此是降序。

给定Widget的两个区间，一个可能以价格排序而另一个可能以年龄排序。因为有很多不同的方式来排序，所以保证给STL所使用的排序相关信息一致是很重要的。

**如果你传一个区间给一个也带有比较函数的算法，确保你传递的比较函数行为和你用于排序这个区间的一样。**

```c++
vector<int> v;
// 建立一个vector，把一些数据放进去
...
sort(v.begin(), v.end(), greater<int>()); // 降序排列

...
// 使用这个vector（没有改变它）
// 在这个vector中搜索5
bool is3Exists =
binary_search(v.begin(), v.end(), 3); // 假设它是升序排列！
```

默认情况下，binary_search假设它搜索的区间是以“<”排序（也就是，值是升序），但在本例中，这个vector是降序。当你在值的排列顺序和算法所期望的不同的区间上调用binary_search (或lower_bound等）会导致未定义的结果。

### 正确排序方式

要让代码行为正确，你必须告诉binary_search要使用和sort同样的比较函数：

```c++
bool is3Exists =
// 搜索3
binary_search(v.begin(), v.end(), 3, greater<int>());
// 比较函数把greater作为
```

<font color=blue>所有需要有序区间的算法（也就是除了unique和unique_copy外本条款的所有算法）通过等价来判断两个值是否“相同”，就像标准关联容器（它们本身是有序的）。相反，unique和unique_copy判断两个对象“相同”的默认方式是通过相等</font>，但是你可以通过传给这些算法一个定义了“相同”的意义的判断式来覆盖这个默认情况。

## 总结

11个需要有序区间的算法为了比其他可能性提供更好的性能而<font color=blue>需要传给它们有序区间，需要保证用于算法的比较函数和用于排序的一致</font>。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_34/  

