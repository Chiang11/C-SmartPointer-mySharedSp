/*
����Ҫ��ӣ�

ʵ�� make_shared �����������ڱ�׼���е� std::make_shared ���������ڷ���ش�������ָ����󣬲������ڴ����Ͷ����졣

֧���Զ�������������������� shared_ptr ʵ������Ӷ��Զ����������֧�֣����û��ܹ��Զ������ķ��䷽ʽ��

֧�����ü����Ż�������Գ���ʵ��һЩ���ü����Ż�������������ǰ�ͷŻ����ӳ��ͷŵȣ��������� shared_ptr �����ܡ�
*/


/*
6.3������ˣ�
�ƶ����캯�����ƶ���ֵ�������ʵ����Դ��ת�ƣ����Ч�ʺ�����ԡ�

��ȡ������ֵ�Ľӿڣ��ṩ������ȡ��ǰ����ָ�������������Դ�����ü�����

֧�ֱȽ��������ʵ������ָ�����֮��ıȽϲ�����������ԱȽϡ�

֧�ֿ�ָ���飺����һ���������������ָ���Ƿ�Ϊ�ա�

֧���������ͣ���չ����ָ��Ĺ��ܣ�ʹ���ܹ�����̬���顣

֧�ֶ�̬���������С���ṩ��������̬��������Ĵ�С�������ڶ�̬�����resize�������Լ�����reserveԤ���ռ�

֧����������֧��ʹ��lambda���ʽĬ��������Զ��������㷨��ʹ�û����Զ�����ָ��������������Զ�������������

֧��������ң��ṩ�����㷨��ʹ�û����Զ�����ָ������������в��Ҳ�����

֧�ֵ�������ʵ�ֵ������ӿڣ��Ա��û��ܹ�������ָ������������е���������

֧�ֶ��̰߳�ȫ�������̰߳�ȫ�Ļ��ƣ��Ա�֤�ڶ��̻߳����¶�����ָ��Ĳ����ǰ�ȫ�ġ�

֧���Զ���ɾ�����������û��Զ�����Դ���ͷŷ�ʽ������ʹ�ú���ָ�����������Ϊɾ������

֧������ת����ʵ�ִ�����ָ���������鵽�����������ͣ���std::vector��std::list�ȣ���ת�������������������������н�����

������֧�֣�������ָ��������Ӷ������õ�֧�֣�ʹ�ÿ��Դ���������ָ�롣�����ò����������ü��������������ڼ������Ƿ��Ѿ����ͷš�

*/

/*
�����ԣ�

ʹ��std::scoped_lock����std::lock_guard��std::scoped_lock��C++17�������һ�������ԣ��������������������Ļ����������ҿ��Ա�֤���쳣�������ȷ������

ʹ��std::optional����ָ�롣std::optional��C++17�������һ�������ԣ������Ա�ʾһ�����ܲ����ڵ�ֵ��ʹ��std::optional���Ա���ʹ�ÿ�ָ�룬���ҿ��Ը������ر����ͼ��

ʹ�ø��������C++20�������һ�������ԣ��������㶨��һ��Ҫ������Լ��ģ��������������������ʹ�ø��������ģ�������������׶������ҿ�����߱����������������

ʹ��std::exchange������ �ƶ����� �� �ƶ���ֵ ��ʵ�֡�std::exchange ��������������������һ��������Ҫ�滻ֵ�Ķ��󣬵ڶ�����������ֵ��
������ʹ�� std::move �������ֵ�ƶ���һ����ʱ�����У�Ȼ��ʹ�� std::forward ����ֵ����������󷵻�ԭ����ֵ��


*/

