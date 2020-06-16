#include <iostream>
#include <gtest/gtest.h>
#include <map>
#include <vector>
#include <iostream>
#include <assert.h>
#include <string.h>

#include "QueryTreeNode.h"
#include "ParseTree.h"
#include "Statistics.h"

extern "C"
{
	int yyparse(void);
	struct YY_BUFFER_STATE* yy_scan_string(const char*);

}
extern struct FuncOperator* finalFunction;
extern struct TableList* tables;
extern struct AndList* boolean;
extern struct NameList* groupingAtts;
extern struct NameList* attsToSelect;
extern int distinctAtts;
extern int distinctFunc;

void GetTables(vector<string>& relations)
{

	TableList* list = tables;

	while (list)
	{

		if (list->aliasAs)
		{
			relations.push_back(list->aliasAs);
		}
		else
		{
			relations.push_back(list->tableName);
		}

		list = list->next;

	}

}



void GetJoinsAndSelects(vector<AndList>& joins, vector<AndList>& selects, vector<AndList>& joinDepSel, Statistics s)
{

	struct OrList* currentOrL;

	// loop over booleans
	while (boolean != 0)
	{

		currentOrL = boolean->left;

		// If equals, check for name and insert into joins vector
		if (currentOrL && currentOrL->left->code == EQUALS && currentOrL->left->left->code == NAME && currentOrL->left->right->code == NAME)
		{
			AndList newAnd = *boolean;
			newAnd.rightAnd = 0;
			joins.push_back(newAnd);
		}
		else
		{
			currentOrL = boolean->left;

			//Can only be one table, check for a.b = c.d or normal equals
			if (0 == currentOrL->left)
			{
				AndList newAnd = *boolean;
				newAnd.rightAnd = 0;
				selects.push_back(newAnd);
			}
			else
			{
				//(a.b = c OR d.e = f) 
				vector<string> involvedTables;

				// loop over the or list
				while (currentOrL != 0)
				{
					Operand* oprnd = currentOrL->left->left;
					if (oprnd->code != NAME)
					{
						oprnd = currentOrL->left->right;
					}
					string rel;
					s.ParseRelation(oprnd, rel);

					if (involvedTables.size() == 0)
					{
						involvedTables.push_back(rel);
					}
					else if (rel.compare(involvedTables[0]) != 0)
					{
						involvedTables.push_back(rel);
					}

					currentOrL = currentOrL->rightOr;
				}

				if (involvedTables.size() > 1)
				{
					AndList newAnd = *boolean;
					newAnd.rightAnd = 0;
					joinDepSel.push_back(newAnd);
				}
				else
				{
					AndList newAnd = *boolean;
					newAnd.rightAnd = 0;
					selects.push_back(newAnd);
				}
			}
		}

		boolean = boolean->rightAnd;

	} // end 

} // end 

vector<AndList> optimizeJoinOrder(vector<AndList> joins, Statistics* s)
{

	vector<AndList> newOrder;
	newOrder.reserve(joins.size());
	AndList join;

	double smallest = -1.0;
	double guess = 0.0;

	string rel1;
	string rel2;
	int i = 0;
	int smallestIdx = 0;
	int count = 0;
	vector<string> joinedRelationsVec;

	// loop until all joins are processed
	while (joins.size() > 1)
	{
		while (i < joins.size())
		{
			join = joins[i];

			// parse relation to get the relation name
			s->ParseRelation(join.left->left->left, rel1);
			s->ParseRelation(join.left->left->right, rel2);

			if (smallest == -1.0)
			{
				char* rels[] = { (char*)rel1.c_str(), (char*)rel2.c_str() };
				smallest = s->Estimate(&join, rels, 2);
				smallestIdx = i;
			}
			else
			{
				char* rels[] = { (char*)rel1.c_str(), (char*)rel2.c_str() };

				// estimate the value
				guess = s->Estimate(&join, rels, 2);
				if (guess < smallest)
				{
					smallest = guess;
					smallestIdx = i;
				}
			}
			i++;
		}

		joinedRelationsVec.push_back(rel1);
		joinedRelationsVec.push_back(rel2);

		newOrder.push_back(joins[smallestIdx]);

		smallest = -1.0;
		i = 0;
		count++;
		joins.erase(joins.begin() + smallestIdx);
	}

	//When joins is size 1, the last one in it must be the largest join, so we just have to do:

	newOrder.insert(newOrder.begin() + count, joins[0]);

	return newOrder;
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

//Test case to verify tables parsing
TEST(TestCase1, SubTest1) {
	char* cnf = "SELECT c.c_name FROM customer AS c, nation AS n, region AS r WHERE(c.c_nationkey = n.n_nationkey) AND (n.n_regionkey = r.r_regionkey)";
	yy_scan_string(cnf);
	yyparse();

	Statistics* s = new Statistics();

	vector<string> relations;
	vector<AndList> joins;
	vector<AndList> selects;
	vector<AndList> joinDepSels;
	string projectStart;

	GetTables(relations);
	ASSERT_TRUE(relations.size() == 3);


	GetJoinsAndSelects(joins, selects, joinDepSels, *s);
	ASSERT_TRUE(joins.size() == 2);
	ASSERT_TRUE(selects.size() == 0);
	ASSERT_TRUE(joinDepSels.size() == 0);


	if (joins.size() > 1)
	{
		joins = optimizeJoinOrder(joins, s);
		ASSERT_TRUE(joins.size() == 2);
	}
}

//Test case to verify tables parsing
TEST(TestCase2, SubTest2) {
	char* cnf = "SELECT l.l_tax FROM lineitem AS l, orders AS o, part AS p WHERE(l.l_orderkey = o.o_orderkey) AND (p.p_partkey = l.l_partkey)";
	yy_scan_string(cnf);
	yyparse();

	Statistics* s = new Statistics();

	vector<string> relations;
	vector<AndList> joins;
	vector<AndList> selects;
	vector<AndList> joinDepSels;
	string projectStart;

	GetTables(relations);
	ASSERT_TRUE(relations.size() == 3);


	GetJoinsAndSelects(joins, selects, joinDepSels, *s);
	ASSERT_TRUE(joins.size() == 2);
	ASSERT_TRUE(selects.size() == 0);
	ASSERT_TRUE(joinDepSels.size() == 0);


	if (joins.size() > 1)
	{
		joins = optimizeJoinOrder(joins, s);
		ASSERT_TRUE(joins.size() == 2);
	}
}
