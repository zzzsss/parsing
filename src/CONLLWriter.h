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
	FILE* writer=0;
public:
	CONLLWriter(){}
	void write(DependencyInstance* x){
		int length = (int)(x->forms->size());
		string* str;
		for(int i = 1; i<length; ++i){
			fprintf(writer, "%d\t", i);

			str = (*x->forms)[i];
			fprintf(writer, "%s\t_\t", str->c_str());

			fprintf(writer, "_\t_\t_\t_\t_\t%d\t",  (*x->heads)[i]);

			fprintf(writer, "_\n");
		}
		fputc('\n', writer);
	}
	void startWriting(const char* file){
		writer = fopen(file, "w");
	}
	void finishWriting(){
		fclose(writer);
	}
};
#endif
