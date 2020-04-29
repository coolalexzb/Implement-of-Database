
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "MyDB_Page.h"
#include <vector>
#include <set>
#include <climits>

using namespace std;

class MyDB_BufferManager {
public:
	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// create the page object ptr
	MyDB_Page* bufferCheck(string filename, long offset, MyDB_TablePtr tablePtr = nullptr);

	// update buffer based on LRU
	int LRU_update();
	
	// find out weather this page is already in the buffer
	MyDB_Page* isMatch(string filename, long offset);

	// return -1, if is full;
	// return i, if i is the index of the first empty space
	int isFull();

	// get empty slot
    set<long, less<long>> getEmptyTmp();

	// update empty slot
	void emptyTmpInsert(int index);

	// update timestamp
	long getSettimeCnt();

private:

	size_t pageSize;
	size_t numPage;
	string tempFile;
    vector <MyDB_Page *> pages;
    set<long, less<long>> emptyTmp;
    long timeCnt;

};

#endif