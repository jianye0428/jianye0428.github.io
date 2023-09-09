# More Effective C++ 阅读笔记


## 一、 基础议题(Basics)

### 条款 1：仔细区别 pointers和references

没有所谓的 null reference。一个 reference 必须总代表某个对象。所以如果你有一个变量，其目的是用来指向（代表）另一个对象，但是也有可能它不指向（代表）任何对象，那么你应该使用 pointer，因为你可以将 pointer设为 null。换个角度看，如果这个变量总是必须代表一个对象，也就是说如果你的设计并不允许这个变量为 null，那么你应该使用reference。

Pointers 和 references 之间的另一个重要差异就是，pointers 可以被重新赋值，指向另一个对象，reference 却总是指向（代表）它最初获得的那个对象。

一般而言，当你需要考虑“不指向任何对象”的可能性时，或是考虑“在不同时间指向不同对象”的能力时，你就应该采用 pointer。前一种情况你可以将 pointer设为 null，后一种情况你可以改变pointer 所指对象。而当你确定“总是会代表某个对象”，而且“一旦代表了该对象就不能够再改变”，那么你应该选用 reference。

当你知道你需要指向某个东西，而且绝不会改变指向其他东西，或是当你实现一个操作符而其语法需求无法由 pointers 达成，你就应该选择 references。任何其他时候，请采用 pointers。

**在任何情况下都不能使用指向空值的引用**。一个引用必须总是指向某些对象。在C++里，引用应被初始化。

不存在指向空值的引用这个事实意味着使用引用的代码效率比使用指针的要高。因为在使用引用之前不需要测试它的合法性。
指针与引用的另一个重要的不同是指针可以被重新赋值以指向另一个不同的对象。但是引用则总是指向在初始化时被指定的对象，以后不能改变。

关于引用的更多介绍参考：https://blog.csdn.net/fengbingchun/article/details/69820184

```c++

void printDouble(const double& rd)
{
	std::cout<<rd; // 不需要测试rd,它肯定指向一个double值
}

void printDouble(const double* pd)
{
	if (pd) { // 检查是否为NULL
		std::cout<<*pd;
	}
}

int test_item_1()
{
	char* pc = 0; // 设置指针为空值
	char& rc = *pc; // 让指针指向空值，这是非常有害的，结果将是不确定的

	//std::string& rs; // 错误，引用必须被初始化
	std::string s("xyzzy");
	std::string& rs = s; // 正确,rs指向s
	std::string* ps; // 未初始化的指针，合法但危险

{
	std::string s1("Nancy");
	std::string s2("Clancy");
	std::string& rs = s1; // rs引用s1
	std::string* ps = &s1; // ps指向s1
	rs = s2; // rs仍旧引用s1,但是s1的值现在是"Clancy"
	ps = &s2; // ps现在指向s2,s1没有改变
}

	std::vector<int> v(10);
	v[5] = 10; // 这个被赋值的目标对象就是操作符[]返回的值，如果操作符[]
		   // 返回一个指针，那么后一个语句就得这样写: *v[5] = 10;

	return 0;
}
```

### 条款 2：最好使用 C++转型操作符

C++通过引进四个新的类型转换(cast)操作符克服了C风格类型转换的缺点(过于粗鲁，能允许你在任何类型之间进行转换；C风格的类型转换在程序语句中难以识别)，这四个操作符是：`static_cast`、`const_cast`、`dynamic_cast`、`reinterpret_cast`。

static_cast在功能上基本上与C风格的类型转换一样强大，含义也一样。它也有功能上限制。例如，不能用static_cast像用C 风格的类型转换一样把struct转换成int类型或者把double类型转换成指针类型，另外，static_cast不能从表达式中去除const属性，因为另一个新的类型转换操作符const_cast有这样的功能。

const_cast用于类型转换掉表达式的const或volatileness属性。如果你试图使用const_cast来完成修改constness或者volatileness属性之外的事情，你的类型转换将被拒绝。

**dynamic_cast**被用于安全地沿着类的继承关系向下进行类型转换。这就是说，你能用dynamic_cast把指向基类的指针或引用转换成指向其派生类或其兄弟类的指针或引用，而且你能知道转换是否成功。失败的转换将返回空指针(当对指针进行类型转换时)或者抛出异常(当对引用进行类型转换时)。dynamic_cast在帮助你浏览继承层次上是有限制的，它不能被用来缺乏虚函数的类型上，也不能用它来转换掉constness。如你想在没有继承关系的类型中进行转换，你可能想到static_cast。如果是为了去除const，你总得用const_cast。

