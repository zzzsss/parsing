/*
 * main.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: zzs
 */

#include "common.h"

//-------------------Configurations---------------
namespace parsing_conf{
//1.pre-training
string CONF_train_file="train.right";	//the training file
string CONF_data_file="data.train";	//the result of pre-training for NN
//2.eval
string CONF_test_file="test.right";	//testing files
string CONF_output_file = "output.txt";
string CONF_gold_file="test.right";	//golden files
string CONF_mach_file;	//mach name
//3.both
string CONF_vocab_file="vocab.list";	//output of pre and input of eval
int CONF_if_consider_pos=0;
int CONF_x_dim=6;			//the input dim to the nn 6 or 10--- default 6
int IND_CONF_x_dim_final=6;
int CONF_x_dim_missing=-1;	//the missing one for the dimension(only missing one)
int CONF_if_y_calss=1;	//if use the softmax to classify the output
int CONF_y_class_size=5;	//how many classes of y(only if before one is true)

void read_conf(const char* file)
{
#define DATA_LINE_LEN 10000
	ifstream fin(file);
	cout << "Dealing conf file '" << file << "'" << endl;
	while (!fin.eof()){
		string buf;
		char line[DATA_LINE_LEN];
		fin >> buf;
		if (buf=="") continue; // HACK
		if (buf[0]=='#') {fin.getline(line, DATA_LINE_LEN); continue;} // skip comments
		if(buf=="test")	//test file
			fin >> CONF_test_file;
		else if(buf=="train")
			fin >> CONF_train_file;
		else if(buf=="gold")
			fin >> CONF_gold_file;
		else if(buf=="mach")
			fin >> CONF_mach_file;
		else if(buf=="pos")
			fin >> CONF_if_consider_pos;
		else if(buf=="xdim")
			fin >> CONF_x_dim;
		else if(buf=="xmissing")
			fin >> CONF_x_dim_missing;
		else if(buf=="yclass")
			fin >> CONF_y_class_size;
		else if(buf=="vocab")
			fin >> CONF_vocab_file;
		else{fin.getline(line, DATA_LINE_LEN); continue;}
	}
	if(CONF_x_dim != 6 && CONF_x_dim != 10)
		Error("Bad x dimension.");
	if(CONF_x_dim_missing>=0 && CONF_x_dim_missing<CONF_x_dim)
		IND_CONF_x_dim_final = CONF_x_dim-1;
	else
		IND_CONF_x_dim_final = CONF_x_dim;

	if(CONF_y_class_size==1)
		CONF_if_y_calss = 0;
	else if(CONF_y_class_size>1)
		CONF_if_y_calss = 1;
	else
		Error("Illegal yclass-size...");
}

void init_parsing_conf(int argc,char ** argv)
{
	for(int i=1;i<argc;i++){
		if(string(argv[i])=="-c"){
			//configuration files
			read_conf(argv[++i]);	//if fault then SIGSEG
		}
		else
			Error("Sorry, only conf file...");
	}
	//print
	cout << "--The configurations: \n";
	cout << CONF_train_file << "\t" << CONF_data_file << "\t" << CONF_test_file << "\t" << CONF_output_file << '\t'
			<< CONF_gold_file << "\t" << CONF_mach_file << "\t" << CONF_vocab_file << "\t"
			<< CONF_if_consider_pos << "\t" << CONF_x_dim << "\t" << IND_CONF_x_dim_final << "\t"
			<< CONF_x_dim_missing << "\t" << CONF_if_y_calss << "\t" << CONF_y_class_size << endl;
}
}

int main(int argc,char ** argv)
{
	parsing_conf::init_parsing_conf(argc,argv);
#ifdef PARSING_PRE
	pre_training();
#elif defined PARSING_EVAL
	eval();
#else
	Error("No programs.");
#endif
}
