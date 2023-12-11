# More Effective C++ 阅读笔记


## 一、 基础议题(Basics)

### 条款 1: 仔细区别 pointers和references

没有所谓的 null reference。一个 reference 必须总代表某个对象。所以如果你有一个变量，其目的是用来指向(代表)另一个对象，但是也有可能它不指向(代表)任何对象，那么你应该使用 pointer，因为你可以将 pointer设为 null。换个角度看，如果这个变量总是必须代表一个对象，也就是说如果你的设计并不允许这个变量为 null，那么你应该使用reference。

Pointers 和 references 之间的另一个重要差异就是，pointers 可以被重新赋值，指向另一个对象，reference 却总是指向(代表)它最初获得的那个对象。

一般而言，当你需要考虑“不指向任何对象”的可能性时，或是考虑“在不同时间指向不同对象”的能力时，你就应该采用 pointer。前一种情况你可以将 pointer设为 null，后一种情况你可以改变pointer 所指对象。而当你确定“总是会代表某个对象”，而且“一旦代表了该对象就不能够再改变”，那么你应该选用 reference。

当你知道你需要指向某个东西，而且绝不会改变指向其他东西，或是当你实现一个操作符而其语法需求无法由 pointers 达成，你就应该选择 references。任何其他时候，请采用 pointers。

**在任何情况下都不能使用指向空值的引用**。一个引用必须总是指向某些对象。在C++里，引用应被初始化。

不存在指向空值的引用这个事实意味着使用引用的代码效率比使用指针的要高。因为在使用引用之前不需要测试它的合法性。
指针与引用的另一个重要的不同是指针可以被重新赋值以指向另一个不同的对象。但是引用则总是指向在初始化时被指定的对象，以后不能改变。

关于引用的更多介绍参考: https://blog.csdn.net/fengbingchun/article/details/69820184

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

### 条款 2: 最好使用 C++转型操作符

C++通过引进四个新的类型转换(cast)操作符克服了C风格类型转换的缺点(过于粗鲁，能允许你在任何类型之间进行转换；C风格的类型转换在程序语句中难以识别)，这四个操作符是: `static_cast`、`const_cast`、`dynamic_cast`、`reinterpret_cast`。

static_cast在功能上基本上与C风格的类型转换一样强大，含义也一样。它也有功能上限制。例如，不能用static_cast像用C 风格的类型转换一样把struct转换成int类型或者把double类型转换成指针类型，另外，static_cast不能从表达式中去除const属性，因为另一个新的类型转换操作符const_cast有这样的功能。

const_cast用于类型转换掉表达式的const或volatileness属性。如果你试图使用const_cast来完成修改constness或者volatileness属性之外的事情，你的类型转换将被拒绝。

**dynamic_cast**被用于安全地沿着类的继承关系向下进行类型转换。这就是说，你能用dynamic_cast把指向基类的指针或引用转换成指向其派生类或其兄弟类的指针或引用，而且你能知道转换是否成功。失败的转换将返回空指针(当对指针进行类型转换时)或者抛出异常(当对引用进行类型转换时)。dynamic_cast在帮助你浏览继承层次上是有限制的，它不能被用来缺乏虚函数的类型上，也不能用它来转换掉constness。如你想在没有继承关系的类型中进行转换，你可能想到static_cast。如果是为了去除const，你总得用const_cast。

reinterpret_cast使用这个操作符的类型转换，其转换结果几乎都是执行期定义(implementation-defined)。因此，使用reinterpret_cast的代码很难移植。此操作符最普通的用途就是在函数指针之间进行转换。

关于类型转换更多介绍参考: https://blog.csdn.net/fengbingchun/article/details/51235498

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

### 条款 3: 绝对不要以多态(polymorphically)方式处理数组

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

### 条款 4: 避免无用的缺省构造函数

构造函数能初始化对象，而**缺省构造函数**则可以不利用任何在建立对象时的外部数据就能初始化对象。有时这样的方法是不错的。例如一些行为特性与数字相仿的对象被初始化为空值或不确定的值也是合理的，还有比如链表、哈希表、图等等数据结构也可以被初始化为空容器。但不是所有的对象都属于上述类型，对于很多对象来说，不利用外部数据进行完全的初始化是不合理的。比如一个没有输入姓名的地址薄对象，就没有任何意义。

利用指针数组代替一个对象数组这种方法有两个缺点: 第一你必须删除数组里每个指针所指向的对象。如果忘了，就会发生内存泄漏。第二增加了内存分配量，因为正如你需要空间来容纳EquipmentPiece对象一样，你也需要空间来容纳指针。

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

## 二、运算符(操作符)

### 条款 5: 谨慎定义类型转换函数

1. C++ 允许内置数据类型之间(例如char和int，int和double等)进行隐式转换，对于内置类型之间的隐式转换有详细的规则，但不管怎样，这些都是语言提供的，既相对安全，我们又无法更改。

对于自定义的类类型，隐式转换可以通过**单参数构造函数(single-argument constructors)**和**隐式类型转换操作符**来实现。所谓”单一自变量(单参数)指的是可以有多个参数，但除了第一个参数其他参数必须有默认实参)。所谓隐式类型转换操作符，是一个 member function: 关键词operator 之后加一个类型名称，例如: `operator double() const`;

```c++
class Rational {
public:
	...
    operator double() const; // 将Rational 转换为 double
}
```

这个函数会在以下情况被自动调用:

```c++
Rational r(1, 2);  // r的值是 1/2
double d = 0.5 * r; // 将r的值转换为double，然后执行运算。
```

2. 但是下面这个情况就会出问题: `std::cout << r`;

如果你忘了为 Rational 类重载一个 operator<<，那么按道理应该打印不成功。但是编译器面对上述动作，它会想尽办法(包括找出一系列可接受的隐式类型转换)让函数调用动作成功。此时编译器发现 只需调用 Rational::operator double， 将 r 转换为 double，就可以成功调用 std::cout << r;，以浮点数的形式输出。

解决办法就是以功能对等的另一个函数取代类型转换操作符。即: 定义一个 doube asDouble() const;函数。虽然使用时有些许不便，但“可因为不再默默调用那些不打算调用的函数而获得弥补”。C++ 标准库中的 string 类从没有 string 到 char* 的隐式类型转换操作符而采用 c_str 函数可能就是这个原因。

3. 拥有单个参数(或除第一个参数外都有默认值的多参数)构造函数的类，很容易被隐式类型转换，最好加上 explicit 防止隐式类型转换。
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

`if(a == b[i])` 并不会报错。因为编译器发现只要调用 `Array\ constructor`(需一个 int 作为自变量)，就可以把 int 转为 `Array\ object`。就会产生类似这样的代码:

`if( a == static_cast<Array<int> >(b[i]))` 将 `b[i]` 转为 `Array`。此时程序会正常运行，但是结果却不尽人意。

解决办法就是使用 C++ 特性: <font color=red>关键词 `explicit`</font>。这个特性之所以被导入，就是为了解决隐式类型转换带来的问题。`explict Array(int size)`;

还有一种被称为 proxy classes 的方法:

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

对于自定义类型的类型转换，有一个规则: "**没有任何一个转换程序可以内含一个以上的‘用户定制转换行为’(亦即单自变量constructor亦即隐式类型转换操作符)**"，也就是说，必要的时候编译器可以先进行内置类型之间的转换再调用带单自变量的构造函数或者先调用隐式类型转换操作符在进行内置类型之间的转换，但不可能连续进行两次用户定制的类型转换！

