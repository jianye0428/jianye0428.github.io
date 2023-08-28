# Effective STL 精度总结 [7] | 在程序中使用STL



# 前言

> Effective-STL总结系列分为七部分，本文为第七部分，涉及原书第七章，内容范围Rule43~50。为方便书写，Rule43简写为R43。

{{< admonition Note "Effective-STL系列List" false >}}
本博客站点系列内容如下：</br>
💡 [Effective STL(第3版)精读总结(一)](https://jianye0428.github.io/posts/partone/)</br>
💡 [Effective STL(第3版)精读总结(二)](https://jianye0428.github.io/posts/parttwo/)</br>
💡 [Effective STL(第3版)精读总结(三)](https://jianye0428.github.io/posts/partthree/)</br>
💡 [Effective STL(第3版)精读总结(四)](https://jianye0428.github.io/posts/partfour/)</br>
{{< /admonition >}}


## R43 算法调用优先于手写的循环

调用算法优于手写循环：
  - 效率：**算法**比手写的循环**效率更高**。
  - 正确性：手写循环比使用算法容易出错。
  - 可维护性：使用算法的代码更加简洁明了。

例子：P155，

算法的名称表明了它的功能，而 for、while、do 循环不能。

手写循环需要维护迭代器的有效性。



Ref:

[1]. https://www.cnblogs.com/Sherry4869/p/15162253.html
[2]. https://blog.csdn.net/zhuikefeng/article/details/108164117#t35
[3]. https://zhuanlan.zhihu.com/p/458156007

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/partseven/  