reinterpret_cast使用这个操作符的类型转换，其转换结果几乎都是执行期定义(implementation-defined)。因此，使用reinterpret_cast的代码很难移植。此操作符最普通的用途就是在函数指针之间进行转换。

关于类型转换更多介绍参考：https://blog.csdn.net/fengbingchun/article/details/51235498

```c++
class Widget {
public:
	virtual void func() {}
};

class SpecialWidget : public Widget {
public:
	virtual void func() {}
};

void update(SpecialWidget* psw) {}
void updateViaRef(SpecialWidget& rsw) {}

typedef void (*FuncPtr)(); // FuncPtr是一个指向函数的指针
int doSomething() { return 1; };

int test_item_2()
{
	int firstNumber = 1, secondNumber = 1;
	double result1 = ((double)firstNumber) / secondNumber; // C风格
	double result2 = static_cast<double>(firstNumber) / secondNumber; // C++风格类型转换

	SpecialWidget sw; // sw是一个非const对象
	const SpecialWidget& csw = sw; // csw是sw的一个引用，它是一个const对象
	//update(&csw); // 错误，不能传递一个const SpecialWidget*变量给一个处理SpecialWidget*类型变量的函数
	update(const_cast<SpecialWidget*>(&csw)); // 正确，csw的const显示地转换掉(csw和sw两个变量值在update函数中能被更新)
	update((SpecialWidget*)&csw); // 同上，但用了一个更难识别的C风格的类型转换

	Widget* pw = new SpecialWidget;
	//update(pw); // 错误，pw的类型是Widget*，但是update函数处理的是SpecialWidget*类型
	//update(const_cast<SpecialWidget*>(pw)); // 错误，const_cast仅能被用在影响constness or volatileness的地方，不能用在向继承子类进行类型转换

	Widget* pw2 = nullptr;
	update(dynamic_cast<SpecialWidget*>(pw2)); // 正确，传递给update函数一个指针是指向变量类型为SpecialWidget的pw2的指针， 如果pw2确实指向一个对象，否则传递过去的将是空指针

	Widget* pw3 = new SpecialWidget;
	updateViaRef(dynamic_cast<SpecialWidget&>(*pw3)); // 正确，传递给updateViaRef函数SpecailWidget pw3指针，如果pw3确实指向了某个对象，否则将抛出异常

	//double result3 = dynamic_cast<double>(firstNumber) / secondNumber; // 错误，没有继承关系
	const SpecialWidget sw4;
	//update(dynamic_cast<SpecialWidget*>(&sw4)); // 错误，dynamic_cast不能转换掉const

	FuncPtr funcPtrArray[10]; // funcPtrArray是一个能容纳10个FuncPtr指针的数组
	//funcPtrArray[0] = &doSomething; // 错误，类型不匹配
	funcPtrArray[0] = reinterpret_cast<FuncPtr>(&doSomething); // 转换函数指针的代码是不可移植的(C++不保证所有的函数指针都被用一样的方法表示)，在一些情况下这样的转换会产生不正确的结果，所以应该避免转换函数指针类型

	return 0;
}
```

### 条款 3：绝对不要以多态（polymorphically）方式处理数组

C++允许你通过基类指针和引用来操作派生类数组。不过这根本就不是一个特性，因为这样的代码几乎从不如你所愿地那样运行。数组与多态不能用在一起。值得注意的是如果你不从一个具体类(concrete classes)(例如BST)派生出另一个具体类(例如BalancedBST)，那么你就不太可能犯这种使用多态性数组的错误。

**原因:**
- derived class 对象一般比base class对象小。所以当使用一个 base class数组存储derived class时，在访问 array[i] 时，会访问array+i*sizeof(base class)的地址，造成访问错误。
- 由base 指针删除一个derived class 数组，结果未定义。因为删除时，析构从最后一个开始，但是计算地址时跟tips1一样，造成访问错误。


```c++
class BST {
public:
	virtual ~BST() { fprintf(stdout, "BST::~BST\n"); }
private:
	int score;
};

class BalancedBST : public BST {
public:
	virtual ~BalancedBST() { fprintf(stdout, "BalancedBST::~BalancedBST\n"); }
private:
	int length;
	int size; // 如果增加此一个int成员，执行test_item_3会segmentation fault，注释掉此变量，运行正常
};

int test_item_3()
{
	fprintf(stdout, "BST size: %d\n", sizeof(BST)); // 16
	fprintf(stdout, "BalancedBST size: %d\n", sizeof(BalancedBST)); // 24

	BST* p = new BalancedBST[10];
	delete [] p; // 如果sizeof(BST) != sizeof(BalancedBST)，则会segmentation fault

	return 0;
}
```

### 条款4 避免无用的缺省构造函数

