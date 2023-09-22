/*
5.29:
添加基本函数：构造析构，拷贝构造和赋值，移动构造和赋值
添加函数：解引用，成员访问

5.30：
添加获得计数器值的函数，添加对象比较函数
支持空指针检查：增加一个方法来检查智能指针是否为空。
支持数组类型：扩展智能指针的功能，使其能够管理动态数组。
*/

/*
还需要添加：
支持自定义删除器：允许用户自定义资源的释放方式，例如使用函数指针或函数对象作为删除器。
实现 make_shared 函数：类似于标准库中的 std::make_shared 函数，用于方便地创建智能指针对象，并进行内存分配和对象构造。
支持动态调整数组大小：提供方法来动态调整数组的大小，类似于动态数组的resize操作。
支持迭代器：实现迭代器接口，以便用户能够对智能指针管理的数组进行迭代操作。
支持切片操作：允许用户对智能指针管理的数组进行切片操作，提取部分数组元素。
支持排序和查找：提供排序和查找算法，使用户可以对智能指针管理的数组进行排序和查找操作。
支持容器转换：实现从智能指针管理的数组到其他容器类型（如std::vector、std::list等）的转换操作，方便与其他容器进行交互。
支持多线程安全：增加线程安全的机制，以保证在多线程环境下对智能指针的操作是安全的。
支持自定义分配器。在 shared_ptr 实现中添加对自定义分配器的支持，让用户能够自定义对象的分配方式。
支持引用计数优化。实现一些引用计数优化技术，例如提前释放或者延迟释放等，来提高 shared_ptr 的性能。
*/




#include<iostream>
#include<stdlib.h>
#include<mutex>

using namespace std;

template<typename T>
class mySharedSp {
private:
	int* m_count; //指向计数器的指针
	T* m_ptr; //多个智能指针对象共享一个引用计数，因此要定义为指针
	size_t m_size; //数组大小，智能指针只提供获取管理数组的方法，该类中不提供修改数组大小的方法，因此定义为非指针，避免多个智能指针管理同一个数组
public:
	mySharedSp(T* p = NULL, size_t size = 0);  //类内声明，需要指定默认参数，类外实现不需要再次指定
	mySharedSp(const mySharedSp<T>& other); //拷贝构造
	mySharedSp<T>& operator = (const mySharedSp<T>& other); //拷贝赋值
	mySharedSp(mySharedSp<T>&& other); //移动构造函数
	mySharedSp<T>& operator = (mySharedSp<T>&& other); //移动赋值，注意不能将参数设置为const
	~mySharedSp();

	//函数后面加const，表示该成员函数不会改变成员变量
	T& opetarer* () const;//解引用，返回指针指向的对象的引用
	T* operator-> () const;//通过指针访问成员变量，返回的是成员变量，在智能指针类中是一个指针
	bool isNULL() const; //空指针检查
	int use_count() const; //获取引用值
	bool operator==(const mySharedSp<T>& other) const; //智能指针的比较操作
	bool operator!=(const mySharedSp<T>& other) const; //智能指针的比较操作
	T& operator[](size_t index) const; //通过 ptr[index] 访问数组元素
	size_t size() const;
};


//构造  
//类内声明，需要指定默认参数，类外实现不需要再次指定
template<typename T>
mySharedSp<T>::mySharedSp(T* p, size_t size) : m_ptr(p), m_count(new int(1)), m_size(size) {
	cout << "调用构造函数" << endl;
}


//拷贝构造
template<typename T>
mySharedSp<T>::mySharedSp(const mySharedSp<T>& other) : m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size) {
	++(*m_count);
	cout << "调用拷贝构造函数" << endl;
}


//拷贝赋值
//注意：返回的是一个引用
//先自增，自减，避免自引用
//为什么拷贝赋值和移动赋值有返回值：支持链式赋值
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (const mySharedSp<T>& other) {
	++(*other.m_count);
	--(*m_count);
	if (*m_count == 0) {
		delete m_ptr;
		m_ptr = NULL; //只delete不置空会产生悬空指针
		delete m_count;
		m_count = NULL; //m_size 是非指针变量，会在对象销毁时自动被销毁
	}
	m_ptr = other.m_ptr;
	m_count = other.m_count;
	m_size = other.m_size;
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
mySharedSp<T>::mySharedSp(mySharedSp<T>&& other) :m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size) {
	other.m_ptr = NULL;
	other.m_count = NULL;
	out << "调用移动构造函数" << endl; //引用数不变化
}

//移动赋值
//不能将参数设置为const，因为转移资源后会对临时对象指针进行置空操作
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (mySharedSp<T>&& other) {
	if (this != &other) {
		--(*m_count);
		if ((*m_count) == 0) {
			delete m_ptr;
			m_ptr = NULL; //只delete不置空会产生悬空指针
			delete m_count;
			m_count = NULL;
		}
		m_ptr = other.m_ptr;
		m_count = other.m_count;
		m_size = other.m_size;

		other.m_ptr = NULL; //不能delete，因为这时候 m_ptr 和 other.m_ptr 都指向同一块资源
		other.m_count = NULL;

		cout << "调用移动赋值函数" << endl;
	}
	return *this; //如果是自赋值，直接返回当前对象
}


//析构
template<typename T>
mySharedSp<T>::~mySharedSp() {
	--(*m_count);
	if ((*m_count) == 0) {
		delete m_ptr;
		m_ptr = NULL;
		delete m_count;
		m_count = NULL;
		cout << "调用析构函数，现在已无指针指向" << endl;
	}
	else {
		cout << "调用析构函数，仍有指针指向" << endl;
	}
}



//解引用
//创建一个智能指针对象p，*p 就等价于 p.operator*(), 返回的是p管理的指针指向的对象
template<typename T>
T& mySharedSp<T>::operator* () const {
	cout << "指针的解引用，得到指针指向的对象" << endl;
	return *m_ptr;
}


//成员访问
//创建一个智能指针对象p，p->xxx 就等价于 p.operator->()->xxx, 返回的是p管理的指针指向的对象的成员
template<typename T>
T* mySharedSp<T>::operator-> () const {
	cout << "通过指针访问成员变量，得到指针指向的对象" << endl;
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

//动态数组管理
template<typename T>
T& mySharedSp<T>::operator[](size_t index) const {
	if (m_ptr != NULL) {
		if (index < m_size) {
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
	return m_size;
}
