#ifndef GENERICDBFILE_H
#define GENERICDBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

typedef enum {heap, sorted, tree} fType;

// stub DBFile header..replace it with your own DBFile.h

class GenericDBFile {

public:
	struct SortInfo {
		OrderMaker *myOrder;
		 int runLength;
	 };

	virtual int Create ( char *fpath, fType file_type, void *startup)=0;
	virtual int Open (const char *fpath)=0;
	virtual int Close ()=0;
	virtual int getlen ()=0;

	virtual void Load (Schema &myschema, const char *loadpath)=0;

	virtual void MoveFirst ()=0;
	virtual void Add (Record *addme)=0;
	virtual int GetNext (Record &fetchme)=0;
	virtual int GetNext (Record &fetchme, CNF &cnf, Record &literal)=0;

};
#endif
