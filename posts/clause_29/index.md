# Effective STL [29] | 需要一个一个字符输入时考虑使用istreambuf_iterator


## istream_iterator 拷贝文件

假设要把一个文本文件拷贝到一个字符串对象中。似乎可以用一种很有道理的方法完成：
```C++
ifstream inputFile("interestingData.txt");
string fileData((istream_iterator<char>(inputFile)), istream_iterator<char>());
// 把inputFile读入fileData；
// 关于为什么它不是很正确请看下文关于这个语法的警告参见条款6
```

很快你就会发现这种方法无法把文件中的空格拷贝到字符串中。

那是因为`istream_iterators`使用`operator>>`函数来进行真的读取，而且`operator>>`函数在默认情况下忽略空格。

假如你想保留空格，你要的做就是覆盖默认情况。只要清除输入流的skipws标志就行了：

```C++
ifstream inputFile("interestingData.txt");
inputFile.unset(ios::skipws);
// 关闭inputFile的忽略空格标志
string fileData((istream_iterator<char>(inputFile)), istream_iterator<char>());
```

现在inputFile中的所有字符都拷贝到fileData中了。

**拷贝速度**

你会发现它们的拷贝速度不像你想象的那么快。

`istream_iterators`所依靠的`operator>>`函数进行的是格式化输入，这意味着每次你调用的时候它们都必须做大量工作：

1. 必须建立和销毁岗哨（sentry）对象（为每个`operator>>`调用进行建立和清除活动的特殊的`iostream`对象）；

2. 必须检查可能影响它们行为的流标志（比如skipws）；

3. 必须进行全面的读取错误检查；

4. 如果它们遇到问题，它们必须检查流的异常掩码来决定是否该抛出一个异常。

如果进行格式化输入，那些都是重要的活动，但如果你需要的只是从输入流中抓取下一个字符，那就过度了。

## istreambuf_iterators 拷贝文件

你可以像`istream_iterator`一样使用`istreambuf_iterator`，但`istream_iterator<char>`对象使用`operator>>`来从输入流中读取单个字符。 `istreambuf_iterator`对象进入流的缓冲区并直接读取下一个字符。（更明确地说，`istreambuf_iterator<char>` 对象从一个`istream s`中读取会调用`s.rdbuf()->sgetc()`来读`s`的下一个字符。）把我们的文件读取代码改为使用`istreambuf_iterator`相当简单，大多数Visual Basic程序员都可以在两次尝试内做对：

```C++
ifstream inputFile("interestingData.txt");
string fileData((istreambuf_iterator<char>(inputFile)),
istreambuf_iterator<char>());
```

注意这里不需要“`unset`” `skipws`标志，`istreambuf_iterator`不忽略任何字符。它们只抓取流缓冲区的下一个字符。

相对于`istream_iterator`，它们抓取得更快。

如果你需要一个一个地读取流中的字符，你不需要格式化输入的威力，你关心的是它们花多少时间来读取流，和明显的性能提高相比，为每个迭代器多键入三个字符的代价是微弱的。对于无格式的一个一个字符输入，你总是应该考虑使用`istreambuf_iterator`。

## 结论

当你了解它之后，你也应该考虑把`ostreambuf_iterator`用于相应的无格式一个一个字符输出的作。

它们没有了`ostream_iterator`的开销（和灵活性），所以它们通常也做得更好。



---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_29/  

