/*
 * training_after.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: zzs
 */

#include "common.h"
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
//eval

void eval()
{
	const char *mach_fname_cstr = CONF_mach_file.c_str();
	Mach *mlp;
	struct stat stat_struct;
	if (stat(mach_fname_cstr, &stat_struct)==0) {
		// read existing network
	    ifstream ifs;
	    ifs.open(mach_fname_cstr,ios::binary);
	    CHECK_FILE(ifs,mach_fname_cstr);
	    mlp = Mach::Read(ifs);
	    ifs.close();
	    cout << "Found existing machine" << endl;
	}
	else {
		Error("No such machine for eval.");
	}

  	//evaluating the test files
  	cout << "Now evaluating the test file..." << endl;
  	CONLLReader* reader = new CONLLReader();
  	CONLLWriter* writer = new CONLLWriter();
  	reader->startReading(CONF_test_file.c_str());
  	writer->startWriting(CONF_output_file.c_str());
  	//the list
  	HashMap * wl = load_wordlist(CONF_vocab_file.c_str());
#ifdef INSANE_DEBUG
  	FILE* x_file = fdopen(3,"w");
#endif

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
		if(sen_num%1000 == 0){
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
		REAL *mach_x = new REAL[num_pair*IND_CONF_x_dim_final];
		REAL *mach_y = new REAL[num_pair*ALL_classes];
		int *word_index = get_word_index(length,x,wl,&oov_num);

		int pair_count = 0;
		REAL* assign_x = mach_x;
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					//build mach_x
					if(lr==E_RIGHT)
						fill_feature(length,ii,j,word_index,assign_x);
					else
						fill_feature(length,j,ii,word_index,assign_x);
					assign_x += IND_CONF_x_dim_final;
				}
			}
		}
		//- give it to nn
		mlp->evaluate(mach_x,mach_y,num_pair,IND_CONF_x_dim_final,ALL_classes);
		REAL* assign_y = mach_y;
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					int index = get_index2(length,ii,j,lr);
					//important ...
					double temp = 0;
					if(CONF_if_y_calss){
						for(int c=0;c<ALL_classes;c++)
							temp += (*assign_y++)*c;
						tmp_scores[index] = temp;
					}
					else
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
		delete x->heads;
		x->heads = ret;
		writer->write(x);
		delete x;
		delete []tmp_scores;
#ifdef INSANE_DEBUG
		fprintf(x_file,"Sentence %d:\n",sen_num);
  		for(int i=0;i<num_pair;i++){
  			for(int j=0;j<IND_CONF_x_dim_final;j++)
  				fprintf(x_file,"%d ",(int)mach_x[i*IND_CONF_x_dim_final+j]);
  			fprintf(x_file,"\n");
  		}
#endif
		delete []mach_x;
		delete []mach_y;
		delete []word_index;
		x = reader->getNext();
	}
#ifdef INSANE_DEBUG
	fclose(x_file);
#endif
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
}


