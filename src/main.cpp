/*
 * main.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

/* the dependency pasing using MSTparser and NN */
#include "Eisner.h"
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include "pre_training.h"
#include "common.h"

int main1(int argc,char ** argv)
{
	srand(time(0));
	pre_training_space::pre_training();
	return 0;
}