所以 此时 `if(a == b[i])` 就会报错。不能从 int 转换成 ArraySize，再从 ArraySize 转为 Array。

4. 总结允许编译器执行隐式转换弊大于利，所以**非必要不要提供转换函数**！

### 条款 6: 区别 increment/decrement 操作符的前置和后置形式s

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
// 前缀形式: 增加然后取回值
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

后置 operator++(int) 的叠加是不允许的，即: i++++。

原因有两个: 一是与内建类型行为不一致(内建类型支持前置叠加)；二是其效果跟调用一次 operator++(int) 效果一样，这是违反直觉的。另外，后置式操作符使用 operator++(int)，参数的唯一目的只是为了区别前置式和后置式而已，当函数被调用时，编译器传递一个0作为int参数的值传递给该函数。

2. 处理用户定制类型时，应该尽可能使用前置式。
3. 后置式increment 和decrement 操作符的实现应以其前置式兄弟为基础。如此一来你就只需维护前置式版本，因为后置式版本会自动调整为一致的行为。

### 条款 7: 千万不要重载&&，|| 和，操作符

C++ 对于“真假值表达式” 采用所谓的“短路” 评估方式(short-circuit evaluation)。意思是一旦该表达式的真价值确定，及时表达式中还以后部分尚未检验，整个评估工作仍然结束。

“函数调用”语义和所谓的“短路” 评估方式语义有两个重大的区别。第一，当函数调用动作被执行，所有参数值都必须评估完成，所以当我们调用 operator&&和 operator||时，两个参数都已评估完成。换句话说没有什么骤死式语义。第二，C++语言规范并未明确定义函数调用动作中各参数的评估顺序，所以没办法知道expression1 和 expression2 哪个会先被评估。这与骤死式评估法形成一个明确的对比，后者总是由左向右评估其自变量。

C++同样也有一些规则用来定义逗号操作符面对内建类型的行为。表达式如果内含逗号，那么逗号左侧会先被评估，然后逗号的右侧再被评估；最后，整个逗号表达式的结果以逗号右侧的值为代表。

你不能重载以下操作符:


|`.`	|`.*`|	`::`|	`?:`|
|:-:|:-:|:-:|:-:|
|`new`	|`delete`	|`sizeof`	|`typeid`|
|`static_cast`	|`dynamic_cast`	|`const_cast`|	`reinterpret_cast`|

### 条款8: 了解各种不同意义的 new 和 delete

- **new operator**: new操作符，用于动态分配内存并进行初始化, 它的动作分为两方面。第一，它分配足够的内存，用来放置某类型的对象。以上例而言，它分配足够放置一个string 对象的内存。第二，它调用一个 constructor，为刚才分配的内存中的那个对象设定初值。;
  - new operator，不能被重载
  - 当你写出这样的代码:
    ```c++
  	string *ps = new string(“Memory Mangement”);
	```
	你所使用的 new 是所谓的 new operator。它的动作分为两个方面: 1、分配足够的内存，用来放置某类型的对象；2、调用 constructor，为刚才的内存中的那个对象设定初值。

- **operator new**: 标准库的函数，只分配内存不进行初始化(或者传递一个可用的内存地址)，可以自己进行重载，也可以主动调用。
  - 和 malloc 一样，operator new 的唯一任务就是分配内存。
  - `void *rawMemory = operator new(sizeof(string));` 返回值类型是 void* ！！！
  - 可以**重载** operator new，但是第一个参数类型必须总是 size_t。
  - `string *ps = new string("Memory Mangement");`等价于
	```c++
	void *rawMemory = operator new(sizeof(string));   // 取得原始内存，用来存放有一个string对象
	call string::string("Memory Mangement") on *memory; // 将内存中的对象初始化
	string *ps = static_cast<string*>(memory); // 让ps 指向新完成的对象
	```
- **placement new (定位new)**: new operator的另外一种用法 ，在已分配的内存上构造对象;


{{<admonition Note "注意">}}
注意: new operator是操作符，placement new是这个操作符的一种用法，而operator new是标准库中的函数，new operator调用了 operator new。

1. 将对象产生与 heap，请使用 new operator。它不但分配内存而且为该对象调用一个 constructor。
2. 如果你只是打算分配内存，请调用 operator new，那就没有任何 constructor 会被调用。
3. 如果你打算在 heap objects 产生时自己决定内存分配方式，请写一个自己的 operator new，并使用 new operator，它会自动调用你所写的 operator new。
4. 如果你打算在已分配并拥有指针的内存中构造对象，请使用 placement new。
{{</admonition>}}


## 三、 异常

程序之所以在 exceptions 出现时仍有良好行为，不是因为碰巧如此，而是因为它们加入了 exceptions 的考虑。

exceptions 无法被忽略。如果一个函数利用“设定状态变量”的方式或是利用“返回错误码”的方式发出一个异常信号，无法保证此函数的调用者会检查那个变量或检验那个错误码。于是程序的执行可能会一直继续下去，远离错误发生地点。但是如果函数以抛出 exception 的方式发出异常信号，而该 exception 未被捕捉，程序的执行便会立刻中止。

如果你需要一个“绝对不会被忽略的”异常信号发射方法，而且发射后的 stack处理过程又能够确保局部对象的 destructors 被调用，那么你需要 C++exceptions。它是最简单的方法了。

### 条款 9: 利用 destructors 避免泄漏资源

每当 new 一个新的对象，一定要确保成功 delete 它，否则就会造成内存泄漏。
```c++
void processAdoptions(istream& dataSource){
	while(dataSource){
    	ALA *pa = readALA(dataSource); // new 新的对象
        pa->processAdoption(); // 处理事务
        delete pa; // 删除pa指向的对象
    }
}
```

但是如果 `pa->processAdoption();`抛出异常，之后的所有语句都会被跳过，不再执行，这意味着 `deleta pa;`不会执行，造成内存泄漏。

解决方法1:

```c++
void processAdoptions(istream& dataSource){
  while(dataSource){
    ALA *pa = readALA(dataSource);
      try{
        pa->processAdoption();
      }
    catch(...){
      delete pa; //在抛出异常的时候避免泄露
      throw;
    }
    delete pa; //在不抛出异常的时候避免泄
  }
}
```

因为这种情况会需要删除两次pa，代码维护很麻烦，所以需要进行优化:

只要我们能够将 “一定得执行的清理代码” 移到 processAdoptions 函数的某个局部对象的 destructors 内即可。因为局部对象总是会在函数结束时被析构，不论函数如何结束。

如何把 delete 动作从 processAdoptions 函数移到函数内的某个局部对象的 destructor 内: 以一个 “类似指针的对象(智能指针)”取代指针 pa。当这个类似指针的对象被(自动)销毁，我们可以令其 destructor 调用 delete。

```c++
void processAdoptions(istream& dataSource){
  while(dataSource){
    auto_ptr<ALA> pa(readALA(dataSource)); // 现在auto_ptr已被弃用，推荐使用 shared_ptr!!!
    pa->processAdoption();
  }
}
```

### 条款 10: 在 constructors 内阻止资源泄漏(resource leak)

考虑下面的情况:
```c++
BookEntry::BookEntry():theImage(0), theAudioClip(0){
	theImage = new Image(imageFileName);
	theAudioClip = new AudioClip(audioClipFileName);
}
BookEntry::~BookEntry(){
	delete theImage;
    delete theAudioClip;
}
```

