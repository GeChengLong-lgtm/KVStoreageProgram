#include <iostream>
#include "skiplist.h"

using namespace std;

#define FILE_PATH './store/dumpFile'

int main() {
	SkipList<int, string> skipList(6);
	skipList.insert_element(1, "����");
	skipList.insert_element(3, "����ķ�");
	skipList.insert_element(7, "�Һ��밮��");
	skipList.insert_element(8, "����");
	skipList.insert_element(9, "�������ʹ");
	skipList.insert_element(19, "�κ���");
	skipList.insert_element(19, "�޵�");

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