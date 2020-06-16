#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Heap.h"
#include "Defs.h"
#include <iostream>
using namespace std;

/*constructor*/ Heap:: Heap()
{
    f = new File();  // a new file instance
    p = new Page() ; // page pointer for write operations
    n = new Page() ;  // Page pointer for read operations
    pnum = 1;  // pnum of pages read so far
    first = true; // to check if its a first page
    pdirty = false; // to verify buffer data
    get = true; //used to initialize if you are trying to use GetNext for first time
}
/*Create a normal File*/
int Heap::Create ( char *f_path, fType f_type, void *startup) {
        f->Open(0,f_path);
        f->AddPage(p,0);
        return 1;//return value is 1 on success

}

/*Assumes that the DBFile already exists and has previously been created and then closed*/
int Heap::Open (const char *f_path) {
        f->Open(1, const_cast<char *>(f_path));
        return 1;
}


/*Load the DBFile instance from a text file*/
void Heap::Load (Schema &f_schema, const char *loadpath) {
                FILE *fp = fopen(loadpath,"r") ;
                Record *r = new Record();
				/*append new data using the SuckNextRecord function from Record.h*/
                while(r->SuckNextRecord (&f_schema,fp)){
                  Add(r);
                  r = new Record();
                }

                if(first)
                  f->AddPage(p,f->GetLength()-2);
                else
                  f->AddPage(p,f->GetLength()-1);
                pdirty=false;


}

/*Movefirst function forces the pointer to correspond to the first record in the file*/
void Heap::MoveFirst () {
        f->GetPage(n,0);
        pnum = 1;
}

//Closes the file

int Heap::Close () {
  if(pdirty==true){
    if(first)
      f->AddPage(p,f->GetLength()-2);
    else
      f->AddPage(p,f->GetLength()-1);
  }
  f->Close();
  return 1;  /*The return value is a 1 on success if the file can be closed*/
}

/*Add new record to the end of the file*/
void Heap::Add (Record *rec)
{
        if(!p->Append(rec)){//if we cannot add record to the existing page because it is full
          if(first){
            f->AddPage(p,f->GetLength()-2);
            delete p;
            p = new Page();
            p->Append(rec);
            first = false;
          }
          else{
            f->AddPage(p,f->GetLength()-1);
            delete p;
            p = new Page();
            p->Append(rec);
          }
        }
        pdirty =true;
}
//returns length
int Heap::getlen () {
  if(pdirty==true){
    if(first)
      f->AddPage(p,f->GetLength()-2);
    else
      f->AddPage(p,f->GetLength()-1);
    delete p;
    p = new Page();
    first = false;
    pdirty =false;
  }
  return f->GetLength()-1;
}


/*Gets the next record from the  file and returns it to the user*/
int Heap::GetNext (Record &fetchme) {
      if(pdirty==true){
        if(first)
          f->AddPage(p,f->GetLength()-2);
        else
          f->AddPage(p,f->GetLength()-1);
        delete p;
        p = new Page();
        first = false;
        pdirty =false;
      }
      if(get){
        MoveFirst();
        get=false;
      }
        int x =  n->GetFirst(&fetchme);
        if(!x && f->GetLength()-1>pnum){
          delete n;
          n = new Page();
          f->GetPage(n,pnum);
          x =  n->GetFirst(&fetchme);
          pnum++;
		  /*the pointer into the file is incremented, so a subsequent call to GetNext won�t return the same record twice.*/
        }
        return x;
		/*return  zero if there is not a valid record if the last record in the file has already been returned*/
}

/*returns the next record in the file that is accefed by the  selection predicate.*/
int Heap::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
 ComparisonEngine comp;
  while(GetNext(fetchme)){
    if(comp.Compare(&fetchme,&literal,&cnf))
      return 1;
  }
  return 0;
  /*return  zero if there is not a valid record if the last record in the file has already been returned*/
}
