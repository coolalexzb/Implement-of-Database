
#ifndef TABLERECITERATOR_H
#define TABLERECITERATOR_H

#include "MyDB_RecordIterator.h"
#include "MyDB_PageReaderWriter.h"

using namespace std;
class MyDB_PageReaderWriter;
class MyDB_TableRecIterator: public MyDB_RecordIterator {

public:

    MyDB_TableRecIterator(MyDB_RecordPtr curRec, vector<MyDB_PageReaderWriter> pages);

    // put the contents of the next record in the file/page into the iterator record
    // this should be called BEFORE the iterator record is first examined
    void getNext ();

    // return true iff there is another record in the file/page
    bool hasNext ();

private:

	MyDB_RecordPtr curRec;
    vector<MyDB_PageReaderWriter> pages;
    size_t pos;
	MyDB_RecordIteratorPtr curPageIt;

};

#endif