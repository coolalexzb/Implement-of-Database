
#ifndef PAGE_C
#define PAGE_C

#include "MyDB_Page.h"
#include "MyDB_BufferManager.h"

MyDB_Page :: MyDB_Page (MyDB_BufferManager* bfmgr, long timeStamp, string filename, long offset, size_t pageSize, MyDB_TablePtr tablePtr) :
		filename(filename), offset(offset), reference(0), pinned(false), dirty(false), buffered(true), anonymous(false), timeStamp(timeStamp),
		pageSize(pageSize),  tablePtr(tablePtr), bfmgr(bfmgr) {
    data = malloc(pageSize);
}

MyDB_Page :: ~MyDB_Page() {
	flush();
	delete [] data;
}

void MyDB_Page :: readFile() {
    string path = "";
    if (tablePtr) {
        path = "./" + tablePtr->getStorageLoc() + "/" + tablePtr->getName();
        mkdir(("./" + tablePtr->getStorageLoc()).c_str(), 0777);
    } else {
        path = "./" + filename;
    }

    int file = open(path.c_str(), O_FSYNC | O_RDWR | O_CREAT, 0777);
    if (file < 0) {
        cout << path.data() << endl;
        cout << "File open failed!" << endl;
        return;
    }
    lseek(file, pageSize * offset, SEEK_SET);
    read(file, data, pageSize);
    timeStamp = bfmgr->getSettimeCnt();
    close(file);
}

void MyDB_Page :: flush() {
	if (dirty) {
        if (anonymous) {
			string path = "./" + filename;
            int file = open(path.c_str(), O_FSYNC | O_RDWR | O_CREAT, 0777);
            if (file < 0) {
                cout << "File open failed!" << endl;
                return;
            }
            if (offset >= 0) {                                         // if anonymous flush again, cover
                lseek(file, pageSize * offset, SEEK_SET);
            } else {
                long pos = 0;
                if (bfmgr->getEmptyTmp().empty()) {                    // if anonymous flush 1st time and tmp file is full, app
                    pos = lseek(file, 0, SEEK_END) / pageSize;
                } else {
                    pos = *bfmgr->getEmptyTmp().begin();               // if anonymous flush 1st time and tmp file is not full, find empty
                    lseek(file, pageSize * pos, SEEK_SET);
                }
                offset = pos;
                bfmgr->getEmptyTmp().erase(offset);
            }
            write(file, data, pageSize);
            timeStamp = bfmgr->getSettimeCnt();
            close(file);
        } else {
            string path = "";
            if (tablePtr) {
                path = "./" + tablePtr->getStorageLoc() + "/" + tablePtr->getName();
                mkdir(("./" + tablePtr->getStorageLoc()).c_str(), 0777);
            } else {
                path = "./" + filename;
            }

            int file = open(path.c_str(), O_FSYNC | O_RDWR | O_CREAT, 0777);
            if (file < 0) {
                cout << "File open failed!" << endl;
                return;
            }
            lseek(file, pageSize * offset, SEEK_SET);
            write(file, data, pageSize);
            timeStamp = bfmgr->getSettimeCnt();
            close(file);
        }
	}
}

#endif