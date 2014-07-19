#ifndef CONLLReader_H
#define CONLLReader_H

#include<iostream>
#include<fstream>
#include<string>
#include<cstdio>
#include"DependencyInstance.h"
using namespace std;

/* simplified version of Reader */
class CONLLReader{
protected:
	FILE* inputReader;
	string* normalize(string* s);
public:
	CONLLReader();
	//~CONLLReader();
	void startReading(const char* file);
	void finishReading();
	DependencyInstance* getNext();
};
#endif
