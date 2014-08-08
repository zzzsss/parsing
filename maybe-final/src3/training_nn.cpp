/*
 * training_nn.cpp
 *
 *  Created on: Aug 8, 2014
 *      Author: zzs
 */

#include "common.h"

#define CSLM_CONF "nn.conf"
#define CSLM_DATA "nn.df"

void prepare_conf()
{
	//prepare conf
	ofstream fout;
	fout.open(CSLM_DATA);
	fout << "DataDescr 1\nPreload\nResamplSeed 12345678\n"
			<< "ShuffleMode 10\nDataAscii " << CONF_data_file << " 1.0\n"
			<< "WordList " << CONF_vocab_file << endl;
	fout.close();
	fout.open(CSLM_CONF);
	fout << "train-data = " << CSLM_DATA << "\n"
			<< "lrate-beg = 0.045\n" << "lrate-mult = 1e-9\n"
			<< "weight-decay = 3e-5\n" << "block-size = 128\n"
			<< "curr-iter = 0\n" << "last-iter = 20\n"
			<< "mach = %CONF\n";
	//currently 4 layers
	int width = IND_CONF_x_dim_final*100;
	fout << "[machine]\n" << "Sequential = \n" << "Parallel = \n";
	for(int i=0;i<IND_CONF_x_dim_final;i++)
		fout << "Tab = 27100x100\n";
	fout << "#End\n";
	fout << "Tanh = " << width << "x" << width/2 << " fanio-init-weights=1.0\n";
	fout << "Tanh = " << width/2 << "x" << width/4 << " fanio-init-weights=1.0\n";
	fout << "Softmax = " << width/4 << "x" << ALL_classes << " fanio-init-weights=1.0\n";
	fout << "#End\n";
	fout.close();
}
void nn_train()
{
	prepare_conf();
	//training it
	int argc = 2;
	char *argv[] = {"nn_train",CSLM_CONF};

	extern int main_nn (int argc, char *argv[]);
	main_nn(argc,argv);
}
