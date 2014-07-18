/*
 * eval.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: zzs
 */
//evaluating for the machine

using namespace std;
#include <iostream>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "../cslm/Tools.h"
#include "../cslm/Mach.h"
#include "../cslm/MachConfig.h"
#include "../cslm/Trainer.h"
#include "../step2_nn/load_pre.h"
#include "../HashMap.h"
#include "../CONLLReader.h"
#include "../CONLLWriter.h"
#include "../Eisner.h"
#include "DependencyEvaluator.h"
#include "../pre_training.h"
#include "../which_main.h"

void usage2 (MachConfig &mc, bool do_exit=true)
{
   cout <<  endl
        << "cslm_train - a tool to train continuous space language models" << endl
	<< "Copyright (C) 2014 Holger Schwenk, University of Le Mans, France" << endl << endl;

  mc.print_help();
  if (do_exit) exit(1);
}

double get_score_from_array(int len,int h,int m,double* a)
{
	int small = (h>m) ? m : h;
	int large = (h>m) ? h : m;
	int lr = (small==h) ? E_RIGHT : E_LEFT;
	return a[get_index2(len,small,large,lr)];
}

int main3(int argc, char *argv[])
{
  MachConfig mach_config(true);
  string mach_fname, test_fname, dev_fname;
  int curr_it = 0;
  Mach *mlp;

  // select available options
  mach_config
    .sel_cmdline_option<std::string>("mach,m"        , true )
    .sel_cmdline_option<std::string>("test-data,t"  , true )
    .sel_cmdline_option<std::string>("dev-data,d"    , true)
    ;

  // parse parameters
  if (mach_config.parse_options(argc, argv)) {
    // get parameters
    mach_fname  = mach_config.get_mach();
    test_fname = mach_config.get_test_data();
    dev_fname   = mach_config.get_dev_data();
    curr_it     = mach_config.get_curr_iter();

    //gold file
    CONF_test_file = test_fname;
    CONF_gold_file = dev_fname;
  }
  else if (mach_config.help_request())
    usage2(mach_config);
  else {
    if (mach_config.parsing_error())
      usage2(mach_config, false);
    Error(mach_config.get_error_string().c_str());
  }

    // Check if existing machine exists
  const char *mach_fname_cstr = mach_fname.c_str();
  struct stat stat_struct;
  if (stat(mach_fname_cstr, &stat_struct)==0) {
      // read existing network
    ifstream ifs;
    ifs.open(mach_fname_cstr,ios::binary);
    CHECK_FILE(ifs,mach_fname_cstr);
    mlp = Mach::Read(ifs);
    ifs.close();
    cout << "Found existing machine with " << mlp->GetNbBackw()
         << " backward passes, continuing training at iteration " << curr_it+1 << endl;
  }
  else {
    Error("No such machine for eval.");
  }
  //mlp->Info();

  	//evaluating the test files
  	cout << "Now evaluating the test file..." << endl;
  	CONLLReader* reader = new CONLLReader();
  	CONLLWriter* writer = new CONLLWriter();
  	reader->startReading(CONF_test_file.c_str());
  	writer->startWriting(CONF_output_file.c_str());
  	//the list
  	HashMap * wl = training_space::load_wordlist(CONF_wl_file.c_str());

  	//some variables
  	int oov_num = 0;	//out of vocabulary
  	int sen_num = 0;	//sentence number
  	int token_num = 0;	//token number
  	int miss_count = 0;	//only work if the testing file already have answers
  	int TIME_start = clock() / 1000;
  	int TIME_start_fine = 0;
  	//calculate
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
		if(sen_num%500 == 0){
			cout << "Having processed " << sen_num << ";period is "<<
					(clock()/1000)-TIME_start_fine << "ms."<< endl;
			TIME_start_fine = clock() / 1000;
		}
		sen_num++;
		int length = x->forms->size();
		token_num += length - 1;
		double *tmp_scores = new double[length*length*2];

		//construct scores using nn
		int num_pair = length*(length-1);	//2 * (0+(l-1))*l/2
		REAL *mach_x = new REAL[num_pair*CONF_X_dim];
		REAL *mach_y = new REAL[num_pair*CONF_Y_dim];
		int* word_index = new int[length+2];	//including <s> and </s>
		for(int i=0;i<length;i++){
			HashMap::iterator iter = wl->find(x->forms->at(i));
			if(iter == wl->end()){
				oov_num++;
				word_index[i+1] = wl->find(&unknown_token)->second;
			}
			else
				word_index[i+1] = iter->second;
		}
		string sen_s = SENTENCE_START;
		string sen_e = SENTENCE_END;
		word_index[0] = wl->find(&sen_s)->second;
		word_index[length+1] = wl->find(&sen_e)->second;

		int pair_count = 0;
		REAL* assign_x = mach_x;
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
#ifdef WHICH_TWO
					//2 word pair
					if(lr==E_RIGHT){
						*assign_x++ = (REAL)word_index[ii+1];
						*assign_x++ = (REAL)word_index[j+1];
					}
					else{
						*assign_x++ = (REAL)word_index[j+1];
						*assign_x++ = (REAL)word_index[ii+1];
					}
#endif
#ifdef WHICH_SIX
					//build mach_x : 6 words group(h-1 h h+1 m-1 m m+1)
					if(lr==E_RIGHT){
						*assign_x++ = (REAL)word_index[ii];
						*assign_x++ = (REAL)word_index[ii+1];
						*assign_x++ = (REAL)word_index[ii+2];
						*assign_x++ = (REAL)word_index[j];
						*assign_x++ = (REAL)word_index[j+1];
						*assign_x++ = (REAL)word_index[j+2];
					}
					else{
						*assign_x++ = (REAL)word_index[j];
						*assign_x++ = (REAL)word_index[j+1];
						*assign_x++ = (REAL)word_index[j+2];
						*assign_x++ = (REAL)word_index[ii];
						*assign_x++ = (REAL)word_index[ii+1];
						*assign_x++ = (REAL)word_index[ii+2];
					}
#endif
				}
			}
		}
		//- give it to nn
		mlp->evaluate(mach_x,mach_y,num_pair,CONF_X_dim,CONF_Y_dim);
		REAL* assign_y = mach_y;
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					int index = get_index2(length,ii,j,lr);
#ifdef ULTIMATE_DEBUG
					cout << ii << "to" << j << ((lr==E_RIGHT)?'r':'l')
							<< ":" << *assign_y << endl;
