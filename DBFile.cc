
#include "DBFile.h"

#include <iostream>
using namespace std;

/*constructor*/
DBFile::DBFile ()
{
    Runlen = 0;
    myorder = NULL;
    DBR = NULL;
}
/*Create a normal File*/
int DBFile::Create ( char *f_path, fType f_type, void *startup) {
  char buffer [50];
  sprintf (buffer, "%s.meta-data", f_path);
  ofstream myfile (buffer);
  if(f_type==heap){
    if (myfile.is_open()){
      myfile << "heap\n";
      myfile.close();
    }
    else
      cout << "Unable to create file\n";
    DBR = new Heap();
    return  DBR->Create(f_path,f_type,startup);
  }
  else if(f_type==sorted){
    if (myfile.is_open()){
      myfile << "sorted\n";
      myfile << ((SortInfo*)startup)->runLength << endl;
      //((SortInfo*) startup)->myOrder->Print();
      ((SortInfo*) startup)->myOrder->Printfile(myfile);
      myfile.close();
    }
    else
      cout << "Unable to create file\n";
    DBR = new Sortfile(((SortInfo*) startup)->myOrder,((SortInfo*)startup)->runLength);
    return  DBR->Create(f_path,f_type,startup);
  }
  else
    cout << "unknow type\n";
    return 0;
}
/*Load the DBFile instance from a text file*/
void DBFile::Load (Schema &f_schema, const char *loadpath) {
    DBR->Load(f_schema,loadpath);

}
/*Assumes that the DBFile already exists and has previously been created and then closed*/
int DBFile::Open (const char *f_path) {
  char buffer [50];
  sprintf (buffer, "%s.meta-data", f_path);
  ifstream myfile (buffer);
  string line;
  if (myfile.is_open())
  {
      getline (myfile,line);
      if(line=="heap"){
        DBR = new Heap();
        return  DBR->Open(f_path);
      }
      else if(line == "sorted"){
        getline (myfile,line);
        Runlen = stoi(line);
        myorder = new OrderMaker(myfile);
        DBR = new Sortfile(myorder,Runlen);
      //  myorder->Print();
        return DBR->Open(f_path);
      }
      else{
        cout << "Meta Data crashed please create file again\n";
        return 0;
      }

    myfile.close();
  }
  else
  {
    cout << "No file to open\n";
    return 0;
  }
}
/*Movefirst function forces the pointer to correspond to the first record in the file*/
void DBFile::MoveFirst () {
    DBR->MoveFirst();
}
//Closes the file
int DBFile::getlen () {
  return  DBR->getlen();
}
int DBFile::Close () {
  return DBR->Close();  /*The return value is a 1 on success if the file can be closed*/
}
/*Add new record to the end of the file*/
void DBFile::Add (Record *rec)
{
    DBR->Add(rec);
}
/*Gets the next record from the  file and returns it to the user*/
int DBFile::GetNext (Record &fetchme) {
  return  DBR->GetNext(fetchme);
}

/*returns the next record in the file that is accepted by the  selection predicate.*/
int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
  return   DBR->GetNext(fetchme,cnf,literal);
}
