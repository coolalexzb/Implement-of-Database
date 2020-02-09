
#ifndef TABLERECITERATOR_C
#define TABLERECITERATOR_C

#include "MyDB_TableRecIterator.h"

MyDB_TableRecIterator :: MyDB_TableRecIterator (MyDB_RecordPtr curRec, vector<MyDB_PageReaderWriter> pages) :
        curRec(curRec), pages (pages), pos (0) {
	curPageIt = pages[0].getIterator(curRec);
}

void MyDB_TableRecIterator :: getNext () {
	curPageIt->getNext();
}

bool MyDB_TableRecIterator::hasNext() {
	if (curPageIt->hasNext()) return true;

	for (int i = pos + 1; i < pages.size(); i++) {
		if (pages[i].getIterator(curRec)->hasNext()) {
			pos = i;
			curPageIt->~MyDB_RecordIterator();
			curPageIt = pages[pos].getIterator(curRec);
			return true;
		}
	}
	return false;
}

#endif