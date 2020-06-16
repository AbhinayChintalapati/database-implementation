#include "RelOp.h"
#include "BigQ.h"
#include <stdio.h>
#include <vector>
#include <cmath>
#include "pthread.h"



// void RelationalOp::WaitUntilDone(){
//     pthread_join(runt, NULL);
// }

// void RelationalOp::Use_n_Pages(int n){
//     bnloop = n;
//     //to do
// }

typedef struct{

    Pipe* inp;
    Pipe* outp;
    Record* rec;
    CNF* cnf;

}pipeSel; // Structure used in SelectPipe

void SelectPipe::WaitUntilDone () { //wait until done
	 pthread_join (runt, NULL);
     //return;
}

void SelectPipe::Use_n_Pages (int runlen) {//set number of pages
	this->bnloop= runlen;
    return;
}

void* procPipe(void* obj){
    Record r;
    pipeSel* p = (pipeSel*) obj;
    ComparisonEngine ce;
    Pipe* pipe1 = p->inp;
    Pipe* pipe2 = p->outp;
    while(pipe1->Remove(&r)){
        if(ce.Compare(&r, p->rec, p->cnf) == 0){     //Does ordermaker cast CNF to ordermaker?? Also,
            pipe2->Insert(&r);
        }
    }

    pipe2->ShutDown();
}

void SelectPipe::Run(Pipe & inPipe, Pipe & outPipe, CNF & selOp, Record & literal){
    pipeSel* p = new pipeSel;
    p->inp = &inPipe;
    p->outp = &outPipe;
    p->cnf = &selOp;
    p->rec = &literal;
    pthread_create(&runt, NULL, procPipe,(void*) p); // Starts the thread
}

void SelectFile::WaitUntilDone () { //wait until done
	 pthread_join (runt, NULL);
     //return;
}

void SelectFile::Use_n_Pages (int runlen) {//set number of pages
	this->bnloop= runlen;
    return;
}

typedef struct{
    DBFile* db;
    Pipe* outp;
    CNF* cnf;
    Record* rec;
}fileSel; // Structure used in SelectFile

void* procFile(void *obj){
    Record r;
    fileSel* f = (fileSel*) obj;
    ComparisonEngine ce;
    DBFile* dbf = f->db;
    dbf->MoveFirst();
    while(dbf->GetNext(r, *(f->cnf), *(f->rec))){
        f->outp->Insert(&r);
    }
    f->outp->ShutDown();
}

void SelectFile::Run(DBFile & inFile, Pipe & outPipe, CNF & selOp, Record & literal){
    fileSel* f = new fileSel;
    f->db = &inFile;
    f->outp = &outPipe;
    f->cnf = &selOp;
    f->rec = &literal;
    pthread_create (&runt, NULL, procFile, (void*) f); // starts the thread process
}

void Project::WaitUntilDone () { //wait until done
	 pthread_join (runt, NULL);
     //return;
}

void Project::Use_n_Pages (int runlen) {//set number of pages
	this->bnloop= runlen;
    return;
}

typedef struct{
    Pipe* inp;
    Pipe* outp;
    int* selAtt;
    int numAttIn;
    int numAttOut;
}projObj; // structure used in Run

void* procProj(void *obj){
    projObj* po = (projObj*) obj;
    Record r;
    ComparisonEngine ce;
    Pipe* p = po->inp;
    while((p->Remove(&r)) != 0){
        r.Project(po->selAtt, po->numAttOut, po->numAttIn);
        po->outp->Insert(&r);
    }
    po->outp->ShutDown();
} // function used by the thread invoked by run

void Project::Run(Pipe & inPipe, Pipe & outPipe, int * keepMe, int numAttsInput, int numAttsOutput){
    projObj* po = new projObj;
    po->inp = &inPipe;
    po->outp = &outPipe;
    po->selAtt = keepMe;
    po->numAttIn = numAttsInput;
    po->numAttOut = numAttsOutput;
    pthread_create(&runt, NULL, procProj, (void*) po); // creates and starts the thread process
}



