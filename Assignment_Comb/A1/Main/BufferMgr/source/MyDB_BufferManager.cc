
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr whichTable, long i) {
	MyDB_Page* thisPage = bufferCheck(whichTable->getName(), i, whichTable);
	thisPage->readFile();
	thisPage->reference++;
    return std::make_shared<MyDB_PageHandleBase>(thisPage, this);
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	MyDB_Page* thisPage = bufferCheck(tempFile, -1);
	thisPage->anonymous = true;
    thisPage->reference++;
    return std::make_shared<MyDB_PageHandleBase>(thisPage, this);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr whichTable, long i) {
	MyDB_Page* thisPage = bufferCheck(whichTable->getName(), i, whichTable);
	thisPage->pinned = true;
    thisPage->reference++;
	thisPage->readFile();
    return std::make_shared<MyDB_PageHandleBase>(thisPage, this);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	MyDB_Page* thisPage = bufferCheck(tempFile, -1);
    thisPage->reference++;
	thisPage->pinned = true;
	thisPage->anonymous = true;
    return std::make_shared<MyDB_PageHandleBase>(thisPage, this);
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
	unpinMe->unPin();
}

MyDB_Page* MyDB_BufferManager :: bufferCheck(string filename, long offset, MyDB_TablePtr tablePtr) {
	MyDB_Page* thisPage = isMatch(filename, offset);
	
	if (thisPage == nullptr || filename == tempFile) {
	   int index = isFull();
       if (index == -1) {
            index = LRU_update();
            pages[index]->flush();
			pages[index]->buffered = false;
            pages[index] = new MyDB_Page(this, ++timeCnt,filename, offset, pageSize, tablePtr);
            return pages[index];
        }
        thisPage = new MyDB_Page(this, ++timeCnt, filename, offset, pageSize, tablePtr);
        pages[index] = thisPage;
	    
	}
	return thisPage;
}

int MyDB_BufferManager :: LRU_update() {
	int index = 0;
	long min = LONG_MAX;
	for (int i = 0; i < numPage; i++) {
	    if (!pages[i]->pinned && pages[i]->timeStamp < min) {
	        min = pages[i]->timeStamp;
	        index = i;
	    }
	}
	return index;
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) :
	pageSize(pageSize), numPage(numPages), tempFile(tempFile), pages(vector<MyDB_Page*>(numPages, nullptr)), timeCnt(0) {
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
	for (int i = 0; i < numPage; i++) {
	    if(pages[i] != nullptr) delete pages[i];                        // not delete[] pages[i]
	}
	pages.clear();
	emptyTmp.clear();
	remove(("./" + tempFile).c_str());
}

MyDB_Page* MyDB_BufferManager :: isMatch(string filename, long offset) {
	for (int i = 0; i < numPage; i++) {
	    if (pages[i] != nullptr && pages[i]->filename == filename && pages[i]->offset == offset) return pages[i];
	}
    return nullptr;
}

int MyDB_BufferManager :: isFull() {
	for (int i = 0; i < numPage; i++) {
		if (pages[i] == nullptr) return i;
	}
	return -1;
}

set<long, less<long>> MyDB_BufferManager :: getEmptyTmp() {
    return emptyTmp;
}

void  MyDB_BufferManager :: emptyTmpInsert(int index) {
    if (index != -1) emptyTmp.insert(index);
}

long MyDB_BufferManager :: getSettimeCnt() {
    return ++timeCnt;
}

#endif