构造函数能初始化对象，而**缺省构造函数**则可以不利用任何在建立对象时的外部数据就能初始化对象。有时这样的方法是不错的。例如一些行为特性与数字相仿的对象被初始化为空值或不确定的值也是合理的，还有比如链表、哈希表、图等等数据结构也可以被初始化为空容器。但不是所有的对象都属于上述类型，对于很多对象来说，不利用外部数据进行完全的初始化是不合理的。比如一个没有输入姓名的地址薄对象，就没有任何意义。

利用指针数组代替一个对象数组这种方法有两个缺点：第一你必须删除数组里每个指针所指向的对象。如果忘了，就会发生内存泄漏。第二增加了内存分配量，因为正如你需要空间来容纳EquipmentPiece对象一样，你也需要空间来容纳指针。

对于类里没有定义缺省构造函数还会造成它们无法在许多基于模板(template-based)的容器类里使用。因为实例化一个模板时，模板的类型参数应该提供一个缺省构造函数。在多数情况下，通过仔细设计模板可以杜绝对缺省构造函数的需求。

```c++
class EquipmentPiece {
public:
	EquipmentPiece(int IDNumber) {}
};

int test_item_4()
{
	//EquipmentPiece bestPieces[10]; // 错误，没有正确调用EquipmentPiece构造函数
	//EquipmentPiece* bestPieces2 = new EquipmentPiece[10]; // 错误，与上面的问题一样

	int ID1 = 1, ID2 = 2;
	EquipmentPiece bestPieces3[] = { EquipmentPiece(ID1), EquipmentPiece(ID2) }; // 正确，提供了构造函数的参数

	// 利用指针数组来代替一个对象数组
	typedef EquipmentPiece* PEP; // PEP指针指向一个EquipmentPiece对象
	PEP bestPieces4[10]; // 正确，没有调用构造函数
	PEP* bestPieces5 = new PEP[10]; // 也正确
	// 在指针数组里的每一个指针被重新赋值，以指向一个不同的EquipmentPiece对象
	for (int i = 0; i < 10; ++i)
		bestPieces5[i] = new EquipmentPiece(ID1);

	// 为数组分配raw memory,可以避免浪费内存，使用placement new方法在内存中构造EquipmentPiece对象
	void* rawMemory = operator new[](10*sizeof(EquipmentPiece));
	// make bestPieces6 point to it so it can be treated as an EquipmentPiece array
	EquipmentPiece* bestPieces6 = static_cast<EquipmentPiece*>(rawMemory);
	// construct the EquipmentPiece objects in the memory使用"placement new"
	for (int i = 0; i < 10; ++i)
		new(&bestPieces6[i]) EquipmentPiece(ID1);
	// ...
	// 以与构造bestPieces6对象相反的顺序解构它
	for (int i = 9; i >= 0; --i)
		bestPieces6[i].~EquipmentPiece(); // 如果使用普通的数组删除方法，程序的运行将是不可预测的
	// deallocate the raw memory
	delete [] rawMemory;

	return 0;
}
```

## 二、运算符（操作符）

### 条款5: 谨慎定义类型转换函数

1. C++ 允许内置数据类型之间(例如char和int，int和double等)进行隐式转换，对于内置类型之间的隐式转换有详细的规则，但不管怎样，这些都是语言提供的，既相对安全，我们又无法更改。

对于自定义的类类型，隐式转换可以通过**单参数构造函数(single-argument constructors)**和**隐式类型转换操作符**来实现。所谓”单一自变量(单参数)指的是可以有多个参数，但除了第一个参数其他参数必须有默认实参)。所谓隐式类型转换操作符，是一个 member function：关键词operator 之后加一个类型名称，例如：`operator double() const`;

```c++
class Rational {
public:
	...
    operator double() const; // 将Rational 转换为 double
}
```

这个函数会在以下情况被自动调用：

```c++
Rational r(1, 2);  // r的值是 1/2
double d = 0.5 * r; // 将r的值转换为double，然后执行运算。
```

2. 但是下面这个情况就会出问题：`std::cout << r`;

如果你忘了为 Rational 类重载一个 operator<<，那么按道理应该打印不成功。但是编译器面对上述动作，它会想尽办法（包括找出一系列可接受的隐式类型转换）让函数调用动作成功。此时编译器发现 只需调用 Rational::operator double， 将 r 转换为 double，就可以成功调用 std::cout << r;，以浮点数的形式输出。

解决办法就是以功能对等的另一个函数取代类型转换操作符。即：定义一个 doube asDouble() const;函数。虽然使用时有些许不便，但“可因为不再默默调用那些不打算调用的函数而获得弥补”。C++ 标准库中的 string 类从没有 string 到 char* 的隐式类型转换操作符而采用 c_str 函数可能就是这个原因。

