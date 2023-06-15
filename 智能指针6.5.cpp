/*
还需要添加：

支持自定义删除器：允许用户自定义资源的释放方式，例如使用函数指针或函数对象作为删除器。

实现 make_shared 函数：类似于标准库中的 std::make_shared 函数，用于方便地创建智能指针对象，并进行内存分配和对象构造。

支持容器转换：实现从智能指针管理的数组到其他容器类型（如std::vector、std::list等）的转换操作，方便与其他容器进行交互。

支持自定义分配器。你可以在你的 shared_ptr 实现中添加对自定义分配器的支持，让用户能够自定义对象的分配方式。

支持引用计数优化。你可以尝试实现一些引用计数优化技术，例如提前释放或者延迟释放等，来提高你的 shared_ptr 的性能。
*/


/*
6.3：添加了：
移动构造函数和移动赋值运算符：实现资源的转移，提高效率和灵活性。

获取计数器值的接口：提供方法获取当前智能指针对象所管理资源的引用计数。

支持比较运算符：实现智能指针对象之间的比较操作，如相等性比较。

支持空指针检查：增加一个方法来检查智能指针是否为空。

支持数组类型：扩展智能指针的功能，使其能够管理动态数组。

支持动态调整数组大小：提供方法来动态调整数组的大小，类似于动态数组的resize操作；以及类似reserve预留空间

支持数组排序：支持使用lambda表达式默认升序的自定义排序算法，使用户可以对智能指针管理的数组进行自定义的排序操作。

支持数组查找：提供查找算法，使用户可以对智能指针管理的数组进行查找操作。

支持迭代器：实现迭代器接口，以便用户能够对智能指针管理的数组进行迭代操作。

支持多线程安全：增加线程安全的机制，以保证在多线程环境下对智能指针的操作是安全的。

*/

#include<iostream>
#include<stdlib.h>
#include<mutex>
#include<cassert>
#include<new>
#include<algorithm>
using namespace std;

template<typename T>
class mySharedSp {
private:
	int* m_count; //指向计数器的指针
	T* m_ptr; //多个智能指针对象共享一个引用计数，因此要定义为指针
	size_t* m_size; //数组大小
	std::mutex* m_mutex; // 互斥锁，定义为指针类型，仅仅用来保护引用计数器

public:
	mySharedSp(T* p = NULL, size_t size = 0);  //类内声明，需要指定默认参数，类外实现不需要再次指定
	mySharedSp(const mySharedSp<T>& other); //拷贝构造
	mySharedSp<T>& operator = (const mySharedSp<T>& other); //拷贝赋值
	mySharedSp(mySharedSp<T>&& other); //移动构造函数
	mySharedSp<T>& operator = (mySharedSp<T>&& other); //移动赋值，注意不能将参数设置为const
	~mySharedSp();

	//指针操作
	//函数后面加const，表示该成员函数不会改变成员变量
	T& operator* () const;//解引用，返回指针指向的对象的引用
	T* operator-> () const;//通过指针访问成员变量，返回的是成员变量，在智能指针类中是一个指针
	bool isNULL() const; //空指针检查
	int use_count() const; //获取引用值
	bool operator==(const mySharedSp<T>& other) const; //智能指针的比较操作
	bool operator!=(const mySharedSp<T>& other) const; //智能指针的比较操作

	// 迭代器
	// iterator 和 const_iterator 是一个类型的别名，因此是成员类型。
	using iterator = T*; // 将 iterator 定义为 T* 的别名，比如 T* find() 和 iterator find()是等价的
	using const_iterator = const T*; // 常量指针
	iterator begin(); // 常量对象调用
	iterator end();
	const_iterator begin() const; // 非常量对象只能调用这个，函数重载，根据对象的常量性选择不同版本，
	const_iterator end() const;

	// 数组操作
	T& operator[](size_t index) const; //通过 ptr[index] 访问数组元素
	size_t size() const; //获取数组大小
	void resize(int newSize); //动态调整数组的大小
	void reserve(int newSize); //预留空间
	void sort(iterator begin = m_ptr, iterator end = m_ptr + *m_size, 
		bool (*comp)(const T&, const T&) = [](const T& a, const T& b) { return a < b; });  // 数组排序函数，使用lambda表达式，默认为升序排列
	T* find(const T& value) const; //查找

private:
	void countAdd();
	void countDelete();

};



//构造  
//类内声明，需要指定默认参数，类外实现不需要再次指定
// 调用的时候需要 显式的指定 第二个参数 数组的大小，
// 因为系统无法通过 new 返回的 指针 知道 数组大小，因此这需要用户指定
template<typename T>
mySharedSp<T>::mySharedSp(T* p, size_t size) : m_ptr(p), m_count(new int(1)), m_size(new int(size)), m_mutex(new mutex) {
	// m_mutex(new mutex) 创建一个互斥锁
	cout << "调用构造函数" << endl;
}


