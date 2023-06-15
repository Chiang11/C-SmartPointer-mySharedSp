template<typename T>
concept Deletable = std::invocable<T, T*>;

template<typename T>
class mySharedSp {
private:
	optional<int*> m_count; //指向计数器的指针
	optional<T*> m_ptr;
	optional<size_t*> m_size; //数组大小
	optional<mutex*> m_mutex; // 互斥锁，定义为指针类型，仅仅用来保护引用计数器
	function<void(T*)> m_deleter; // 删除器对象

public:
	template<Deletable<T> Deleter = std::default_delete<T>> //构造函数 这个 成员函数模板 的 模板参数列表，指定了默认类型
	mySharedSp(T* p = nullptr, size_t size = 0, Deleter d = std::default_delete<T>());  //类内声明，需要指定默认参数，类外实现不需要再次指定
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

	// 容器转换
	template<typename Container>
	Container convertToContainer() const;


private:
	void countAdd();
	void countDelete();
};

/*------------------------------------------- myWeakSp 类内声明-----------------------------------------------------*/
template<typename T>
class myWeakSp {
private:
	int* m_w_count;
	T* m_w_ptr;
public:
	myWeakSp(const mySharedSp<T>& other);
	~myWeakSp();
	bool expired() const;
	mySharedSp<T> lock() const;
};


/*---------------------------------- mySharedSp 类外定义--------------------------------------------------*/

// 构造  
mySharedSp<T>::mySharedSp(T* p, size_t size, Deleter d) :
	m_ptr(p), m_count(new int(1)), m_size(new size_t(size)), m_mutex(new mutex), m_deleter(d) { // m_mutex(new mutex) 创建一个互斥锁
	cout << "调用构造函数" << endl;
}


//拷贝构造
template<typename T>
mySharedSp<T>::mySharedSp(const mySharedSp<T>& other) :
	m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size), m_mutex(other.m_mutex), m_deleter(other.m_deleter) {
	countAdd(); // 计数器加一
	cout << "调用拷贝构造函数" << endl;
}


//拷贝赋值
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (const mySharedSp<T>& other) {
	scoped_lock<std::mutex> lock(*other.m_mutex);
	++(*other.m_count);
	countDelete();

	m_ptr = other.m_ptr;
	m_count = other.m_count;
	m_size = other.m_size;
	m_mutex = other.m_mutex;
	m_deleter = other.m_deleter;

	cout << "调用拷贝赋值函数" << endl;

	return *this;
}



//移动构造函数
template<typename T>
mySharedSp<T>::mySharedSp(mySharedSp<T>&& other) {
	m_ptr = std::exchange(other.m_ptr, nullptr); 
	m_count = std::exchange(other.m_count, nullptr);
	m_size = std::exchange(other.m_size, nullptr);
	m_mutex = std::exchange(other.m_mutex, nullptr);
	m_deleter = move(other.m_deleter); // m_deleter 是std::function<void(T*)> 类型的对象，使用移动语义减少不必要的开销

	cout << "调用移动构造函数" << endl; //引用数不变化
}

//移动赋值
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (mySharedSp<T>&& other) {
	if (this != &other) {
		countDelete();
		m_ptr = std::exchange(other.m_ptr, nullptr);
		m_count = std::exchange(other.m_count, nullptr);
		m_size = std::exchange(other.m_size, nullptr);
		m_mutex = std::exchange(other.m_mutex, nullptr);
		m_deleter = move(other.m_deleter);

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
		delete m_size;
		m_size = NULL;
		delete m_mutex;
		m_mutex = NULL;
		return;
	}
	countDelete();
}



//解引用
template<typename T>
T& mySharedSp<T>::operator* () const {
	cout << "指针的解引用，得到指针指向的对象" << endl;
	assert(m_ptr != NULL); // 如果是空，则使用线程的一个函数报错
	return *m_ptr;
}


//成员访问
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
template<typename T>
typename mySharedSp<T>::iterator mySharedSp<T>::begin() {
	if (m_ptr != nullptr) {
		return m_ptr;
	}
	throw std::runtime_error("Invalid begin iterator");
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
	if (m_ptr != nullptr) {
		return m_ptr;
	}
	throw std::runtime_error("Invalid begin iterator");
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
template<typename T>
void mySharedSp<T>::resize(int newSize) {
	scoped_lock<mutex> lock(*m_mutex); // 用来保护 *m_size 这个共享资源
	if (newSize < 0) {
		throw invalid_argument("Invalid argument");
		return;
	}
	T* newPtr = new T[newSize](); //使用了 值初始化 语法，初始化数组元素全为0
	if (m_ptr != NULL) { //原来的数组非空，则赋值
		size_t eleNumsToCopy = min{ static_cast<size_t>newSize, *m_size };
		for (int i = 0; i < eleNumsToCopy; ++i) {
			newPtr[i] = m_ptr[i];
		}
		delete[] m_ptr; // 释放原指针
	}
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
	if (capacity <= *m_size) return;

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

//数组排序
template<typename T>
void mySharedSp<T>::sort(iterator begin, iterator end, bool (*comp)(const T&, const T&)) {
	std::sort(begin, end, comp);
}

// 数组查找
template<typename T>
T* mySharedSp<T>::find(const T& value) const {
	return std::find(m_ptr, m_ptr + *m_size, value);
}


template<typename T>
void mySharedSp<T>::countAdd() {
	scoped_lock<std::mutex> lock(*m_mutex)
		++(*m_count);
}

template<typename T>
void mySharedSp<T>::countDelete() {
	{
		scoped_lock<mutex> lock(*m_mutex);
		--(*m_count);
		bool ifDeleteMutex = false; // 是否销毁互斥锁的标志位，默认false
		if (*m_count == 0) {
			m_deleter(m_ptr);
			delete m_count;
			m_count = NULL;
			delete m_size;
			m_size = NULL;
			ifDeleteMutex = true;
		}
	} // 局部作用域
	if (ifDeleteMutex) { 
		delete m_mutex;
		m_mutex = NULL;
	}
}

// 容器转换
template<typename T>
template<typename Container>
Container mySharedSp<T>::convertToContainer() const {
	Container result(begin(), end());
	return result;
}



/*---------------------------------- myWeakSp 类外定义--------------------------------------------------*/
// 拷贝构造
template<typename T>
myWeakSp<T>::myWeakSp(const mySharedSp<T>& other) : m_ptr(other.m_w_ptr), m_w_count(other.m_count) {}

// 析构，弱引用不会减少引用计数
template<typename T>
myWeakSp<T>::~myWeakSp() {}

// 检查是否过期。
template<typename T>
bool myWeakSp<T>::expired() const {
	return (m_w_count == nullptr || *m_w_count == 0);
}

template<typename T>
mySharedSp<T> myWeakSp<T>::lock() const {
	return expired() ? mySharedSp<T>() : mySharedSp<T>(*this);
}


