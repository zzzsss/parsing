/*
 * Eisner.h
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

#ifndef EISNER_H_
#define EISNER_H_

#include"Util.h"
#include"DependencyInstance.h"
#include <iostream>
#include <cstdlib>
using namespace std;

extern double Negative_Infinity;

#define E_LEFT 0
#define E_RIGHT 1
#define E_INCOM 0
#define E_COM 1

//the index explanation --- C[len][len][2][2]
inline int get_index(int len,int s,int t,int lr,int c)
{
	int key = s;
	key = key * len + t;
	key = key * 2 + lr;
	key = key * 2 + c;
	return key;
}
//the index2
inline int get_index2(int len,int s,int t,int lr)
{
	int key = s;
	key = key * len + t;
	key = key * 2 + lr;
	return key;
}

#define E_ERROR(str) \
	cout << str << endl;\
	exit(1)

vector<int>* decodeProjective(int length,double* scores);

#endif /* EISNER_H_ */
