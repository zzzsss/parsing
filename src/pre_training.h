/*
 * pre_training.h
 *
 *  Created on: Jul 11, 2014
 *      Author: zzs
 */

#ifndef PRE_TRAINING_H_
#define PRE_TRAINING_H_

#include <string>
#include "DependencyInstance.h"
namespace pre_training_space{
	extern void pre_training();
	extern std::string* get_feature(DependencyInstance* x,int i,int j);
}



#endif /* PRE_TRAINING_H_ */
