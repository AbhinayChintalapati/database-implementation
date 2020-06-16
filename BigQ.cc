#include "BigQ.h"
#include<vector>
#include <algorithm>
#include <queue>
#include <time.h>


// Compfun :: Compfun(OrderMaker *sortorder){  // constructor for records comparing class
// 	SortOrder = sortorder;
// }
// Compfunk :: Compfunk(OrderMaker *sortorder){  // constructor for k way merge of runs
// 	SortOrder = sortorder;
// }
 void* worker(void *args) {               // worker thread

	  //BigQInfo *mybqi=new BigQInfo();
    BigQ *mybqi=(BigQ*)args;
    Pipe *In1=mybqi->in;
    Pipe *Out1=mybqi->out;
    OrderMaker* Sortorder=mybqi->order;
    int Runlen=mybqi->runLength;

	Record *ne = new Record();
	Record *dne = new Record();
	int length = Runlen;
	int numruns = 1;
	Page *dummy = new Page();

	vector<Record*> recordvec;
	vector<int> pageslen;
	pageslen.push_back(0);
	Compfun cf(Sortorder);
	char filename[50];
	// Calculate the time in nanoseconds - need for BigQ temp file
	// timespec time;
	// clock_gettime(CLOCK_REALTIME, &time);
	// int timestamp = time.tv_sec * 1000000000 + time.tv_nsec;
	// if (timestamp < 0) timestamp *= -1;
	// sprintf(filename, "Runs_%d.bin", timestamp);

	DBFile dbf ;
	dbf.Create("xyz",heap,NULL);
	while(In1->Remove(ne)){							// remove each record from p
	//	cout << Runlen <<" adding bq \n";
		if(length>0){											// create a run
			if(!dummy->Append(ne)){
				while(dummy->GetFirst(dne)){
					recordvec.push_back(dne);
					dne = new Record();
				}
				dummy = new Page();
				dummy->Append(ne);
				length--;
			}
		}
		if(length==0){ 										// sort the run and push to dbfile
					sort(recordvec.begin(),recordvec.end(),cf);			//sort the run
					for(int i=0;i<recordvec.size();i++)
						dbf.Add(recordvec[i]);
					pageslen.push_back(dbf.getlen ());
					numruns++;
					recordvec.clear();
					length = Runlen;
		}
	}

	while(dummy->GetFirst(dne)){				// push last run into dbfile
		recordvec.push_back(dne);
		dne = new Record();
	}
	sort(recordvec.begin(),recordvec.end(),cf); 			//sort the run
	for(int i=0;i<recordvec.size();i++)
		dbf.Add(recordvec[i]);
	pageslen.push_back(dbf.getlen());
	recordvec.clear();
//	cout << "length of file is : " <<dbf.GetLength() <<endl;
	dbf.Close();
	// typedef std::priority_queue<int,std::vector<int>,mycomparison> mypq_type;
			// 2nd part //

	File pq;
	pq.Open(1, filename);
	Page *run[ numruns];
	typedef priority_queue<pqr*,std::vector<pqr*>,Compfunk> mypq_type;
	mypq_type qu(Sortorder);
	//qu.Compfunin(Sortorder);
	pqr* structpq = new pqr();
	Record* recordpq = new Record();
//	cout << "length is " << pq.GetLength() << " numruns is " << numruns << endl;
	for(int j=0;j<numruns;j++){ 					//  get all runs initial pointers
		run[j] = new Page();
		pq.GetPage(run[j],pageslen[j]);
		if(run[j]->GetFirst(recordpq)){			// read first record for each run
			structpq->rec = recordpq;
			structpq->run = j;
			structpq->page = pageslen[j];
			qu.push(structpq);
			structpq = new pqr();
			recordpq = new Record();
			}
	}
	int count = 1;
	Record *last = NULL, *prev = NULL;
	while (!qu.empty())							// sort using priority queue
	{

			structpq = qu.top();
			Out1->Insert(structpq->rec); //output the min element
			qu.pop(); // remove it
			if(run[structpq->run]->GetFirst(recordpq)){
				structpq->rec = recordpq;
				qu.push(structpq);
				recordpq = new Record();
			}
			else if( (structpq->page+1 < pageslen[structpq->run+1] )&& structpq->page < pq.GetLength()-1 ){ //add next page in the run
				delete run[structpq->run];
				run[structpq->run] = new Page();
				structpq->page = structpq->page+1 ;
				pq.GetPage(run[structpq->run],structpq->page);
				if(run[structpq->run]->GetFirst(recordpq)){
					structpq->rec = recordpq;
					qu.push(structpq); // push into our priority queue
					recordpq = new Record();
				}
			}
	}

	pq.Close();
	char buffer [50];
  	sprintf (buffer, "%s.meta-data", filename);
	remove(filename);
	remove(buffer);


	Out1->ShutDown ();
}
//  void* BigQ :: helper(void *context)   //----https://stackoverflow.com/questions/1151582/pthread-function-from-a-class
// 	 {
// 			   ((BigQ *)context)->worker();   // static function to use thread in class
// 	 }


BigQ :: BigQ (Pipe &inp, Pipe &outp, OrderMaker &sortorder, int runlen) {

	//BigQInfo *bqi=new BigQInfo();
	in=&inp;
	out=&outp;
  order=&sortorder;
	runLength=runlen;

	// In1 = &in;
	// Out1 = &out;
	// Sortorder = &sortorder;
	// Runlen = runlen;

	// read data from in pipe sort them into runlen pages
	pthread_t thread1;
	pthread_create (&thread1, NULL, worker,(void*)this );   // create thread

//	pthread_join (thread1, NULL);   // wait until thread finishes -- removed in 2.2

}
BigQ::~BigQ () {
}