如果 `theAudioClip = new AudioClip(audioClipFileName);` 有 exception 抛出，那么函数构造失败，destructor 自然不会被调用。但是 theImage 对象构造成功了，这就导致 BookEntry constructor 所分配的 Image object 还是泄漏了。

由于C++ 不自动清理那些 “构造期间抛出 exceptions” 的对象，所以你必须设计你的 constructor，使得它们能够自动清理。通常只需将所有可能的 exceptions 捕捉起来，执行某种清理工作，然后重新抛出 exception，使它继续传播出去即可。

解决办法一:
```c++
BookEntry::BookEntry(){
    try{
    	theImage = new Image(imageFileName);
    	theAudioClip = new AudioClip(audioClipFileName);
    }
    catch(...){ // 在构造函数内捕捉异常
    	delete theImage;
    	delete theAudioClip;
    	throw;
    }
}
```

一个更好的解答是，接收条款9的忠告，将 theImage 和 theAudioClip 所指对象视为资源，交给局部对象来管理。 不论 theImage 和 theAudioClip 都是指向动态分配而得的对象，当指针本身停止活动，那些对象都应该被删除。

```c++
class BookEntry{
public:......
private:
	const auto_ptr<Image> theImage; // 同样的，现在auto_ptr已被弃用，推荐使用 shared_ptr!!!
	const auto_ptr<AudioClip> theAudioClip;
}
BookEntry::BookEntry(const string& imageFileName, const string& audioClipFileName):
	theImage(imageFileName != "" ? new Image(imageFileName) : 0),
	theAudioClip(audioClipFileName != "" ? new AudiaClip(audioClipFileName) : 0)
    {}
```
这样不仅解决了在 constructors 内阻止资源泄漏，而且还大幅简化 destructor。

`BookEntry::~BookEntry(){} // 不需要做什么事！`

### 条款 11: 禁止异常(exceptions)流出destructors之外

两种情况下 destructor 会被调用。第一种情况是当对象在正常状态下被销毁，也就是当它离开了它的生存空间(scope)或是被明确地删除；第二种情况是当对象被 exception 处理机制——也就是exception 传播过程中的 stack-unwinding(栈展开)机制——销毁。

因为如果控制权基于 exception 的因素离开 destructor，而此时正有另一个 exception 处于作用状态，C++会调用 terminate 函数。此函数的作为正如其名: 将你的程序结束掉——它会立刻动手，甚至不等局部对象被销毁。

因此，有两个好理由支持我们“全力阻止exceptions传出 destructors之外”。第一，它可以避免 terminate函数在 exception传播过程的栈展开(stack-unwinding)机制中被调用；第二，它可以协助确保 destructors 完成其应该完成的所有事情。

如何避免exception传出destructor之外呢?

**在析构函数中使用`try{} catch(){}`结构, 并且在catch的`{}`中什么也不做。**

### 条款12: 了解“抛出一个exception”与“传递一个参数”或“调用一个虚函数”之间的差异

你调用函数时，程序的控制权最终还会返回到函数的调用处，但是当你抛出一个异常时，控制权永远不会回到抛出异常的地方。

C++规范要求被作为异常抛出的对象必须被复制。即使被抛出的对象不会被释放，也会进行拷贝操作。抛出异常运行速度比参数传递要慢。

当异常对象被拷贝时，拷贝操作是由对象的拷贝构造函数完成的。该拷贝构造函数是对象的静态类型(static type)所对应类的拷贝构造函数，而不是对象的动态类型(dynamic type)对应类的拷贝构造函数。

catch子句中进行异常匹配时可以进行两种类型转换:
 - 第一种是继承类与基类间的转换。一个用来捕获基类的catch子句也可以处理派生类类型的异常。这种派生类与基类(inheritance_based)间的异常类型转换可以作用于数值、引用以及指针上。
 - 第二种是允许从一个类型化指针(typed pointer)转变成无类型指针(untyped pointer)，所以带有const void*指针的catch子句能捕获任何类型的指针类型异常。

catch子句匹配顺序总是取决于它们在程序中出现的顺序。因此一个派生类异常可能被处理其基类异常的catch子句捕获，即使同时存在有能直接处理该派生类异常的catch子句，与相同的try块相对应。不要把处理基类异常的catch子句放在处理派生类异常的catch子句的前面。

把一个对象传递给函数或一个对象调用虚拟函数与把一个对象作为异常抛出，这之间有三个主要区别:
 - 第一，异常对象在传递时总被进行拷贝；当通过传值方式捕获时，异常对象被拷贝了两次。对象作为参数传递给函数时不一定需要被拷贝。
 - 第二，对象作为异常被抛出与作为参数传递给函数相比，前者类型转换比后者要少(前者只有两种转换形式)。
 - 最后一点，catch子句进行异常类型匹配的顺序是它们在源代码中出现的顺序，第一个类型匹配成功的catch将被用来执行。当一个对象调用一个虚拟函数时，被选择的函数位于与对象类型匹配最佳的类里，即使该类不是在源代码的最前头。

try_catch 介绍参考: https://blog.csdn.net/fengbingchun/article/details/65939258

### 条款13: 以 by reference方式捕捉 exceptions
**catch by pointer的问题:**
他们是否应该删除他们接受的指针？如果是在堆中建立的异常对象，那他们必须删除它，否则会造成资源泄漏。如果不是在堆中建立的异常对象，他们绝对不能删除它，否则程序的行为将不可预测。通过指针捕获异常，将遇到一个哈姆雷
特式的难题: 是删除还是不删除？这是一个难以回答的问题。所以你最好避开它。

**catch by value的问题:**

- 当它们被抛出时系统将对异常对象拷贝两次(参见条款 M12)。
- 当抛出的是派生类对象，但是用基类捕获，会场生slicing 问题。

**catch by reference的优势:**
如果 catch by reference，你就可以避开对象删除问题，你也可以避开 exception objects 的切割(slicing)问题；你可以保留捕捉标准 exceptions 的能力；你也约束了 exception objects 需被复制的次数。

通过指针捕获异常不符合C++语言本身的规范。四个标准的异常----bad_alloc(当operator new不能分配足够的内存时被抛出)；bad_cast(当dynamic_cast针对一个引用(reference)操作失败时被抛出)；bad_typeid(当dynamic_cast对空指针进行操作时被抛出)；bad_exception(用于unexpected异常)----都不是指向对象的指针，所以你必须通过值或引用来捕获它们。

std::exception的介绍参考: https://blog.csdn.net/fengbingchun/article/details/78303734


### 条款14: 审慎使用异常规格(exception specifications)

如果一个函数抛出一个不在异常规格范围里的异常，系统在运行时能够检测出这个错误，然后一个特殊函数std::unexpected将被自动地调用(This function is automatically called when a function throws an exception that is not listed in its dynamic-exception-specifier.)。std::unexpected缺省的行为是调用函数std::terminate，而std::terminate缺省的行为是调用函数abort。应避免调用std::unexpected。

- **避免踏上 unexpected之路的第一个技术是:** 不应该将 templates 和 exceptionspecifications 混合使用。
- **避免踏上 unexpected之路的第二个技术是:** 如果A 函数内调用了 B 函数，而B 函数无 exceptionspecifications，那么 A 函数本身也不要设定exception specifications。
- **避免踏上 unexpected 之路的第三个技术是:** 处理“系统”可能抛出的exceptions。其中最常见的就是 bad_alloc，那是在内存分配失败时由operator new和 operator new[]抛出的(见条款8)。

### 条款15: 了解异常处理的系统开销

