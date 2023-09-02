# Effective STL [37] | 用accumulate或for_each来统计区间


## 操作区间的函数

有时候你需要把整个区间提炼成一个单独的数，或，更一般地，一个单独的对象。

对于一般需要的信息，`count`告诉你区间中有多少等于某个值的元素，而`count_if`告诉你有多少元素满足一个判断式。

区间中的最小和最大值可以通过`min_element`和`max_element`获得。

但有时，你需要用一些自定义的方式统计（summarize）区间，而且在那些情况中，你需要比`count`、`count_if`、`min_element`或`max_element`更灵活的东西。

你可能想要对一个容器中的字符串长度求和。你可能想要数的区间的乘积。你可能想要point区间的平均坐标。在那些情况中，**你需要统计一个区间，但你需要有定义你需要统计的东西的能力。**

## `accumulate`

`accumulate`和`inner_product`、`adjacent_difference`和`partial_sum`算法在头文件<numeric>中。

`accumulate`存在两种形式。

**带有一对迭代器和初始值的形式可以返回初始值加由迭代器划分出的区间中值的和**

```c++
// 建立一个list，放一些double进去
list<double> ld;

...
// 计算它们的和，从0.0开始
double sum = accumulate(ld.begin(), ld.end(), 0.0);
```

{{<admonition Note "注意">}}
意初始值指定为0.0，不是简单的0。
0.0的类型是double，所以accumulate内部使用了一个double类型的变量来存储计算的和。
{{</admonition>}}

如果这么写这个调用:

```c++
// 计算它们的和，从0开始；这不正确！
double sum = accumulate(ld.begin(), ld.end(), 0);
```

如果初始值是int 0，所以**accumulate内部就会使用一个int来保存它计算的值。那个int最后变成accumulate的返回值，而且它用来初始化和变量**。这代码可以编译和运行，但和的值可能不对。不是保存真的double的list的和，它可能保存了所有的double加起来的结果，但每次加法后把结果转换为一个int。

**输入迭代器**

`accumulate`只需要输入迭代器，所以你甚至可以使用`istream_iterator`和`istreambuf_iterator`

```c++
// 打印cin中 那些int的和
cout << "The sum of the ints on the standard input is"
         << accumulate(istream_iterator<int>(cin), istream_iterator<int>(), 0);
```

**带有一个初始和值与一个任意的统计函数**

比如，考虑怎么使用accumulate来计算容器中的字符串的长度和。要计算这个和，accumulate需要知道两个东 西。第一，**它必须知道和的开始**。在我们的例子中，它是0。第二，**它必须知道每次看到一个新的字 符串时怎么更新这个和**。要完成这个任务，我们写一个函数，它带有目前的和与新的字符串，而且返回更新的和:

```c++
// string::size_type的内容
 string::size_type stringLengthSum(string::size_type sumSoFar, const string& s) {
 return sumSoFar + s.size();
}
```

每个标准STL容器都有一个typedef叫做size_type，那是容器计量东西的类型。比如，这是容器的size函数的返回类型。对于所有的标准容器，size_type必须是size_t。

`stringLengthSum`是`accmulate`使用的统计函数的代表。它带有到目前为止区间的统计值和区间的下一个元素，它返回新的统计值。

```c++
set<string> ss;
// 建立字符串的容器，进行一些操作
...
// 把lengthSum设为对 ss中的每个元素调用stringLengthSum的结果，使用0作为初始统计值
string::size_type lengthSum =
      accumulate(ss.begin(), ss.end(), 0, stringLengthSum);
```

计算数值区间的积甚至更简单，因为我们不用写自己的求和函数。我们可以使用标准multiplies仿函数类:

```c++
// 建立float的容器
vector<float> vf;
// 进行一些操作
...

// 把product设为对vf中的每个元素调用， multiplies<float>的结果，用1.0f作为初始统计值
float product =
         accumulate(vf.begin(), vf.end(),1.0f, multiplies<float>()); //
```

这里唯一需要小心的东西是记得把`1.0f`作为初始统计值，而不是0。如果我们使用0作为开始值，结果会总是0，因为0乘以任何东西也是0。

**寻找point的区间的平均值**

寻找point的区间的平均值，point看起来像这样:

```c++
struct Point {
 Point(double initX, double initY): x(initX), y(initY) {}
 double x, y;
};
```
求和函数应该是一个叫做PointAverage的仿函数类的对象，但在我们察看PointAverage之前，让我们看看它在调用accumulate中的使用方法：

```c++
list<Point> lp;
...
// 对Ip中的point求平均值
Point avg = accumulate(lp.begin(), lp.end(), Point(0, 0), PointAverage());
```

初始和值是在原点的point对象，我们需要记得的是当计算区间的平均值时不要考虑那个点。

`PointAverage`通过记录它看到的point的个数和它们x和y部分的和的来工作。每次调用时，它更新那些值并返回目前检查过的point的平均坐标，因为它对于区间中的每个点只调用一次，它把x与y的和除以区间中的point的个数，忽略传给accumulate的初始point值，它就应该是这样：

```c++
class PointAverage : public binary_function<Point, Point, Point> {
 public:
  PointAverage() : numPoints(0), xSum(0), ySum(0) {}
  const Point operator()(const Point& avgSoFar, const Point& p) {
    ++numPoints;
    xSum += p.x;
    ySum += p.y;
    return Point(xSum / numPoints, ySum / numPoints);
  }

 private:
  size_t numPoints;
  double xSum;
  double ySum;
};
```

成员变量numPoints、xSum和ySum的修改造成了一个副作用，所以，技术上讲，上述例子展示的代码会导致结果未定义。

## for_each

`for_each`带有一个区间和一个函数（一般是一个函数对象）来调用区间中的每个元素，但传给`for_each`的函数只接收一个实参（当前的区间元素），而且当完成时`for_each`返回它的函数。（实际上，它返回它的函数的一个拷贝。

首先，`accumulate`的名字表示它是一个产生区间统计的算法，`for_each`听起来好像你只是要对区间的每个元素进行一些操作。

用`for_each`来统计一个区间是合法的，但是它没有accumulate清楚。

`accumulate`直接返回那些我们想要的统计值，而for_each返回一个函数对象，我们必须从这个对象中提取想要的统计信息。

在C++里，那意味着我们必须给仿函数类添加一个成员函数，让我们找回我们追求的统计信息。

```c++
struct Point {...};
// 同上
class PointAverage : public unary_function<Point, void> {
  // 参见条款40
public:
  PointAverage() : xSum(0), ySum(0), numPoints(0) {}
  void operator()(const Point& p) {
    ++numPoints;
    xSum += p.x;
    ySum += p.y;
  }
  Point result() const { return Point(xSum / numPoints, ySum / numPoints); }

private:
  size_t numPoints;
  double xSum;
  double ySum;
};
list<Point> lp;
Point avg = for_each(lp.begin(), lp.end(), PointAverage()).result;
```

就个人来说，我更喜欢用`accumulate`来统计，因为我认为它最清楚地表达了正在做什么，但是`for_each`也可以，而且不像`accumulate`，副作用的问题并不跟随`for_each`。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_37/  