/*
�������Լ�ʵ�ֵ�����ָ���ࡣ���ڼ�������һ������Ĺ���ʦ��������������ָ�����ʵ����Ϊһ����Ŀд�������ְ�����ϡ�
��������Ҫ���ܽ���������ݣ�
1����Ŀ���ƣ�������15�֣�Ҫ���ص�ͻ��������ʲô����������Ҫ�Ĺ��ܣ�����ʲô���ģ���
2���ܽ��õ��ļ���ջ������C++ģ�塣
3���ܽ������Ŀ�Ƚϳ��ʣ��м�ֵ�ĵط���Ҫ������һ����ʹ����ʲôC++�����ԣ���
Ҫ��ʹ�þ�ʽ��ʹ�û����ʲô�����򷽷���֧��ʲô���ܣ�Ȼ���Ҫ����������ܣ�ʵ����ʲô���Ӷ���ʲô���á�����һ����Ҫ���г�����������ÿһ��������40�֣�
*/

#include<iostream>
#include<mutex>
#include<cassert>
#include<new>
#include<algorithm>
#include <type_traits>
using namespace std;

/*------------------------------------------- mySharedSp ��������-----------------------------------------------------*/
// C++20 ���
// ������һ����Ϊ Deletable �ĸ����ʹ���� std::invocable ������������� T �Ƿ��ǿɵ��õġ�
// �������Ҫ������ T �����ǿɵ��õģ������ܹ�����һ��ָ�� T ���͵�ָ����Ϊ������
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
// ���õ�ʱ����Ҫ ��ʽ��ָ�� �ڶ������� ����Ĵ�С��
// ��Ϊϵͳ�޷�ͨ�� new ���ص� ָ�� ֪�� �����С���������Ҫ�û�ָ��
template<typename T> // ��ģ�� ��ģ������б����ÿ����Ա�������У����� ��Ա����ģ�� ֻ�������Ա������������ʱ�Ż���
template<Deletable<T> Deleter> // ��Ա����ģ�� ��ģ������б���ʾ������캯����һ����Ա����ģ�塣
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
//ע�⣺���ص���һ������
//���������Լ�������������
//Ϊʲô������ֵ���ƶ���ֵ�з���ֵ��֧����ʽ��ֵ
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
/*
* ��һ�ַ�ʽʵ�ֿ������죺
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (const mySharedSp<T>& other) {
	if (this != &other) {
		countDelete();
		scoped_lock<std::mutex> lock(*other.m_mutex);
		++(*other.m_count);

		m_ptr = other.m_ptr;
		m_count = other.m_count;
		m_size = other.m_size;
		m_mutex = other.m_mutex;
		m_deleter = other.m_deleter;

	}
	cout << "���ÿ�����ֵ����" << endl;

	return *this;
}
*/



//�ƶ����캯��
//��ֵ���ã���Ҫ����ʱ����Ҫ��otherָ��ת������ֵ
//�� mySharedSp<int> p(move(new int(10)))
template<typename T>
mySharedSp<T>::mySharedSp(mySharedSp<T>&& other) {
	m_ptr = std::exchange(other.m_ptr, nullptr); // std::exchange �������� other �����еĳ�Ա������ֵ �ƶ� ����ǰ�����У����� other �����еĳ�Ա��������Ϊ nullptr��
	m_count = std::exchange(other.m_count, nullptr);
	m_size = std::exchange(other.m_size, nullptr);
	m_mutex = std::exchange(other.m_mutex, nullptr);
	m_deleter = move(other.m_deleter); // m_deleter ��std::function<void(T*)> ���͵Ķ���ʹ���ƶ�������ٲ���Ҫ�Ŀ���

	cout << "�����ƶ����캯��" << endl; //���������仯
}

//�ƶ���ֵ
//���ܽ���������Ϊconst����Ϊת����Դ������ʱ����ָ������ÿղ���
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
//����һ������ָ�����p��*p �͵ȼ��� p.operator*(), ���ص���p�����ָ��ָ��Ķ���
template<typename T>
T& mySharedSp<T>::operator* () const {
	cout << "ָ��Ľ����ã��õ�ָ��ָ��Ķ���" << endl;
	assert(m_ptr != NULL); // ����ǿգ���ʹ���̵߳�һ����������
	return *m_ptr;
}


