#ifndef CONLLWriter_H
#define CONLLWriter_H

#include<iostream>
#include<fstream>
#include<string>
#include<cstdio>
#include"DependencyInstance.h"
using namespace std;

class CONLLWriter{
protected:
	FILE* writer;
public:
	CONLLWriter();
	void write(vector<string*> &forms , vector<int> &heads);
	void startWriting(const char* file);
	void finishWriting();
};
#endif
