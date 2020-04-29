
#ifndef SORTMERGE_CC
#define SORTMERGE_CC

#include "Aggregate.h"
#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "SortMergeJoin.h"
#include "Sorting.h"

SortMergeJoin :: SortMergeJoin (MyDB_TableReaderWriterPtr leftInputIn, MyDB_TableReaderWriterPtr rightInputIn,
	MyDB_TableReaderWriterPtr outputIn, string finalSelectionPredicateIn, vector <string> projectionsIn,
	pair <string, string> equalityCheckIn, string leftSelectionPredicateIn, string rightSelectionPredicateIn) {
	leftInput = leftInputIn;
	rightInput = rightInputIn;
	output = outputIn;
	finalSelectionPredicate = finalSelectionPredicateIn;
	projections = projectionsIn;
	equalityCheck = equalityCheckIn;
	leftSelectionPredicate = leftSelectionPredicateIn;
	rightSelectionPredicate = rightSelectionPredicateIn;
}

void SortMergeJoin :: run () {
	MyDB_BufferManagerPtr myBufferMgr = leftInput->getBufferMgr();

	MyDB_RecordPtr leftRec = leftInput->getEmptyRecord();
	MyDB_RecordPtr rightRec = rightInput->getEmptyRecord();
	MyDB_RecordPtr outputRec = output->getEmptyRecord();

	MyDB_RecordPtr leftRec_lhs = leftInput->getEmptyRecord();
	MyDB_RecordPtr leftRec_rhs = leftInput->getEmptyRecord();
	MyDB_RecordPtr rightRec_lhs = rightInput->getEmptyRecord();
	MyDB_RecordPtr rightRec_rhs = rightInput->getEmptyRecord();
	function<bool()> leftComparator = buildRecordComparator(leftRec_lhs, leftRec_rhs, equalityCheck.first);
	function<bool()> rightComparator = buildRecordComparator(rightRec_lhs, rightRec_rhs, equalityCheck.second);

	MyDB_RecordPtr leftRec_tmp = leftInput->getEmptyRecord();
	function<bool()> leftTmpCmp_LT = buildRecordComparator(leftRec, leftRec_tmp, equalityCheck.first);
	function<bool()> leftTmpCmp_GT = buildRecordComparator(leftRec_tmp, leftRec, equalityCheck.first);
	
	int runSize = myBufferMgr->numPages;
	MyDB_RecordIteratorAltPtr leftRecIt = buildItertorOverSortedRuns(runSize, *leftInput, leftComparator, leftRec_lhs, leftRec_rhs, leftSelectionPredicate);
	MyDB_RecordIteratorAltPtr rightRecIt = buildItertorOverSortedRuns(runSize, *rightInput, rightComparator, rightRec_lhs, rightRec_rhs, rightSelectionPredicate);

	// and get the schema that results from combining the left and right records
	MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema>();
	for (auto &p : leftInput->getTable()->getSchema()->getAtts())
		mySchemaOut->appendAtt(p);
	for (auto &p : rightInput->getTable()->getSchema()->getAtts())
		mySchemaOut->appendAtt(p);

	// get the combined record
	MyDB_RecordPtr combinedRec = make_shared <MyDB_Record>(mySchemaOut);
	combinedRec->buildFrom(leftRec, rightRec);

	// compares the two input recs
	func leftSmaller = combinedRec->compileComputation(" < (" + equalityCheck.first + ", " + equalityCheck.second + ")");
	func rightSmaller = combinedRec->compileComputation(" > (" + equalityCheck.first + ", " + equalityCheck.second + ")");
	func areEqual = combinedRec->compileComputation(" == (" + equalityCheck.first + ", " + equalityCheck.second + ")");

	func finalSelectFunc = combinedRec->compileComputation(finalSelectionPredicate);

	vector<func> projectFunc;
	for (string s : projections) {
		projectFunc.push_back(combinedRec->compileComputation(s));
	}

	if (!leftRecIt->advance() || !rightRecIt->advance())
		return;
	
	leftRecIt->getCurrent(leftRec);
	rightRecIt->getCurrent(rightRec);
	while (true) {

		if (leftSmaller()->toBool()) {
			if (!leftRecIt->advance())
				break;

			leftRecIt->getCurrent(leftRec);
		}
		else if (rightSmaller()->toBool()) {
			if (!rightRecIt->advance())
				break;

			rightRecIt->getCurrent(rightRec);
		}
		else if (areEqual()->toBool()) {
			bool canLeftAdvance = true;
			bool canRightAdvance = true;

			vector<MyDB_PageReaderWriter> pages;
			MyDB_PageReaderWriter curPage(true, *myBufferMgr);
			curPage.clear();

			leftRecIt->getCurrent(leftRec_tmp);
			while ((!leftTmpCmp_LT() && !leftTmpCmp_GT())) {
				
				if (!curPage.append(leftRec_tmp)) {
					MyDB_PageReaderWriter newPage(true, *myBufferMgr);
					newPage.clear();
					newPage.append(leftRec_tmp);

					pages.push_back(curPage);
					curPage = newPage;
				}

				if (leftRecIt->advance()) {
					leftRecIt->getCurrent(leftRec_tmp);
				}
				else {
					canLeftAdvance = false;
					break;
				}
			}
			
			pages.push_back(curPage);

			while (areEqual()->toBool()) {
				
				MyDB_RecordIteratorAltPtr pageIt = getIteratorAlt(pages);
				while (pageIt->advance()) {
					pageIt->getCurrent(leftRec);

					if (finalSelectFunc()->toBool()) {
						int i = 0;
						for (auto &f : projectFunc) {
							outputRec->getAtt(i++)->set(f());
						}

						outputRec->recordContentHasChanged();
						output->append(outputRec);
					}
				}

				if (rightRecIt->advance()) {
					rightRecIt->getCurrent(rightRec);
				}
				else {
					canRightAdvance = false;
					break;
				}
			}

			if (!canLeftAdvance || !canRightAdvance)
				break;

			leftRecIt->getCurrent(leftRec);
		}
		else {
			cout << "Should not reach here." << endl;
			break;
		}
	}
}

#endif
