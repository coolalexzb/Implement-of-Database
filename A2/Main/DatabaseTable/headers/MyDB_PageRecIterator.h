//
// Created by 郑博 on 2/2/20.
//

#ifndef MAIN_MYDB_PAGERECITERATOR_H
#define MAIN_MYDB_PAGERECITERATOR_H

#include "MyDB_RecordIterator.h"
#include "MyDB_PageHandle.h"

using namespace std;

class MyDB_PageRecIterator: public MyDB_RecordIterator {
public:
    MyDB_PageRecIterator(MyDB_PageHandle page, size_t pos = 0);

    // put the contents of the next record in the file/page into the iterator record
    // this should be called BEFORE the iterator record is first examined
    void getNext ();

    // return true iff there is another record in the file/page
    bool hasNext ();

private:
    MyDB_PageHandle page;
    size_t pos;
};

#endif //MAIN_MYDB_PAGERECITERATOR_H
