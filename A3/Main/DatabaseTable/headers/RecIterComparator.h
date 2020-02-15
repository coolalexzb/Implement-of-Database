//
// Created by 郑博 on 2/15/20.
//

#ifndef A3_RECITERCOMPARATOR_H
#define A3_RECITERCOMPARATOR_H

#include "MyDB_Record.h"
#include "MyDB_TableRecIteratorAlt.h"
#include <iostream>
using namespace std;

class RecIterComparator {

public:

    RecIterComparator (function <bool ()> &comparatorIn, MyDB_RecordPtr lhsIn,  MyDB_RecordPtr rhsIn) {
        comparator = comparatorIn;
        lhs = lhsIn;
        rhs = rhsIn;
    }

    bool operator () (MyDB_RecordIteratorAltPtr lhsPtr, MyDB_RecordIteratorAltPtr rhsPtr) {
        lhsPtr->getCurrent (lhs);
        rhsPtr->getCurrent (rhs);
        return !comparator ();
    }

private:

    function <bool ()> comparator;
    MyDB_RecordPtr lhs;
    MyDB_RecordPtr rhs;

};

#endif //A3_RECITERCOMPARATOR_H
