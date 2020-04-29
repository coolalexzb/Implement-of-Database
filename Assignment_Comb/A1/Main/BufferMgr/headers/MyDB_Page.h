
#ifndef PAGE_H
#define PAGE_H

#include <string>
#include <memory>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MyDB_Table.h"

using namespace std;
class MyDB_BufferManager;

class MyDB_Page {
public:
    friend class MyDB_BufferManager;
    friend class MyDB_PageHandleBase;

	MyDB_Page(MyDB_BufferManager* bfmgr, long timeStamp, string filename = "", long offset = -1, size_t pageSize = 0, MyDB_TablePtr tablePtr = nullptr);
	
	~MyDB_Page();
	
	// read from file
	void readFile();

	// flush back
	void flush();

private:

	void * data;
    string filename;
    int offset;
    int reference;
    bool pinned;
	bool dirty;
	bool buffered;
	bool anonymous;
    long timeStamp;
	size_t pageSize;
    MyDB_TablePtr tablePtr;
    MyDB_BufferManager* bfmgr;

};

#endif