#endif
					tmp_scores[index] = *assign_y++;
				}
			}
		}
		//- decode and write
		vector<int> *ret = decodeProjective(length,tmp_scores);
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*ret)[i2] != (*(x->heads))[i2])
				miss_count ++;
		}
#ifdef ULTIMATE_DEBUG
		{
			double score_here=0;
			double score_right=0;
			for(int i2=1;i2<length;i2++){	//ignore root
				int head_here = (*ret)[i2];
				int head_right = (*(x->heads))[i2];
				score_here += get_score_from_array(length,head_here,i2,tmp_scores);
				score_right += get_score_from_array(length,head_right,i2,tmp_scores);
			}
			cout << "Right:" << score_right << ";Output:" << score_here << endl;
		}
#endif
		delete x->heads;
		x->heads = ret;
		writer->write(x);
		delete x;
		delete []tmp_scores;
		delete []mach_x;
		delete []mach_y;
		delete []word_index;
		x = reader->getNext();
	}
	reader->finishReading();
	writer->finishWriting();
	delete reader;
	delete writer;
	cout << "Finished testing in " << (clock()/1000-TIME_start) << "ms" << endl;

	//conclude and evaluate
	cout << "Testing data description:\n"
			<< "Sentences: " << sen_num << '\n'
			<< "Tokens: " << token_num << '\n'
			<< "OOV token: " << oov_num << '\n'
			<< "Miss token: " << miss_count << endl;
	string t;
	DependencyEvaluator::evaluate(CONF_gold_file,CONF_output_file,t,false);

  return 0;
}




