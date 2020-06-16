#ifndef DBFILE_H
#define DBFILE_H


#include "File.h"
#include "GenericDBFile.h"
#include "Heap.h"
#include "Sort.h"
#include <fstream>
#include <string>




// stub DBFile header..replace it with your own DBFile.h

class DBFile {
private:
	GenericDBFile *DBR;
	struct SortInfo {
		OrderMaker *myOrder;
		 int runLength;
	 };
	 int Runlen;
	 OrderMaker *myorder;
public:
	DBFile ();

	int Create ( char *fpath, fType file_type, void *startup);
	int Open (const char *fpath);
	int Close ();
	int getlen();

	void Load (Schema &myschema, const char *loadpath);

	void MoveFirst ();
	void Add (Record *addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

};
#endif
