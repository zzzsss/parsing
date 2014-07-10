/*
 * main.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

/* the dependency pasing using MSTparser and NN */
#include "Eisner.h"
#include <ctime>

//test1 for the eisner
void test1()
{
	srand((int)time(0));
	int l=10;
	double* tmp = new double[l*l*2];
	for(int i=0;i<l*l*2;i++)
		tmp[i]=((double)rand()/RAND_MAX);
	vector<int>* t = decodeProjective(l,tmp);
	for(int i=0;i<t->size();i++)
		cout << (*t)[i] << endl;
	delete t;
}

int main(int argc,char ** argv)
{
	test1();
}
