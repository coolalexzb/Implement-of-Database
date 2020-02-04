
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"

void MyDB_PageReaderWriter :: clear () {
    page->page->metaData->numRecs = 0;
    page->page->metaData->offsetToNextUnwritten = 0;
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
    return page->page->metaData->pageType;
	return MyDB_PageType :: RegularPage;
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr) {
	return nullptr;
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType toMe) {
    page->page->metaData->pageType = toMe;
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr appendMe) {
    if(canWrite(appendMe)) {

        //void *next = appendMe->toBinary (&(page->page->metaData->bytes[page->page->metaData->offsetToNextUnwritten]));
        //page->page->metaData->offsetToNextUnwritten += (char *) next - &(page->page->metaData->bytes[page->page->metaData->offsetToNextUnwritten]);
        return true;
    }
	return false;
}

bool MyDB_PageReaderWriter :: canWrite(MyDB_RecordPtr appendMe) {
    return page->page->numBytes - appendMe->getBinarySize() >= 0;
}

MyDB_PageReaderWriter :: MyDB_PageReaderWriter(MyDB_PageHandle page, size_t size) :
        page(page), curSize(size){}

void MyDB_PageReaderWriter :: writeIntoTextFile() {
    PageOverlay* mypage =  (PageOverlay *)page->getBytes();
    mypage->offsetToNextUnwritten;

    page->wroteBytes();
}
#endif
