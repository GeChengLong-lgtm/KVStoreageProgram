#pragma once
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
#define STORE_FILE "./dumpFile"

mutex mtx;
string delimiter = ":";

//�����ڵ���
template<typename K, typename V>
class Node {

public:

	Node(){}

	~Node();

	Node(K key, V value, int);

	K get_key() const;

	V get_value() const;

	void set_value(V);

public:

	int level; //ÿ���ڵ�Ĳ㼶

	Node<K, V>** forward; //ÿ���ڵ��next�ڵ�

private:

	K key;

	V value;
};

template<typename K, typename V>
Node<K, V>::~Node() {
	delete []forward;
}

template<typename K, typename V>
Node<K, V>::Node(K key, V value, int level) {
	this->key = key;
	this->value = value;
	this->level = level;
	
	//level+1 : 0 - level
	this->forward = new Node<K, V>* [level + 1];
	//��ʼ��forwar����
	memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<typename K, typename V>
K Node<K, V>::get_key() const {
	return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
	return value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V value) {
	this->value = value;
}

//����������
template<typename K, typename V>
class SkipList {

public:

	SkipList(int);

	~SkipList();

	int get_random_level();

	Node<K, V>* create_node(K, V, int);

	bool search_element(K);

	int insert_element(K, V);

	void delete_element(K);

	void display_list();

	void dump_file();

	void load_file();

	int size();

	void clear(Node<K, V>*);

private:

	bool is_valid_string(const string& str);

	void get_key_value_from_string(
		const string& str, string* key, string* value
	);

private:

	int _max_level; //��������㼶

	int _skip_list_level; //��ǰ����Ĳ㼶

	int _count_element; //��ǰ�����Ԫ�ظ���

	Node<K, V>* _header; //�������в��ͷ�ڵ�

	ofstream _file_writer; //д�ļ�������

	ifstream _file_reader; //���ļ�������
};

template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
	this->_max_level = max_level;
	this->_skip_list_level = 0;
	this->_count_element = 0;

	K k = K();
	V v = V();
	this->_header = new Node<K, V>(k, v, max_level);
}

//����һ���½ڵ�
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(
	const K k, const V v, int level) 
{
	Node<K, V>* node = new Node<K, V>(k, v, level);
	return node;
}

//�����ȡ�²㼶
template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;

	//rand()%2�Ľ��Ϊ���������ڶ�Ӧ����ڣ�k++
	while (rand() % 2) {
		k++;
	}

	k = (k < _max_level) ? k : _max_level;
	return k;
}

