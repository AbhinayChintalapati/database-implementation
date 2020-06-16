#ifndef BIGQ_H
#define BIGQ_H
#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include<vector>
#include "DBFile.h"


using namespace std;

struct pqr
{
	Record *rec;
	int run;
	int page;			// Current page in a sorted run
};



class BigQ {
	// Pipe *In1;
	// Pipe *Out1;
	// OrderMaker *Sortorder;
	// int Runlen;

	public:
		Pipe *in;
		Pipe *out;
		OrderMaker *order;
		int runLength;
		File myFile;

		BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

	//void worker();
	//static void *helper(void *context);
		~BigQ ();
};

class BigQInfo{
public:
    Pipe *in;
    Pipe *out;
    OrderMaker *order;
    int runLength;
    File myFile;
};


class Compfunk{
	OrderMaker *SortOrder;

	public:
		Compfunk(OrderMaker *sortorder){SortOrder = sortorder;};
	//	void Compfunin(OrderMaker *sortorder);
		bool operator() (pqr* r1,pqr* r2){
				ComparisonEngine ce;
				//cout << " comparing k  " << r1->rec << " with " << r2->rec <<endl;
				// if(ce.Compare(r1->rec,r2->rec,SortOrder)==0){
				// 	cout << "they are equal " << endl;
				// }
				if(ce.Compare(r1->rec,r2->rec,SortOrder)>0)
					return true;

				return false;
			}
};

class Compfun{
	OrderMaker *SortOrder;

	public:
		Compfun(OrderMaker *sortorder){SortOrder = sortorder;};
	//	void Compfunin(OrderMaker *sortorder);
		bool operator() (Record* r1,Record* r2){
				ComparisonEngine ce;
			//	cout << " comparing " << r1 << " with " << r2 <<endl;

				if(ce.Compare(r1,r2,SortOrder)<0){
				//	cout << "they are equal " << endl;
					return true;
				}


				return false;
			}
};

#endif