异常功能是需要一定开销的,即使是完全没有进行使用,虽然在某些情况下可以进行异常功能的关闭,但前提是,当前的所有代码所有模块都没有进行异常功能的使用,一旦有一个模块使用了异常,将导致程序无法运行.

抛出异常这个工作是比较消耗资源的,相对于平常的函数返回值,大约是3倍的资源消耗,但是不必恐慌,除非将异常作为了一种常规手段,否则偶尔的使用基本是不会影响整体效率的

异常功能整体上会使程序变大 5%~10%,同时也一定比例的减慢程序的运行速度.

这就是异常处理的系统开销。

## 四、效率

本章的内容从两个角度阐述效率的问题。
- 第一是从语言独立的角度，关注那些你能在任何语言里都能使用的东西。C++为它们提供了特别吸引人的实现途径，因为它对封装的支持非常好，从而能够用更好的算法与数据结构来替代低效的类似实现，同时接口可以保持不变。
- 第二是关注 C++语言本身。高性能的算法与数据结构虽然非常好，但如果实
际编程中代码实现得很粗糙，效率也会降低得相当多。潜在危害性最大的错误是
既容易犯而又不容易察觉的错误，濒繁地构造和释放大量的对象就是一种这样的
错误。过多的对象构造和对象释放对于你的程序性能来说就象是在大出血，在每
次建立和释放不需要的对象的过程中，宝贵的时间就这么流走了。这个问题在
C++程序中很普遍，我将用四个条款来说明这些对象从哪里来的，在不影响程序
代码正确性的基础上又如何消除它们。

### 条款16: 谨记 80-20 法则

80-20准则说的是大约20%的代码使用了80%的程序资源；大约20%的代码耗用了大约80%的运行时间；大约20%的代码使用了80%的内存；大约20%的代码执行80%的磁盘访问；80%的维护投入于大约20%的代码上。

**基本的观点:** 软件整体的性能取决于代码组成中的一小部分。一个程序大量的资源是消耗在少部分的代码上面,所有的程序都符合这个规则,所以,我们要做的并不是对每一处代码都进行优化,虽然这么做固然很好,但是每个人的能力和精力是一个固定值,一味的优化80%部分的代码,提升的效果可能达不到20%中的几行代码,我们要善于利用各种工具,找到真正需要进行优化的逻辑,然后去进行优化.

### 条款17: 考虑使用 lazy evaluation(缓式评估)

lazy evaluation(缓式评估)。一旦你采用 lazy evaluation，就是以某种方式撰写你的 classes，使它们延缓运算，直到那些运算结果刻不容缓地被迫切需要为止。如果其运算结果一直不被需要，运算也就一直不执行。
- **引用计数**
这种“数据共享”的行动细节(及相应代码)在条款 29有详细叙述，其观念便是 lazy evaluation：在你真正需要之前，不必着急为某物做一个副本。取而代之的是，以拖延战术应付之——只要能够，就使用其他副本。在某些应用领域，你常有可能永远不需要提供那样一个副本。
- **区别对待读取和写入**
  ```c++
  string s = "Homer's liad";
  ...
  cout << s[3];
  s[3] = 'x';
  ```
  首先调用 operator[]用来读取 string 的部分值，但是第二次调用该函数是为了完成写操作。我们应能够区别对待读调用和写调用，因为读取reference-counted string 是很容易的，而写入这个 string 则需要在写入前对该string 值制作一个新拷贝。
  为了能够这样做，需要在 operator[]里采取不同的措施(根据是为了完成读取操作而调用该函数还是为了完成写入操作而调用该函数)。我们如果判断调用 operator[]的 context 是读取操作还是写入操作呢？残酷的事实是我们不可能判断出来。通过使用 lazy evaluation 和条款 M30 中讲述的proxy class，我们可以推迟做出是读操作还是写操作的决定，直到我们能判断出正确的答案。
- **Lazy Fetching (懒惰提取)**
实现 lazy fetching 时，你必须面对一个问题：null 指针可能会在任何 member functions(包括const member functions，如 field1)内被赋值，以指向真正的数据。然而当你企图在 constmember functions 内修改 data members，编译器不会同意。所以你必须用某种方法告诉编译器说：“放轻松，我知道我正在干什么”。说这句话的最好方法就是将指针字段声明为 mutable，意思是这样的字段可以在任何member function 内被修改，甚至是在 const member functions 内(见条款 E21)。
- **Lazy Expression Evaluation(懒惰表达式计算)**
lazy evaluation 在许多领域中都可能有用途：可避免非必要的对象复制，可区别 operator[]的读取和写动作，可避免非必要的数据库读取动作，可避免非必要的数值计算动作。

### 条款18: 分期摊还预期的计算成本

现在我鼓励你改善软件性能的方法是：令你的代码超前进度地做“要求以外”的更多工作。此条款背后的哲学可称为超急评估(over-eager evaluation): 在被要求之前就先把事情做下去。

Over-eager evaluation 背后的观念是，如果你预期程序常常会用到某个计算，你可以降低每次计算的平均成本，办法就是设计一份数据结构以便能够极有效率地处理需求。

**Caching** 是“分期摊还预期计算之成本”的一种做法,即caching(缓存)那些已经被计算出来而以后还有可能需要的值。

**Prefetching**(预先取出)是另一种做法。Prefetch需要空间放置被 prefetch 的东西，但是它减少了访问它们所需
的时间。

以上两种方法都是通过**以空间换时间**的方式来提高代码的运行效率。

可通过over-eager evaluation 如 caching和prefetching 等做法分期摊还预期运算成本——和我在条款 17 所提的 lazy evaluation 并不矛盾。当你必须支持某些运算而其结果并不总是需要的时候，lazy evaluation 可以改善程序效率。当你必须支持某些运算而其结果几乎总是被需要，或其结果常常被多次需要的时候，over-eager evaluation 可以改善程序效率。

### 条款19: 理解临时对象的来源

```c++
size_t countChar(const std::string& str, char ch)
{
	// 建立一个string类型的临时对象，通过以buffer做为参数调用string的构造函数来初始化这个临时对象,
	// countChar的参数str被绑定在这个临时的string对象上，当countChar返回时，临时对象自动释放

	// 将countChar(const std::string& str, char ch)修改为countChar(std::string& str, char ch)则会error
	return 1;
}

#define MAX_STRING_LEN 64

int test_item_19()
{
	char buffer[MAX_STRING_LEN];
	char c;

	std::cin >> c >> std::setw(MAX_STRING_LEN) >> buffer;
	std::cout<<"There are "<<countChar(buffer, c)<<" occurrences of the character "<<c<<" in "<<buffer<<std::endl;

	return 0;
}
```
C++真正的所谓的临时对象是不可见的——不会在你的源代码中出现。只要你产生一个 non-heap object(非堆对象) 而没有为它命名，便诞生了一个临时对象。此等匿名对象通常发生于两种情况：一是当隐式类型转换(implicit type conversions)被施行起来以求函数调用能够成功；二是当函数返回对象的时候。

仅当通过传值(by value)方式传递对象或传递常量引用(reference-to-const)参数时，才会发生这些类型转换。当传递一个非常量引用(reference-to-non-const)参数对象，就不会发生。

C++语言禁止为**非常量引用(reference-to-non-const)**产生临时对象。

在这些优化策略中，最常见也最有用的就是所谓的“返回值优化(return value optimization)”。

