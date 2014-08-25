/*
 * training_nn.cpp
 *
 *  Created on: Aug 8, 2014
 *      Author: zzs
 */

#include "common.h"

#define CSLM_CONF "nn.conf"
#define CSLM_DATA "nn.df"

#define TESTING_CONF "conf.test.file"
//just test, really badly rely the version before

int nn_conf_wordcount;

void prepare_conf()
{
	ofstream fout;
	//for the test files
	fout.open(TESTING_CONF);
	fout << "test " << CONF_test_file << endl;
	fout << "gold " << CONF_gold_file << endl;
	fout << "mach " << "nn.curr.mach" << endl;
	fout << "pos " << CONF_if_consider_pos << endl;
	fout << "xdim " << CONF_x_dim << endl;
	fout << "yclass " << ALL_classes << endl;	//use version before...
	fout.close();
	//prepare conf
	fout.open(CSLM_DATA);
	fout << "DataDescr 1\nPreload\nResamplSeed 12345678\n"
			<< "ShuffleMode 10\nDataAscii " << CONF_data_file << " 1.0\n"
			<< "WordList " << CONF_vocab_file << endl;
	fout.close();
	fout.open(CSLM_CONF);
	fout << "train-data = " << CSLM_DATA << "\n"
			<< "lrate-beg = " << CONF_NN_LRATE << "\n" << "lrate-mult = "<< CONF_NN_LMULT <<"\n"
			<< "weight-decay = "<< CONF_NN_WD << "\n" << "block-size = 128\n"
			<< "curr-iter = 0\n" << "last-iter = " << CONF_NN_ITER << "\n"
			<< "mach = %CONF\n";
	//currently 4 layers
	int width = IND_CONF_x_dim_final*100;
	fout << "[machine]\n" << "Sequential = \n" << "Parallel = \n";
	for(int i=0;i<IND_CONF_x_dim_final;i++)
		fout << "Tab = " << nn_conf_wordcount << "x100\n";
	fout << "#End\n";
	fout << "Tanh = " << width << "x" << width/2 << " fanio-init-weights=1.0\n";
	fout << "Tanh = " << width/2 << "x" << width/4 << " fanio-init-weights=1.0\n";
	fout << "Tanh = " << width/4 << "x" << width/8 << " fanio-init-weights=1.0\n";
	if(CONF_if_y_calss)
		fout << "Softmax = " << width/8 << "x" << ALL_classes << " fanio-init-weights=1.0\n";
	else
		fout << "Tanh = " << width/8 << "x" << 1 << " fanio-init-weights=1.0\n";
	fout << "#End\n";
	fout.close();
}
void nn_train()
{
	prepare_conf();
	//training it
	int argc = 2;
	char *argv[] = {"nn_train",CSLM_CONF};

	extern int main_nn_softmax (int argc, char *argv[]);
	extern int main_nn_simple (int argc, char *argv[]);
	if(CONF_if_y_calss)
		main_nn_softmax(argc,argv);
	else
		main_nn_simple(argc,argv);
}
