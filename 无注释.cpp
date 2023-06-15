template<typename T>
concept Deletable = std::invocable<T, T*>;

template<typename T>
class mySharedSp {
private:
	optional<int*> m_count; //ָ���������ָ��
	optional<T*> m_ptr;
	optional<size_t*> m_size; //�����С
	optional<mutex*> m_mutex; // ������������Ϊָ�����ͣ����������������ü�����
	function<void(T*)> m_deleter; // ɾ��������

public:
	template<Deletable<T> Deleter = std::default_delete<T>> //���캯�� ��� ��Ա����ģ�� �� ģ������б�ָ����Ĭ������
	mySharedSp(T* p = nullptr, size_t size = 0, Deleter d = std::default_delete<T>());  //������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
	mySharedSp(const mySharedSp<T>& other); //��������
	mySharedSp<T>& operator = (const mySharedSp<T>& other); //������ֵ
	mySharedSp(mySharedSp<T>&& other); //�ƶ����캯��
	mySharedSp<T>& operator = (mySharedSp<T>&& other); //�ƶ���ֵ��ע�ⲻ�ܽ���������Ϊconst
	~mySharedSp();

	//ָ�����
	//���������const����ʾ�ó�Ա��������ı��Ա����
	T& operator* () const;//�����ã�����ָ��ָ��Ķ��������
	T* operator-> () const;//ͨ��ָ����ʳ�Ա���������ص��ǳ�Ա������������ָ��������һ��ָ��
	bool isNULL() const; //��ָ����
	int use_count() const; //��ȡ����ֵ
	bool operator==(const mySharedSp<T>& other) const; //����ָ��ıȽϲ���
	bool operator!=(const mySharedSp<T>& other) const; //����ָ��ıȽϲ���

	// ������
	// iterator �� const_iterator ��һ�����͵ı���������ǳ�Ա���͡�
	using iterator = T*; // �� iterator ����Ϊ T* �ı��������� T* find() �� iterator find()�ǵȼ۵�
	using const_iterator = const T*; // ����ָ��
	iterator begin(); // �����������
	iterator end();
	const_iterator begin() const; // �ǳ�������ֻ�ܵ���������������أ����ݶ���ĳ�����ѡ��ͬ�汾��
	const_iterator end() const;

	// �������
	T& operator[](size_t index) const; //ͨ�� ptr[index] ��������Ԫ��
	size_t size() const; //��ȡ�����С
	void resize(int newSize); //��̬��������Ĵ�С
	void reserve(int newSize); //Ԥ���ռ�
	void sort(iterator begin = m_ptr, iterator end = m_ptr + *m_size,
		bool (*comp)(const T&, const T&) = [](const T& a, const T& b) { return a < b; });  // ������������ʹ��lambda���ʽ��Ĭ��Ϊ��������
	T* find(const T& value) const; //����

	// ����ת��
	template<typename Container>
	Container convertToContainer() const;


private:
	void countAdd();
	void countDelete();
};

/*------------------------------------------- myWeakSp ��������-----------------------------------------------------*/
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


/*---------------------------------- mySharedSp ���ⶨ��--------------------------------------------------*/

// ����  
mySharedSp<T>::mySharedSp(T* p, size_t size, Deleter d) :
	m_ptr(p), m_count(new int(1)), m_size(new size_t(size)), m_mutex(new mutex), m_deleter(d) { // m_mutex(new mutex) ����һ��������
	cout << "���ù��캯��" << endl;
}


//��������
template<typename T>
mySharedSp<T>::mySharedSp(const mySharedSp<T>& other) :
	m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size), m_mutex(other.m_mutex), m_deleter(other.m_deleter) {
	countAdd(); // ��������һ
	cout << "���ÿ������캯��" << endl;
}


//������ֵ
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

	cout << "���ÿ�����ֵ����" << endl;

	return *this;
}



//�ƶ����캯��
template<typename T>
mySharedSp<T>::mySharedSp(mySharedSp<T>&& other) {
	m_ptr = std::exchange(other.m_ptr, nullptr); 
	m_count = std::exchange(other.m_count, nullptr);
	m_size = std::exchange(other.m_size, nullptr);
	m_mutex = std::exchange(other.m_mutex, nullptr);
	m_deleter = move(other.m_deleter); // m_deleter ��std::function<void(T*)> ���͵Ķ���ʹ���ƶ�������ٲ���Ҫ�Ŀ���

	cout << "�����ƶ����캯��" << endl; //���������仯
}

