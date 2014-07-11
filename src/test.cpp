/*
 * test.cpp
 *
 *  Created on: Jul 11, 2014
 *      Author: zzs
 */

//the test parts...
#include "Eisner.h"
#include <ctime>

//test1 for the eisner
void test1()
{
	srand((int)time(0));
	int l=40;
	double* tmp = new double[l*l*2];
	for(int i=0;i<l*l*2;i++)
		tmp[i]=((double)rand()/RAND_MAX);
	int ALL = 100000;

	int start = clock() / (CLOCKS_PER_SEC/1000);
	for(int i=0;i<ALL;i++){
		if(i%100 == 0){
			cout << (double)i/ALL * 100 << "% with "<< clock() / (CLOCKS_PER_SEC/1000) - start  << endl;
		}
		//vector<int>* t = decodeProjective(l,tmp);
		//delete t;
	}
	/*
	for(int i=0;i<t->size();i++)
		cout << (*t)[i] << endl;
	delete t;
	*/
}
