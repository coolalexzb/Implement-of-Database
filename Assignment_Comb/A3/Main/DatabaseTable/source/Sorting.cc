
#ifndef SORT_C
#define SORT_C

#include "Sorting.h"
#include <queue>

using namespace std;

void TPMMS_append(MyDB_BufferManagerPtr parent, MyDB_PageReaderWriter &thisPage, vector<MyDB_PageReaderWriter> &pageList, MyDB_RecordPtr recPtr) {
	if (!thisPage.append(recPtr)) {
		pageList.push_back(thisPage);
		MyDB_PageReaderWriter newPage(*parent);
		newPage.append(recPtr);
		thisPage = newPage;
	}
}

void mergeIntoFile(MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs,
	function <bool()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	while (mergeUs.size() > 0) {
		auto min = mergeUs.begin();
		(*min)->getCurrent(rhs);
		
		for (auto it = mergeUs.begin()+1; it != mergeUs.end(); it++) {
			(*it)->getCurrent(lhs);
			if (comparator()) {
				min = it;
				(*min)->getCurrent(rhs);
			}
		}

		sortIntoMe.append(rhs);
		if (!(*min)->advance()) mergeUs.erase(min);
	}
}

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter,
	MyDB_RecordIteratorAltPtr rightIter, function <bool()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	vector<MyDB_PageReaderWriter> resultList;
	MyDB_PageReaderWriter thisPage(*parent);
	
	while (true) {
		leftIter->getCurrent(lhs);
		rightIter->getCurrent(rhs);

		if (comparator()) {
			TPMMS_append(parent, thisPage, resultList, lhs);

			if (!leftIter->advance()) {
				TPMMS_append(parent, thisPage, resultList, rhs);
				while (rightIter->advance()) {
					rightIter->getCurrent(rhs);
					TPMMS_append(parent, thisPage, resultList, rhs);
				}
				break;
			}
		}
		else {
			TPMMS_append(parent, thisPage, resultList, rhs);

			if (!rightIter->advance()) {
				TPMMS_append(parent, thisPage, resultList, lhs);
				while (leftIter->advance()) {
					leftIter->getCurrent(lhs);
					TPMMS_append(parent, thisPage, resultList, lhs);
				}
				break;
			}
		}
	}

	resultList.push_back(thisPage);
	return resultList; 
}
	
void sort(int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe,
	function <bool()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	queue<vector<MyDB_PageReaderWriter> > curRunPageList;
	vector<MyDB_RecordIteratorAltPtr> recItEachRun;

	for (int i = 0; i < sortMe.getNumPages(); i++) {
		MyDB_PageReaderWriter curPage = *(sortMe[i].sort(comparator, lhs, rhs));
		vector<MyDB_PageReaderWriter> curPageList{curPage};
		curRunPageList.push(curPageList);

		if (curRunPageList.size() == runSize || i == sortMe.getNumPages() - 1) {
			while (curRunPageList.size() > 1) {
				vector<MyDB_PageReaderWriter> firstList = curRunPageList.front();
				curRunPageList.pop();
				vector<MyDB_PageReaderWriter> secondList = curRunPageList.front();
				curRunPageList.pop();

				curRunPageList.push(mergeIntoList(sortMe.getBufferMgr(), getIteratorAlt(firstList), getIteratorAlt(secondList), comparator, lhs, rhs));
			}
			
			vector<MyDB_PageReaderWriter> curRunMerged{curRunPageList.front()};
			recItEachRun.push_back(getIteratorAlt(curRunMerged));
			curRunPageList.pop();
		}
	}

	mergeIntoFile(sortIntoMe, recItEachRun, comparator, lhs, rhs);
} 

#endif