临时对象可能很耗成本，所以你应该尽可能消除它们。然而更重要的是，如何训练出锐利的眼力，看出可能产生临时对象的地方。任何时候只要你看到一个 reference-to-const 参数，就极可能会有一个临时对象被产生出来绑定至该参数上。任何时候只要你看到函数返回一个对象，就会产生临时对象(并于稍后销毁)。

### 条款20: 协助完成返回值优化 (RVO)
```c++
class Rational20 {
public:
	Rational20(int numerator = 0, int denominator = 1) {}

	int numerator() const { return 1; }
	int denominator() const { return 2; }
};

const Rational20 operator*(const Rational20& lhs, const Rational20& rhs)
{
	// 以某种方法返回对象，能让编译器消除临时对象的开销：这种技巧是返回constructor argument而不是直接返回对象
	return Rational20(lhs.numerator() * rhs.numerator(), lhs.denominator() * rhs.denominator());
}

int test_item_20()
{
	Rational20 a = 10;
	Rational20 b(1, 2);
	Rational20 c = a * b;

	return 0;
}
```
我们可以用某种特殊写法来撰写函数，使它在返回对象时，能够让编译器消除临时对象的成本。我们的伎俩是：**返回所谓的 constructor arguments以取代对象。**

此特殊的优化行为——利用函数的 return 点消除一个局部临时对象（并可能用函数调用端的某对象取代）——不但广为人知而且很普遍地被实现出来。它甚至有个专属名称：return value optimization。


### 条款21: 通过重载避免隐式类型转换
```c++
class UPInt21 { // unlimited precision integers class
public:
	UPInt21() {}
	UPInt21(int value) {}
};

const UPInt21 operator+(const UPInt21& lhs, const UPInt21& rhs) // add UPInt21+UPInt21
{
	return UPInt21(1);
}

const UPInt21 operator+(const UPInt21& lhs, int rhs) // add UPInt21+int
{
	return UPInt21(1);
}

const UPInt21 operator+(int lhs, const UPInt21& rhs) // add int+UPInt21
{
	return UPInt21(1);
}

int test_item_21()
{
	UPInt21 upi1, upi2;
	UPInt21 upi3 = upi1 + upi2; // 正确，没有由upi1或upi2生成临时对象
	upi3 = upi1 + 10; // 正确,没有由upi1或10生成临时对象
	upi3 = 10 + upi2; // 正确，没有由10或upi2生成临时对象

	// 注意：注释掉上面的operator+(UPInt21&, int)和operator+(int, UPInt21&)也正确，但是会通过临时对象把10转换为UPInt21

	return 0;
}
```
在C++中有一条规则是每一个重载的operator必须带有一个用户定义类型(user-defined type)的参数。

利用重载避免临时对象的方法不只是用在operator函数上。

没有必要实现大量的重载函数，除非你有理由确信程序使用重载函数以后其整体效率会有显著的提高。

### 条款22: 考虑用运算符的赋值形式(op=)取代其单独形式(op)

```c++
class Rational22 {
public:
	Rational22(int numerator = 0, int denominator = 1) {}
	Rational22& operator+=(const Rational22& rhs) { return *this; }
	Rational22& operator-=(const Rational22& rhs) { return *this; }
};

// operator+根据operator+=实现
const Rational22 operator+(const Rational22& lhs, const Rational22& rhs)
{
	return Rational22(lhs) += rhs;
}

// operator-根据operator-=实现
const Rational22 operator-(const Rational22& lhs, const Rational22& rhs)
{
	return Rational22(lhs) -= rhs;
}
```
就C++来说，operator+、operator=和operator+=之间没有任何关系，因此如果你想让三个operator同时存在并具有你所期望的关系，就必须自己实现它们。同理，operator-, *, /, 等等也一样。

要确保操作符的复合形式（例如，operator+=）和其独身形式（例如，operator+）之间的自然关系能够存在，一个好方法就是以前者为基础实现后者（见条款 6）。

3 个与效率有关的情况值得注意:
- 第一，一般而言，复合操作符比其对应的独身版本效率高，因为独身版本通常必须返回一个新对象，而我们必须因此负担一个临时对象的构造和析构成本（见条款 19和 20及条款 E23）。至于复合版本则是直接将结果写入其左端自变量，所以不需要产生一个临时对象来放置返回值。
- 第二，如果同时提供某个操作符的复合形式和独身形式，便允许你的客户在效率与便利性之间做取舍（虽然那是极其困难的抉择）。
- 第三、自古以来匿名对象总是比命名对象更容易被消除，所以当你面临命名对象或临时对象的抉择时，最好选择临时对象。它应该绝不会比其命名兄弟耗用更多成本，反倒是极有可能降低成本（尤其在搭配旧式编译器时）。

### 条款23: 考虑使用其他程序库

不同的程序库在效率、可扩展性、移植性、类型安全和其它一些领域上蕴含着不同的设计理念，通过变换使用给予性能更多考虑的程序库，你有时可以大幅度地提供软件的效率。

重点是，不同的程序库即使提供相似的机能，也往往表现出不同的性能取舍策略，所以一旦你找出程序的瓶颈（通过分析器，见条款16），你应该思考是否有可能因为改用另一个程序库而移除了那些瓶颈。

### 条款24: 理解虚拟函数、多继承、虚基类和RTTI所需的代码

当调用一个虚拟函数时，被执行的代码必须与调用函数的对象的动态类型相一致；指向对象的指针或引用的类型是不重要的。大多数编译器是使用virtual table和virtual table pointers，通常被分别地称为vtbl和vptr。

一个vtbl通常是一个函数指针数组。(一些编译器使用链表来代替数组，但是基本方法是一样的)在程序中的每个类只要声明了虚函数或继承了虚函数，它就有自己的vtbl，并且类中vtbl的项目是指向虚函数实现体的指针。例如，如下这个类定义:

```c++
class C1 {
public:
  C1();
  virtual ~C1();
  virtual void f1();
  virtual int f2(char c) const;
  virtual void f3(const string& s);
  void f4() const;
  ...
};
```

C1 的 virtual table 数组看起来如下图所示：

![](images/4-24-1.png)


vtbl 通常是一个由“函数指针”架构而成的数组。某些编译器会以链表（linked list）取代数组，但其基本策略相同。程序中的每一个class 凡声明（或继承）虚函数者，都有自己的一个 vtbl，而其中的条目（entries）就是该 class 的各个虚函数实现体的指针。

这份讨论带出虚函数的第一个成本：你必须为每个拥有虚函数的class耗费一个vtbl 空间，其大小视虚函数的个数（包括继承而来的）而定。类的vtbl的大小与类中声明的虚函数的数量成正比(包括从基类继承的虚函数)。每个类应该只有一个virtual table，所以virtual table所需的空间不会太大，但是如果你有大量的类或者在每个类中有大量的虚函数，你会发现vtbl会占用大量的地址空间。

Virtual tables 只是虚函数实现机构的一半而已。如果只有它，不能成气候。一旦有某种方法可以指示出每个对象相应于哪一个 vtbl，vtbl 才真的有用。而这正是virtual table pointer（vptr）的任务。
关于虚函数表的介绍参考：https://blog.csdn.net/fengbingchun/article/details/79592347

凡声明有虚函数的 class，其对象都含有一个隐藏的 data member，用来指向该class 的 vtbl。这个隐藏的 data member——所谓的vptr——被编译器加入对象内某个唯编译器才知道的位置。

此刻，只需注意到虚函数的第二个成本：你必须在每一个拥有虚函数的对象内付出“一个额外指针”的代价。

