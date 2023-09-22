/*
5.29:
��ӻ���������������������������͸�ֵ���ƶ�����͸�ֵ
��Ӻ����������ã���Ա����

5.30��
��ӻ�ü�����ֵ�ĺ�������Ӷ���ȽϺ���
֧�ֿ�ָ���飺����һ���������������ָ���Ƿ�Ϊ�ա�
֧���������ͣ���չ����ָ��Ĺ��ܣ�ʹ���ܹ�����̬���顣
*/

/*
����Ҫ��ӣ�
֧���Զ���ɾ�����������û��Զ�����Դ���ͷŷ�ʽ������ʹ�ú���ָ�����������Ϊɾ������
ʵ�� make_shared �����������ڱ�׼���е� std::make_shared ���������ڷ���ش�������ָ����󣬲������ڴ����Ͷ����졣
֧�ֶ�̬���������С���ṩ��������̬��������Ĵ�С�������ڶ�̬�����resize������
֧�ֵ�������ʵ�ֵ������ӿڣ��Ա��û��ܹ�������ָ������������е���������
֧����Ƭ�����������û�������ָ���������������Ƭ��������ȡ��������Ԫ�ء�
֧������Ͳ��ң��ṩ����Ͳ����㷨��ʹ�û����Զ�����ָ�����������������Ͳ��Ҳ�����
֧������ת����ʵ�ִ�����ָ���������鵽�����������ͣ���std::vector��std::list�ȣ���ת�������������������������н�����
֧�ֶ��̰߳�ȫ�������̰߳�ȫ�Ļ��ƣ��Ա�֤�ڶ��̻߳����¶�����ָ��Ĳ����ǰ�ȫ�ġ�
֧���Զ������������ shared_ptr ʵ������Ӷ��Զ����������֧�֣����û��ܹ��Զ������ķ��䷽ʽ��
֧�����ü����Ż���ʵ��һЩ���ü����Ż�������������ǰ�ͷŻ����ӳ��ͷŵȣ������ shared_ptr �����ܡ�
*/




#include<iostream>
#include<stdlib.h>
#include<mutex>

using namespace std;

template<typename T>
class mySharedSp {
private:
	int* m_count; //ָ���������ָ��
	T* m_ptr; //�������ָ�������һ�����ü��������Ҫ����Ϊָ��
	size_t m_size; //�����С������ָ��ֻ�ṩ��ȡ��������ķ����������в��ṩ�޸������С�ķ�������˶���Ϊ��ָ�룬����������ָ�����ͬһ������
public:
	mySharedSp(T* p = NULL, size_t size = 0);  //������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
	mySharedSp(const mySharedSp<T>& other); //��������
	mySharedSp<T>& operator = (const mySharedSp<T>& other); //������ֵ
	mySharedSp(mySharedSp<T>&& other); //�ƶ����캯��
	mySharedSp<T>& operator = (mySharedSp<T>&& other); //�ƶ���ֵ��ע�ⲻ�ܽ���������Ϊconst
	~mySharedSp();

	//���������const����ʾ�ó�Ա��������ı��Ա����
	T& opetarer* () const;//�����ã�����ָ��ָ��Ķ��������
	T* operator-> () const;//ͨ��ָ����ʳ�Ա���������ص��ǳ�Ա������������ָ��������һ��ָ��
	bool isNULL() const; //��ָ����
	int use_count() const; //��ȡ����ֵ
	bool operator==(const mySharedSp<T>& other) const; //����ָ��ıȽϲ���
	bool operator!=(const mySharedSp<T>& other) const; //����ָ��ıȽϲ���
	T& operator[](size_t index) const; //ͨ�� ptr[index] ��������Ԫ��
	size_t size() const;
};


//����  
//������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
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
	return *m_ptr;
}


//��Ա����
//����һ������ָ�����p��p->xxx �͵ȼ��� p.operator->()->xxx, ���ص���p�����ָ��ָ��Ķ���ĳ�Ա
template<typename T>
T* mySharedSp<T>::operator-> () const {
	cout << "ͨ��ָ����ʳ�Ա�������õ�ָ��ָ��Ķ���" << endl;
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

//��̬�������
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
