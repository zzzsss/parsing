/*
 * main.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: zzs
 */

#include "common.h"

string SENTENCE_START = "<s>";
string SENTENCE_END = "</s>";
string SENTENCE_START_2 = "<ss>";
string SENTENCE_END_2 = "</ss>";
string WORD_UNKNOWN = "<unk>";
string POS_START = "<s-POS>";
string POS_END = "</s-POS>";
string POS_START_2 = "<ss-POS>";
string POS_END_2 = "</ss-POS>";
string POS_UNKNOWN = "<unk-POS>";

int ALL_classes=1;	//1 is for the simple sigmoid way(they don't change it)
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
int CONF_vocab_out=0;	//outside vocab...
int CONF_if_consider_pos=0;	//0:no;1:word+pos;2:add pos;3:both	-- default 1
int CONF_x_dim=6;			//the input dim to the nn 6 or 10--- default 6
int IND_CONF_x_dim_final=6;
int CONF_x_dim_missing=-1;	//the missing one for the dimension(only missing one)

//4.scores
int CONF_if_y_calss=1;	//if use the softmax to classify the output
int CONF_y_class_size=5;	//how many classes of y(only if before one is true)<naive way>

double CONF_score_expbase = -1;	//exp score classify base<exp way>
int CONF_score_reverse = 0;

//nn: conf of the nn
string CONF_NN_LRATE = "0.045";	//learning rate
string CONF_NN_LMULT = "1e-9";
string CONF_NN_ITER = "20";	//iteration
string CONF_NN_WD = "3e-5";

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
		else if(buf=="exp")
			fin >> CONF_score_expbase;
		else if(buf=="exp-rev")
			CONF_score_reverse = 1;
		else if(buf=="nn_iter")
			fin >> CONF_NN_ITER;
		else if(buf=="nn_lrate")
			fin >> CONF_NN_LRATE;
		else if(buf=="nn_wd")
			fin >> CONF_NN_WD;
		else if(buf=="nn_lmult")
			fin >> CONF_NN_LMULT;
		else if(buf=="vocab-out")
			CONF_vocab_out = 1;
		else{fin.getline(line, DATA_LINE_LEN); continue;}
	}

	//about input
	if(CONF_x_dim != 6 && CONF_x_dim != 10)
		Error("Bad x dimension.");
	if(CONF_x_dim_missing>=0 && CONF_x_dim_missing<CONF_x_dim)
		IND_CONF_x_dim_final = CONF_x_dim-1;
	else
		IND_CONF_x_dim_final = CONF_x_dim;
	if(CONF_if_consider_pos & 0x2)	//pos
		IND_CONF_x_dim_final *= 2;

	//about output
	if(CONF_y_class_size>1 || CONF_score_expbase>1)
		CONF_if_y_calss = 1;
	else if(CONF_y_class_size==1)
		CONF_if_y_calss = 0;
	else
		Error("Illegal output specification...");
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
			<< CONF_x_dim_missing << "\t" << CONF_if_y_calss << "\t" << CONF_y_class_size << "\t"
			<< ((CONF_vocab_out)?"Vocab-out":"Vocab-not-out") << "\t" << CONF_score_expbase << "\t"
			<< "rev-or-not:" << CONF_score_reverse << endl;
	cout << CONF_NN_LRATE << '\t' << CONF_NN_LMULT << '\t'
			<< CONF_NN_WD << '\t' << CONF_NN_ITER << endl;
}
}

int main(int argc,char ** argv)
{
	srand(time(0));
	parsing_conf::init_parsing_conf(argc,argv);
	//1.preparing training data
	pre_training();
	//2.nn train
	nn_train();
	//3.final eval
	eval();
}
