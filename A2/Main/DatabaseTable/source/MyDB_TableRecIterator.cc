//
// Created by 郑博 on 2/2/20.
//

#include "MyDB_TableRecIterator.h"

MyDB_TableRecIterator :: MyDB_TableRecIterator (vector<MyDB_PageReaderWriter>* pages, size_t pos) :
        pages (pages), pos (pos) {}

void MyDB_TableRecIterator:: getNext () {

}


bool MyDB_TableRecIterator:: hasNext () {
    return true;
}