编译器必须产生代码，完成以下动作：
  1.根据对象的 vptr 找出其 vtbl。这是一个简单的动作，因为编译器知道到对象的哪里去找出 vptr（毕竟那个位置正是编译器决定的）。成本只有一个偏移调整（offset adjustment，以便获得 vptr）和一个指针间接动作（以便获得 vtbl）。
  2.找出被调用函数（本例为 f1）在 vtbl 内的对应指针。这也很简单，因为编译器为每个虚函数指定了一个独一无二的表格索引。本步骤的成本只是一个差移（offset）以求进入 vtbl 数组。
  3.调用步骤 2所得指针所指向的函数。

一些原因导致现在的编译器一般总是忽略虚函数的inline指令。虚函数真正的运行时期成本发生在和 inlining 互动的时候。对所有实用目的而言，虚函数不应该 inlined。因为“inline”意味“在编译期，将调用端的调用动作被调用函数的函数本体取代”，而“virtual”则意味着“等待，直到运行时期才知道哪个函数被调用”。(这是因为”内联”是指”在编译期间用被调用的函数体本身来代替函数调用的指令”，但是虚函数的”虚”是指”直到运行时才能知道要调用的是哪一个函数”。) 当编译器面对某个调用动作，却无法知道哪个函数该被调用时，你就可以了解为什么它们没有能力将该函数调用加以 inlining了。这便是虚函数的第三个成本：你事实上等于放弃了 inlining。

运行时期类型辨识（runtime typeidentification，RTTI）的成本。RTTI 让我们得以在运行时期获得 objects 和 classes 的相关信息，所以一定得有某些地方用来存放那些信息才行——是的，它们被存放在类型为 type_info 的对象内。你可以利用 typeid 操作符取得某个class 相应的 type_info 对象。

C++规范书上说，只有当某种类型拥有至少一个虚函数，才保证我们能够检验该类型对象的动态类型。这使得 RTTI 相关信息听起来有点像一个 vtbl：面对一个 class，我们只需一份相关信息，而我们需要某种方法，让任何一个内含虚函数的对象都有能力取得其专属信息。RTTI 和vtbl 之间的这种平行关系并非偶发，RTTI 的设计理念是：根据 class 的 vtbl来实现。

关于typeid的使用参考：https://blog.csdn.net/fengbingchun/article/details/51866559

RTTI被设计为在类的vtbl基础上实现。

## 五、技术 (Techniques，Idioms，Patterns)

### 条款25: 将构造函数和非成员函数虚拟化

由于它产生新对象，所以行为仿若 constructor，但它能够产生不同类型的对象，所以我们称它为一个 virtual constructor。所谓 virtualconstructor 是某种函数，视其获得的输入，可产生不同类型的对象。**Virtual constructors 在许多情况下有用，其中之一就是从磁盘（或网络或磁带等）读取对象信息。**

有一种特别的 virtual constructor——所谓 virtual copyconstructor——也被广泛地运用。Virtual copy constructor 会返回一个指针，指向其调用者（某对象）的一个新副本。基于这种行为，virtual copy constructors 通常以 copySelf 或cloneSelf 命名，或者像下面一样命名为 clone。

当 derived class 重新定义其base class 的一个虚函数时，不再需要一定得声明与原本相同的返回类型。如果函数的返回类型是个指针（或reference），指向一个base class，那么 derived class 的函数可以返回一个指针（或reference），指向该 base class 的一个 derived class。

既然一个函数能够构造出不同类型的新对象是可以理解的，那么同样也存在这样的非成员函数，可以根据参数的不同动态类型而其行为特性也不同。


### 条款26: 限制某个类所能产生的对象数量

每当即将产生一个对象，我们确知一件事情：会有一个 constructor被调用。“阻止某个 class 产出对象”的最简单方法就是将其constructors 声明为 private。

### 条款27: 要求（或禁止）对象产生于 heap之中

所谓 abstract base class是一个不能够被实例化的 base class。也就是说它至少有一个纯虚函数。所谓 mixin（“mix in”）class则提供一组定义完好的能力，能够与其derived class所可能提供的其他任何能力（条款 E7）兼容。如此的 classes几乎总是abstract。我们于是可以形成一个所谓的 abstract mixin base class，用来为 derivedclasses提供“判断某指针是否以 operator new 分配出来”的能力。

### 条款28: 灵巧(smart)指针
当你以 smart pointers 取代 C++的内建指针（亦即所谓的 dumbpointers），你将获得以下各种指针行为的控制权：

- 构造和析构（Construction and Destruction）。你可以决定smart pointer 被产生以及被销毁时发生什么事。通常我们会给smart pointers 一个默认值 0，以避免“指针未获初始化”的头痛问题。某些 smart pointers 有责任删除它们所指的对象——当指向该对象的最后一个 smart pointer 被销毁时。这是消除资源泄漏问题的一大进步。
- 复制和赋值（Copying and Assignment）。当一个 smartpointer 被复制或涉及赋值动作时，你可以控制发生什么事。某些smart pointer 会希望在此时刻自动为其所指之物进行复制或赋值动作，也就是执行所谓的深复制（deep copy）。另一些 smartpointer则可能只希望指针本身被复制或赋值就好。还有一些则根本不允许复制和赋值。不论你希望什么样的行为，smart pointers 都可以让你如愿。
- 解引（Dereferencing）。当 client 解引（取用）smart pointer所指之物时，你有权决定发生什么事情。例如你可以利用 smartpointers 协助实现出条款 17所说的 lazy fetching 策略。

Smart pointer的构造行为通常明确易解：确定一个目标物（通常是利用smart pointer的 constructor自变量），然后让 smart pointer内部的 dumb pointer指向它。如果尚未决定目标物，就将内部指针设为 0，或是发出一个错误消息（可能是抛出 exception）。

重点很简单：不要提供对 dumb pointers的隐式转换操作符，除非不得已。

大多数灵巧指针模板如下:
```c++
// 大多数灵巧指针模板
template<class T>
class SmartPtr {
public:
	SmartPtr(T* realPtr = 0); // 建立一个灵巧指针指向dumb pointer(内建指针)所指的对象，未初始化的指针，缺省值为0(null)
	SmartPtr(const SmartPtr& rhs); // 拷贝一个灵巧指针
	~SmartPtr(); // 释放灵巧指针
	// make an assignment to a smart ptr
	SmartPtr& operator=(const SmartPtr& rhs);
	T* operator->() const; // dereference一个灵巧指针以访问所指对象的成员
	T& operator*() const; // dereference灵巧指针

private:
	T* pointee; // 灵巧指针所指的对象
};
```

灵巧指针是一种外观和行为都被设计成与内建指针相类似的对象，不过它能提供更多的功能。它们有许多应用的领域，包括资源管理和重复代码任务的自动化。

在C++11中auto_ptr已经被废弃，用unique_ptr替代。

std::unique_ptr的使用参考：https://blog.csdn.net/fengbingchun/article/details/52203664


### 条款29: Reference counting（引用计数）

