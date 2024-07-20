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

//创建节点类
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

	int level; //每个节点的层级

	Node<K, V>** forward; //每个节点的next节点

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
	//初始化forwar数组
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

//创建跳表类
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

	int _max_level; //跳表的最大层级

	int _skip_list_level; //当前跳表的层级

	int _count_element; //当前跳表的元素个数

	Node<K, V>* _header; //跳表所有层的头节点

	ofstream _file_writer; //写文件操作符

	ifstream _file_reader; //读文件操作符
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

//建立一个新节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(
	const K k, const V v, int level) 
{
	Node<K, V>* node = new Node<K, V>(k, v, level);
	return node;
}

//随机获取新层级
template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;

	//rand()%2的结果为奇数代表在对应层存在，k++
	while (rand() % 2) {
		k++;
	}

	k = (k < _max_level) ? k : _max_level;
	return k;
}

//查找元素是否在跳表中
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
	//起始于跳表的最顶层
	for (int i = _skip_list_level; i >= 0; i--) {
		while (cur->forward[i] 
			&& cur->forward[i]->get_key() < key) {
			cur = cur->forward[i];
		}
	}

	cur = cur->forward[0];
	//找到节点
	if (cur != NULL && cur->get_key() == key) {
		return true;
	}

	return false;
}

//在跳表中插入一个节点
//0 代表 成功插入新节点
//1 代表 未能插入新节点（该节点已经在跳表中存在）
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

	//update数组是存储插入节点的前驱节点的数组，初始化为0
	Node<K, V>** update = new Node<K, V>* [_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	//遍历所有层，记录所有层待插入节点的前驱节点，存放在update数组中
	for (int i = _skip_list_level; i >= 0; i--) {
		while (cur->forward[i] && cur->forward[i]->get_key() < k) {
			cur = cur->forward[i];
		}

		update[i] = cur;
	}

	//定位到最底层
	cur = cur->forward[0];

	//待插入节点在跳表中已经存在
	if (cur != NULL && cur->get_key() == k) {
		cout << "key: " << k << ", exists" << endl;
		mtx.unlock();
		return 1;
	}

	//准备插入节点（插入到跳表的最后或者中间）
	if (cur == NULL || cur->get_key()!= k) {
		//获取新节点的层级
		int random_level = get_random_level();

		//如果random_level > _skip_list_level，就更新_skip_list_level
		if (random_level > _skip_list_level) {
			for (int i = _skip_list_level + 1;
				i <= random_level; i++) {
				update[i] = _header;
			}
		}

		//建立节点并插入
		Node<K, V>* insert_node = create_node(k, v, random_level);
		for (int i = 0; i <= random_level; i++) {
			insert_node->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = insert_node;
		}

		//更新元素个数
		_count_element++;
	}

	mtx.unlock();
	return 0;
}

//从跳表中删除元素
template<typename K, typename V>
void SkipList<K, V>::delete_element(K k) {
	mtx.lock();
	//类似于插入节点，在整个跳表中定位所有层要删除的节点，将其放入update数组中
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
	//删节点
	if (cur != NULL && cur->get_key() == k) {
		for (int i = _skip_list_level; i >= 0; i--) {
			update[i]->forward[i] = cur->forward[i];
		}

		//如果删完发现其中一层为空，_skip_list_level--
		while (_skip_list_level > 0 
			&& _header->forward[_skip_list_level] == NULL) {
			_skip_list_level--;
		}

		//更新元素个数
		_count_element--;
		cout << "Successfully deleted key " << k << endl;
		delete cur;
	}

	mtx.unlock();
	return;
}

//输出跳表
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

//将跳表中的数据保存到文件中
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

//从磁盘中读取数据
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

//获取跳表的大小（节点个数）
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

//判断字符串是否合法（字符串不为空且字符串中有“：”）
template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const string& str) {
	return !str.empty() && str.find(delimiter) != str.npos;
}

//分析文件的数据，获取key和value的值
template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(
	const string& str, string* key, string* value
) {
	if (!is_valid_string(str)) {
		return;
	}

	//从0开始，截取到“：”的位置（数组下标从0开始）
	*key = str.substr(0, str.find(delimiter));
	//截取“：”到str末尾
	*value = str.substr(str.find(delimiter) + 1);
}

