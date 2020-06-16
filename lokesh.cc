#include <iostream>
#include "Record.h"
#include "DBFile.h"
#include "Pipe.h"
#include "BigQ.h"
//#include "test.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>"
using namespace std;

extern "C" {
	int yyparse(void);   // defined in y.tab.c
}

extern struct AndList *final;

int main() {

	cout << "Enter in your CNF: ";
	if (yyparse() != 0) {
		cout << "Can't parse your CNF.\n";
		exit(1);
	}

	Schema testSchema("catalog", "customer");
	CNF myComparison;
	myComparison.Print(); cout << endl;
	Record literal;
	myComparison.GrowFromParseTree(final, &testSchema, literal);
	myComparison.Print(); cout << endl;
	OrderMaker dummy;
	OrderMaker sortorder;
	myComparison.GetSortOrders(sortorder, dummy);
	sortorder.Print();
	cout << endl;

	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	cout << ms << endl;
	string s = to_string(ms);


	SortInfo start;
	int runlength = 5;
	start.myOrder = &sortorder;
	start.runLength = runlength;

	DBFile sorted;
	sorted.Create("trysorted", fType::sorted, &start);
	Schema mySchema("catalog", "customer");
	sorted.Load(mySchema, "customer.tbl");
	sorted.Close();

/*	char test[100];
	strcpy(test, "testme");
	DBFile db;
	db.Create(test, heap, NULL);
	db.Close();
	char test_merge[100];
	strcpy(test_merge, "testme-merge");
	DBFile db1;
	db1.Create(test_merge, heap, NULL);
	db1.Close();

	remove(test);
	rename(test_merge, test);
*/

	//To print the contents of  the bigQ file created when we use option 3

	
	/*	
DBFile db1;
SortInfo start;
OrderMaker *order = new OrderMaker;
int runlength = 5;

start.myOrder = &sortorder;
start.runLength = runlength;

db1.Create("trial", sorted, &start);
db1.Close();

DBFile db2;
int r = db2.Open("trial");
db2.Close();

db1.Open("partsupp.bin.bigq");
Schema testSchema("catalog", "partsupp");
Record r;
while (db1.GetNext(r)) {
	r.Print(&testSchema);
	cout << endl;
}
*/
	return 0;
}

void normalTest() {
	cout << "Enter in your CNF: ";
	if (yyparse() != 0) {
		cout << "Can't parse your CNF.\n";
		exit(1);
	}

	Schema testSchema("catalog", "customer");
	CNF myComparison;
	Record literal;
	myComparison.GrowFromParseTree(final, &testSchema, literal);
	OrderMaker dummy;
	OrderMaker sortorder;
	myComparison.GetSortOrders(sortorder, dummy);

	//createFile();
	std::cout << "start" << endl;
	Pipe myPipe(200000);
	Record tr;

	FILE *tableFile = fopen("customer.tbl", "r");

	while (tr.SuckNextRecord(&testSchema, tableFile) == 1) {
		myPipe.Insert(&tr);
		//dbf.Add(tr);
	}
	fclose(tableFile);

	myPipe.ShutDown();
	cout << "after shutdown" << endl;
	//OrderMaker sortorder(&testSchema);
	//relation *o = new relation("orders", new Schema("catalog", "orders"), "");
	//o->get_sort_order(sortorder);
	BigQ bq(myPipe, myPipe, sortorder, 3);
}

void createFile() {
	DBFile dbf;
	dbf.Create("input.bin", heap, NULL);
	Schema testSchema("catalog", "lineitem");
	Record tr;
	FILE *tableFile = fopen("lineitem.tbl", "r");
	while (tr.SuckNextRecord(&testSchema, tableFile) == 1) {
		dbf.Add(tr);
	}
	fclose(tableFile);
	dbf.Close();
}