```c++
class String {
public:
	String(const char* initValue = "");
	String(const String& rhs);
	String& operator=(const String& rhs);
	const char& operator[](int index) const; // for const String
	char& operator[](int index); // for non-const String
	~String();

private:
	// StringValue的主要目的是提供一个空间将一个特别的值和共享此值的对象的数目联系起来
	struct StringValue { // holds a reference count and a string value
		int refCount;
		char* data;
		bool shareable; // 标志，以指出它是否为可共享的
		StringValue(const char* initValue);
		~StringValue();
	};

	StringValue* value; // value of this String
};

String::String(const char* initValue) : value(new StringValue(initValue))
{}

String::String(const String& rhs)
{
	if (rhs.value->shareable) {
		value = rhs.value;
		++value->refCount;
	} else {
		value = new StringValue(rhs.value->data);
	}
}

String& String::operator=(const String& rhs)
{
	if (value == rhs.value) { // do nothing if the values are already the same
		return *this;
	}

	if (value->shareable && --value->refCount == 0) { // destroy *this's value if no one else is using it
		delete value;
	}

	if (rhs.value->shareable) {
		value = rhs.value; // have *this share rhs's value
		++value->refCount;
	} else {
		value = new StringValue(rhs.value->data);
	}

	return *this;
}

const char& String::operator[](int index) const
{
	return value->data[index];
}

char& String::operator[](int index)
{
	// if we're sharing a value with other String objects, break off a separate copy of the value fro ourselves
	if (value->refCount > 1) {
		--value->refCount; // decrement current value's refCount, becuase we won't be using that value any more
		value = new StringValue(value->data); // make a copy of the value for ourselves
	}

	value->shareable = false;
	// return a reference to a character inside our unshared StringValue object
	return value->data[index];
}

String::~String()
{
	if (--value->refCount == 0) {
		delete value;
	}
}

String::StringValue::StringValue(const char* initValue) : refCount(1), shareable(true)
{
	data = new char[strlen(initValue) + 1];
	strcpy(data, initValue);
}

String::StringValue::~StringValue()
{
	delete[] data;
}

// 基类，任何需要引用计数的类都必须从它继承
class RCObject {
public:
	void addReference() { ++refCount; }
	void removeReference() { if (--refCount == 0) delete this; } // 必须确保RCObject只能被构建在堆中
	void markUnshareable() { shareable = false; }
	bool isShareable() const { return shareable; }
	bool isShared() const { return refCount > 1; }

protected:
	RCObject() : refCount(0), shareable(true) {}
	RCObject(const RCObject& rhs) : refCount(0), shareable(true) {}
	RCObject& operator=(const RCObject& rhs) { return *this; }
	virtual ~RCObject() = 0;

private:
	int refCount;
	bool shareable;

};

RCObject::~RCObject() {} // virtual dtors must always be implemented, even if they are pure virtual and do nothing

// template class for smart pointers-to-T objects. T must support the RCObject interface, typically by inheriting from RCObject
template<class T>
class RCPtr {
public:
	RCPtr(T* realPtr = 0) : pointee(realPtr) { init(); }
	RCPtr(const RCPtr& rhs) : pointee(rhs.pointee) { init(); }
	~RCPtr() { if (pointee) pointee->removeReference(); }

	RCPtr& operator=(const RCPtr& rhs)
	{
		if (pointee != rhs.pointee) { // skip assignments where the value doesn't change
			if (pointee)
				pointee->removeReference(); // remove reference to current value

			pointee = rhs.pointee; // point to new value
			init(); // if possible, share it else make own copy
		}

		return *this;
	}

	T* operator->() const { return pointee; }
	T& operator*() const { return *pointee; }

private:
	T* pointee; // dumb pointer this object is emulating

	void init() // common initialization
	{
		if (pointee == 0) // if the dumb pointer is null, so is the smart one
			return;

		if (pointee->isShareable() == false) // if the value isn't shareable copy it
			pointee = new T(*pointee);

		pointee->addReference(); // note that there is now a new reference to the value
	}
};

// 将StringValue修改为是从RCObject继承
// 将引用计数功能移入一个新类(RCObject)，增加了灵巧指针(RCPtr)来自动处理引用计数
class String2 {
public:
	String2(const char* value = "") : value(new StringValue(value)) {}
	const char& operator[](int index) const { return value->data[index]; } // for const String2

	char& operator[](int index) // for non-const String2
	{
		if (value->isShared())
			value = new StringValue(value->data);
		value->markUnshareable();
		return value->data[index];
	}

private:
	// StringValue的主要目的是提供一个空间将一个特别的值和共享此值的对象的数目联系起来
	struct StringValue : public RCObject { // holds a reference count and a string value
		char* data;

		StringValue(const char* initValue) { init(initValue); }
		StringValue(const StringValue& rhs) { init(rhs.data); }

		void init(const char* initValue)
		{
			data = new char[strlen(initValue) + 1];
			strcpy(data, initValue);
		}

		~StringValue() { delete [] data; }
	};

	RCPtr<StringValue> value; // value of this String2

};

int test_item_29()
{
	String s1("More Effective C++");
	String s2 = s1;
	s1 = s2;
	fprintf(stdout, "char: %c\n", s1[2]);
	String s3 = s1;
	s3[5] = 'x';

	return 0;
}
```

引用计数是这样一个技巧，它允许多个有相同值的对象共享这个值的实现。这个技巧有两个常用动机。第一个是简化跟踪堆中的对象的过程。一旦一个对象通过调用new被分配出来，最要紧的就是记录谁拥有这个对象，因为其所有者----并且只有其所有者----负责对这个对象调用delete。但是，所有权可以被从一个对象传递到另外一个对象(例如通过传递指针型参数)。引用计数可以免除跟踪对象所有权的担子，因为当使用引用计数后，对象自己拥有自己。当没人再使用它时，它自己自动销毁自己。因此，引用计数是个简单的垃圾回收体系。第二个动机是由于一个简单的常识。如果很多对象有相同的值，将这个值存储多次是很无聊的。更好的办法是让所有的对象共享这个值的实现。这么做不但节省内存，而且可以使得程序运行更快，因为不需要构造和析构这个值的拷贝。

引用计数介绍参考：https://blog.csdn.net/fengbingchun/article/details/85861776

实现引用计数不是没有代价的。每个被引用的值带一个引用计数，其大部分操作都需要以某种形式检查或操作引用计数。对象的值需要更多的内存，而我们在处理它们时需要执行更多的代码。引用计数是基于对象通常共享相同的值的假设的优化技巧。如果假设不成立的话，引用计数将比通常的方法使用更多的内存和执行更多的代码。另一方面，如果你的对象确实有具有相同值的趋势，那么引用计数将同时节省时间和空间。

reference counting 建构出垃圾回收机制（garbage collection）的一个简单形式。Reference counting 的第二个发展动机则只是为了实现一种常识。如果许多对象有相同的值，将那个值存储多次是件愚蠢的事。最好是让所有等值对象共享一份实值就好。

“和其他对象共享一份实值，直到我们必须对自己所拥有的那一份实值进行写动作”，这个观念在计算机科学领域中有很长的历史。特别是在操作系统领域，各进程（processes）之间往往允许共享某些内存分页（memory pages），直到它们打算修改属于自己的那一分页。这项技术是如此普及，因而有一个专用名称：copy-on-write（写时才复制）。这是提升效率的一般化做法（也就是 lazyevaluation，缓式评估，见条款 17）中的一剂“特效药”。

第一个步骤是，首先产生一个 base class RCObject，作为“reference-counted 对象”之用。任何 class 如果希望自动拥有reference counting 能力，都必须继承自这个 class。RCObject 将“引用计数器”本身以及用以增减计数值的函数封装进来。此外还包括一个函数，用来将不再被使用（也就是其引用次数为 0）的对象值销毁掉。最后，它还内含一个成员，用来追踪其值是否“可共享”，并提供查询其值、将该成员设为 false 等相关函数。没有必要提供一个函数让外界设定该成员为true，因为所有的对象值在默认情况下均为可共享。一如先前所提示，一旦某个对象被贴上“不可共享”标签，就没有办法再恢复其“可共享”的身份了。

