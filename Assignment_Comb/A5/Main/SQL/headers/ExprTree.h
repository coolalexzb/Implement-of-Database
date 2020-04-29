
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include <string.h>
#include <vector>
#include <map>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:

	virtual string toString () = 0;
	virtual bool exprAttrCheck (map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) = 0;
	virtual string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) = 0;
	virtual void addGroupByAttrs (vector <pair <string, string>> &groupByAttrs) = 0;
	virtual bool exprAggrCheck (vector <pair <string, string>> groupByAttrs) = 0;
	virtual bool exprOpCheck (map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) = 0;
	virtual ~ExprTree () {}
};

class BoolLiteral : public ExprTree {

private:

	bool myVal;

public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}

	bool exprAttrCheck (map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs (vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck (vector <pair <string, string>> groupByAttrs) {
		return true;
	}

	bool exprOpCheck (map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	~BoolLiteral() {}
};

class DoubleLiteral : public ExprTree {

private:

	double myVal;

public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return true;
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	~DoubleLiteral () {}
};

class IntLiteral : public ExprTree {

private:

	int myVal;

public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return true;
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:

	string myVal;

public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "string";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return true;
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:

	string tableName;
	string attName;

public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (tableDef.find(tableName) == tableDef.end()) {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "------ [ Symbol NOT Defined ] -------" << endl;
			cout << "[ " + where + " ] Symbol \"" + tableName + "\" NOT defined as a Table." << endl;
			return false;
		}

		string completeTableName = tableDef[tableName];
		map<string, string> curTable = allInfo[completeTableName];
		if (curTable.find(attName) == curTable.end()) {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "----- [ Attribute NOT Existed ] -----" << endl;
			cout << "[ " + where + " ] Attribute \"" + attName + "\" NOT existed in the Table \"" + completeTableName + "\"." << endl;
			return false;
		}
		return true;
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		string completeTableName = tableDef[tableName];
		map<string, string> curTable = allInfo[completeTableName];
		if (curTable[attName] == "int" || curTable[attName] == "double")
			return "int/double";
		else
			return curTable[attName];
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		groupByAttrs.push_back(make_pair(tableName, attName));
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		for (pair<string, string> attr : groupByAttrs) {
			if (attr.first == tableName && attr.second == attName) return true;
		}

		cout << "------------- [ ERROR ] -------------" << endl;
		cout << "----- [ Aggregation Violation ] -----" << endl;
		cout << "[ SELECT ] Attribute \"" + tableName + "." + attName + "\" NOT existed in the GROUP_BY clause." << endl;
		return false;
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return true;
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) != "int/double" || rhs->getType(tableDef, allInfo) != "int/double") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] MINUS can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \"" 
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		if (lhs->getType(tableDef, allInfo) == "string" || rhs->getType(tableDef, allInfo) == "string")
			return "string";
		else
			return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) == "bool" || rhs->getType(tableDef, allInfo) == "bool") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] PLUS can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) != "int/double" || rhs->getType(tableDef, allInfo) != "int/double") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] TIMES can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) != "int/double" || rhs->getType(tableDef, allInfo) != "int/double") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] DIVIDE can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) == "bool" || rhs->getType(tableDef, allInfo) == "bool"
			|| lhs->getType(tableDef, allInfo) != rhs->getType(tableDef, allInfo)) {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] GT Comparison can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) == "bool" || rhs->getType(tableDef, allInfo) == "bool"
			|| lhs->getType(tableDef, allInfo) != rhs->getType(tableDef, allInfo)) {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] LT Comparison can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) == "bool" || rhs->getType(tableDef, allInfo) == "bool"
			|| lhs->getType(tableDef, allInfo) != rhs->getType(tableDef, allInfo)) {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] NE Comparison can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) != "bool" || rhs->getType(tableDef, allInfo) != "bool") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] OR can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return (lhs->exprAttrCheck(tableDef, allInfo, where) && rhs->exprAttrCheck(tableDef, allInfo, where));
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return lhs->exprAggrCheck(groupByAttrs) && rhs->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!lhs->exprOpCheck(tableDef, allInfo, where) || !rhs->exprOpCheck(tableDef, allInfo, where)) return false;

		if (lhs->getType(tableDef, allInfo) == "bool" || rhs->getType(tableDef, allInfo) == "bool"
			|| lhs->getType(tableDef, allInfo) != rhs->getType(tableDef, allInfo)) {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] EQ Comparison can NOT be done over \"" + lhs->getType(tableDef, allInfo) + "\" and \""
				<< rhs->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return child->exprAttrCheck(tableDef, allInfo, where);
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return child->exprAggrCheck(groupByAttrs);
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!child->exprOpCheck(tableDef, allInfo, where)) return false;

		if (child->getType(tableDef, allInfo) != "bool") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] NOT can NOT be done over \"" + child->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return child->exprAttrCheck(tableDef, allInfo, where);
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return true;
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!child->exprOpCheck(tableDef, allInfo, where)) return false;

		if (child->getType(tableDef, allInfo) != "int/double") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] SUM can NOT be done over \"" + child->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}

	bool exprAttrCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		return child->exprAttrCheck(tableDef, allInfo, where);
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int/double";
	}

	void addGroupByAttrs(vector <pair <string, string>> &groupByAttrs) {
		return;
	}

	bool exprAggrCheck(vector <pair <string, string>> groupByAttrs) {
		return true;
	}

	bool exprOpCheck(map <string, string> tableDef, map <string, map <string, string>> allInfo, string where) {
		if (!child->exprOpCheck(tableDef, allInfo, where)) return false;

		if (child->getType(tableDef, allInfo) != "int/double") {
			cout << "------------- [ ERROR ] -------------" << endl;
			cout << "-------- [ Option Violation ] -------" << endl;
			cout << "[ " + where + " ] AVG can NOT be done over \"" + child->getType(tableDef, allInfo) + "\"." << endl;
			return false;
		}
		return true;
	}

	~AvgOp () {}
};

#endif