
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include "MyDB_PageHandle.h"
#include "MyDB_BufferManager.h"

using namespace std;

void *MyDB_PageHandleBase :: getBytes () {
	if (!page->buffered) {
        page = bfmgr->bufferCheck(page->filename, page->offset, page->tablePtr);
        page->readFile();
		page->buffered = true;
	}
	page->timeStamp = bfmgr->getSettimeCnt();
	return page->data;
}

void MyDB_PageHandleBase :: wroteBytes () {
	page->dirty = true;
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    page->reference--;
    if (page->reference == 0) page->pinned = false;
    if (page->anonymous) bfmgr->emptyTmpInsert(page->offset);
}

MyDB_PageHandleBase::MyDB_PageHandleBase(MyDB_Page* page, MyDB_BufferManager* bfmgr) :
	page(page), bfmgr(bfmgr) {}

void MyDB_PageHandleBase :: unPin() {
	page->pinned = false;
}

#endif