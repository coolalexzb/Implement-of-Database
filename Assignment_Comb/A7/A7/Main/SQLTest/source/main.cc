
#include "Lexer.h"
#include "Parser.h"
#include "ParserTypes.h"
#include "MyDB_BufferManager.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include <string>      
#include <iostream>   
#include <sstream>
#include <algorithm>
#include <iterator>
#include <time.h>
#include "RegularSelection.h"
#include "Aggregate.h"

using namespace std;
string toLower (string data) {
	transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}

MyDB_TableReaderWriterPtr getInputTableJoined(vector <pair <string, string>> tablesToProcess, map <string, MyDB_TableReaderWriterPtr> allTables) {
	string tableName = tablesToProcess[0].first;
	return allTables[tableName];
}

void executeQuery(SQLStatement *sqlQuery, map<string, map<string, string>> allInfo, 
	map <string, MyDB_TableReaderWriterPtr> allTables, MyDB_BufferManagerPtr bufferMgr) {
	SFWQuery myQuery = sqlQuery->getSFWQuery();

	vector <ExprTreePtr> valuesToSelect = myQuery.getSelectClause();
	vector <pair <string, string>> tablesToProcess = myQuery.getFromClause();
	vector <ExprTreePtr> allDisjunctions = myQuery.getWhereClause();
	vector <ExprTreePtr> groupingClauses = myQuery.getGroupByClause();

	// update schema for sqlQuery
	map <string, string> tableDef;
	for (auto a : tablesToProcess) {
		tableDef[a.second] = a.first;
		allTables[a.first]->getTable()->getSchema()->updateAttName(a.second);
	}

	MyDB_TableReaderWriterPtr inputTable = getInputTableJoined(tablesToProcess, allTables);
	MyDB_SchemaPtr mySchemaOut = make_shared<MyDB_Schema>();
	
	vector<string> projections;
	vector<string> groupings;
	vector<pair<MyDB_AggType, string>> aggsToCompute;

	for (auto a : valuesToSelect) {
		
		// no aggregate attributes
		if (a->aggType() < 0) {
			projections.push_back(a->toString());
			groupings.push_back(a->toString());
			
			string attType = a->getType(tableDef, allInfo);
			if (attType == "int") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_IntAttType>()));
			}
			else if (attType == "double") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_DoubleAttType>()));
			}
			else if (attType == "string") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_StringAttType>()));
			}
			else if (attType == "bool") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_BoolAttType>()));
			}
			else {
				cout << "Wrong attribute type!" << endl;
				exit(0);
			}
		}
	}

	for (auto a : valuesToSelect) {

		// aggregate attributes
		if (a->aggType() > 0) {
			if (a->aggType() == 1) {
				aggsToCompute.push_back(make_pair(MyDB_AggType :: SUM, a->childToString()));
			}
			else if (a->aggType() == 2) {
				aggsToCompute.push_back(make_pair(MyDB_AggType :: AVG, a->childToString()));
			}
			else {
				cout << "Wrong aggregation type!" << endl;
				exit(0);
			}
			
			string attType = a->getType(tableDef, allInfo);
			if (attType == "int") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_IntAttType>()));
			}
			else if (attType == "double") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_DoubleAttType>()));
			}
			else if (attType == "string") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_StringAttType>()));
			}
			else if (attType == "bool") {
				mySchemaOut->appendAtt(make_pair(a->toString(), make_shared<MyDB_BoolAttType>()));
			}
			else {
				cout << "Wrong attribute type!" << endl;
			}
		}
	}

	vector<string> selectionPredicate;
	for (auto a : allDisjunctions) {
		selectionPredicate.push_back(a->toString());
	}

	string finalSelectionPredicate = "bool[true]";
	for (string s : selectionPredicate) {
		finalSelectionPredicate = "&& (" + finalSelectionPredicate + "," + s + ")";
	}
	
	MyDB_TablePtr myOutputTable = make_shared <MyDB_Table> ("result", "result.bin", mySchemaOut);
	MyDB_TableReaderWriterPtr outputTable = make_shared <MyDB_TableReaderWriter> (myOutputTable, bufferMgr);

	if (aggsToCompute.size() == 0) {
		RegularSelection operation(inputTable, outputTable, finalSelectionPredicate, projections);
		operation.run();
	}
	else {
		Aggregate operation(inputTable, outputTable, aggsToCompute, groupings, finalSelectionPredicate);
		operation.run();
	}
	
	int resNum = 0;
	cout << "First 30 records of the results:" << endl;
	
	MyDB_RecordPtr outputRec = outputTable->getEmptyRecord();
	MyDB_RecordIteratorAltPtr recIt = outputTable->getIteratorAlt();
	while (recIt->advance()) {
		recIt->getCurrent(outputRec);
		resNum++;
		if (resNum <= 30) {
			stringstream os;
			os << outputRec;
			cout << os.str() << endl;
		}
	}

	cout << "------------------------------------------------" << endl;
	printf("[ Total Results Number ] %d\n", resNum);

	// retrieve original schema
	for (auto a : tablesToProcess) {
		allTables[a.first]->getTable()->getSchema()->retrieveAttName();
	}
}