typedef struct{
  Pipe* inputPipe;
  Pipe* outputPipe;
  Schema* mySchema;
  int runlen;
} CreateBigQUtil; // struct used by createBigQThread in DuplicateRemoval

void* createBigQRoutine(void* ptr){    // function used by the thread invoked by run
    //   CreateBigQUtil* myT = (CreateBigQUtil*) ptr;
    //   new BigQ(*(myT->inputPipe),*(myT->outputPipe),*(myT->orderMaker),myT->runlen);
    //   return 0;
   CreateBigQUtil *bigutilobj = (CreateBigQUtil*) ptr;
   OrderMaker sortorder(bigutilobj->mySchema);
   Pipe spipe(1000);
   BigQ biq(*bigutilobj->inputPipe, spipe, sortorder, 100);
   Record c,n;
   ComparisonEngine cmp;

   if(spipe.Remove(&c)){
       while(spipe.Remove(&n)){
           if(cmp.Compare(&c,&n,&sortorder)){
               bigutilobj->outputPipe->Insert(&c);
               c.Consume(&n);
           }
       }
     bigutilobj->outputPipe->Insert(&c);
   }
   bigutilobj->outputPipe->ShutDown();
}

void DuplicateRemoval::Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema) {
	CreateBigQUtil *bigutilobj= new CreateBigQUtil;// form arguments
	bigutilobj->inputPipe=&inPipe;
	bigutilobj->outputPipe=&outPipe;
	bigutilobj->mySchema=&mySchema;
	bigutilobj->runlen=bnloop;
	pthread_create (&runt, NULL, createBigQRoutine,(void*)bigutilobj  );   // create thread

}

void DuplicateRemoval::WaitUntilDone () {//wait until done
	 pthread_join (runt, NULL);
}

void DuplicateRemoval::Use_n_Pages (int runlen) {//set number of pages
		//cout << "runlen satrt is :" << runlen;
	this->bnloop= runlen;
}

typedef struct{
  Pipe* inputPipe;
  FILE* outFile;
  Schema* mySchema;
  //int runlen;
} writeoutobj; // structure used in WriteOut

void* WriteOutworker(void *args) {               // worker thread
  writeoutobj *woo= (writeoutobj*)args;
  Record rec;
  while (woo->inputPipe->Remove(&rec))
			rec.Write(woo->outFile, woo->mySchema);
			//	cout << "endding" <<endl;

}

void WriteOut::Run (Pipe &inPipe, FILE *outFile, Schema &mySchema) {
	writeoutobj *woo=new writeoutobj();// form arguments
	woo->inputPipe=&inPipe;
	woo->outFile=outFile;
	woo->mySchema=&mySchema;
	pthread_create (&runt, NULL, WriteOutworker,(void*)woo );   // create thread

}

void WriteOut::WaitUntilDone () {//wait until done
	 pthread_join (runt, NULL);
}

void WriteOut::Use_n_Pages (int runlen) {//set number of pages
	this->bnloop= runlen;
}

typedef struct{
  Pipe* inputPipe;
  Pipe* outputPipe;
  Function* computeMe;
  //int runlen;
} sumobj; // Structure used in sum

