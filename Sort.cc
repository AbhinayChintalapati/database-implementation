#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <queue>
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Defs.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
//#include "GenericDBFile.h"
#include "DBFile.h"
#include "Sort.h"

using namespace std;


Sortfile::Sortfile(OrderMaker *sord,int rlength){
    f = new File(); //file instance
    p = new Page(); // page poiner
    n = new Page(); // another page pointer
    pnum = 1;      // number of pages accesed so far
    first = true;   // first page indicator
    pdirty = false;  // verify the status of the page
    get = true;       //used in getnext

    name = NULL;
    runlength = rlength;
    myorder = sord;
    bigq = NULL;    // BIGQ class instance
    in = NULL;      // input pipe
    out = NULL;     // output pipe
    cnfsortorder = NULL;
}


Sortfile::~Sortfile ()
{
    pnum = 1;  // count of pages read so far
    first = true; // to check if its a first page
    pdirty = false; // to verify buffer data
    get = true; //used to initialize if you are trying to use GetNext for first time
    delete bigq;
    delete in;
    delete out;
    name = NULL;
    bigq = NULL;
    in = NULL;
    out = NULL;

  //  cout << "Run length is " << Runlen <<endl;
}

//Create a sorted file
int Sortfile :: Create( char *f_path,fType file_type,void * startup){
  f = new File();
  f->Open(0,f_path);
  f->AddPage(p,0);
  return 1;
  name = (char*)f_path;
  //in = new Pipe(1000);
  //out = new Pipe(1000);
  bigq = NULL;
  p = new Page();
  n = new Page();
  pnum = 1;
  pdirty = false;
  first = true;
  get = true;
  // return 1;
}
//tries to open an already created file
int Sortfile :: Open(const char* f_path){
  // f = new File();
  // f->Open(1,(char*)f_path);
  // name = (char*)f_path;
  // //in = new Pipe(1000);
  // //out = new Pipe(1000);
  bigq = NULL;
  p = new Page();
  //pnum = 1;
  pdirty = false;
  // return 1;
    name = f_path;
    f->Open(1,const_cast<char *>(f_path));
    return 1;
 }

int Sortfile :: getlen(){
  return 1;
}

//Loads the dbfile from a text file
void Sortfile :: Load(Schema &f_schema, const char* loadpath){
  FILE* f1 = fopen(loadpath,"r");
  Record * r = new Record();
  //uses SuckNextRecord to append new records
  while(r->SuckNextRecord(&f_schema,f1)){
    Add(r);
    r = new Record();
  }
}

// void Sortfile::Open ( const char *f_path){
//   name = f_path;
//   f->Open(1,const_cast<char *>(f_path));
//   return 1;
// }

// changes the pointer to point to the first record
void Sortfile :: MoveFirst(){
  f->GetPage(n,0);
  pnum = 1 ;
}

// as it states , it closes the file
int Sortfile:: Close (){
  if(pdirty == true){
    pdirty = false;
    Merge();
  }
  f->Close();
  delete f;
  f = NULL;
  return 1;
}

//Adds new record at the end
void Sortfile :: Add(Record *rec){
  // if(bigq == NULL){
  //   bigq = new BigQ(*in, *out, * sortorder, runlength);
  // }
  // in->Insert(rec);
  // pdirty = true;
  if(bigq==NULL){
    cout << "ADD" << endl;
    in = new Pipe(1000);
    out = new Pipe(1000);
    bigq = new BigQ(*in,*out,*myorder,runlength);
  }
  in->Insert(rec);
  pdirty =true;
}

// Merges the data from outpipe and data currently in file
void Sortfile::Merge(){
  in->ShutDown();
  DBFile db;
  db.Create("temp.bin",heap,NULL);
  Record* recp = new Record();
  Record* recf = new Record();
  ComparisonEngine ce;
  Page* pg;

  MoveFirst();
  int a = n->GetFirst(recf);
  int b = out->Remove(recp);

  while((a==1)||(b==1)){
    //int comp = ce.Compare(recf,recp,myorder);
    if((!a)||(b&&ce.Compare(recf,recp,myorder)>0)){
      db.Add(recp);
      b = out -> Remove(recp);
    }
    else
    if((!b)||(a && ce.Compare(recf,recp,myorder) <= 0)){
      db.Add(recf);
      a = n->GetFirst(recf);
      if(!a && (f->GetLength()-1>pnum)){
        delete n;
        n = new Page();
        f->GetPage(n,pnum);
        a = n->GetFirst(recf);
        pnum = pnum + 1;
      }
    }

  }

  db.Close();
  f->Close();
  delete bigq;
  delete in;
  delete out;
  in = NULL;
  out = NULL;
  bigq = NULL;
  rename("temp.bin",name);

  Open(name);
}

//gives the next record from the file
int Sortfile :: GetNext(Record& fetchme){
  if(pdirty == true){
    pdirty = false;
    Merge();
  }
  if(get){
    MoveFirst();
    get = false;
  }
  int a = n->GetFirst(&fetchme);
  if(!a){
    if(f->GetLength()-1 > pnum){
      delete n;
      n = new Page();
      f->GetPage(n,pnum);
      a =  n->GetFirst(&fetchme);
      pnum++;
    }
  }
  return a;
}
//
int Sortfile :: bSearch(Record& fetchme, OrderMaker& cnforder, Record& literal, OrderMaker& Sortord, OrderMaker& qorder){

  if(GetNext(fetchme)==0)
    return 0;

  ComparisonEngine ce;
  int ret = ce.Compare(&fetchme, &Sortord, &literal, &cnforder);
  if(ret > 0){
    return 0;
  }
  else
  if(ret == 0){
    return 1;
  }

  int low = pnum;
  int high = f->GetLength()-1;
  int mid = (low+high) / 2;

  while(low < mid){
    mid = (low + high) / 2;
    f->GetPage(n,mid);
    GetNext(fetchme);

    int temp = ce.Compare(&fetchme,&qorder,&literal,&cnforder);
    if(temp < 0){
      low = mid+1;
    }
    else if(temp > 0){
      high = mid - 1;
    }
    else {
      high = mid;
    }

  }
  int cnum;
  if((high < low)&&high < mid){
    cnum = high;
  }
  else if (low < mid){
    cnum = low;
  }
  else {
    cnum = mid;
  }

  pnum = cnum;

  f->GetPage(n,cnum);
  int result;

  do{
    if(!GetNext(fetchme))
      return 0;

    result = ce.Compare(&fetchme,&qorder,&literal, &cnforder);
  }while (result < 0);

  return result == 0;

}

//gives the next record from the file according to the given cnf
int Sortfile :: GetNext(Record & fetchme, CNF &cnf,Record& literal){
  OrderMaker ord1;
  OrderMaker ord2;
  ComparisonEngine ce;
  OrderMaker::qoMaker(myorder, cnf, ord1,ord2);

  cnfsortorder = cnf.CnfqoMaker(*myorder);
  //checking for the record using binary search
  // if(!bSearch(fetchme,ord2,literal,*myorder,ord1)){
  //
  //   return 0;
  // }

  while (GetNext(fetchme)){
    if(ce.Compare(&fetchme,&literal,&cnf))
      return 1;
    }
  return 0;
}
