
#ifndef PAGERECITERATOR_C
#define PAGERECITERATOR_C

#include "MyDB_PageRecIterator.h"

MyDB_PageRecIterator :: MyDB_PageRecIterator (MyDB_RecordPtr curRec, MyDB_PageHandle pageH) :
	curRec(curRec), pageH(pageH), pos(0) {
}

void MyDB_PageRecIterator :: getNext () {
	pageOverlayPtr thisPage = (pageOverlayPtr)pageH->getBytes();
	void *loc = curRec->fromBinary(&(thisPage->bytes[pos]));
	pos = (char *)loc - &(thisPage->bytes[0]);
}

bool MyDB_PageRecIterator:: hasNext () {
	pageOverlayPtr thisPage = (pageOverlayPtr)pageH->getBytes();
	return pos < thisPage->offsetToNextUnwritten;
}

#endif