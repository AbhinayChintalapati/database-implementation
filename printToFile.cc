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

	Record rec;
	DBFile Heapfile;
	Heapfile.Open("customer.bin");
	Schema mySchema("catalog", "customer");
	while(Heapfile.GetNext(rec)) {
		rec.Print(&mySchema);
		cout << endl;
	}
	Heapfile.Close();

	return 0;
}

void normalTest() {
	cout << "Enter in your CNF: ";
	if (yyparse() != 0) {
		cout << "Can't parse your CNF.\n";
		exit(1);
	}

	Schema testSchema("catalog", "customer");
	CNF myCnf;
	Record literal;
	myCnf.GrowFromParseTree(final, &testSchema, literal);
	OrderMaker tempom;
	OrderMaker so;
	myCnf.GetSortOrders(so, tempom);

	//createFile();
	std::cout << "start" << endl;
	Pipe myPipe(200000);
	Record rec;

	FILE *FilePtr = fopen("customer.tbl", "r");

	while (rec.SuckNextRecord(&testSchema, FilePtr) == 1) {
		myPipe.Insert(&rec);

	}
	fclose(FilePtr);

	myPipe.ShutDown();
	cout << "after shutdown" << endl;

	BigQ bq(myPipe, myPipe, so, 3);
}

void createFile() {
	DBFile dbf;
	dbf.Create("input.bin", heap, NULL);
	Schema testSchema("catalog", "lineitem");
	Record rec;
	FILE *FilePtr = fopen("lineitem.tbl", "r");
	while (rec.SuckNextRecord(&testSchema, FilePtr) == 1) {
		dbf.Add(rec);
	}
	fclose(FilePtr);
	dbf.Close();
}
