/*
5.29:
��ӻ���������������������������͸�ֵ���ƶ�����͸�ֵ
��Ӻ����������ã���Ա����
*/

/*
����Ҫ��ӣ�
�ƶ����캯�����ƶ���ֵ�������ʵ����Դ��ת�ƣ����Ч�ʺ�����ԡ�
֧���Զ���ɾ�����������û��Զ�����Դ���ͷŷ�ʽ������ʹ�ú���ָ�����������Ϊɾ������
��ȡ������ֵ�Ľӿڣ��ṩ������ȡ��ǰ����ָ�������������Դ�����ü�����
֧�ֱȽ��������ʵ������ָ�����֮��ıȽϲ�����������ԱȽϡ�
֧�ֿ�ָ���飺����һ���������������ָ���Ƿ�Ϊ�ա�
ʵ�� make_shared �����������ڱ�׼���е� std::make_shared ���������ڷ���ش�������ָ����󣬲������ڴ����Ͷ����졣
֧���������ͣ���չ����ָ��Ĺ��ܣ�ʹ���ܹ�����̬���顣
*/


#include<iostream>
#include<stdlib.h>
using namespace std;

template<typename T>
class mySharedsp {
private:
	int* m_count; //ָ���������ָ��
	T* m_ptr;
public:
	mySharedsp(T* p = NULL);  //������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
	mySharedsp(const mySharedsp<T>& other); //��������
	mySharedsp<T>& operator = (const mySharedsp<T>& other); //������ֵ
	mySharedsp(mySharedsp<T>&& other); //�ƶ����캯��
	mySharedsp<T>& operator = (mySharedsp<T>&& other); //�ƶ���ֵ��ע�ⲻ�ܽ���������Ϊconst
	~mySharedsp();

	T& opetarer* () const;//�����ã�����ָ��ָ��Ķ��������
	T* operator-> () const;//ͨ��ָ����ʳ�Ա���������ص��ǳ�Ա������������ָ��������һ��ָ��
};

//����  
//������������Ҫָ��Ĭ�ϲ���������ʵ�ֲ���Ҫ�ٴ�ָ��
template<typename T>
mySharedsp<T>::mySharedsp(T* p) : m_ptr(p), m_count(new int(1)) {
	cout << "���ù��캯��" << endl;
}

//��������
template<typename T>
mySharedsp<T>::mySharedsp(const mySharedsp<T>& other) : m_ptr(other.m_ptr), m_count(other.m_count) {
	++(*m_count);
	cout << "���ÿ������캯��" << endl;
}

//������ֵ
//ע�⣺���ص���һ������
//���������Լ�������������
//Ϊʲô������ֵ���ƶ���ֵ�з���ֵ��֧����ʽ��ֵ
template<typename T>
mySharedsp<T>& mySharedsp<T>::operator = (const mySharedsp<T>& other) {
	++(*other.m_count);
	--(*m_count);
	if (*m_count == 0) {
		delete m_ptr;
		m_ptr = NULL; //ֻdelete���ÿջ��������ָ��
		delete m_count;
		m_count = NULL;
	}
	m_ptr = other.m_ptr;
	m_count = other.m_count;
	cout << "���ÿ�����ֵ����" << endl;

	return *this;
}
/*
* ��һ�ַ�ʽʵ�ֿ������죺
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
		++(*other.m_count); //ȷ����ǰ�����other�������������ͬ������++��
		m_ptr = other.m_ptr;
		m_count = other.m_count;
	}
	cout << "���ÿ�����ֵ����" << endl;

	return *this;
}
*/


//�ƶ����캯��
//��ֵ���ã���Ҫ����ʱ����Ҫ��otherָ��ת������ֵ
//�� mySharedsp<int> p(move(new int(10)))
template<typename T>
mySharedsp<T>::mySharedsp(mySharedsp<T>&& other) :m_ptr(other.m_ptr), m_count(other.m_count) {
	other.m_ptr = NULL;
	other.m_count = NULL;
	out << "�����ƶ����캯��" << endl; //���������仯
}

//�ƶ���ֵ
//���ܽ���������Ϊconst����Ϊת����Դ������ʱ����ָ������ÿղ���
template<typename T>
mySharedsp<T>& mySharedsp<T>::operator = (mySharedsp<T>&& other) {
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

		other.m_ptr = NULL; //����delete����Ϊ��ʱ�� m_ptr �� other.m_ptr ��ָ��ͬһ����Դ
		other.m_count = NULL;

		cout << "�����ƶ���ֵ����" << endl;
	}
	return *this; //������Ը�ֵ��ֱ�ӷ��ص�ǰ����
}


//����
template<typename T>
mySharedsp<T>::~mySharedsp() {
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
T& mySharedsp<T>::operator* () const {
	cout << "ָ��Ľ����ã��õ�ָ��ָ��Ķ���" << endl;
	return *m_ptr;
}

//��Ա����
//����һ������ָ�����p��p->xxx �͵ȼ��� p.operator->()->xxx, ���ص���p�����ָ��ָ��Ķ���ĳ�Ա
template<typename T>
T* mySharedsp<T>::operator-> () const {
	cout << "ͨ��ָ����ʳ�Ա�������õ�ָ��ָ��Ķ���" << endl;
	return m_ptr;
}