void* Sumworker(void *args){    // worker thread
    sumobj* smo = (sumobj*) args;
    int intres;
    double doubleres;
    ostringstream res;
    string ressum;
    int ints = 0;
    double doubles = 0;
    Record rec;
    Record resrec;
    Type retype;
    // while(smo->inputPipe->Remove(&rec)){
    //     retype = smo->computeMe->Apply(rec,intres,doubleres);
    //     if(retype == Int){
    //         ints = ints+intres;
    //     }
    //     else if(retype == Double){
    //         doubles = doubles+doubleres;
    //     }
    // }
    if (smo->computeMe->resultType() == Int) {
				int sum=0; Record rec;
				while (smo->inputPipe->Remove(&rec)){
					sum += smo->computeMe->Apply<int>(rec);
                }
				res << sum;
				ressum = res.str();
				ressum.append("|");
				Attribute IS = {"int", Int};
				Schema out_sch("out_sch", 1, &IS);
				resrec.ComposeRecord(&out_sch, ressum.c_str());
				smo->outputPipe->Insert(&resrec);
        //         Attribute ISUM = {(char*)"sum",Int};
        //         Schema schemasum((char*)"schemasum_file_unused",1,&ISUM);
        //         char sstring[50];
        //         sprintf(sstring,"%f|",ints);
        //         resrec->ComposeRecord(&schemasum,sstring);
		 }
    else{
             double sum=0; Record rec;
				while (smo->inputPipe->Remove(&rec)){
					sum += smo->computeMe->Apply<double>(rec);
                }
				res << sum;
				ressum = res.str();
				ressum.append("|");
				Attribute DS = {"double", Double};
				Schema out_sch("out_sch", 1, &DS);
				resrec.ComposeRecord(&out_sch, ressum.c_str());
				smo->outputPipe->Insert(&resrec);
                // Attribute DSUM = {(char*)"sum",Double};
                // Schema schemasum((char*)"schemasum_file_unused",1,&DSUM);
                // char sstring[50];
                // sprintf(sstring,"%f|",doubles);
                // resrec->ComposeRecord(&schemasum,sstring);
                // resrec->Print(&schemasum);
                //smo->outputPipe->Insert(&resrec);
		}
        // smo->outputPipe->Insert(resrec);
        smo->outputPipe->ShutDown();

}

void Sum::Run (Pipe &inPipe, Pipe &outPipe,Function &computeMe){
	sumobj* smo = new sumobj;;// form arguments
	smo->inputPipe=&inPipe;
	smo->outputPipe=&outPipe;
	smo->computeMe=&computeMe;
	pthread_create (&runt, NULL, Sumworker,(void*)smo );   // create thread

}


void Sum::WaitUntilDone () {//wait until done
	pthread_join (runt, NULL);
}

void Sum::Use_n_Pages (int runlen) {//set number of pages
	this->bnloop= runlen;
}

typedef struct{
    Pipe* inputpipe;
    Pipe* outputpipe;
    OrderMaker* om;
    Function* fun;
    int runl;
}gbo;  // structure used in Groupby

void Combine(Record * rec,int sum,Pipe *outPipe, OrderMaker *groupAtts,Type x) { // used in griupbyworker which is invoked by worker thred
	Record * aggregate = new Record;
	Record * toPipe = new Record;
	// Code similar to sum to create sum record
	Attribute * atts = new Attribute[1];
	std::stringstream s;
	s << sum;
	s << "|";
	atts[0].name = "SUM";
	atts[0].myType = x;
	Schema sumSchema("dummy", 1, atts);
	aggregate -> ComposeRecord(&sumSchema, s.str().c_str());

	// create rest of the attribures and merge them to record
	int numAttsToKeep = 1 + groupAtts->numAtts;
	int * attsToKeep = new int[numAttsToKeep];
	attsToKeep[0] = 0;
	for (int i = 1; i < numAttsToKeep; i++) {
		attsToKeep[i] = groupAtts->whichAtts[i - 1];
	}
	toPipe -> MergeRecords(aggregate, rec, 1, groupAtts->numAtts,
		attsToKeep, numAttsToKeep, 1);
	//cout << "after MR in CAIR in Groupby*******************" << endl;
	outPipe->Insert(toPipe);
	//cout << "after insert in CAIR in Groupby*******************" << endl;

	delete toPipe;
	delete aggregate;
	delete[] atts;
	delete[] attsToKeep;

}



