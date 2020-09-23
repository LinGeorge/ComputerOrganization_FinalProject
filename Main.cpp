#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

#define LOW 0
#define HIGH 10000

enum DIR { LEFT, RIGHT };
enum CEND {ASCEND, DESCEND};
int wayCount = 4;

class Node {
public:
	Node(int i = 0, int ad = 0, bool v = false) :index(i), addr(ad), visited(v) {}
	int getIndex() const { return index; }
	int getAddr() const { return addr; }
	bool isVisited() const { return visited; }
	void visit() { visited = true; }
	void notVisit() { visited = false; }
	static bool ascend(const Node& c1, const Node& c2) { return c1.addr < c2.addr; }
	static bool descend(const Node& c1, const Node& c2) { return c1.addr > c2.addr; }
	static bool initialize(const Node& c1, const Node& c2) { return c1.index < c2.index; }
private:
	int index;
	int addr;
	bool visited;
};

//template<class T>
//void swap(T &a, T&b);

template<class out_type, class in_value>
out_type convert(const in_value & t)
{
	stringstream stream;
	stream << t;

	out_type result;
	stream >> result;
	return result;
}

void selfCheck(int* sche);

void sortFCFS(int *sche);
void sortSSTF(int *sche);
void sortSCAN(int *sche);
void sortCSCAN(int *sche);
void sortBlockAscendOri(int *sche);
void sortBlockAscend(int *sche);

int getTotalDistance(int *sche);
void initializeReqSeq();
bool inRange(int i);
// map<int, Node> requestedSeq;
int reqSize = 0; // 包含第0格
Node *requestedSeq = new Node[10000];
vector<Node> requestedSeq2;
Node *sorted_req = new Node[10000];
int qos;


int main(int argc, char *argv[])
{

	if (argc != 3) return 0;
	// 取得未排序的內容
	fstream inputFile(argv[1], ios::in);
	requestedSeq[0] = Node();
	reqSize++;
	string element;
	requestedSeq2.push_back(0);
	while (inputFile >> element) {
		reqSize++;
		istringstream ss(element);
		string seq, addr;

		getline(ss, seq, ':');

		getline(ss, addr, ':');
		requestedSeq[convert<int>(seq)] = Node(convert<int>(seq), convert<int>(addr), false);
		requestedSeq2.push_back(Node(convert<int>(seq), convert<int>(addr), false));
	}
	// 取得QoS
	qos = convert<int>(argv[2]);

	*sorted_req = *requestedSeq;
	for (int i = 1; i < reqSize; i++) {
		for (int j = 1; j < reqSize - i; j++) {
			if (sorted_req[j].getAddr() > sorted_req[j + 1].getAddr()) {
				Node temp = sorted_req[j];
				sorted_req[j] = sorted_req[j + 1];
				sorted_req[j + 1] = temp;
			}
		}
	}
	// 測試用code
	/*fstream inputFile("open4.in", ios::in);
	requestedSeq[0] = Node();
	reqSize++;
	requestedSeq2.push_back(0);
	string element;
	while (inputFile >> element) {
		istringstream ss(element);
		string seq, addr;
		reqSize++;
		getline(ss, seq, ':');

		getline(ss, addr, ':');
		requestedSeq[convert<int>(seq)] = Node(convert<int>(seq), convert<int>(addr), false);
		requestedSeq2.push_back(Node(convert<int>(seq), convert<int>(addr), false));
	}
	qos = 50;*/


	/**sorted_req = *requestedSeq;
	for (int i = 1; i < reqSize; i++) {
		for (int j = 1; j < reqSize - i; j++) {
			if (sorted_req[j].getAddr() > sorted_req[j + 1].getAddr()) {
				Node temp = sorted_req[j];
				sorted_req[j] = sorted_req[j + 1];
				sorted_req[j + 1] = temp;
			}
		}
	}*/

	// 創立已排序陣列
	// map<int, int> scheduledSeq;
	int *scheduledSeq = new int[reqSize];
	scheduledSeq[0] = 0;
	sortFCFS(scheduledSeq);
	int distance = getTotalDistance(scheduledSeq);
	for (int i = 0; i < wayCount - 1; i++) {
		int *scheduledSeq2 = new int[reqSize];
		scheduledSeq2[0] = 0;
		initializeReqSeq();
		sort(requestedSeq2.begin(), requestedSeq2.end(), Node::initialize);
		switch (i) {
		case 0:
			sortSSTF(scheduledSeq2);
			break;
		case 1:
			sortCSCAN(scheduledSeq2); // better SSTF
			break;
		case 2:
			sortBlockAscend(scheduledSeq2); // new way
			//sortSCAN(scheduledSeq2);
			break;
		}
		if (distance > getTotalDistance(scheduledSeq2)) {
			distance = getTotalDistance(scheduledSeq2);
			delete[]scheduledSeq;
			scheduledSeq = scheduledSeq2;
		}
	}
	// sortSSTF(scheduledSeq); // 待填入陣列中，從第一格開始填，一開始是requestedSeq[0] = 0 的位置

	selfCheck(scheduledSeq);
	fstream outputFile("access.out", ios::out);
	int totalDistance = 0;
	for (int i = 1; i <= reqSize - 1; i++) {
		totalDistance += abs(requestedSeq[scheduledSeq[i]].getAddr() - requestedSeq[scheduledSeq[i - 1]].getAddr());
		outputFile << i << ":" << scheduledSeq[i] << endl;
	}
	cout << "The total seek distance is " << totalDistance << "." << endl;

	inputFile.close();
	outputFile.close();

	// system("PAUSE");
	return 0;
}