//拷贝构造
template<typename T>
mySharedSp<T>::mySharedSp(const mySharedSp<T>& other) : m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size), m_mutex(other.m_mutex) {
	countAdd(); // 计数器加一
	cout << "调用拷贝构造函数" << endl;
}


//拷贝赋值
//注意：返回的是一个引用
//先自增，自减，避免自引用
//为什么拷贝赋值和移动赋值有返回值：支持链式赋值
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (const mySharedSp<T>& other) {
	lock_guard<std::mutex> lock(*other.m_mutex);
	++(*other.m_count);
	countDelete();

	m_ptr = other.m_ptr;
	m_count = other.m_count;
	m_size = other.m_size;
	m_mutex = other.m_mutex;

	cout << "调用拷贝赋值函数" << endl;

	return *this;
}
/*
* 另一种方式实现拷贝构造：
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (const mySharedSp<T>& other) {
	if (this != &other) {
		--(*m_count);
		if ((*m_count) == 0) {
			delete m_ptr;
			m_ptr = NULL;
			delete m_count;
			m_count = NULL;
		}
		++(*other.m_count); //确保当前对象和other对象的引用数相同并都是++的
		m_ptr = other.m_ptr;
		m_count = other.m_count;
		m_size = other.m_size;
	}
	cout << "调用拷贝赋值函数" << endl;

	return *this;
}
*/



//移动构造函数
//右值引用，需要调用时，需要把other指针转换成右值
//如 mySharedSp<int> p(move(new int(10)))
template<typename T>
mySharedSp<T>::mySharedSp(mySharedSp<T>&& other) {
	m_ptr = other.m_ptr;
	m_count = other.m_count;
	m_size = other.m_size;
	m_mutex = other.m_mutex;

	other.m_ptr = NULL;
	other.m_count = NULL;
	other.m_size = NULL;
	other.m_mutex = NULL;
	out << "调用移动构造函数" << endl; //引用数不变化
}

//移动赋值
//不能将参数设置为const，因为转移资源后会对临时对象指针进行置空操作
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (mySharedSp<T>&& other) {
	if (this != &other) {
		countDelete();
		m_ptr = other.m_ptr;
		m_count = other.m_count;
		m_size = other.m_size;
		m_mutex = other.m_mutex;

		other.m_ptr = NULL; //不能delete，因为这时候 m_ptr 和 other.m_ptr 都指向同一块资源
		other.m_count = NULL;
		other.size = NULL;
		other.m_mutex = NULL;

		cout << "调用移动赋值函数" << endl;
	}
	return *this; //如果是自赋值，直接返回当前对象
}


//析构
template<typename T>
mySharedSp<T>::~mySharedSp() {
	if (m_ptr == NULL) {  
		cout << "指针为空，直接返回" << endl;
		delete m_count;
		m_count = NULL;
		delete m_size；
		m_size = NULL;
		delete m_mutex;
		m_mutex = NULL;
		return;
	}
	countDeleete();
}



//解引用
//创建一个智能指针对象p，*p 就等价于 p.operator*(), 返回的是p管理的指针指向的对象
template<typename T>
T& mySharedSp<T>::operator* () const {
	cout << "指针的解引用，得到指针指向的对象" << endl;
	assert(m_ptr != NULL); // 如果是空，则使用线程的一个函数报错
	return *m_ptr;
}


//成员访问
//创建一个智能指针对象p，p->xxx 就等价于 p.operator->()->xxx, 返回的是p管理的指针指向的对象的成员
template<typename T>
T* mySharedSp<T>::operator-> () const {
	cout << "通过指针访问成员变量，得到指针指向的对象" << endl;
	assert(m_ptr != NULL); // 如果是空，则使用线程的一个函数报错
	return m_ptr;
}

//空指针检查
template<typename T>
bool mySharedSp<T>::isNULL() const {
	return m_ptr == NULL;
}

//获取引用数
template<typename T>
int mySharedSp<T>::use_count() const {
	return *m_count;
}


//智能指针的比较操作
//智能指针的比较运算符通常只比较它们所指向的对象
template<typename T>
bool mySharedSp<T>::operator==(const mySharedSp<T>& other) const {
	return m_ptr == other.m_ptr;
}
//智能指针的比较操作
template<typename T>
bool mySharedSp<T>::operator!=(const mySharedSp<T>& other) const {
	return !(m_ptr == other.m_ptr);
}


