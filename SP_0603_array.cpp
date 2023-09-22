/*
5.29:
��ӻ���������������������������͸�ֵ���ƶ�����͸�ֵ
��Ӻ����������ã���Ա����

5.30��
��ӻ�ü�����ֵ�ĺ�������Ӷ���ȽϺ���
֧�ֿ�ָ���飺����һ���������������ָ���Ƿ�Ϊ�ա�
֧���������ͣ���չ����ָ��Ĺ��ܣ�ʹ���ܹ�����̬���顣

6.3��
֧�ֶ�̬���������С�������ڶ�̬�����resize�������Լ�����reserveԤ���ռ�
֧����������֧��ʹ��lambda���ʽĬ��������Զ��������㷨��ʹ�û����Զ�����ָ��������������Զ�������������
֧�ֵ�������ʵ�ֵ������ӿڣ��Ա��û��ܹ�������ָ������������е���������
*/

/*
����Ҫ��ӣ�
֧���Զ���ɾ�����������û��Զ�����Դ���ͷŷ�ʽ������ʹ�ú���ָ�����������Ϊɾ������
ʵ�� make_shared �����������ڱ�׼���е� std::make_shared ���������ڷ���ش�������ָ����󣬲������ڴ����Ͷ����졣
֧����Ƭ�����������û�������ָ���������������Ƭ��������ȡ��������Ԫ�ء�
֧������ת����ʵ�ִ�����ָ���������鵽�����������ͣ���std::vector��std::list�ȣ���ת�������������������������н�����
֧�ֶ��̰߳�ȫ�������̰߳�ȫ�Ļ��ƣ��Ա�֤�ڶ��̻߳����¶�����ָ��Ĳ����ǰ�ȫ�ġ�
֧���Զ������������ shared_ptr ʵ������Ӷ��Զ����������֧�֣����û��ܹ��Զ������ķ��䷽ʽ��
֧�����ü����Ż���ʵ��һЩ���ü����Ż�������������ǰ�ͷŻ����ӳ��ͷŵȣ������ shared_ptr �����ܡ�
*/



#include<iostream>
#include<stdlib.h>
#include<mutex>
#include<cassert>

using namespace std;

template<typename T>
class mySharedSp {
private:
	int* m_count; //ָ���������ָ��
	T* m_ptr; //�������ָ�������һ�����ü��������Ҫ����Ϊָ��
	size_t m_size; //�����С������������ָ�����ͬһ������

public:
	mySharedSp(T* p = NULL, size_t size = 0);  //������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
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
	void resize(size_t newSize); //��̬��������Ĵ�С
	void reserve(size_t newSize); //Ԥ���ռ�
	void sort(iterator begin, iterator end, bool(*comp)(const T&, const T&) = [](const T& a, const T& b) { return a < b; }); // ������������ʹ��lambda���ʽ��Ĭ��Ϊ��������
	T* find(const T& value) const; //����

};



//����  
//������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
// ���õ�ʱ����Ҫ ��ʽ��ָ�� �ڶ������� ����Ĵ�С��
// ��Ϊϵͳ�޷�ͨ�� new ���ص� ָ�� ֪�� �����С���������Ҫ�û�ָ��
template<typename T>
mySharedSp<T>::mySharedSp(T* p, size_t size) : m_ptr(p), m_count(new int(1)), m_size(size) {
	cout << "���ù��캯��" << endl;
}


//��������
template<typename T>
mySharedSp<T>::mySharedSp(const mySharedSp<T>& other) : m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size) {
	++(*m_count);
	cout << "���ÿ������캯��" << endl;
}


//������ֵ
//ע�⣺���ص���һ������
//���������Լ�������������
//Ϊʲô������ֵ���ƶ���ֵ�з���ֵ��֧����ʽ��ֵ
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (const mySharedSp<T>& other) {
	++(*other.m_count);
	--(*m_count);
	if (*m_count == 0) {
		delete m_ptr;
		m_ptr = NULL; //ֻdelete���ÿջ��������ָ��
		delete m_count;
		m_count = NULL; //m_size �Ƿ�ָ����������ڶ�������ʱ�Զ�������
	}
	m_ptr = other.m_ptr;
	m_count = other.m_count;
	m_size = other.m_size;
	cout << "���ÿ�����ֵ����" << endl;

	return *this;
}
/*
* ��һ�ַ�ʽʵ�ֿ������죺
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
		++(*other.m_count); //ȷ����ǰ�����other�������������ͬ������++��
		m_ptr = other.m_ptr;
		m_count = other.m_count;
		m_size = other.m_size;
	}
	cout << "���ÿ�����ֵ����" << endl;

	return *this;
}
*/