//����Ԫ���Ƿ���������
/*
						   +------------+
						   |  select 60 |
						   +------------+
level 4     +-->1+                                                      100
				 |
				 |
level 3         1+-------->10+------------------>50+           70       100
												   |
												   |
level 2         1          10         30         50|           70       100
												   |
												   |
level 1         1    4     10         30         50|           70       100
												   |
												   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V>
bool SkipList<K, V>::search_element(K key) {

	Node<K, V>* cur = _header;
	//��ʼ����������
	for (int i = _skip_list_level; i >= 0; i--) {
		while (cur->forward[i] 
			&& cur->forward[i]->get_key() < key) {
			cur = cur->forward[i];
		}
	}

	cur = cur->forward[0];
	//�ҵ��ڵ�
	if (cur != NULL && cur->get_key() == key) {
		return true;
	}

	return false;
}

//�������в���һ���ڵ�
//0 ���� �ɹ������½ڵ�
//1 ���� δ�ܲ����½ڵ㣨�ýڵ��Ѿ��������д��ڣ�
/*
						   +------------+
						   |  insert 50 |
						   +------------+
level 4     +-->1+                                                      100
				 |
				 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
											   |    |
											   |    |
level 2         1          10         30       | 50 |          70       100
											   |    |
											   |    |
level 1         1    4     10         30       | 50 |          70       100
											   |    |
											   |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
											   +----+

*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K k, const V v) {
	mtx.lock();

	Node<K, V>* cur = this->_header;

	//update�����Ǵ洢����ڵ��ǰ���ڵ�����飬��ʼ��Ϊ0
	Node<K, V>** update = new Node<K, V>* [_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	//�������в㣬��¼���в������ڵ��ǰ���ڵ㣬�����update������
	for (int i = _skip_list_level; i >= 0; i--) {
		while (cur->forward[i] && cur->forward[i]->get_key() < k) {
			cur = cur->forward[i];
		}

		update[i] = cur;
	}

	//��λ����ײ�
	cur = cur->forward[0];

	//������ڵ����������Ѿ�����
	if (cur != NULL && cur->get_key() == k) {
		cout << "key: " << k << ", exists" << endl;
		mtx.unlock();
		return 1;
	}

	//׼������ڵ㣨���뵽������������м䣩
	if (cur == NULL || cur->get_key()!= k) {
		//��ȡ�½ڵ�Ĳ㼶
		int random_level = get_random_level();

		//���random_level > _skip_list_level���͸���_skip_list_level
		if (random_level > _skip_list_level) {
			for (int i = _skip_list_level + 1;
				i <= random_level; i++) {
				update[i] = _header;
			}
		}

		//�����ڵ㲢����
		Node<K, V>* insert_node = create_node(k, v, random_level);
		for (int i = 0; i <= random_level; i++) {
			insert_node->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = insert_node;
		}

		//����Ԫ�ظ���
		_count_element++;
	}

	mtx.unlock();
	return 0;
}

//��������ɾ��Ԫ��
template<typename K, typename V>
void SkipList<K, V>::delete_element(K k) {
	mtx.lock();
	//�����ڲ���ڵ㣬�����������ж�λ���в�Ҫɾ���Ľڵ㣬�������update������
	Node<K, V>* cur = this->_header;

	//Node<K, V>* update[_max_level + 1];
	Node<K, V>** update = new Node<K, V>* [_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	for (int i = _skip_list_level; i >= 0; i--) {
		while (cur->forward[i] && cur->forward[i]->get_key() < k) {
			cur = cur->forward[i];
		}

		update[i] = cur;
	}

	cur = cur->forward[0];
	//ɾ�ڵ�
	if (cur != NULL && cur->get_key() == k) {
		for (int i = _skip_list_level; i >= 0; i--) {
			update[i]->forward[i] = cur->forward[i];
		}

		//���ɾ�귢������һ��Ϊ�գ�_skip_list_level--
		while (_skip_list_level > 0 
			&& _header->forward[_skip_list_level] == NULL) {
			_skip_list_level--;
		}

		//����Ԫ�ظ���
		_count_element--;
		cout << "Successfully deleted key " << k << endl;
		delete cur;
	}

	mtx.unlock();
	return;
}

//�������
template<typename K, typename V>
void SkipList<K, V>::display_list() {

	cout << "**********Skip List**********" << "\n";
	for (int i = _skip_list_level; i >= 0; i--) {
		Node<K, V>* node = this->_header->forward[i];
		cout << "level " << i << ": ";

		while (node != NULL) {
			cout << node->get_key() << ":" << node->get_value()<<";";
			node = node->forward[i];
		}
		cout << endl;
	}
}

//�������е����ݱ��浽�ļ���
template<typename K, typename V>
void SkipList<K, V>::dump_file() {
	cout << "Now file is being dumpped ... " << endl;
	_file_writer.open(STORE_FILE);
	Node<K, V>* node = this->_header->forward[0];

	while (node != NULL) {
		_file_writer << node->get_key() << ":" << node->get_value() << endl;
		cout << node->get_key() << ":" << node->get_value() << endl;
		node = node->forward[0];
	}

	_file_writer.flush();
	_file_writer.close();
}

//�Ӵ����ж�ȡ����
template<typename K, typename V>
void SkipList<K, V>::load_file() {
	_file_reader.open(STORE_FILE);
	cout << "Now file is being loading ... " << endl;
	string line;
	string* key;
	string* value;
	
	while (getline(_file_reader, line)) {
		get_key_value_from_string();
		if (key->empty() || value->empty()) {
			continue;
		}

		int ret = insert_element(stoi(*key), *value);
		cout << "key:" << *key << "value:" << *value << endl;
	}

	delete key;
	delete value;
	_file_reader.close();
}

//��ȡ����Ĵ�С���ڵ������
template<typename K, typename V>
int SkipList<K, V>::size() {
	return _count_element;
}

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
	if (_file_writer.is_open()) {
		_file_writer.close();
	}

	if (_file_reader.is_open()) {
		_file_reader.close();
	}

	if (_header->forward[0] != NULL) {
		clear(_header->forward[0]);
	}

	delete _header;
}


template<typename K, typename V>
void SkipList<K, V>::clear(Node<K, V>* cur) {
	if (cur->forward[0] != NULL) {
		clear(cur->forward[0]);
	}
	delete(cur);
}

//�ж��ַ����Ƿ�Ϸ����ַ�����Ϊ�����ַ������С�������
template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const string& str) {
	return !str.empty() && str.find(delimiter) != str.npos;
}

//�����ļ������ݣ���ȡkey��value��ֵ
template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(
	const string& str, string* key, string* value
) {
	if (!is_valid_string(str)) {
		return;
	}

	//��0��ʼ����ȡ����������λ�ã������±��0��ʼ��
	*key = str.substr(0, str.find(delimiter));
	//��ȡ��������strĩβ
	*value = str.substr(str.find(delimiter) + 1);
}

