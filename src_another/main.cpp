/*
 * main.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: zzs
 */

#include "param.h"

int main(int argc,char **argv)
{
	srand(time(0));
	//1.deal with the params
	//2.vocab
	HashMap *vocabs = load_wordlist(CONF_vocab_file_name.c_str());
	//3.train-file --> indexed and scores
	REAL *train_x;
	REAL *train_y;
	int train_num=load_trainfile(CONF_train_file.c_str(),vocabs,&train_x,&train_y);
	Data * train_data = new Data(train_x,train_y,CONF_X_dim,CONF_Y_dim,train_num);
	//4.feed training to nn and get mach --- nn_train maybe
	//5.testing
}