3. 拥有单个参数（或除第一个参数外都有默认值的多参数）构造函数的类，很容易被隐式类型转换，最好加上 explicit 防止隐式类型转换。
```c++
template<class T>
class Array{
public:
	Array(int size);
	T& operator[](int index);
};

bool operator==(const Array<int> &lhs, const Array<int> & rhs);
Array<int> a(10), b(10);
for(int i=0; i<10; ++i){
    if(a == b[i]){ //想要写 a[i] == b[i]，但是这时候编译器并不会报错
        //  do something
    }
    else{
        // do something
    }
}
```

`if(a == b[i])` 并不会报错。因为编译器发现只要调用 `Array\ constructor`（需一个 int 作为自变量），就可以把 int 转为 `Array\ object`。就会产生类似这样的代码：

`if( a == static_cast<Array<int> >(b[i]))` 将 `b[i]` 转为 `Array`。此时程序会正常运行，但是结果却不尽人意。

解决办法就是使用 C++ 特性：<font color=red>关键词 `explicit`</font>。这个特性之所以被导入，就是为了解决隐式类型转换带来的问题。`explict Array(int size)`;

还有一种被称为 proxy classes 的方法：

```c++
class Array {
public:
    class ArraySize { // 这个类是新的
    public:
    	ArraySize(int numElements):theSize(numElements){}
    	int size() const { return theSize;}
    private:
    	int theSize;
    };
	Array(int lowBound, int highBound);
	Array(ArraySize size); // 注意新的声明
...
};
```

这样写的代码在 Array\ a(10); 的时候，编译器会先通过类型转换将 int 转换成 ArraySize，然后再进行构造，虽然麻烦很多，效率也低了很多，但是在一定程度上可以避免隐式转换带来的问题。

对于自定义类型的类型转换，有一个规则："**没有任何一个转换程序可以内含一个以上的‘用户定制转换行为’(亦即单自变量constructor亦即隐式类型转换操作符)**"，也就是说，必要的时候编译器可以先进行内置类型之间的转换再调用带单自变量的构造函数或者先调用隐式类型转换操作符在进行内置类型之间的转换，但不可能连续进行两次用户定制的类型转换！

所以 此时 `if(a == b[i])` 就会报错。不能从 int 转换成 ArraySize，再从 ArraySize 转为 Array。

4. 总结允许编译器执行隐式转换弊大于利，所以**非必要不要提供转换函数**！

### 条款6: 区别 increment/decrement 操作符的前置和后置形式s

1. 由于 increment/decrement 操作符的前置和后置式都是一元运算符，没有参数。因此重载时通过在后置式中加一个 int 型参数(哑元参数)加以区分，当后置式被调用时，编译器自动在为该参数指定一个0值。

```c++
class UPInt{
public:
    UPInt& operator++();          // 前置式++
    const UPInt operator++(int);  // 后置式++
    UPInt& operator--();          // 前置式--
    const UPInt operator++(int);  // 前置式--
}
```
前置累加操作符和后置累加操作符实现:
```c++
// 前缀形式：增加然后取回值
UPInt& UPInt::operator++()
{
*this += 1; // 增加
return *this; // 取回值
}
// postfix form: fetch and increment
const UPInt UPInt::operator++(int)
{
UPInt oldValue = *this;
++(*this);
// 取回值
// 增加
return oldValue;
// 返回被取回的值
}
```

**前置式返回 reference，后置式返回 const 对象！**

后置 operator++(int) 的叠加是不允许的，即：i++++。

原因有两个：一是与内建类型行为不一致(内建类型支持前置叠加)；二是其效果跟调用一次 operator++(int) 效果一样，这是违反直觉的。另外，后置式操作符使用 operator++(int)，参数的唯一目的只是为了区别前置式和后置式而已，当函数被调用时，编译器传递一个0作为int参数的值传递给该函数。



2. 处理用户定制类型时，应该尽可能使用前置式。
3. 后置式increment 和decrement 操作符的实现应以其前置式兄弟为基础。如此一来你就只需维护前置式版本，因为后置式版本会自动调整为一致的行为。

<iframe
	height=500 width=100%
	src="https://player.youku.com/embed/XMzY0MzgxNDMyOA=="
	frameborder=0 allowfullscreen>
</iframe>

Ref:</br>
[1]. [More Effective C++](https://hr-insist.github.io/C/More_Effective_C++%E9%98%85%E8%AF%BB%E7%AC%94%E8%AE%B0/)
[2]. [《More Effective C++》读书笔记](https://zhuanlan.zhihu.com/p/368342605)

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/more_effective_c-/  

