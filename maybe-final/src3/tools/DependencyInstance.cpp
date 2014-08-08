#include<iostream>
#include"DependencyInstance.h"
using namespace std;

void DependencyInstance::init(){
	forms = NULL;
	heads = NULL;
	postags = NULL;
	combined_feats = NULL;
}
DependencyInstance::DependencyInstance(){
	init();
	forms = new vector<string*>();
	heads = new vector<int>();
	postags = new vector<string*>();
	combined_feats = new vector<string*>();
}
DependencyInstance::DependencyInstance(std::vector<string*> *forms,
		std::vector<string*> *postags,std::vector<int> *heads){
	init();
	this->forms = forms;
	this->heads = heads;
	this->postags = postags;
	combined_feats = new vector<string*>();
	for(int i=0;i<forms->size();i++){
		string tmp = *(forms->at(i)) + "^_^" + *(postags->at(i));
		this->combined_feats->push_back(new string(tmp));
	}
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
	for(iter = postags->begin(); iter != postags->end(); ++iter){
		delete (*iter);
	}
	if(combined_feats){
		for(iter = combined_feats->begin(); iter != combined_feats->end(); ++iter){
			delete (*iter);
		}
	}
	delete(combined_feats);
	delete(postags);
	delete(forms);
	delete(heads);
	forms = NULL;
	heads = NULL;
}

