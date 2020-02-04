//
// Created by 郑博 on 2/2/20.
//

#include "MyDB_PageRecIterator.h"

MyDB_PageRecIterator :: MyDB_PageRecIterator (MyDB_PageHandle page, size_t pos) :
        page (page), pos (pos) {}

void MyDB_PageRecIterator:: getNext () {

}


bool MyDB_PageRecIterator:: hasNext () {
    return true;
}
