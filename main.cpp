#include <iostream>
#include "skiplist.h"

using namespace std;

#define FILE_PATH './store/dumpFile'

int main() {
	SkipList<int, string> skipList(6);
	skipList.insert_element(1, "晴天");
	skipList.insert_element(3, "夏天的风");
	skipList.insert_element(7, "我很想爱她");
	skipList.insert_element(8, "宁夏");
	skipList.insert_element(9, "会呼吸的痛");
	skipList.insert_element(19, "奈何桥");
	skipList.insert_element(19, "宿敌");

	cout << "skiplist size: " << skipList.size() << endl;

	skipList.dump_file();

	skipList.search_element(9);
	skipList.search_element(18);

	skipList.display_list();

	skipList.delete_element(3);
	skipList.delete_element(7);

	cout << "skiplist size: " << skipList.size() << endl;

	skipList.display_list();
}