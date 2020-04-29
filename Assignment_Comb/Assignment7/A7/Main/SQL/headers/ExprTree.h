
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include <string>
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
	virtual string childToString () = 0;
	virtual int aggType () = 0;
	virtual string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) = 0;
	virtual ~ExprTree () {}
};

class BoolLiteral : public ExprTree {

private:

	bool myVal;

public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string childToString () {
		return "";
	}
	
	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
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

	string childToString () {
		return "";
	}
	
	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "double";
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}	

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:

	int myVal;

public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "int";
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "string";
	}

	string toString () {
		return "string[" + myVal + "]";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		string completeTableName = tableDef[tableName];
		map<string, string> curTable = allInfo[completeTableName];
		return curTable[attName];
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		if (lhs->getType(tableDef, allInfo) == "double" || rhs->getType(tableDef, allInfo) == "double")
			return "double";
		else
			return "int";
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		if (lhs->getType(tableDef, allInfo) == "string" && rhs->getType(tableDef, allInfo) == "string")
			return "string";
		
		if (lhs->getType(tableDef, allInfo) == "double" || rhs->getType(tableDef, allInfo) == "double")
			return "double";
		else
			return "int";
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType(map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		if (lhs->getType(tableDef, allInfo) == "double" || rhs->getType(tableDef, allInfo) == "double")
			return "double";
		else
			return "int";
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "double";
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}
	
	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
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

	string childToString () {
		return "";
	}

	int aggType () {
		return -1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "bool";
	}

	string toString () {
		return "!(" + child->toString () + ")";
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

	string childToString () {
		return child->toString();
	}

	int aggType () {
		return 1;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return child->getType(tableDef, allInfo);
	}

	string toString () {
		return "sum(" + child->toString () + ")";
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

	string childToString () {
		return child->toString();
	}

	int aggType () {
		return 2;
	}

	string getType (map <string, string> tableDef, map <string, map <string, string>> allInfo) {
		return "double";
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	~AvgOp () {}
};

#endif
