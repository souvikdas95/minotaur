/*
	Example: CGraph create using Quadratic and Linear Functions
	
	Vars:

	x1	x2	x3

	H:

	1	2	3
	4	5	6
	7	8	9

	f:

	1	2	3

	Output:

	(((0.5 * (x1)^2) + (6 * (x1 * x2)) + (10 * (x1 * x3)) + (2.5 * (x2)^2)
	+ (14 * (x2 * x3)) + (4.5 * (x3)^2)) + ((1 * x1) + (2 * x2) + (3 * x3)))	
*/
#include <cmath>
#include <iostream>
#include <iomanip>

#include "CGraph.h"
#include "CNode.h"
#include "Problem.h"
#include "Variable.h"
#include "LinearFunction.h"
#include "QuadraticFunction.h"

using namespace Minotaur;

int main()
{
	double H[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	double f[] = {1, 2, 3};

	ProblemPtr p = (ProblemPtr) new Problem();

	VariablePtr x1 = (VariablePtr) (p->newVariable(-INFINITY, +INFINITY, Integer, "x1"));
	VariablePtr x2 = (VariablePtr) (p->newVariable(-INFINITY, +INFINITY, Integer, "x2"));
	VariablePtr x3 = (VariablePtr) (p->newVariable(-INFINITY, +INFINITY, Integer, "x3"));

	VariableConstIterator vbeg = p->varsBegin();
	VariableConstIterator vend = p->varsEnd();

	QuadraticFunctionPtr qf = (QuadraticFunctionPtr) new QuadraticFunction(H, vbeg, vend);
	CGraphPtr cg1 = (CGraphPtr) new CGraph(qf);
	cg1->finalize();
	
	LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction(f, vbeg, vend);
	CGraphPtr cg2 = (CGraphPtr) new CGraph(lf);
	cg2->finalize();

	CGraphPtr cg = (CGraphPtr) new CGraph();
	cg->setOut(cg->newNode(OpPlus, (CNode*) cg1->getOut(), (CNode*) cg2->getOut()));
	cg->finalize();

	cg->write(std::cout);
	std::cout<<std::endl;

	return 0;
}
