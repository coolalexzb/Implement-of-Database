
#ifndef BPLUS_C
#define BPLUS_C

#include "MyDB_INRecord.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageListIteratorSelfSortingAlt.h"
#include "RecordComparator.h"

MyDB_BPlusTreeReaderWriter::MyDB_BPlusTreeReaderWriter(string orderOnAttName, MyDB_TablePtr forMe,
	MyDB_BufferManagerPtr myBuffer) : MyDB_TableReaderWriter(forMe, myBuffer) {
	// find the ordering attribute
	auto res = forMe->getSchema()->getAttByName(orderOnAttName);

	// remember information about the ordering attribute
	orderingAttType = res.second;
	whichAttIsOrdering = res.first;

	// and the root location
	rootLocation = getTable()->getRootLocation();
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter::getSortedRangeIteratorAlt(MyDB_AttValPtr low, MyDB_AttValPtr high) {
	return myGetRangeIteratorAlt(low, high, true);
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter::getRangeIteratorAlt(MyDB_AttValPtr low, MyDB_AttValPtr high) {
	return myGetRangeIteratorAlt(low, high, false);
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter::myGetRangeIteratorAlt(MyDB_AttValPtr low, MyDB_AttValPtr high, bool sortedOrNot) {
	vector<MyDB_PageReaderWriter> pageList;
	discoverPages(rootLocation, pageList, low, high);

	// sorting comparator
	MyDB_RecordPtr lhs = getEmptyRecord();
	MyDB_RecordPtr rhs = getEmptyRecord();
	function <bool()> comparator = buildComparator(lhs, rhs);

	// range examining comparator
	MyDB_RecordPtr myRec = getEmptyRecord();
	MyDB_INRecordPtr lowerBound = getINRecord();
	MyDB_INRecordPtr higherBound = getINRecord();
	lowerBound->setKey(low);
	higherBound->setKey(high);

	function <bool()> lowComparator = buildComparator(myRec, lowerBound);		// !lowComparator() indicates myRec >= lowerBound
	function <bool()> highComparator = buildComparator(higherBound, myRec);		// !highComparator() indicates myRec <= higherBound

	return make_shared <MyDB_PageListIteratorSelfSortingAlt>(pageList, lhs, rhs, comparator, myRec, lowComparator, highComparator, sortedOrNot);
}

bool MyDB_BPlusTreeReaderWriter::discoverPages(int whichpage, vector <MyDB_PageReaderWriter> &pageList, MyDB_AttValPtr low, MyDB_AttValPtr high) {
	MyDB_PageReaderWriter thispage = (*this)[whichpage];
	if (thispage.getType() == MyDB_PageType::RegularPage) {
		// directly append the leaf pages to the list

		pageList.push_back(thispage);
		return true;
	}
	else if (thispage.getType() == MyDB_PageType::DirectoryPage) {
		// recursively check all the pages this inner page points to

		MyDB_RecordIteratorAltPtr thispageIt = thispage.getIteratorAlt();

		MyDB_INRecordPtr myINRec = getINRecord();
		MyDB_INRecordPtr lowerBound = getINRecord();
		MyDB_INRecordPtr higherBound = getINRecord();
		lowerBound->setKey(low);
		higherBound->setKey(high);

		function <bool()> lowComparator = buildComparator(myINRec, lowerBound);		// !lowComparator() indicates myINRec >= lowerBound
		function <bool()> highComparator = buildComparator(higherBound, myINRec);		// !highComparator() indicates myINRec <= higherBound

		bool reachLeaf = false;
		while (thispageIt->advance()) {
			thispageIt->getCurrent(myINRec);
			if (!lowComparator()) {		// The first key larger than higher bound should be take into consideration
				if (reachLeaf)
					// directly append the leaf page to the list 
					pageList.push_back((*this)[myINRec->getPtr()]);
				else
					reachLeaf = discoverPages(myINRec->getPtr(), pageList, low, high);
			}

			if (highComparator()) break;
		}

		return false;
	}
	else {
		// wrong page type

		cout << "Wrong page type detected at table " << forMe->getName() << " page " << whichpage << endl;
		exit(1);
	}
}

void MyDB_BPlusTreeReaderWriter::append(MyDB_RecordPtr appendMe) {
	if (getNumPages() <= 1) {
		// initialize a new tree

		rootLocation = 0;
		forMe->setRootLocation(0);
		MyDB_PageReaderWriter rootpage = (*this)[0];
		rootpage.setType(MyDB_PageType::DirectoryPage);

		MyDB_PageReaderWriter leafpage = (*this)[1];
		leafpage.append(appendMe);

		MyDB_INRecordPtr myInRec = getINRecord();
		myInRec->setPtr(1);
		rootpage.append(myInRec);
	}
	else {
		// already build the tree

		MyDB_RecordPtr appendResult = append(rootLocation, appendMe);
		if (appendResult != nullptr) {
			// root still need spliting

			MyDB_PageReaderWriter oldRoot = (*this)[rootLocation];
			MyDB_PageReaderWriter newRoot = (*this)[forMe->lastPage() + 1];
			newRoot.setType(MyDB_PageType::DirectoryPage);
			newRoot.append(appendResult);

			MyDB_INRecordPtr myInRec = getINRecord();
			myInRec->setPtr(rootLocation);
			newRoot.append(myInRec);

			rootLocation = forMe->lastPage();
			forMe->setRootLocation(forMe->lastPage());
		}
	}
}

MyDB_RecordPtr MyDB_BPlusTreeReaderWriter::split(MyDB_PageReaderWriter splitMe, MyDB_RecordPtr appendMe) {
	MyDB_PageReaderWriter newPage = (*this)[forMe->lastPage() + 1];
	MyDB_PageType pageType = splitMe.getType();
	MyDB_PageReaderWriterPtr splitMe_copy;
	MyDB_RecordPtr tmpRec;
	vector<void *> recList;

	// initial space for storing appendMe 
	void *appendMeLoc = malloc(appendMe->getBinarySize());
	appendMe->toBinary(appendMeLoc);

	if (pageType == MyDB_PageType::RegularPage) {

		// create a copy of splitMe and sort
		MyDB_RecordPtr lhs = getEmptyRecord();
		MyDB_RecordPtr rhs = getEmptyRecord();
		function <bool()> comparator = buildComparator(lhs, rhs);
		splitMe_copy = splitMe.sort(comparator, lhs, rhs);

		splitMe.clear();

		tmpRec = getEmptyRecord();
		function <bool()> newComparator = buildComparator(tmpRec, appendMe);
		
		// go over all the records
		// insert appendMe into the list
		bool isInserted = true;
		MyDB_RecordIteratorAltPtr pageIt = splitMe_copy->getIteratorAlt();
		while (pageIt->advance()) {
			pageIt->getCurrent(tmpRec);
			if (!newComparator() && isInserted) {
				recList.push_back(appendMeLoc);
				isInserted = false;
			}

			recList.push_back(pageIt->getCurrentPointer());
		}

		if (isInserted) recList.push_back(appendMeLoc);
	}
	else if (pageType == MyDB_PageType::DirectoryPage) {

		// create a copy of splitMe and sort
		MyDB_INRecordPtr lhs = getINRecord();
		MyDB_INRecordPtr rhs = getINRecord();
		function <bool()> comparator = buildComparator(lhs, rhs);
		MyDB_PageReaderWriterPtr splitMe_copy = splitMe.sort(comparator, lhs, rhs);

		splitMe.clear();
		splitMe.setType(MyDB_PageType::DirectoryPage);
		newPage.setType(MyDB_PageType::DirectoryPage);

		tmpRec = getINRecord();
		function <bool()> newComparator = buildComparator(tmpRec, appendMe);

		// go over all the records
		// insert appendMe into the list
		bool isInserted = true;
		MyDB_RecordIteratorAltPtr pageIt = splitMe_copy->getIteratorAlt();
		while (pageIt->advance()) {
			pageIt->getCurrent(tmpRec);
			if (!newComparator() && isInserted) {
				recList.push_back(appendMeLoc);
				isInserted = false;
			}

			recList.push_back(pageIt->getCurrentPointer());
		}

		if (isInserted) recList.push_back(appendMeLoc);
	}
	else {
		// wrong page type

		cout << "Wrong page type detected at table " << forMe->getName() << endl;
		exit(1);
	}

	// inital the record to return
	MyDB_INRecordPtr resRec = getINRecord();
	resRec->setPtr(forMe->lastPage());

	// split records
	int numRec = recList.size();
	for (int i = 0; i < numRec; i++) {
		tmpRec->fromBinary(recList[i]);

		if (i < numRec / 2) {
			newPage.append(tmpRec);
		}
		else if (i == numRec / 2) {
			newPage.append(tmpRec);
			resRec->setKey(getKey(tmpRec));
		}
		else {
			splitMe.append(tmpRec);
		}
	}

	free(appendMeLoc);
	return resRec;
}

MyDB_RecordPtr MyDB_BPlusTreeReaderWriter::append(int whichpage, MyDB_RecordPtr appendMe) {
	MyDB_PageReaderWriter thispage = (*this)[whichpage];
	if (thispage.getType() == MyDB_PageType::RegularPage) {
		// directly append the record to the leaf page

		if (thispage.append(appendMe)) {
			return nullptr;
		}
		else {
			// need spliting
			return split(thispage, appendMe);
		}
	}
	else if (thispage.getType() == MyDB_PageType::DirectoryPage) {
		// percolate down to find the corresponding leaf page to insert the record

		MyDB_RecordIteratorAltPtr thispageIt = thispage.getIteratorAlt();

		MyDB_INRecordPtr myINRec = getINRecord();
		function <bool()> comparator = buildComparator(appendMe, myINRec);

		while (thispageIt->advance()) {
			thispageIt->getCurrent(myINRec);
			if (comparator()) {
				// find the corresponing ptr to go down 

				MyDB_RecordPtr appendResult = append(myINRec->getPtr(), appendMe);
				if (appendResult != nullptr) {
					// the child of this inner page splits

					if (thispage.append(appendResult)) {
						// direct insert and sort

						MyDB_INRecordPtr lhs = getINRecord();
						MyDB_INRecordPtr rhs = getINRecord();
						function <bool()> comparator = buildComparator(lhs, rhs);

						thispage.sortInPlace(comparator, lhs, rhs);
						return nullptr;
					}
					else {
						// still need spliting

						return split(thispage, appendResult);
					}
				}

				return nullptr;
			}
		}
		// bug - no place to insert

		cout << "No place to insert the record. " << endl;
		exit(1);
	}
	else {
		// wrong page type

		cout << "Wrong page type detected at table " << forMe->getName() << " page " << whichpage << endl;
		exit(1);
	}
}

MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter::getINRecord() {
	return make_shared <MyDB_INRecord>(orderingAttType->createAttMax());
}

MyDB_AttValPtr MyDB_BPlusTreeReaderWriter::getKey(MyDB_RecordPtr fromMe) {
	// in this case, got an IN record
	if (fromMe->getSchema() == nullptr)
		return fromMe->getAtt(0)->getCopy();

	// in this case, got a data record
	else
		return fromMe->getAtt(whichAttIsOrdering)->getCopy();
}

void MyDB_BPlusTreeReaderWriter::printTree() {
	printing(rootLocation, 0);
}

void MyDB_BPlusTreeReaderWriter::printing(int whichpage, int depth) {
	MyDB_PageReaderWriter thispage = (*this)[whichpage];

	if (thispage.getType() == MyDB_PageType::RegularPage) {
		// print the leaf page
		
		MyDB_RecordPtr myRec = getEmptyRecord();
		MyDB_RecordIteratorAltPtr pageIt = thispage.getIteratorAlt();
		while (pageIt->advance()) {
			pageIt->getCurrent(myRec);			
			cout << "(" << depth << ", " << myRec << ")"<< endl;
		}
	}
	else if (thispage.getType() == MyDB_PageType::DirectoryPage) {
		// recursively print out all the pages

		MyDB_INRecordPtr myINRec = getINRecord();
		MyDB_RecordIteratorAltPtr pageIt = thispage.getIteratorAlt();
		while (pageIt->advance()) {
			pageIt->getCurrent(myINRec);
			printing(myINRec->getPtr(), depth + 1);
			
			cout << "(" << depth << ", " << (MyDB_RecordPtr)myINRec << ")" << endl;
		}
	}
	else {
		// wrong page type

		cout << "Wrong page type detected at table " << forMe->getName() << " page " << whichpage << endl;
		exit(1);
	}
}

function <bool()>  MyDB_BPlusTreeReaderWriter::buildComparator(MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	MyDB_AttValPtr lhAtt, rhAtt;

	// in this case, the LHS is an IN record
	if (lhs->getSchema() == nullptr) {
		lhAtt = lhs->getAtt(0);

		// here, it is a regular data record
	}
	else {
		lhAtt = lhs->getAtt(whichAttIsOrdering);
	}

	// in this case, the LHS is an IN record
	if (rhs->getSchema() == nullptr) {
		rhAtt = rhs->getAtt(0);

		// here, it is a regular data record
	}
	else {
		rhAtt = rhs->getAtt(whichAttIsOrdering);
	}

	// now, build the comparison lambda and return
	if (orderingAttType->promotableToInt()) {
		return [lhAtt, rhAtt] {return lhAtt->toInt() < rhAtt->toInt(); };
	}
	else if (orderingAttType->promotableToDouble()) {
		return [lhAtt, rhAtt] {return lhAtt->toDouble() < rhAtt->toDouble(); };
	}
	else if (orderingAttType->promotableToString()) {
		return [lhAtt, rhAtt] {return lhAtt->toString() < rhAtt->toString(); };
	}
	else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit(1);
	}
}

#endif