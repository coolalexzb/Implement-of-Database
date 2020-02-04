//
// Created by 郑博 on 2/2/20.
//

#ifndef MAIN_MYDB_TABLERECITERATOR_H
#define MAIN_MYDB_TABLERECITERATOR_H

#include "MyDB_RecordIterator.h"
#include "MyDB_PageReaderWriter.h"

using namespace std;
class MyDB_PageReaderWriter;

class MyDB_TableRecIterator: public MyDB_RecordIterator {
public:
    MyDB_TableRecIterator(vector<MyDB_PageReaderWriter>* pages, size_t pos = 0);

    // put the contents of the next record in the file/page into the iterator record
    // this should be called BEFORE the iterator record is first examined
    void getNext ();

    // return true iff there is another record in the file/page
    bool hasNext ();

private:
    vector<MyDB_PageReaderWriter>* pages;
    size_t pos;
};

#endif //MAIN_MYDB_TABLERECITERATOR_H
