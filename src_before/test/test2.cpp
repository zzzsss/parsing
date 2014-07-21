/*
 * test2.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include<fstream>
#include<ctime>
#include<cstdio>
#include<set>
#include"Util.h"
#include"StringPool.h"
#include"DependencyEvaluator.h"
#include"DependencyParser.h"
#include"DependencyParser2.h"
#include"../new_ones.h"
#include"CONLLWriter.h"
#include"Eisner.h"
using namespace std;
extern StringPool* pool;
extern set<string> punctSet;
extern set<string> coordSet;
extern set<string> commaSet;
extern double Negative_Infinity;
extern double Positive_Infinity;

extern vector<vector<vector<double> > > probs;
extern vector<vector<vector<vector<double> > > > nt_probs;
extern string No_type;

extern double* testing_get_probs();

int Test2Main(int argc,char **argv)
{
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

	DependencyPipe* pipe = new DependencyPipe(options);
	DependencyParser* dp = new DependencyParser(pipe, options);
	cout<<"\tLoading model...";
	dp->loadModel(options->modelName);
	cout<<"done."<<endl;

	/* 1.init files */
		string* tFile = dp->options->testfile;
		string* file = dp->options->outfile;
		long start = clock() / CLOCKS_PER_SEC;
		dp->pipe->initInputFile(tFile);
		dp->pipe->initOutputFile(file);

		/* 2.start */
		cout<<"Processing Sentence: ";
		DependencyInstance* instance = dp->pipe->nextInstance();
		int cnt = 0;
		while(instance != NULL){
			printf("%d ", cnt++);
			int length = instance->length();
					probs = vector<vector<vector<double> > >(length);
					for(int i = 0; i < length; i++){
						probs[i] = vector<vector<double> >(length);
						for(int j = 0; j < length; j++){
							probs[i][j] = vector<double>(2);
						}
					}
			pipe->fillFeatureVectors(instance, probs, nt_probs,dp->params);
			double *scores = testing_get_probs();
			vector<int>*r2 = decodeProjective(length,scores);
			//vector<int>*r2 = newone_outputparse_one(dp,instance,0);
			//print
			CONLLWriter *w = (CONLLWriter *)pipe->depWriter;
			FILE* writer = w->writer;
			/*
			for(int i = 0; i<length-1; ++i){
				fprintf(writer, "%d\t", i);
				fprintf(writer, "%s\t_\t", (*instance->forms)[i]->c_str());
				fprintf(writer, "_\t_\t_\t%d\t", (*r2)[i]);
				fprintf(writer, "_\n");
			}*/
			for(int i = 1; i<length; ++i){
				fprintf(writer, "%d\t", i);
				fprintf(writer, "%s\t_\t", (*instance->forms)[i]->c_str());
				fprintf(writer, "_\t_\t_\t%d\t", (*r2)[i]);
				fprintf(writer, "_\n");
			}
			fputc('\n', writer);

			delete(instance);
			delete []scores;
			delete r2;
			instance = dp->pipe->nextInstance();
		}
		dp->pipe->close();
	delete(dp);

	cout<<endl;
		if(options->eval){
			cout<<"\nEVALUATION PERFORMANCE... \n";
			DependencyEvaluator::evaluate(*(options->goldfile), *(options->outfile), *(options->format), options->labeled);
			printf("%s\n", "Done.");
		}
		delete(options);
		delete(pool);
		return 0;

}


