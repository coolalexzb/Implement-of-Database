
#ifndef BPLUS_SELECTION_C                                        
#define BPLUS_SELECTION_C

#include "BPlusSelection.h"

BPlusSelection :: BPlusSelection (MyDB_BPlusTreeReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
	MyDB_AttValPtr lowIn, MyDB_AttValPtr highIn, string selectionPredicateIn, vector <string> projectionsIn) {
	input = inputIn;
	output = outputIn;
	low = lowIn;
	high = highIn;
	selectionPredicate = selectionPredicateIn;
	projections = projectionsIn;
}

void BPlusSelection :: run () {
	MyDB_RecordPtr inputRec = input->getEmptyRecord();
	MyDB_RecordPtr outputRec = output->getEmptyRecord();
	MyDB_RecordIteratorAltPtr inputRecIt = input->getSortedRangeIteratorAlt(low, high);

	func selectFunc = inputRec->compileComputation(selectionPredicate);

	vector<func> projectFunc;
	for (string s : projections) {
		projectFunc.push_back(inputRec->compileComputation(s));
	}

	while (inputRecIt->advance()) {
		inputRecIt->getCurrent(inputRec);

		if (selectFunc()->toBool()) {
			int i = 0;
			for (auto &f : projectFunc) {
				outputRec->getAtt(i++)->set(f());
			}

			outputRec->recordContentHasChanged();
			output->append(outputRec);
		}
	}
}

#endif
