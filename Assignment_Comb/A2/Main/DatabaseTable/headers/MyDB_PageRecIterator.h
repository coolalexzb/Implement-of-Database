
#ifndef PAGERECITERATOR_H
#define PAGERECITERATOR_H

#include "MyDB_RecordIterator.h"
#include "MyDB_PageHandle.h"

using namespace std;

class MyDB_PageRecIterator: public MyDB_RecordIterator {

public:

    MyDB_PageRecIterator(MyDB_RecordPtr curRec, MyDB_PageHandle page);

    // put the contents of the next record in the file/page into the iterator record
    // this should be called BEFORE the iterator record is first examined
    void getNext ();

    // return true iff there is another record in the file/page
    bool hasNext ();

private:

	MyDB_RecordPtr curRec;
    MyDB_PageHandle pageH;
    size_t pos;

};

#endif