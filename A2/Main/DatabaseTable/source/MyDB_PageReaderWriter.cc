
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"

void MyDB_PageReaderWriter :: clear () {
	pageOverlayPtr thisPage = (pageOverlayPtr)pageH->getBytes();
	thisPage->offsetToNextUnwritten = 0;
	thisPage->pageType = MyDB_PageType :: RegularPage;
	pageH->page->wroteBytes();
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
	pageOverlayPtr thisPage = (pageOverlayPtr)pageH->getBytes();
	return thisPage->pageType;
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr curRec) {
	return make_shared <MyDB_PageRecIterator>(curRec, pageH);;
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType setMe) {
	pageOverlayPtr thisPage = (pageOverlayPtr)pageH->getBytes();
	thisPage->pageType = setMe;
	pageH->page->wroteBytes();
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr appendMe) {
	pageOverlayPtr thisPage = (pageOverlayPtr)pageH->getBytes();
	if (sizeof(unsigned) + sizeof(MyDB_PageType) + thisPage->offsetToNextUnwritten + appendMe->getBinarySize() > pageH->getParent().getPageSize()) {
		return false;
	}
	
	void *loc = appendMe->toBinary(&(thisPage->bytes[thisPage->offsetToNextUnwritten]));
	thisPage->offsetToNextUnwritten = (char *)loc - &(thisPage->bytes[0]);
	pageH->page->wroteBytes();
	return true;
}

MyDB_PageReaderWriter :: MyDB_PageReaderWriter(MyDB_PageHandle page) : pageH(page) {}

MyDB_PageReaderWriter :: ~MyDB_PageReaderWriter() {}

#endif