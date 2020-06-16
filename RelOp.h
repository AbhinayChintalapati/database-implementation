#ifndef _REOP_H
#define _REOP_H

#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"
#include "pthread.h"
#include <sstream>

using namespace std;

class RelationalOp {
    protected:
        pthread_t runt;  // worker thread
        int bnloop;
    public:
      	// blocks the caller until the particular relational operator has run completed
        virtual void WaitUntilDone () = 0 ;
        // tells how much internal memory the operation can use
        virtual void Use_n_Pages (int n) =0 ;
};

//SelectPipe class
class SelectPipe: public RelationalOp{
    public:
        void Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);
        void WaitUntilDone () ;
	      void Use_n_Pages (int n) ;
        // void* procPipe(void* obj);
};

//SelectFile class
class SelectFile: public RelationalOp{
    public:
        void Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
       virtual void WaitUntilDone () ;
	      void Use_n_Pages (int n) ;
};

//Project class
class Project : public RelationalOp {
  public:
    void Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
    virtual void WaitUntilDone () ;
	  void Use_n_Pages (int n) ;
};

//DuplicteRemoval class which is used to remove duplicates
class DuplicateRemoval : public RelationalOp {
  public:
    void Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
    virtual void WaitUntilDone () ;
	      void Use_n_Pages (int n) ;
};

//Sum class 
class Sum : public RelationalOp {
  public:
    void Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe);
    virtual void WaitUntilDone () ;
	  void Use_n_Pages (int n) ;
};

//GroupyBy class
class GroupBy : public RelationalOp {
  public:
    void Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
    virtual void WaitUntilDone () ;
	      void Use_n_Pages (int n) ;
};

//WriteOut class to write out the records
class WriteOut : public RelationalOp {
  public:
    void Run (Pipe &inPipe, FILE *outFile, Schema &mySchema);
    virtual void WaitUntilDone () ;
	      void Use_n_Pages (int n) ;
};

//Join class 
class Join : public RelationalOp {
    public:
        Join();
        ~Join();
	void Run (Pipe &inPipe1, Pipe &inPipe2, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);

    private:       
        Pipe *inputPipe1;
       
        Pipe *inputPipe2;
        Pipe *outputPipe;
        CNF *incnf;
        Record *inliteral;

        int runlen;
	pthread_t runt; // worker thread

        static void* procjoin(void *op);
        void recjoin();

        void sortjoin(OrderMaker *left, OrderMaker *right);
        bool search(Pipe &pipe, Record *rec, vector<Record*> &vec, OrderMaker *sortOrder);
        void mergerecs(Record *l, Record *r);
        void waitedjoin();

};


#endif
