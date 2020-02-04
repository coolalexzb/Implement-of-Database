
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"

using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer):
        tablePtr(forMe), myBuffer(myBuffer){
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t i) {

	if(i > pages.size()) {
	    cout << "out of table size" << endl;
	    return pages[0];
	}

	return pages[i];
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	return nullptr;
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
	return pages[pages.size() - 1];
}


void MyDB_TableReaderWriter :: append (MyDB_RecordPtr) {
}

void MyDB_TableReaderWriter :: loadFromTextFile (string) {
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr) {
	return nullptr;
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string) {
    for(int i = 0; i < pages.size(); i++) {
        pages[i].writeIntoTextFile();
    }
}

#endif

