#ifndef SORTFILE_H
#define SORTFILE_H

#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "GenericDBFile.h"
#include "BigQ.h"
#include "TwoWayList.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
// #include "Pipe.h"

class BigQ;
class Sortfile : public GenericDBFile{

private:
    File* f;
    OrderMaker* cnfsortorder;
    OrderMaker* myorder;
    const char* name;
    int pnum;
    Page* p;
    Page* n;
    Pipe* in;
    Pipe* out;
    bool pdirty;
    bool first;
    bool get;
    int runlength;
    BigQ *bigq;

    int bSearch(Record& fetchme, OrderMaker& cnfOrder, Record& literal, OrderMaker& sortOrder, OrderMaker& queryOrder);


public:
  Sortfile(OrderMaker *sord,int rlength);
	~Sortfile();
	int Create( char *fpath,fType file_type,void *startup);
	int Open(const char *fpath);
	int Close();
  int getlen();
	void Load(Schema &myschema, const char *loadpath);
  void Merge();
	void MoveFirst();
	void Add(Record *addme);
	int GetNext(Record &fetchme);
	int GetNext(Record &fetchme, CNF &cnf, Record &literal);

};

#endif