void* GroupByworker(void *args) {               // worker thread

	gbo *tgbo=new gbo();
    tgbo=(gbo*)args;
	Pipe sortpipe(1000);
    BigQ biq(*tgbo->inputpipe, sortpipe, *tgbo->om, tgbo->runl);
    Record c, ne;
    ComparisonEngine cmp;
		if (tgbo->fun->resultType() == Int) {
				if(sortpipe.Remove(&c)) {  // c holds the current group
					int sum = tgbo->fun->Apply<int>(c);   // holds the sum for the current group
					while(sortpipe.Remove(&ne)){
						if(cmp.Compare(&c, &ne, tgbo->om)) {
							Combine(&c,sum,tgbo->outputpipe,tgbo->om,Int);
							c.Consume(&ne);
							sum = tgbo->fun->Apply<int>(c);
						}
						else
							 sum += tgbo->fun->Apply<int>(ne);
					}
					Combine(&c,sum,tgbo->outputpipe,tgbo->om,Int);
				}
		}
  else {
			if(sortpipe.Remove(&c)) {  // c holds the current group
					double sum = tgbo->fun->Apply<double>(c);   // holds the sum for the current group
					while(sortpipe.Remove(&ne)){
						if(cmp.Compare(&c, &ne, tgbo->om)) {
						Combine(&c,sum,tgbo->outputpipe,tgbo->om,Double);
							c.Consume(&ne);
							sum = tgbo->fun->Apply<double>(c);
						}
						else sum += tgbo->fun->Apply<double>(ne);
					}
					Combine(&c,sum,tgbo->outputpipe,tgbo->om,Double);
				}
	}
	tgbo->outputpipe->ShutDown();
}

void GroupBy::Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &om, Function &fun){
	gbo *gbo1=new gbo();// form arguments
	gbo1->inputpipe=&inPipe;
	gbo1->outputpipe=&outPipe;
	gbo1->om=&om;
	gbo1->fun=&fun;
	gbo1->runl=bnloop;
	pthread_create (&runt, NULL, GroupByworker,(void*)gbo1 );   // create thread

}

void GroupBy::WaitUntilDone () {//wait until done
	 pthread_join (runt, NULL);
}

void GroupBy::Use_n_Pages (int runlen) { //set number of pages
	this->bnloop= runlen;
}

Join::Join()
{

}

Join::~Join()
{

}
 // All the variable for run are declared in the header file
void Join::Run (Pipe &inPipe1, Pipe &inPipe2, Pipe &outPipe, CNF &selOp, Record &literal)
{
    inputPipe1 = &inPipe1;
    inputPipe2 = &inPipe2;
    outputPipe = &outPipe;
    incnf = &selOp;
    inliteral = &literal;

    pthread_create(&runt, NULL, procjoin, (void *) this);  // starts the worker thread
}

void* Join::procjoin(void *op)   // called by worker thread from join
{
    Join *join = static_cast<Join*>(op);
    join->recjoin();
	join->outputPipe->ShutDown();

    pthread_exit(NULL);

}

void Join::recjoin()          //called in procjoin. join the records
{
    OrderMaker *left = new OrderMaker();
    OrderMaker *right = new OrderMaker();

    if(incnf->GetSortOrders(*left, *right) == 0)
    {
        //block-nested loops join
        waitedjoin();
    }
    else
    {
        //sort merge join
        sortjoin(left, right);

    }


}


void Join::sortjoin(OrderMaker *left, OrderMaker *right)  // sorts and merges the join recs
{
    Pipe sortedPipeL(1000);
    Pipe sortedPipeR(1000);
	bool isLeftPipeEmpty = false;
	bool isRightPipeEmpty = false;
	Record *leftRec = new Record;
	Record *rightRec = new Record;
	vector<Record*> leftVec;
	vector<Record*> rightVec;
	ComparisonEngine comp;

    //Sort the records using BIGQ
	Record temp1, temp2;
	inputPipe1->Remove(&temp1);
	inputPipe2->Remove(&temp2);

    BigQ bigL(*inputPipe1, sortedPipeL, *left, runlen);
    BigQ bigR(*inputPipe2, sortedPipeR, *right, runlen);

	//Initialising the empty vars
	isLeftPipeEmpty = (0 == sortedPipeL.Remove(leftRec));
	isRightPipeEmpty = (0 == sortedPipeR.Remove(rightRec));

	int c = 0;
    while(!isLeftPipeEmpty && !isRightPipeEmpty)
    {
        int res = comp.Compare(leftRec, left, rightRec, right);
        if(res < 0)
        {
			isLeftPipeEmpty = (0 == sortedPipeL.Remove(leftRec));
        }
        else if(res > 0)
        {
			isRightPipeEmpty = (0 == sortedPipeR.Remove(rightRec));
        }
        else
        {
            //Logic to join and merge the records
			isLeftPipeEmpty = search(sortedPipeL, leftRec, leftVec, left);
			isRightPipeEmpty = search(sortedPipeR, rightRec, rightVec, right);

            for(int i=0; i < leftVec.size(); i++)
            {
                for(int j=0; j<rightVec.size(); j++)
                {
					c++;
                   mergerecs(leftVec[i], rightVec[i]);

                }
            }
        }
		c++;
    }
	Record t;
    // remove all leftoers
	while (sortedPipeL.Remove(&t));
	while (sortedPipeR.Remove(&t));

}


