# Effective STL [11] | 理解自定义分配器的正确用法


<!-- {{< admonition quote "quote" false >}}
note abstract info tip success question warning failure danger bug example quote
{{< /admonition >}} -->

<!--more-->

## 自定义分配器

很多时候，你会有建立自定义分配器的想法：

- `allocator<T>`对线程安全采取了措拖，但是你只对单线程的程序感兴趣，你不想花费不需要的同步开销
- 在某些容器里的对象通常一同被使用，所以你想在一个特别的堆里把它们放得很近使引用的区域性最大化
- 你想建立一个相当共享内存的唯一的堆，然后把一个或多个容器放在那块内存里，因为这样它们可以被其他进程共享。

### 管理共享内存

假定你有仿效`malloc`和`free`的特别程序，用于管理共享内存的堆
```c++
void* mallocShared(size_t bytesNeeded);
void freeShared(void *ptr);
```

并且你希望能把STL容器的内容放在共享内存中:
```c++
template<typename T>
class SharedMemoryAllocator {
public:
...
    pointer allocate(size_type numObiects, const void *localityHint = 0)
    {
        return static_cast<pointer>(mallocShared(numObiects * sizeof(T)));
    }

    void deallocate(pointer ptrToMemory, size_type numObjects)
    {
        freeShared(ptrToMiemory);
    }
...
};
```

使用`SharedMemoryAllocator`：

```c++
// 方便的typedef
typedef vector<double, SharedMemoryAllocator<double> >
SharedDoubleVec;
...
{ // 开始一个块
SharedDoubleVec v; // 建立一个元素在
// 共享内存中的vector
... // 结束这个块
}
```

「问题：」**v**使用`SharedMemoryAllocator`，所以**v**分配来容纳它元素的内存将来自共享内存，但**v本身——包括它的全部数据成员——几乎将肯定不被放在共享内存里，v只是一个普通的基于堆的对象，所以它将被放在运行时系统为所有普通的基于堆的对象使用的任何内存**。那几乎不会是共享内存。</br>

为了把v的内容和v本身放进共享内存，必须这么做:
```c++
void *pVectorMemory = mallocShared(sizeof(SharedDoubleVec));// 分配足够的共享内存来容纳一个SharedDoubleVec对象
SharedDoubleVec *pv = new (pVectorMemory) SharedDoubleVec; // 使用“placement new”来 在那块内存中建立 一个SharedDoubleVec对象；
// 参见下面这个对象的使用（通过pv）
...
pv->~SharedDoubleVec(); // 销毁共享内存中的对象
freeShared(pVectorMemory); // 销毁原来的共享内存块
```
这就是`「手工的四步分配/建造/销毁/回收的过程」`：获得一些共享内存 `——>` 在里面建立一个用共享内存为自己内部分配的vector `——>` 用完这个vector时，调用它的析构函数 `——>` 释放vector占用的内存。

这段代码有2点需要注意：

- 忽略了`mallocShared`可能返回一个null指针。

- 共享内存中的vector的建立由“placement new”完成。

### 管理分配和回收的堆

假设有2个堆，类`Heap1`和`Heap2`。

每个堆类有用于进行「分配」和「回收」的「静态成员函数」：

```c++
class Heap1 {
public:
...
    static void* alloc(size_t numBytes, const void *memoryBlockToBeNear);
    static void dealloc(void *ptr);
...
};

class Heap2 { ... }; // 有相同的alloc/dealloc接口
```

你想在不同的堆里联合定位一些STL容器的内容。

首先，设计一个分配器，使用像Heap1和Heap2那样用于真实内存管理的类：

```c++
template<typenameT, typename Heap>
class SpecificHeapAllocator {
public:
    pointer allocate(size_type numObjects, const void *localityHint = 0) {
        return static_cast<pointer>(Heap::alloc(numObjects * sizeof(T), localityHint));
    }
    void deallocate(pointer ptrToMemory, size_type numObjects) {
        Heap::dealloc(ptrToMemory);
    }
...
};
```

然后，使用`SpecificHeapAllocator`来把容器的元素集合在一起：

```c++
vector<int, SpecificHeapAllocator<int, Heap1 >> v; // 把v和s的元素放进Heap1
set<int, SpecificHeapAllocator<int Heap1 >> s;
list<Widget, SpecificHeapAllocator<Widget, Heap2>> L; // 把L和m的元素 放进Heap2
map<int, string, less<int>, SpecificHeapAllocator<pair<const int, string>, Heap2>> m;
```
在这个例子里，很重要的一点是「**Heap1和Heap2是类型而不是对象**」。

STL为用不同的分配器对象初始化相同类型的不同STL容器提供了语法。那是「**因为如果Heap1和Heap2是对象而不是类型，那么它们将是不等价的分配器，那就违反了分配器的等价约束**」。

只要遵循「**相同类型的所有分配器都一定等价的限制条件**」，你将毫不费力地使用自定义分配器来「**控制一般内存管理策略，群集关系和使用共享内存以及其他特殊的堆**」。

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/clause_11/  

