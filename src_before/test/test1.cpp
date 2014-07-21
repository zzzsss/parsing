/*
 * test1.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

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
#include"../new_ones.h"
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

double test_score(int len,double* sco,int a, int b)
{
	int dir = (a>b) ? 1 : 0;
	int small = (a>b) ? b : a;
	int large = (a>b) ? a : b;
	double x1 = sco[small*len*2+large*2+dir];
	double x2 = probs[small][large][1-dir];
	if(x1 != x2){
		cout << x1 << ';' << x2 << endl;
		return 0;
	}
	else
		return x1;
}

void check_two(int s,double *r)
{
	if(s!=probs.size())
		cout << "size-mismatch: " << s << ';' << probs.size() << endl;
	for(int i=0;i<s;i++)
			for(int j=0;j<s;j++)
				for(int t=0;t<2;t++){
					//differenr directions
					if (r[i*s*2+j*2+t] != probs[i][j][1-t]){
						cout << "value-mismatch: "<< endl;
					}
				}
}


//test for the Eisner method's implement
int Test1Main(int argc,char **argv)
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

	//testing and timing
	DependencyPipe* pipe = new DependencyPipe(options);
	DependencyParser* dp = new DependencyParser(pipe, options);
	/* 0.init sth */
	srand((int)time(0));
	/* 1.init files */
	string* tFile = dp->options->testfile;
	string* file = dp->options->outfile;
	long start,end;
	dp->pipe->initInputFile(tFile);
	dp->pipe->initOutputFile(file);

	/* 2.start */
	cout<<"Processing Sentence: \n";
	DependencyInstance* instance = dp->pipe->nextInstance();
	int cnt = 0;
	while(instance != NULL){
		printf("%d: ", cnt++);
		int length = instance->length();
		//1.the original one
		start = clock() / (CLOCKS_PER_SEC/1000);
		vector<int>*r1 = newone_outputparse_one(dp,instance,0);
		end = clock() / (CLOCKS_PER_SEC/1000);
		cout << end-start << "; ";
		//2.the new one
		double* temp_probs = testing_get_probs();

		check_two(length,temp_probs);

		start = clock() / (CLOCKS_PER_SEC/1000);
		vector<int>*r2 = decodeProjective(length,temp_probs);
		end = clock() / (CLOCKS_PER_SEC/1000);
		cout << end-start << "|| ";
		//3.compare
		cout << r1->size() << '-' << r2->size() << "---";
		if(r1->size() == (r2->size()-1)){
			//r2 has the-root-element
			int error=0;
			for(int i=0;i<r1->size();i++)
				if((*r1)[i] != (*r2)[i+1])
					error++;
			cout << error;
			cout << endl;
		}
		/*
		cout << '\n';
		for(int i=0;i<r1->size();i++)
			cout << (*r1)[i] << ';';
		cout << '\n';
		for(int i=0;i<r2->size()-1;i++)
			cout << (*r2)[i+1] << ';';
		cout << '\n';
			//calculate
		double score1=0,score2=0;

		//check_two(length,temp_probs);

		for(int i=0;i<r1->size();i++)
			score1 += test_score(r2->size(),temp_probs,i+1,(*r1)[i]);
		for(int i=0;i<r1->size();i++)
			score2 += test_score(r2->size(),temp_probs,i+1,(*r2)[i+1]);
		*/

		delete r1;
		delete r2;
		delete []temp_probs;
		delete(instance);
		instance = dp->pipe->nextInstance();
	}
	dp->pipe->close();
	return 0;
}