void sortFCFS(int *sche) {
	for (int index = 1; index < reqSize; index++) { // 從1跑到底
		requestedSeq[index].visit();
		sche[index] = index;
	}
}

void sortSSTF(int* sche) {

	int initialIndex = 0;

	for (int index = 1; index < reqSize; index++) { // 從1跑到底
		int lastAddr = requestedSeq[initialIndex].getAddr(); // 上一次針頭在的位置
		int deltaabs = 2147483646; // infinity，與針頭的距離
		int minIndex = -1; // in order to check error => set -1，與針頭的距離最短欄位的index
		for (int i = index - qos; i <= index + qos; i++) { // 搜尋qos範圍內的值，找addr最近的
			if (i == index - qos && inRange(i)) {// 如果這次不選他(尾巴節點&&在1~requestedSeq.size()-1內)，就沒機會選了
				if (!requestedSeq[index - qos].isVisited()) { // 選了，直接跳出搜尋迴圈
					minIndex = index - qos;
					break;
				}
			}
			else if (inRange(i)) { // 看還沒搜           尋的節點誰比較近，並更新index
				if (!requestedSeq[i].isVisited() && abs(lastAddr - requestedSeq[i].getAddr()) < deltaabs) {
					minIndex = i;
					deltaabs = abs(lastAddr - requestedSeq[i].getAddr());
				}
			}
		}
		// cout << "minIndex = " << minIndex << endl;
		// 最後跳出迴圈時，更新requestedSeq(visited)，填寫schedule陣列，遞回呼叫
		requestedSeq[minIndex].visit();
		sche[index] = minIndex;
		initialIndex = minIndex;
	}




}

void sortSCAN(int *sche) {
	int initialIndex = 0;

	DIR dir = RIGHT;

	for (int index = 1; index < reqSize; index++) { // 從1跑到底

		int lastAddr = requestedSeq[initialIndex].getAddr(); // 上一次針頭在的位置

		int deltaabs = 2147483646; // infinity，與針頭的距離
		int minIndex = -1; // in order to check error => set -1，與針頭的距離最短欄位的index

		int leftDelta = 2147483646, rightDelta = 2147483646;
		int leftTotal = 0, rightTotal = 0;

		int *leftIndex = new int[qos * 2];
		int *rightIndex = new int[qos * 2];
		int leftAmount = 0, rightAmount = 0;

		for (int i = index - qos; i <= index + qos; i++) { // 搜尋qos範圍內的值，找addr最近的
			if (i == index - qos && inRange(i)) {// 如果這次不選他(尾巴節點&&在1~requestedSeq.size()-1內)，就沒機會選了
				if (!requestedSeq[index - qos].isVisited()) { // 選了，直接跳出搜尋迴圈
					minIndex = index - qos;
					break;
				}
			}
			else if (inRange(i)) { // 看還沒搜尋的節點誰比較近，並更新index
				if (!requestedSeq[i].isVisited() && abs(lastAddr - requestedSeq[i].getAddr()) < deltaabs) {
					if (lastAddr - requestedSeq[i].getAddr() >= 0 && dir == RIGHT) {
						rightIndex[rightAmount++] = i;
						deltaabs = abs(lastAddr - requestedSeq[i].getAddr());
					}
					else if (lastAddr - requestedSeq[i].getAddr() < 0 && dir == LEFT) {
						leftIndex[leftAmount++] = i;
						deltaabs = abs(lastAddr - requestedSeq[i].getAddr());
					}
				}
			}
		}
		// cout << "minIndex = " << minIndex << endl;
		// 最後跳出迴圈時，更新requestedSeq(visited)，填寫schedule陣列，遞回呼叫

		

		/*if (minIndex == -1) {
			if (rightAmount >= leftAmount) {
				minIndex = rightIndex[--rightAmount];
			}
			else {
				minIndex = leftIndex[--leftAmount];
			}
		}*/
		if (minIndex == -1 && dir == RIGHT) {
			minIndex = rightIndex[--rightAmount];
		}
		else if(minIndex == -1 && dir == LEFT) {
			minIndex = leftIndex[--leftAmount];
		}
		if (minIndex == reqSize - 1) {
			dir = LEFT;
		}
		requestedSeq[minIndex].visit();
		sche[index] = minIndex;
		initialIndex = minIndex;
		
	}
}

