/*
 * new_main.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: zzs
 */

//a new main file for the new parser --- almost the same
//provide a framework for the next ...
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include<fstream>
#include<ctime>
#include<set>
#include"Util.h"
#include"StringPool.h"
#include"DependencyEvaluator.h"
#include"DependencyParser.h"
#include"DependencyParser2.h"
#include"new_ones.h"
using namespace std;
StringPool* pool = new StringPool(0);
set<string> punctSet = set<string>();
set<string> coordSet = set<string>();
set<string> commaSet = set<string>();
double Negative_Infinity = -1e100;
double Positive_Infinity = 1e100;

int main(int argc, char* argv[]){
	/* 1.init(arguments) --- the same as the original one */
	punctSet.insert("''");
	punctSet.insert("``");
	punctSet.insert(".");
	punctSet.insert(":");
	punctSet.insert(",");
	ifstream config_in(argv[1]);
	vector<string*> args;
	char line[100];
	while(config_in.getline(line, 100)){
		string arg = string(line);
		args.push_back(pool->get(arg));
	}
	ParserOptions* options = new ParserOptions(args);
	pool->addString(options->pool_size);
	int length = (int)(args.size());
	for(int i = 0; i < length; i++){
		pool->push_back(args[i]);
	}
	config_in.close();

	/* 2.train */
	if(options->train){

	}

	/* 3.test
	 * currently only 1st-order
	 */
	if(options->test){
		DependencyPipe* pipe = new DependencyPipe(options);
		DependencyParser* dp = new DependencyParser(pipe, options);
		pipe->closeAlphabets();
		newone_outputparse(dp);
		delete(dp);
	}

	/* 4.eval --- the same */
	if(options->eval){
		cout<<"\nEVALUATION PERFORMANCE... \n";
		DependencyEvaluator::evaluate(*(options->goldfile), *(options->outfile), *(options->format), options->labeled);
		printf("%s\n", "Done.");
	}
	delete(options);
	delete(pool);
	return 0;
}

