/*
 * which_score.h
 *
 *  Created on: Jul 16, 2014
 *      Author: zzs
 */

#ifndef WHICH_SCORE_H_
#define WHICH_SCORE_H_

//For the scoring, there are three ways
//1:no iteration,2:increase step,3:decrease step
#define SCO_WAY 1
//init for each pos/neg train sample
extern double SCO_EACH_FEAT_INIT;
extern double SCO_EACH_FEAT_INIT_NEG;

//adjusts amounts
extern double SCO_STEP;
extern double SCO_CHANGE_AMOUNT;
//extern int SCO_ENLARGE_TRIGGER_CHANGE;
#define SCO_STEP_HIGH 5.0
#define SCO_STEP_LOW 0.5
#define SCO_STEP_CHANGE 0.5
#define SCO_MAX_TIMES 9

//constrain the scores
#define CONSTRAIN_HIGH 1.25
#define CONSTRAIN_LOW -10


#endif /* WHICH_SCORE_H_ */
