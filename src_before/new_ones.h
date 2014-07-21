/*
 * new_ones.h
 *
 *  Created on: Jul 4, 2014
 *      Author: zzs
 */

#ifndef NEW_ONES_H_
#define NEW_ONES_H_


void newone_outputparse(DependencyParser* dp);
vector<int>* newone_outputparse_one(DependencyParser* dp,DependencyInstance *instance,bool write);

//sth convinient for testing
//#define OringalMain
#define Test2Main main

#endif /* NEW_ONE_H_ */
