
#ifndef AGG_CC
#define AGG_CC

#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"
#include <unordered_map>

using namespace std;

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
	vector <pair <MyDB_AggType, string>> aggsToComputeIn, vector <string> groupingsIn, string selectionPredicateIn) {
	input = inputIn;
	output = outputIn;
	aggsToCompute = aggsToComputeIn;
	groupings = groupingsIn;
	selectionPredicate = selectionPredicateIn;
}

void Aggregate :: run() {
	MyDB_BufferManagerPtr myBufferMgr = input->getBufferMgr();

	MyDB_RecordPtr inputRec = input->getEmptyRecord();
	MyDB_RecordPtr outputRec = output->getEmptyRecord();
	MyDB_RecordIteratorAltPtr inputRecIt = input->getIteratorAlt();

	func selectFunc = inputRec->compileComputation(selectionPredicate);

	int groupNum = groupings.size();
	int aggNum = aggsToCompute.size();

	MyDB_SchemaPtr aggSchema = make_shared <MyDB_Schema>();
	for (int i = 0; i < groupNum; i++)
		aggSchema->appendAtt(make_pair("MyDB_GroupAtt" + to_string(i), output->getTable()->getSchema()->getAtts()[i].second));
	for (int i = 0; i < aggNum; i++)
		aggSchema->appendAtt(make_pair("MyDB_AggAtt" + to_string(i), output->getTable()->getSchema()->getAtts()[groupNum + i].second));
	aggSchema->appendAtt(make_pair("MyDB_CntAtt", make_shared <MyDB_IntAttType>()));

	MyDB_SchemaPtr combinedSchema = make_shared <MyDB_Schema>();
	for (auto &p : input->getTable()->getSchema()->getAtts())
		combinedSchema->appendAtt(p);
	for (auto &p : aggSchema->getAtts())
		combinedSchema->appendAtt(p);
	
	MyDB_RecordPtr aggRec = make_shared <MyDB_Record>(aggSchema);
	MyDB_RecordPtr combinedRec = make_shared <MyDB_Record>(combinedSchema);
	combinedRec->buildFrom(inputRec, aggRec);

	vector<func> groupFunc;
	for (string s : groupings)
		groupFunc.push_back(inputRec->compileComputation(s));

	string groupCmpExpr = "bool[true]";
	for (int i = 0; i < groupNum; i++) {
		string curExpr = "== (" + groupings[i] + ", [" + aggSchema->getAtts()[i].first + "])";
		groupCmpExpr = "&& (" + groupCmpExpr + ", " + curExpr + ")";
	}
	func groupCmpFunc = combinedRec->compileComputation(groupCmpExpr);

	int index = groupNum;
	string funcExpr = "";
	vector<func> aggFunc;
	for (pair <MyDB_AggType, string> agg : aggsToCompute) {
		if (agg.first == MyDB_AggType :: sum)
			funcExpr = "+ (" + agg.second + ", [" + aggSchema->getAtts()[index].first + "])";
		else if (agg.first == MyDB_AggType :: avg)
			funcExpr = "/ (+ (" + agg.second + ", * ([" + aggSchema->getAtts()[index].first + "], [MyDB_CntAtt]) ), + (int[1], [MyDB_CntAtt])";
		else if (agg.first == MyDB_AggType :: cnt)
			funcExpr = "+ (int[1], [" + aggSchema->getAtts()[index].first + "])";

		aggFunc.push_back(combinedRec->compileComputation(funcExpr));
		index++;
	}
	aggFunc.push_back(combinedRec->compileComputation("+ (int[1], [MyDB_CntAtt])"));

	unordered_map<size_t, vector<void *>> myHash;

	vector<MyDB_PageReaderWriter> pages;
	MyDB_PageReaderWriter curPage(true, *myBufferMgr);
	curPage.clear();

	while (inputRecIt->advance()) {
		inputRecIt->getCurrent(inputRec);

		if (!selectFunc()->toBool())
			continue;
		
		size_t hashVal = 0;
		for (auto &f : groupFunc) {
			hashVal ^= f()->hash();
		}
		
		vector<void *> &potentialMatches = myHash[hashVal];
		void *pos = nullptr;
		for (auto curPos : potentialMatches) {
			aggRec->fromBinary(curPos);
			
			if (groupCmpFunc()->toBool()) {
				pos = curPos;
				break;
			}
		}
		
		if (pos == nullptr) {
			int i = 0;
			for (auto &f : groupFunc)
				aggRec->getAtt(i++)->set(f());
			for (auto &f : aggFunc)
				aggRec->getAtt(i++)->set(make_shared <MyDB_IntAttVal>());

			i = groupNum;
			for (auto &f : aggFunc)
				aggRec->getAtt(i++)->set(f());
			
			aggRec->recordContentHasChanged();
			pos = curPage.appendAndReturnLocation(aggRec);
			
			if (pos == nullptr) {
				MyDB_PageReaderWriter newPage(true, *myBufferMgr);
				newPage.clear();
				pos = newPage.appendAndReturnLocation(aggRec);

				pages.push_back(curPage);
				curPage = newPage;
			}

			myHash[hashVal].push_back(pos);
		}
		else {
			int i = groupNum;
			for (auto &f : aggFunc)
				aggRec->getAtt(i++)->set(f());
			
			aggRec->recordContentHasChanged();
			aggRec->toBinary(pos);
		}
	}
	pages.push_back(curPage);

	MyDB_RecordIteratorAltPtr pageIt = getIteratorAlt(pages);
	while (pageIt->advance()) {
		pageIt->getCurrent(aggRec);

		for (int i = 0; i < groupNum + aggNum; i++) {
			outputRec->getAtt(i)->set(aggRec->getAtt(i));
		}

		outputRec->recordContentHasChanged();
		output->append(outputRec);
	}
}

#endif
