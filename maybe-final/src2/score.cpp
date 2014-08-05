/*
 * score.cpp
 *
 *  Created on: Aug 4, 2014
 *      Author: zzs
 */
#include "common.h"

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
	for(int i=0;i<num;i++){
		//floating point number...
		scores[i] = ((int)(ashead[i]+0.1)>=(CONF_y_class_size-1))?(CONF_y_class_size-1):ashead[i];
	}
	return scores;
}


