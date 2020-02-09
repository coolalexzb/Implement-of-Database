
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"

using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter(MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer) :
	tablePtr(forMe), myBuffer(myBuffer) {
	if (forMe->lastPage() == -1) {
		mytable_pages.push_back(MyDB_PageReaderWriter(myBuffer->getPage(tablePtr, 0)));
		mytable_pages[0].clear();
		forMe->setLastPage(0);
	}
	else {
		for (int i = 0; i <= forMe->lastPage(); i++) {
			mytable_pages.push_back(MyDB_PageReaderWriter(myBuffer->getPage(tablePtr, i)));
		}
	}
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t i) {
	return mytable_pages[i];
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord() {
	return make_shared <MyDB_Record>(tablePtr->getSchema());
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last() {
	return mytable_pages[tablePtr->lastPage()];
}

void MyDB_TableReaderWriter :: append(MyDB_RecordPtr appendMe) {
	if (!mytable_pages[tablePtr->lastPage()].append(appendMe)) {
		tablePtr->setLastPage(tablePtr->lastPage() + 1);
		MyDB_PageReaderWriter newPage(myBuffer->getPage(tablePtr, tablePtr->lastPage()));
		newPage.clear();
		newPage.append(appendMe);
		mytable_pages.push_back(newPage);
	}
}

void MyDB_TableReaderWriter :: loadFromTextFile(string fromMe) {
	tablePtr->setLastPage(0);
	mytable_pages[0].clear();
	
	ifstream read_file;
	read_file.open(fromMe, ios::in);
	if (read_file.is_open()) {
		string line;
		MyDB_RecordPtr curRec = getEmptyRecord();
		while (getline(read_file, line)) {
			//cout << "line:" << line.c_str() << endl;
			curRec->fromString(line);
			append(curRec);
		}
		read_file.close();
	}
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator(MyDB_RecordPtr curRec) {
	return make_shared <MyDB_TableRecIterator>(curRec, mytable_pages);
}

void MyDB_TableReaderWriter :: writeIntoTextFile(string toMe) {
	ofstream write_file;
	write_file.open(toMe, ios::out);
	if (write_file.is_open()) {
		MyDB_RecordPtr curRec = getEmptyRecord();
		MyDB_RecordIteratorPtr recIter = getIterator(curRec);
		while (recIter->hasNext()) {
			recIter->getNext();
			write_file << curRec << endl;
		}
		write_file.close();
	}
}

#endif