bool Join::search(Pipe &pipe, Record *rec, vector<Record*> &vecOfRecs, OrderMaker *sortOrder) // searches for similar records
{

    for(int i = 0 ; i < vecOfRecs.size(); i++)

        vecOfRecs.clear();

    Record *curr = new Record;
    ComparisonEngine comp;

    curr->Consume(rec);
    vecOfRecs.push_back(curr);

    bool isEmpty = (0 == pipe.Remove(rec));
    //Handle case where pipe becomes empty, return True and assign it to isLEmpty
    while(!isEmpty && comp.Compare(curr, rec, sortOrder) == 0)
    {

        vecOfRecs.push_back(rec);
        curr->Consume(rec);
        isEmpty = (0 == pipe.Remove(rec));
    }

    return isEmpty;
}

void Join::mergerecs(Record *left, Record *right)   // merges the records and outputs them
{

    Record *toPipe = new Record;

    int numAttsLeft = left->GetNumAtts();
    int numAttsRight = right->GetNumAtts();
    int numAttsTotal = numAttsLeft + numAttsRight;

    int * attsToKeep = (int *) alloca(sizeof(int) * numAttsTotal);
    int index = 0;

    for (int i = 0; i < numAttsLeft; i++){
        attsToKeep[index] = i;
        index++;
    }

    for (int i = 0; i < numAttsRight; i++){
        attsToKeep[index] = i;
        index++;
    }

    // Merge the left and right records
    toPipe->MergeRecords(left, right, numAttsLeft, numAttsRight,
	    	       attsToKeep, numAttsTotal, numAttsLeft);
	Attribute IA = { "int", Int };
	Attribute SA = { "string", String };
	Attribute DA = { "double", Double };
	int sAtts = 7;
	int psAtts = 5;
	int outAtts = sAtts + psAtts;
	Attribute ps_supplycost = { "ps_supplycost", Double };
	Attribute joinatt[] = { IA,SA,SA,IA,SA,DA,SA, IA,IA,IA,ps_supplycost,SA };
	Schema join_sch("join_sch", outAtts, joinatt);
	toPipe->Print(&join_sch);

    outputPipe->Insert(toPipe);


}

void Join::waitedjoin()                   //checks for blocked nested loops
{
    DBFile db;

    //Creating a heap file to store the records from the First Pipe
    char *path = "join_temp.bin";
    db.Create(path, heap, NULL);

    //Insert all the records from the First Pipe into the DBFile
    Record *recToFile = new Record;
    while(inputPipe1->Remove(recToFile))
    {
        db.Add(recToFile);
    }

    Record recFromFile;
    Record recFromPipe;
    ComparisonEngine comp;
    while(inputPipe2->Remove(&recFromPipe))
    {

        while(db.GetNext(recFromFile))
        {

            if(!comp.Compare(&recFromPipe, &recFromFile,inliteral, incnf))
            {
				mergerecs(&recFromPipe, &recFromFile);
            }
        }

        db.MoveFirst();
    }

    remove(path);
    return;

}


void Join::WaitUntilDone ()    //  wait until done
{

    pthread_join(runt, NULL);

}

void Join::Use_n_Pages (int n)   // sets the number of pages
{
    runlen = n;
}