//�ƶ���ֵ
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (mySharedSp<T>&& other) {
	if (this != &other) {
		countDelete();
		m_ptr = std::exchange(other.m_ptr, nullptr);
		m_count = std::exchange(other.m_count, nullptr);
		m_size = std::exchange(other.m_size, nullptr);
		m_mutex = std::exchange(other.m_mutex, nullptr);
		m_deleter = move(other.m_deleter);

		cout << "�����ƶ���ֵ����" << endl;
	}
	return *this; //������Ը�ֵ��ֱ�ӷ��ص�ǰ����
}


//����
template<typename T>
mySharedSp<T>::~mySharedSp() {
	if (m_ptr == NULL) {
		cout << "ָ��Ϊ�գ�ֱ�ӷ���" << endl;
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



//������
template<typename T>
T& mySharedSp<T>::operator* () const {
	cout << "ָ��Ľ����ã��õ�ָ��ָ��Ķ���" << endl;
	assert(m_ptr != NULL); // ����ǿգ���ʹ���̵߳�һ����������
	return *m_ptr;
}


//��Ա����
template<typename T>
T* mySharedSp<T>::operator-> () const {
	cout << "ͨ��ָ����ʳ�Ա�������õ�ָ��ָ��Ķ���" << endl;
	assert(m_ptr != NULL); // ����ǿգ���ʹ���̵߳�һ����������
	return m_ptr;
}

//��ָ����
template<typename T>
bool mySharedSp<T>::isNULL() const {
	return m_ptr == NULL;
}

//��ȡ������
template<typename T>
int mySharedSp<T>::use_count() const {
	return *m_count;
}


//����ָ��ıȽϲ���
template<typename T>
bool mySharedSp<T>::operator==(const mySharedSp<T>& other) const {
	return m_ptr == other.m_ptr;
}
//����ָ��ıȽϲ���
template<typename T>
bool mySharedSp<T>::operator!=(const mySharedSp<T>& other) const {
	return !(m_ptr == other.m_ptr);
}


// ������
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


//��̬����������������±�
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

//��ȡ�����С
template<typename T>
size_t mySharedSp<T>::size() const {
	return *m_size;
}

//���������С
template<typename T>
void mySharedSp<T>::resize(int newSize) {
	scoped_lock<mutex> lock(*m_mutex); // �������� *m_size ���������Դ
	if (newSize < 0) {
		throw invalid_argument("Invalid argument");
		return;
	}
	T* newPtr = new T[newSize](); //ʹ���� ֵ��ʼ�� �﷨����ʼ������Ԫ��ȫΪ0
	if (m_ptr != NULL) { //ԭ��������ǿգ���ֵ
		size_t eleNumsToCopy = min{ static_cast<size_t>newSize, *m_size };
		for (int i = 0; i < eleNumsToCopy; ++i) {
			newPtr[i] = m_ptr[i];
		}
		delete[] m_ptr; // �ͷ�ԭָ��
	}
	m_ptr = newPtr; // ���µ�����ָ��ʹ�Сͬ������Ա����
	*m_size = newSize;
}

// Ԥ���ռ�
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
			newPtr[i] = m_ptr[i]; // ����Ԫ��
		}
		delete[] m_ptr; // �ͷ�ԭ�����ڴ�
	}
	m_ptr = newPtr; // ����ָ��
	//������m_size
}

//��������
template<typename T>
void mySharedSp<T>::sort(iterator begin, iterator end, bool (*comp)(const T&, const T&)) {
	std::sort(begin, end, comp);
}

// �������
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
		bool ifDeleteMutex = false; // �Ƿ����ٻ������ı�־λ��Ĭ��false
		if (*m_count == 0) {
			m_deleter(m_ptr);
			delete m_count;
			m_count = NULL;
			delete m_size;
			m_size = NULL;
			ifDeleteMutex = true;
		}
	} // �ֲ�������
	if (ifDeleteMutex) { 
		delete m_mutex;
		m_mutex = NULL;
	}
}

// ����ת��
template<typename T>
template<typename Container>
Container mySharedSp<T>::convertToContainer() const {
	Container result(begin(), end());
	return result;
}



/*---------------------------------- myWeakSp ���ⶨ��--------------------------------------------------*/
// ��������
template<typename T>
myWeakSp<T>::myWeakSp(const mySharedSp<T>& other) : m_ptr(other.m_w_ptr), m_w_count(other.m_count) {}

// �����������ò���������ü���
template<typename T>
myWeakSp<T>::~myWeakSp() {}

// ����Ƿ���ڡ�
template<typename T>
bool myWeakSp<T>::expired() const {
	return (m_w_count == nullptr || *m_w_count == 0);
}

template<typename T>
mySharedSp<T> myWeakSp<T>::lock() const {
	return expired() ? mySharedSp<T>() : mySharedSp<T>(*this);
}


