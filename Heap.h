#ifndef HEAPDBFILE_H
#define HEAPDBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "GenericDBFile.h"

// stub DBFile header..replace it with your own DBFile.h

class Heap:public GenericDBFile {
private:
	File *f;
	  Page *p ;
		Page *n;
		int pnum;
		bool first;
		int pdirty;
		bool get;
		bool unfinishedpage;
public:
	Heap ();

	int Create ( char *fpath, fType file_type, void *startup);
	int Open (const char *fpath);
	int Close ();
	int getlen ();

	void Load (Schema &myschema, const char *loadpath);

	void MoveFirst ();
	void Add (Record *addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

};
#endif
