
#ifndef REG_SELECTION_C                                        
#define REG_SELECTION_C

#include "RegularSelection.h"

RegularSelection :: RegularSelection (MyDB_TableReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
	string selectionPredicateIn, vector <string> projectionsIn) {
	input = inputIn;
	output = outputIn;
	selectionPredicate = selectionPredicateIn;
	projections = projectionsIn;
}

void RegularSelection :: run () {
	MyDB_RecordPtr inputRec = input->getEmptyRecord();
	MyDB_RecordPtr outputRec = output->getEmptyRecord();
	MyDB_RecordIteratorAltPtr inputRecIt = input->getIteratorAlt();

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