//��Ա����
//����һ������ָ�����p��p->xxx �͵ȼ��� p.operator->()->xxx, ���ص���p�����ָ��ָ��Ķ���ĳ�Ա
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
//����ָ��ıȽ������ͨ��ֻ�Ƚ�������ָ��Ķ���
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
// ���� iterator �� mySharedSp ��ģ���һ����Ա���ͣ������Ҫʹ�� mySharedSp<T>::iterator ��ָ���������͡�
// ���ǣ����ڱ������ڴ���ģ�����ʱ��Ĭ�� mySharedSp<T>::iterator Ϊһ�� ��̬���ݳ�Ա ��������һ�����ͣ�
// �����Ҫ��ǰ����� typename �ؼ�������ʽ�ظ��߱����� mySharedSp<T>::iterator ��һ�����͡�
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
//����ǿ�ָ�룬����resize�򴴽�������
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
	//��ԭ�������ǿ�ָ�룬resize��ֱ�Ӵ���һ���µ�����
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

	// �����µ����顣������Ĭ�ϳ�ʼ����ֵ��ʼ�������ԣ���Ϊ������û�иı�m_size��ֵ��
	//���Բ������ݺ�����ֵ��δ���廹��0������size()���صĶ���ԭ����size
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
//������������Ϊ����ָ�룬��ָ��һ���������� const T& ���͵Ĳ���������bool���͵ĺ�����
//������������ʹ��lambda���ʽ��Ĭ��Ϊ��������
template<typename T>
void mySharedSp<T>::sort(iterator begin, iterator end, bool (*comp)(const T&, const T&)) {
	std::sort(begin, end, comp);
}

// �������
// ����һ��ָ���Ԫ�ص� T���� ָ��
// m_ptrָ��һ������ĵ�һ��Ԫ�أ���ô m_ptr + m_size �ͻ�õ�һ��ָ������ĩβ��ָ��
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
			cout << "���ü���Ϊ0��������Դ" << endl;
		}
	} // �ֲ�������
	if (ifDeleteMutex) { //���ݱ�־λ��������٣���ʱ������ *m_count == 0 �жϣ���Ϊ m_count �Ѿ�������
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
// ����һ�� mySharedSp<T>���͵Ķ���������ָ�� mySharedSp<T>���� ָ���ͬһ����Դ
template<typename T>
myWeakSp<T>::myWeakSp(const mySharedSp<T>& other) : m_ptr(other.m_w_ptr), m_w_count(other.m_count) {}

// �����������ò���������ü���
template<typename T>
myWeakSp<T>::~myWeakSp() {
	cout << "myWeakSp ����" << endl;
}

// ����Ƿ���ڡ�
// ͨ��������ü����Ƿ�Ϊ����ʵ�֡�������ü���Ϊ�㣬���ʾ�������Ѿ����ڣ�������ָ��Ķ����Ѿ������١�
template<typename T>
bool myWeakSp<T>::expired() const {
	return (m_w_count == nullptr || *m_w_count == 0);
}

// ���ڴ��������д���һ���µ� mySharedSp ����
// lock �������԰������ǰ�ȫ��ʹ�������á������ȼ���������Ƿ���ڣ�Ȼ���ٴ���һ���µ� mySharedSp ����
// ����������Ѿ����ڣ��� lock �����᷵��һ���յ� mySharedSp ����
// ʹ�� lock �������Ա���ֱ��ʹ�������ã��Ӷ���������ָ������⡣
// �����ȵ��� lock ����������һ���µ� mySharedSp ����Ȼ����ʹ������µ� mySharedSp ���������ʶ���
// �����ָ��û�й��ڣ�lock �����ᴴ��һ���µ�����ָ�룬�������ü�������ֵ��һ��
// ��������ʹ���´���������ָ��������ԭʼ����ʱ��ԭʼ����Ͳ��ᱻ�������١�
template<typename T>
mySharedSp<T> myWeakSp<T>::lock() const {
	return expired() ? mySharedSp<T>() : mySharedSp<T>(*this);
}


