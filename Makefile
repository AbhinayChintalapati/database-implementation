
CC = g++ -O2 -Wno-deprecated

tag = -i

ifdef linux
tag = -n
endif

test_42.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o main.o BigQ.o   QueryTreeNode.o RelationStatistics.o  Heap.o Sort.o Statistics.o RelOp.o      Function.o     DBFile.o Pipe.o yyfunc.tab.o lex.yyfunc.o y.tab.o lex.yy.o
	$(CC) -o test_42.out Record.o Comparison.o ComparisonEngine.o Schema.o main.o File.o BigQ.o   QueryTreeNode.o RelationStatistics.o  Heap.o Sort.o Statistics.o RelOp.o Function.o  DBFile.o Pipe.o y.tab.o yyfunc.tab.o lex.yyfunc.o lex.yy.o -ll -lpthread

gTest_42.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o gTest_42.o BigQ.o   QueryTreeNode.o RelationStatistics.o  Heap.o Sort.o Statistics.o RelOp.o Function.o  DBFile.o Pipe.o yyfunc.tab.o lex.yyfunc.o y.tab.o lex.yy.o
	$(CC) -o gTest_42.out Record.o Comparison.o ComparisonEngine.o Schema.o gTest_42.o File.o BigQ.o   QueryTreeNode.o RelationStatistics.o  Heap.o Sort.o Statistics.o RelOp.o      WriteOut.o Function.o          Project.o         DBFile.o Pipe.o y.tab.o yyfunc.tab.o lex.yyfunc.o lex.yy.o -ll -lpthread -lgtest

test_21.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o    Heap.o Sort.o DBFile.o Pipe.o y.tab.o lex.yy.o test_21.o
	$(CC) -o test_21.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o    Heap.o Sort.o DBFile.o Pipe.o y.tab.o lex.yy.o test_21.o -lfl -lpthread

a2test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-test.o
	$(CC) -o a2test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-test.o -lfl -lpthread


printToFile.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o    Heap.o Sort.o DBFile.o Pipe.o y.tab.o lex.yy.o printToFile.o
	$(CC) -o printToFile.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o    Heap.o Sort.o DBFile.o Pipe.o y.tab.o lex.yy.o printToFile.o -lfl -lpthread

printToFile.o: printToFile.cc
	$(CC) -g -c printToFile.cc

gTest.o: gTest.cc
	$(CC) -g -c gTest.cc

QueryTreeNode.o: QueryTreeNode.cc
	$(CC) -g -c QueryTreeNode.cc

main.o: main.cc
	$(CC) -g -c main.cc

RelationStatistics.o: RelationStatistics.cc
	$(CC) -g -c RelationStatistics.cc


Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc


gtestNew.o: gtestNew.cc
	$(CC) -g -c gtestNew.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc


Function.o: Function.cc
	$(CC) -g -c Function.cc


gtestexec: gtestNew.o
	$(CC) -o gtestexec Record.o Comparison.o ComparisonEngine.o Schema.o File.o    Heap.o Sort.o DBFile.o Pipe.o BigQ.o RelOp.o      WriteOut.o Function.o          Project.o         DBFile.o Pipe.o y.tab.o yyfunc.tab.o lex.yyfunc.o lex.yy.o y.tab.o lex.yy.o gTestNew.o -ll -lgtest -lpthread

gTestSorted.o: gTestSorted.cc
	$(CC) -g -c gTestSorted.cc

test.o: test.cc
	$(CC) -g -c test.cc

test_21.o: test_21.cc
	$(CC) -g -c test_21.cc


a2-test.o: a2-test.cc
	$(CC) -g -c a2-test.cc

a1-test.o: a1-test.cc
	$(CC) -g -c a1-test.cc

Heap.o: Heap.cc
	$(CC) -g -c Heap.cc

Sort.o: Sort.cc
	$(CC) -g -c Sort.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc

ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc

y.tab.o: Parser.y
	yacc -d Parser.y
	gsed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/"
	g++ -c y.tab.c

yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	gsed $(tag) yyfunc.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/"
	g++ -c yyfunc.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

lex.yyfunc.o: LexerFunc.l
	lex -Pyyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c

clean:
	rm -f *.o
	rm -f *.out
	rm -f y.tab.*
	rm -f yyfunc.tab.*
	rm -f lex.yy.*
	rm -f lex.yyfunc*
