#include<iostream>
#include"DependencyInstance.h"
using namespace std;

void DependencyInstance::init(){
	forms = NULL;
	heads = NULL;
}
DependencyInstance::DependencyInstance(){
	init();
	forms = new vector<string*>();
	heads = new vector<int>();
}
DependencyInstance::DependencyInstance(std::vector<string*> *forms, std::vector<int> *heads){
	init();
	this->forms = forms;
	this->heads = heads;
}
int DependencyInstance::length(){
	return (int)(forms->size());
}
string* DependencyInstance::toString(){
	string tmp = string("[");
	string* sb = new string(tmp);
	vector<string*>::iterator iter;
	for(iter = forms->begin(); iter != forms->end(); iter++){
		if(iter != forms->begin()){
			sb->append(", ");
		}
		string* s = *iter;
		sb->append(*s);
	}
	sb->append("]\n");
	return sb;
}

DependencyInstance::~DependencyInstance(){
	vector<string*>::iterator iter;
	for(iter = forms->begin(); iter != forms->end(); ++iter){
		delete (*iter);
	}
	delete(forms);
	delete(heads);
	forms = NULL;
	heads = NULL;
}

