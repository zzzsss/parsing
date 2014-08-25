/*
 * score.cpp
 *
 *  Created on: Aug 4, 2014
 *      Author: zzs
 */
#include "common.h"

//for the softmax output ways...
REAL* count_scores(vector<int>& ashead)
{
	int num = ashead.size();
	REAL* scores = new REAL[num];
	//1.statistical table
	vector<int> stat;
	for(int i=0;i<num;i++){
		int s = stat.size();
		if(s<=ashead[i]){
			//push till ok
			for(int j=0;j<ashead[i]-s;j++)
				stat.push_back(0);
			stat.push_back(1);	//1 count
		}
		else{
			stat[ashead[i]]++;
		}
	}
	//-print stat
	cout << "--2.5-the result of the counts." << endl;
	for(int i=0;i<stat.size();i++){
		//cout << stat[i] << "||" << stat[i]/((double)num-stat[0])*100 << endl;
	}

	//2.decide scores
	//-just simple way of deciding
	if(CONF_score_expbase <= 1){	//no exp mode
		cout << "--Not exp mode" << endl;
		for(int i=0;i<num;i++){
		//floating point number...
			scores[i] = ((int)(ashead[i]+0.1)>=(CONF_y_class_size-1))?(CONF_y_class_size-1):ashead[i];
		}
		ALL_classes = CONF_y_class_size;
	}
	else{
		//score as exp
		cout << "--Exp mode with "<< CONF_score_expbase << endl;
		int* the_classes = new int[stat.size()];
		int curr_class = 0;
		double curr_bound = 1;
		for(int i=0;i<stat.size();i++){
			if((double)i > (curr_bound-0.00001)){
				curr_class++;
				curr_bound *= CONF_score_expbase;
			}
			the_classes[i] = curr_class;
		}
		//reverse version
		if(CONF_score_reverse){
			int* tmp_c = new int[stat.size()];
			tmp_c[0] = 0;
			for(int i=1;i<stat.size();i++)
				tmp_c[i] = curr_class - the_classes[stat.size()-i] + 1;
			delete [] the_classes;
			the_classes = tmp_c;
		}
		for(int i=0;i<num;i++){
			scores[i] = the_classes[ashead[i]];
		}
		ALL_classes = (curr_class+1);
		cout << "--Exp classes " << (curr_class+1) << endl;
	}
	return scores;
}