int main (int numArgs, char **args) {

	// make sure we have the correct arguments
	if (numArgs != 3) {
		cout << "args: catalog_file directory_for_tables\n";
		return 0;
	}

	// open up the catalog file
	MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> (args [1]);

	// start up the buffer manager
	MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (131072, 4028, "tempFile");

	// and create tables for everything in the database
	static map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);

	// this is all of the tables
	static map <string, MyDB_TableReaderWriterPtr> allTableReaderWriters;

	// and this is just the B+-Trees
	static map <string, MyDB_BPlusTreeReaderWriterPtr> allBPlusReaderWriters;

	// load 'em up
	for (auto &a : allTables) {
		if (a.second->getFileType () == "heap") {
			allTableReaderWriters[a.first] =  make_shared <MyDB_TableReaderWriter> (a.second, myMgr);
		} else if (a.second->getFileType () == "bplustree") {
			allBPlusReaderWriters[a.first] = make_shared <MyDB_BPlusTreeReaderWriter> (a.second->getSortAtt (), a.second, myMgr);
			allTableReaderWriters[a.first] = allBPlusReaderWriters[a.first];	
		}
	}

	// print out the intro notification
	cout << "\n          Welcome to MyDB v0.1\n\n";
	cout << "\"Not the worst database in the world\" (tm) \n\n";

	// and repeatedly accept queries
	while (true) {
		
		cout << "MyDB> ";

		// this will be used to collect the query
		stringstream ss;

		// get a line
		for (string line; getline (cin, line);) {
			
			// see if it has a ";" at the end
			size_t pos = line.find (';');

			// it does!!  so we are ready yo parse
			if (pos != string :: npos) {

				// append the last line
				line.resize (pos);
				ss << line;

				// see if someone wants to load a file
				vector <string> tokens {istream_iterator<string>{ss},
					istream_iterator<string>{}};

				// see if we got a "quit" or "exit"
				if (tokens.size () == 1 && (toLower (tokens[0]) == "exit" || toLower (tokens[0]) == "quit")) {
					cout << "OK, goodbye.\n";
					// before we get outta here, write everything into the catalog
					for (auto &a : allTables) {
						a.second->putInCatalog (myCatalog);
					}
					return 0;
				}

				// see if we got a "load soandso from afile"
				if (tokens.size () == 4 && toLower(tokens[0]) == "load" && toLower(tokens[2]) == "from") {

					// make sure the table is there
					if (allTableReaderWriters.count (tokens[1]) == 0) {
						cout << "Could not find table " << tokens[1] << ".\n";
						break;
					} else {
						cout << "OK, loading " << tokens[1] << " from text file.\n";

						// load up the file
						pair <vector <size_t>, size_t> res = allTableReaderWriters[tokens[1]]->loadFromTextFile (tokens[3]);

						// and record the tuple various counts
						allTableReaderWriters[tokens[1]]->getTable ()->setDistinctValues (res.first);
						allTableReaderWriters[tokens[1]]->getTable ()->setTupleCount (res.second);
						break;
					}
				}

				// get the string to parse
				string parseMe = ss.str ();

				// see if we got a load-from
				
				// add an extra zero at the end; needed by lexer
				parseMe.push_back ('\0');

				// now parse it
				yyscan_t scanner;
				LexerExtra extra { "" };
				yylex_init_extra (&extra, &scanner);
				const YY_BUFFER_STATE buffer { yy_scan_string (parseMe.data(), scanner) };
				SQLStatement *final = nullptr;
				const int parseFailed { yyparse (scanner, &final) };
				yy_delete_buffer (buffer, scanner);
				yylex_destroy (scanner);

				// if we did not parse correctly
				if (parseFailed) {

					// print out the parse error
					cout << "Parse error: " << extra.errorMessage;

					// get outta here
					if (final != nullptr)
						delete final;
					break;

				// if we did parse correctly, just print out the query
				} else {

					// see if we got a create table
					if (final->isCreateTable ()) {

						string tableName = final->addToCatalog (args[2], myCatalog);
						if (tableName != "nothing") {
							allTables = MyDB_Table :: getAllTables (myCatalog);
							if (allTables [tableName]->getFileType () == "heap") {
								allTableReaderWriters[tableName] = 
									make_shared <MyDB_TableReaderWriter> (allTables [tableName], myMgr);
							} else if (allTables [tableName]->getFileType () == "bplustree") {
								allBPlusReaderWriters[tableName] = 
									make_shared <MyDB_BPlusTreeReaderWriter> 
										(allTables [tableName]->getSortAtt (), allTables [tableName], myMgr);
								allTableReaderWriters[tableName] = allBPlusReaderWriters[tableName];
							}
							cout << "Added table " << final->addToCatalog (args[2], myCatalog) << "\n";
						}	

					} else if (final->isSFWQuery ()) {

						// table - attribute - type map
						static map<string, map<string, string>> allInfo;
						for (auto it = allTables.begin(); it != allTables.end(); it++) {
							string thisTable = it->first;
							map<string, string> tableInfo;
							vector<string> attrList;
							myCatalog->getStringList(thisTable + ".attList", attrList);
							for (string attr : attrList) {
								string type;
								myCatalog->getString(thisTable + "." + attr + ".type", type);
								tableInfo[attr] = type;
							}
							allInfo[thisTable] = tableInfo;
						}

						//execute the query
						cout << "-------------------- Result --------------------" << endl;
						clock_t startTime = clock();
						executeQuery(final, allInfo, allTableReaderWriters, myMgr);
						clock_t endTime = clock();
						double runtime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
						printf("[ Total Execution Time ] %.3f (seconds).\n", runtime);
						cout << "------------------------------------------------" << endl;
					}

					// get outta here
					if (final != nullptr)
						delete final;
					break;
				}

			} else {
				ss << line << "\n";
				cout << "    > ";
			}
		}
	}

}