void sortCSCAN(int *sche) {

	int initialIndex = 0;

	DIR dir = RIGHT;

	for (int index = 1; index < reqSize; index++) { // 從1跑到底

		int lastAddr = requestedSeq[initialIndex].getAddr(); // 上一次針頭在的位置

		int deltaabs = 2147483646; // infinity，與針頭的距離
		int minIndex = -1; // in order to check error => set -1，與針頭的距離最短欄位的index

		int leftDelta = 2147483646, rightDelta = 2147483646;
		int leftTotal = 0, rightTotal = 0;

		int *leftIndex = new int[qos * 2];
		int *rightIndex = new int[qos * 2];
		int leftAmount = 0, rightAmount = 0;

		for (int i = index - qos; i <= index + qos; i++) { // 搜尋qos範圍內的值，找addr最近的
			if (i == index - qos && inRange(i)) {// 如果這次不選他(尾巴節點&&在1~requestedSeq.size()-1內)，就沒機會選了
				if (!requestedSeq[index - qos].isVisited()) { // 選了，直接跳出搜尋迴圈
					minIndex = index - qos;
					break;
				}
			}
			else if (inRange(i)) { // 看還沒搜尋的節點誰比較近，並更新index
				if (!requestedSeq[i].isVisited() && abs(lastAddr - requestedSeq[i].getAddr()) < deltaabs) {
					if (lastAddr - requestedSeq[i].getAddr() >= 0) {
						rightIndex[rightAmount++] = i;
						deltaabs = abs(lastAddr - requestedSeq[i].getAddr());
					}
					else if (lastAddr - requestedSeq[i].getAddr() < 0) {
						leftIndex[leftAmount++] = i;
						deltaabs = abs(lastAddr - requestedSeq[i].getAddr());
					}
				}
			}
		}
		// cout << "minIndex = " << minIndex << endl;
		// 最後跳出迴圈時，更新requestedSeq(visited)，填寫schedule陣列，遞回呼叫
		
		if (minIndex == reqSize - 1) {
			dir = LEFT;
		}

		if(minIndex == -1){
			if (rightAmount >= leftAmount) {
				minIndex = rightIndex[--rightAmount];
			}
			else {
				minIndex = leftIndex[--leftAmount];
			}
		}
		requestedSeq[minIndex].visit();
		sche[index] = minIndex;
		initialIndex = minIndex;
	}
}

void sortBlockAscend(int *sche) {
	int blockSize = qos + 1; // -qos ~ qos 共 qos + 1 個
	int blockAmount;
	if (blockSize >= (reqSize - 1)) { // blockSize >= maxReqSize
		blockAmount = 1;
		blockSize = reqSize - 1;
	}
	else if (blockSize == 1) { // qos = 0
		blockAmount = 0;
	}
	else {
		blockAmount = ceil((reqSize - 1) / static_cast<float>(blockSize)); // 小數點直接進位
	}

	int blockIndex = 0;
	CEND cend = ASCEND; // previous cend

	//第一個block
	vector<Node>::iterator it1 = requestedSeq2.begin() + 1 + blockIndex * blockSize;
	vector<Node>::iterator it2 = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize;
	sort(it1, it2, Node::ascend); // [it1, it2-1]
	blockIndex++;
	cend = DESCEND;

	for (; blockIndex < blockAmount; blockIndex++) {
		it1 = requestedSeq2.begin() + 1 + blockIndex * blockSize;
		if (blockIndex == blockAmount - 1) it2 = requestedSeq2.end() - 1;
		else it2 = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize;

		
		if (cend == ASCEND) {

			sort(it1, it2, Node::ascend);

			vector<Node>::iterator itPrevious = (requestedSeq2.begin() + 1 + blockIndex * blockSize) - 1; // 左block右下點
			vector<Node>::iterator itCurrent; // 右block右上點
			if(blockIndex == blockAmount - 1) itCurrent = requestedSeq2.end() - 1;
			else itCurrent = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize - 1;

			vector<Node>::iterator itPreviousLeft = (requestedSeq2.begin() + 1 + (blockIndex - 1) * blockSize); // 左block左上點
			vector<Node>::iterator itCurrentLeft = requestedSeq2.begin() + 1 + blockIndex * blockSize; // 右block左下點

			int originCost = abs((*itCurrentLeft).getAddr() - (*itPrevious).getAddr());
			int newCost = abs((*itCurrent).getAddr() - (*itPrevious).getAddr());

			if (((*itPrevious).getAddr() >= (*itCurrent).getAddr()) || (originCost > newCost)) {
				sort(it1, it2, Node::descend);
			}
			else cend = DESCEND;
		}
		else if(cend == DESCEND){

			sort(it1, it2, Node::descend);

			vector<Node>::iterator itPrevious = (requestedSeq2.begin() + 1 + blockIndex * blockSize) - 1;
			vector<Node>::iterator itCurrent;
			if (blockIndex == blockAmount - 1) itCurrent = requestedSeq2.end() - 1;
			else itCurrent = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize - 1;

			vector<Node>::iterator itPreviousLeft = (requestedSeq2.begin() + 1 + (blockIndex - 1) * blockSize); // 左block左上點
			vector<Node>::iterator itCurrentLeft = requestedSeq2.begin() + 1 + blockIndex * blockSize; // 右block左下點

			int originCost = abs((*itCurrentLeft).getAddr() - (*itPrevious).getAddr());
			int newCost = abs((*itCurrent).getAddr() - (*itPrevious).getAddr());

			if (((*itPrevious).getAddr() <= (*itCurrent).getAddr()) || (originCost > newCost)) {
				sort(it1, it2, Node::ascend);
			}
			else cend = ASCEND;
		}


	}

	for (int i = 1; i < reqSize; i++) {
		sche[i] = requestedSeq2[i].getIndex();
	}

}

