/*
 * which_main.h
 *
 *  Created on: Jul 16, 2014
 *      Author: zzs
 */

#ifndef WHICH_MAIN_H_
#define WHICH_MAIN_H_


//wihch feature 2 or 6
//#define WHICH_TWO
#define WHICH_SIX

#ifdef WHICH_TWO
#define main26 main
#endif

#ifdef WHICH_SIX
#define main26 main
#endif

//z.which main (again forgive my laziness...)
//#define main1 main26
//#define main2test main26
//#define main2trans main26
#define main2 main26
//#define main3 main26
//#define main3test main26



#endif /* WHICH_MAIN_H_ */
