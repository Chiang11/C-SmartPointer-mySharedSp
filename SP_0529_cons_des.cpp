/*
5.29:
添加基本函数：构造析构，拷贝构造和赋值，移动构造和赋值
添加函数：解引用，成员访问
*/

/*
还需要添加：
移动构造函数和移动赋值运算符：实现资源的转移，提高效率和灵活性。
支持自定义删除器：允许用户自定义资源的释放方式，例如使用函数指针或函数对象作为删除器。
获取计数器值的接口：提供方法获取当前智能指针对象所管理资源的引用计数。
支持比较运算符：实现智能指针对象之间的比较操作，如相等性比较。
支持空指针检查：增加一个方法来检查智能指针是否为空。
实现 make_shared 函数：类似于标准库中的 std::make_shared 函数，用于方便地创建智能指针对象，并进行内存分配和对象构造。
支持数组类型：扩展智能指针的功能，使其能够管理动态数组。
*/


#include<iostream>
#include<stdlib.h>
using namespace std;

template<typename T>
class mySharedsp {
private:
	int* m_count; //指向计数器的指针
	T* m_ptr;
public:
	mySharedsp(T* p = NULL);  //类内声明，需要指定默认参数，类外实现不需要再次指定
	mySharedsp(const mySharedsp<T>& other); //拷贝构造
	mySharedsp<T>& operator = (const mySharedsp<T>& other); //拷贝赋值
	mySharedsp(mySharedsp<T>&& other); //移动构造函数
	mySharedsp<T>& operator = (mySharedsp<T>&& other); //移动赋值，注意不能将参数设置为const
	~mySharedsp();

	T& opetarer* () const;//解引用，返回指针指向的对象的引用
	T* operator-> () const;//通过指针访问成员变量，返回的是成员变量，在智能指针类中是一个指针
};

//构造  
//类内声明，需要指定默认参数，类外实现不需要再次指定
template<typename T>
mySharedsp<T>::mySharedsp(T* p) : m_ptr(p), m_count(new int(1)) {
	cout << "调用构造函数" << endl;
}

//拷贝构造
template<typename T>
mySharedsp<T>::mySharedsp(const mySharedsp<T>& other) : m_ptr(other.m_ptr), m_count(other.m_count) {
	++(*m_count);
	cout << "调用拷贝构造函数" << endl;
}

//拷贝赋值
//注意：返回的是一个引用
//先自增，自减，避免自引用
//为什么拷贝赋值和移动赋值有返回值：支持链式赋值
template<typename T>
mySharedsp<T>& mySharedsp<T>::operator = (const mySharedsp<T>& other) {
	++(*other.m_count);
	--(*m_count);
	if (*m_count == 0) {
		delete m_ptr;
		m_ptr = NULL; //只delete不置空会产生悬空指针
		delete m_count;
		m_count = NULL;
	}
	m_ptr = other.m_ptr;
	m_count = other.m_count;
	cout << "调用拷贝赋值函数" << endl;

	return *this;
}
/*
* 另一种方式实现拷贝构造：
template<typename T>
mySharedsp<T>& mySharedsp<T>::operator = (const mySharedsp<T>& other) {
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
	}
	cout << "调用拷贝赋值函数" << endl;

	return *this;
}
*/


//移动构造函数
//右值引用，需要调用时，需要把other指针转换成右值
//如 mySharedsp<int> p(move(new int(10)))
template<typename T>
mySharedsp<T>::mySharedsp(mySharedsp<T>&& other) :m_ptr(other.m_ptr), m_count(other.m_count) {
	other.m_ptr = NULL;
	other.m_count = NULL;
	out << "调用移动构造函数" << endl; //引用数不变化
}

//移动赋值
//不能将参数设置为const，因为转移资源后会对临时对象指针进行置空操作
template<typename T>
mySharedsp<T>& mySharedsp<T>::operator = (mySharedsp<T>&& other) {
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

		other.m_ptr = NULL; //不能delete，因为这时候 m_ptr 和 other.m_ptr 都指向同一块资源
		other.m_count = NULL;

		cout << "调用移动赋值函数" << endl;
	}
	return *this; //如果是自赋值，直接返回当前对象
}


//析构
template<typename T>
mySharedsp<T>::~mySharedsp() {
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
T& mySharedsp<T>::operator* () const {
	cout << "指针的解引用，得到指针指向的对象" << endl;
	return *m_ptr;
}

//成员访问
//创建一个智能指针对象p，p->xxx 就等价于 p.operator->()->xxx, 返回的是p管理的指针指向的对象的成员
template<typename T>
T* mySharedsp<T>::operator-> () const {
	cout << "通过指针访问成员变量，得到指针指向的对象" << endl;
	return m_ptr;
}