void sortBlockAscendOri(int *sche) {
	int blockSize = qos; // -qos ~ qos 共 qos + 1 個
	int blockAmount;
	if (blockSize >= (reqSize - 1)) {
		blockAmount = 1;
		blockSize = reqSize - 1;
	}
	else if (blockSize == 0) {
		blockAmount = 0;
	}
	else {
		blockAmount = ceil((reqSize - 1) / static_cast<float>(blockSize)); // 小數點直接進位
	}

	int blockIndex = 0;
	CEND cend = ASCEND; // previous cend

	//第一個block
	vector<Node>::iterator it1 = requestedSeq2.begin() + 1 + blockIndex * blockSize;
	vector<Node>::iterator it2 = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize;
	sort(it1, it2, Node::ascend);
	blockIndex++;
	cend = DESCEND;

	for (; blockIndex < blockAmount; blockIndex++) {
		it1 = requestedSeq2.begin() + 1 + blockIndex * blockSize;
		if (blockIndex == blockAmount - 1) it2 = requestedSeq2.end() - 1;
		else it2 = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize;


		if (cend == ASCEND) {

			sort(it1, it2, Node::ascend);

			vector<Node>::iterator itPrevious = (requestedSeq2.begin() + 1 + blockIndex * blockSize) - 1; // 左block右下點
			vector<Node>::iterator itCurrent; // 右block右上點
			if (blockIndex == blockAmount - 1) itCurrent = requestedSeq2.end() - 1;
			else itCurrent = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize - 1;

			

			if (((*itPrevious).getAddr() >= (*itCurrent).getAddr())) {
				sort(it1, it2, Node::descend);
			}
			else cend = DESCEND;
		}
		else if (cend == DESCEND) {
			sort(it1, it2, Node::descend);

			vector<Node>::iterator itPrevious = (requestedSeq2.begin() + 1 + blockIndex * blockSize) - 1;
			vector<Node>::iterator itCurrent;
			if (blockIndex == blockAmount - 1) itCurrent = requestedSeq2.end() - 1;
			else itCurrent = requestedSeq2.begin() + 1 + (blockIndex + 1) * blockSize - 1;


			if (((*itPrevious).getAddr() <= (*itCurrent).getAddr())) {
				sort(it1, it2, Node::ascend);
			}
			else cend = ASCEND;
		}


	}

	for (int i = 1; i < reqSize; i++) {
		sche[i] = requestedSeq2[i].getIndex();
	}
}

int getTotalDistance(int *sche) {
	int totalDistance = 0;
	for (int i = 1; i <= reqSize - 1; i++) {
		totalDistance += abs(requestedSeq[sche[i]].getAddr() - requestedSeq[sche[i - 1]].getAddr());
	}
	return totalDistance;
}

void initializeReqSeq() {
	for (int i = 1; i <= reqSize - 1; i++) {
		requestedSeq[i].notVisit();
	}
}

bool inRange(int i) {
	if (1 <= i && i <= reqSize - 1) return true;
	else return false;
}

void selfCheck(int * sche) {
	for (int i = 0; i < reqSize; i++) {
		if (i - qos <= sche[i] && sche[i] <= i + qos) {

		}
		else if (sche[i] <= 0 || sche[i] > reqSize - 1) {
			cout << "error::algorithm_error" << endl;
		}
		else {
			cout << "error::out_of_qos_range" << endl;
		}
	}
}