简单地说，以下是使用 reference counting 改善效率的最适当时机：

相对多数的对象共享相对少量的实值。如此的共享行为通常是通过assignment operators 和 copy constructors。“对象/实值”数量比愈高，reference counting 带来的利益愈大。
对象实值的产生或销毁成本很高，或是它们使用许多内存。不过即使这种情况，reference counting还是不能为你带来任何利益，除非实值可被多个对象共享。
这一次我们以惯例规范来达成目标。RCObject 的设计目的是用来作为有引用计数能力之“实值对象”的基类，而那些“实值对象”应该只被 RCPtr smart pointers取用。此外，应该只有确知“实值对象”共享性的所谓“应用对象”才能将“实值对象”实例化。描述“实值对象”的那些 classes 不应该被外界看到。在我们的例子中，描述“实值对象”者为 StringValue，我们令它成为“应用对象”String内的私有成员，以限制其用途。只有 String 才能够产生 StringValue对象，所以，确保所有 StringValue 对象皆以 new 分配而得，是String class 作者的责任。

### 条款30: Proxy classes（替身类、代理类）

```c++
template<class T>
class Array2D { // 使用代理实现二维数组
public:
	Array2D(int i, int j) : i(i), j(j)
	{
		data.reset(new T[i*j]);
	}

	class Array1D { // Array1D是一个代理类，它的实例扮演的是一个在概念上不存在的一维数组
	public:
		Array1D(T* data) : data(data) {}
		T& operator[](int index) { return data[index]; }
		const T& operator[](int index) const { return data[index]; }

	private:
		T* data;
	};

	Array1D operator[](int index) { return Array1D(data.get()+j*index); }
	const Array1D operator[](int index) const { return Array1D(data.get()+j*index); }

private:
	std::unique_ptr<T[]> data;
	int i, j;
};

// 可以通过代理类帮助区分通过operator[]进行的是读操作还是写操作
class String30 {
public:
	String30(const char* value = "") : value(new StringValue(value)) {}

	class CharProxy { // proxies for string chars
	public:
		CharProxy(String30& str, int index) : theString(str), charIndex(index) {}

		CharProxy& operator=(const CharProxy& rhs)
		{
			// if the string is haring a value with other String objects,
			// break off a separate copy of the value for this string only
			if (theString.value->isShared())
				theString.value = new StringValue(theString.value->data);

			// now make the assignment: assign the value of the char
			// represented by rhs to the char represented by *this
			theString.value->data[charIndex] = rhs.theString.value->data[rhs.charIndex];
			return *this;
		}

		CharProxy& operator=(char c)
		{
			if (theString.value->isShared())
				theString.value = new StringValue(theString.value->data);
			theString.value->data[charIndex] = c;
			return *this;
		}

		operator char() const { return theString.value->data[charIndex]; }

	private:
		String30& theString;
		int charIndex;
	};

	const CharProxy operator[](int index) const // for const String30
	{
		return CharProxy(const_cast<String30&>(*this), index);
	}

	CharProxy operator[](int index) // for non-const String30
	{
		return CharProxy(*this, index);
	}

	//friend class CharProxy;
private:
	// StringValue的主要目的是提供一个空间将一个特别的值和共享此值的对象的数目联系起来
	struct StringValue : public RCObject { // holds a reference count and a string value
		char* data;

		StringValue(const char* initValue) { init(initValue); }
		StringValue(const StringValue& rhs) { init(rhs.data); }

		void init(const char* initValue)
		{
			data = new char[strlen(initValue) + 1];
			strcpy(data, initValue);
		}

		~StringValue() { delete [] data; }
	};

	RCPtr<StringValue> value; // value of this String30

};

int test_item_30()
{
	Array2D<float> data(10, 20);
	fprintf(stdout, "%f\n", data[3][6]);

	String30 s1("Effective C++"), s2("More Effective C++"); // reference-counted strings using proxies
	fprintf(stdout, "%c\n", s1[5]); // still legal, still works
	s2[5] = 'x'; // also legal, also works
	s1[3] = s2[8]; // of course it's legal, of course it works

	//char* p = &s1[1]; // error, 通常,取proxy对象地址的操作与取实际对象地址的操作得到的指针，其类型是不同的,重载CharProxy类的取地址运算可消除这个不同

	return 0;
}
```

凡“用来代表（象征）其他对象”的对象，常被称为 proxy objects（替身对象），而用以表现 proxy objects者，我们称为 proxy classes。

虽然或许不可能知道operator[] 是在左值或右值情境下被调用，我们还是可以区分读和写——只要将我们所要的处理动作延缓，直至知道operator[] 的返回结果将如何被使用为止。我们需要知道的，就是如何延缓我们的决定（决定对象究竟是被读或被写），直到 operator[] 返回。这是条款 17 的缓式评估（lazyevaluation）例子之一。

Proxy class 让我们得以买到我们所要的时间，因为我们可以修改operator[]，令它返回字符串中字符的 proxy，而不返回字符本身。然后我们可以等待，看看这个 proxy如何被运用。如果它被读，我们可以（有点过时地）将 operator[] 的调用动作视为一个读取动作。如果它被写，我们必须将 operator[] 的调用视为一个写动作。

稍后你会看到代码。首先，重要的是了解我们即将使用的 proxies。对于一个proxy，你只有 3件事情可做：

产生它，本例也就是指定它代表哪一个字符串中的哪一个字符。
以它作为赋值动作（assignment）的目标（接受端），这种情况下你是对它所代表的字符串内的字符做赋值动作。如果这么使用，proxy 代表的将是“调用operator[] 函数”的那个字符串的左值运用。
以其他方式使用之。如果这么使用，proxy 表现的是“调用operator[] 函数”的那个字符串的右值运用。
Proxy classes 允许我们完成某些十分困难或几乎不可能完成的行为。多维数组是其中之一，左值/右值的区分是其中之二，压抑隐式转换（见条款 5）是其中之三。

最后，当 class 的身份从“与真实对象合作”移转到“与替身对象（proxies）合作”，往往会造成 class语义的改变，因为 proxyobjects 所展现的行为常常和真正对象的行为有些隐微差异。

### 条款31: 让函数根据一个以上的对象类型来决定如何虚化

假设你必须以 C++完成任务，也就是你必须自行想办法完成上述需求（常被称为 double-dispatching）。此名称来自面向对象程序设计社区，在那个领域里，人们把一个“虚函数调用动作”称为一个“message dispatch”（消息分派）。因此某个函数调用如果根据两个参数而虚化，自然而然地就被称为“double dispatch”。更广泛的情况（函数根据多个参数而虚化）则被称为 multiple dispatch。

虚函数+ RTTI（运行时期类型辨识）
只使用虚函数
自行仿真虚函数表格（Virtual Function Tables）
访问者模式

## 六、杂项讨论

### 条款32:
### 条款33:
### 条款34:
### 条款35:
Ref:</br>
[1]. [More Effective C++](https://hr-insist.github.io/C/More_Effective_C++%E9%98%85%E8%AF%BB%E7%AC%94%E8%AE%B0/)</br>
[2]. [《More Effective C++》读书笔记](https://zhuanlan.zhihu.com/p/368342605)</br>
[3]. https://blog.csdn.net/fengbingchun/article/details/102990753

---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://jianye0428.github.io/posts/more_effective_c-/  