// 迭代器
// 由于 iterator 是 mySharedSp 类模板的一个成员类型，因此需要使用 mySharedSp<T>::iterator 来指定返回类型。
// 但是，由于编译器在处理模板代码时会默认 mySharedSp<T>::iterator 为一个 静态数据成员 ，而不是一个类型，
// 因此需要在前面加上 typename 关键字来显式地告诉编译器 mySharedSp<T>::iterator 是一个类型。
template<typename T>
typename mySharedSp<T>::iterator mySharedSp<T>::begin() {
	return m_ptr;
}
template<typename T>
typename mySharedSp<T>::iterator mySharedSp<T>::end() {
	if (m_ptr != nullptr && m_size != nullptr && *m_size > 0) {
		return m_ptr + *m_size;
	}
	throw std::runtime_error("Invalid end iterator");
}
template<typename T>
typename mySharedSp<T>::const_iterator mySharedSp<T>::begin() const {
	return m_ptr;
}
template<typename T>
typename mySharedSp<T>::const_iterator mySharedSp<T>::end() const {
	if (m_ptr != nullptr && m_size != nullptr && *m_size > 0) {
		return m_ptr + *m_size;
	}
	throw std::runtime_error("Invalid end iterator");
}


//动态数组管理，根据索引下标
template<typename T>
T& mySharedSp<T>::operator[](size_t index) const {
	if (m_ptr != NULL) {
		if (index < *m_size) {
			return m_ptr[index];
		}
		else {
			throw out_of_range("Index out of range");
		}
	}
	else {
		throw out_of_range("Null pointer exception");
	}
}

//获取数组大小
template<typename T>
size_t mySharedSp<T>::size() const {
	return *m_size;
}

//调整数组大小
//如果是空指针，调用resize则创建新数组
template<typename T>
void mySharedSp<T>::resize(int newSize) {
	lock_guard<mutex> lock(*m_mutex); // 用来保护 *m_size 这个共享资源
	if (newSize < 0) {
		throw invalid_argument("Invalid argument");
		return;
	}
	T* newPtr = new T[newSize](); //使用了 值初始化 语法，初始化数组元素全为0
	if (m_ptr != NULL) { //原来的数组非空，则赋值
		size_t eleNumsToCopy = min{ newSize, *m_size };
		for (int i = 0; i < eleNumsToCopy; ++i) {
			newPtr[i] = m_ptr[i];
		}
		delete[] m_ptr; // 释放原指针
	}
	//若原本就是是空指针，resize则直接创建一个新的数组
	m_ptr = newPtr; // 将新的数组指针和大小同步给成员变量
	*m_size = newSize;
}

// 预留空间
template<typename T>
void mySharedSp<T>::reserve(int capacity) {
	if (capacity < 0) {
		throw invalid_argument("Invalid argument");
		return;
	}
	if (capacity > *m_size) {
		// 创建新的数组。这里用默认初始化和值初始化都可以，因为函数中没有改变m_size的值，
		//所以不管扩容后后面的值是未定义还是0，调用size()返回的都是原数组size
		T* newPtr = new T[capacity]; 
		if (m_ptr != NULL) {
			for (size_t i = 0; i < *m_size; ++i) {
				newPtr[i] = m_ptr[i]; // 复制元素
			}
			delete[] m_ptr; // 释放原数组内存
		}
		m_ptr = newPtr; // 更新指针
		//不更新m_size
	}
}

//数组排序
//参数：第三个为函数指针，它指向一个接受两个 const T& 类型的参数并返回bool类型的函数。
//在类内声明中使用lambda表达式，默认为升序排列
template<typename T>
void mySharedSp<T>::sort(iterator begin, iterator end, bool (*comp)(const T&, const T&)) {
		std::sort(begin, end, comp);
}

// 数组查找
// 返回一个指向该元素的 T类型 指针
// m_ptr指向一个数组的第一个元素，那么 m_ptr + m_size 就会得到一个指向数组末尾的指针
template<typename T>
T* mySharedSp<T>::find(const T& value) const {
	return std::find(m_ptr, m_ptr + *m_size, value);
}


template<typename T>
void mySharedSp<T>::countAdd() {
	std::lock_guard<std::mutex> lock(*m_mutex)
	++(*m_count);
}

template<typename T>
void mySharedSp<T>::countDelete() {
	{
		lock_guard<mutex> lock(*m_mutex);
		--(*m_count); 
		bool ifDeleteMutex = false; // 是否销毁互斥锁的标志位，默认false
		if (*m_count == 0) {
			delete m_ptr;
			m_ptr = NULL;
			delete m_count;
			m_count = NULL;
			delete m_size;
			m_size = NULL;
			ifDeleteMutex = true; 
		}
	} // 局部作用域
	if (ifDeleteMutex) { //根据标志位最后再销毁，此时不能用 *m_count == 0 判断，因为 m_count 已经销毁了
		delete m_mutex;
		m_mutex = NULL;
	}
}