//�ƶ����캯��
//��ֵ���ã���Ҫ����ʱ����Ҫ��otherָ��ת������ֵ
//�� mySharedSp<int> p(move(new int(10)))
template<typename T>
mySharedSp<T>::mySharedSp(mySharedSp<T>&& other) :m_ptr(other.m_ptr), m_count(other.m_count), m_size(other.m_size) {
	other.m_ptr = NULL;
	other.m_count = NULL;
	out << "�����ƶ����캯��" << endl; //���������仯
}

//�ƶ���ֵ
//���ܽ���������Ϊconst����Ϊת����Դ������ʱ����ָ������ÿղ���
template<typename T>
mySharedSp<T>& mySharedSp<T>::operator = (mySharedSp<T>&& other) {
	if (this != &other) {
		--(*m_count);
		if ((*m_count) == 0) {
			delete m_ptr;
			m_ptr = NULL; //ֻdelete���ÿջ��������ָ��
			delete m_count;
			m_count = NULL;
		}
		m_ptr = other.m_ptr;
		m_count = other.m_count;
		m_size = other.m_size;

		other.m_ptr = NULL; //����delete����Ϊ��ʱ�� m_ptr �� other.m_ptr ��ָ��ͬһ����Դ
		other.m_count = NULL;

		cout << "�����ƶ���ֵ����" << endl;
	}
	return *this; //������Ը�ֵ��ֱ�ӷ��ص�ǰ����
}


//����
template<typename T>
mySharedSp<T>::~mySharedSp() {
	if (m_ptr == NULL) {  // ��Ҫ�ж��𣿣���������������������
		cout << "ָ��Ϊ�գ�ֱ�ӷ���" << endl;
		delete m_count;
		m_count = NULL;
		return;
	}
	--(*m_count);
	if ((*m_count) == 0) {
		delete m_ptr;
		m_ptr = NULL;
		delete m_count;
		m_count = NULL;
		cout << "����������������������ָ��ָ��" << endl;
	}
	else {
		cout << "������������������ָ��ָ��" << endl;
	}
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
	return m_ptr;
}
template<typename T>
typename mySharedSp<T>::iterator mySharedSp<T>::end() {
	return m_ptr + m_size;
}
template<typename T>
typename mySharedSp<T>::const_iterator mySharedSp<T>::begin() const {
	return m_ptr;
}
template<typename T>
typename mySharedSp<T>::const_iterator mySharedSp<T>::end() const {
	return m_ptr + m_size;
}


//��̬����������������±�
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

//��ȡ�����С
template<typename T>
size_t mySharedSp<T>::size() const {
	return m_size;
}

//���������С
//����ǿ�ָ�룬����resize�򴴽�������
template<typename T>
void mySharedSp<T>::resize(size_t newSize) {
	T* newPtr = new T[newSize](); //ʹ���� ֵ��ʼ�� �﷨����ʼ������Ԫ��ȫΪ0
	if (m_ptr != NULL) { //ԭ��������ǿգ���ֵ
		size_t eleNumsToCopy = min{ newSize, m_size };
		for (int i = 0; i < eleNumsToCopy; ++i) {
			newPtr[i] = m_ptr[i];
		}
		delete[] m_ptr; // �ͷ�ԭָ��
	}
	//��ԭ�������ǿ�ָ�룬resize��ֱ�Ӵ���һ���µ�����
	m_ptr = newPtr; // ���µ�����ָ��ʹ�Сͬ������Ա����
	m_size = newSize;
}

// Ԥ���ռ�
template<typename T>
void mySharedSp<T>::reserve(size_t capacity) {
	if (capacity > m_size) {
		// �����µ����顣������Ĭ�ϳ�ʼ����ֵ��ʼ�������ԣ���Ϊ������û�иı�m_size��ֵ��
		//���Բ������ݺ�����ֵ��δ���廹��0������size()���صĶ���ԭ����size
		T* newPtr = new T[capacity];
		if (m_ptr != NULL) {
			for (size_t i = 0; i < m_size; ++i) {
				newPtr[i] = m_ptr[i]; // ����Ԫ��
			}
			delete[] m_ptr; // �ͷ�ԭ�����ڴ�
		}
		m_ptr = newPtr; // ����ָ��
		//������m_size
	}
}

//��������
//������������Ϊ����ָ�룬��ָ��һ���������� const T& ���͵Ĳ���������bool���͵ĺ�����
//������������ʹ��lambda���ʽ��Ĭ��Ϊ��������
template<typename T>
void mySharedSp<T>::sort(iterator begin, iterator end, bool(*comp)(const T&, const T&)) {
	std::sort(begin, end, comp);
}

// �������
// ����һ��ָ���Ԫ�ص� T���� ָ��
// m_ptrָ��һ������ĵ�һ��Ԫ�أ���ô m_ptr + m_size �ͻ�õ�һ��ָ������ĩβ��ָ��
template<typename T>
T* mySharedSp<T>::find(const T& value) const {
	return std::find(m_ptr, m_ptr + m_size, value);
}