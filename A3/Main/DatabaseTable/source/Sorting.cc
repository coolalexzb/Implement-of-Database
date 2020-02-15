
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;

void TPMMS_append(MyDB_BufferManagerPtr parent, MyDB_PageReaderWriter &thisPage, vector<MyDB_PageReaderWriter> &pageList, MyDB_RecordPtr recPtr) {
    if (!thisPage.append(recPtr)) {
        pageList.push_back(thisPage);
        MyDB_PageReaderWriter newPage(*parent);
        newPage.append(recPtr);
        thisPage = newPage;
    }
}

void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs,
                    function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
    RecIterComparator comp (comparator, lhs, rhs);
    priority_queue <MyDB_RecordIteratorAltPtr, vector<MyDB_RecordIteratorAltPtr>, RecIterComparator> pq(comp);

    for(int i = 0; i < mergeUs.size(); i++) {
        pq.push(mergeUs[i]);
    }

    while(!pq.empty()) {
        MyDB_RecordIteratorAltPtr recItem = pq.top();
        pq.pop();
        recItem->getCurrent(lhs);
        sortIntoMe.append(lhs);
        if(recItem->advance()) {
            pq.push(recItem);
        }
    }
}


vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter,
                                              MyDB_RecordIteratorAltPtr rightIter, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs){
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
void sort (int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe,
           function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

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
