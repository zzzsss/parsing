/*
 * main3test.cpp
 *
 *  Created on: Jul 17, 2014
 *      Author: zzs
 */

//test the x.bin

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

void usage23 (MachConfig &mc, bool do_exit=true)
{
   cout <<  endl
        << "cslm_train - a tool to train continuous space language models" << endl
	<< "Copyright (C) 2014 Holger Schwenk, University of Le Mans, France" << endl << endl;

  mc.print_help();
  if (do_exit) exit(1);
}

int main3test(int argc, char *argv[])
{
  MachConfig mach_config(true);
  string mach_fname, test_fname, dev_fname;
  int curr_it = 0;
  Mach *mlp;

  // select available options
  mach_config
    .sel_cmdline_option<std::string>("mach,m"        , true )
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
    usage23(mach_config);
  else {
    if (mach_config.parsing_error())
      usage23(mach_config, false);
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


  Data* data_train = new Data(dev_fname.c_str());
  int total = data_train->GetNb();
  REAL* output = new REAL[total];
  mlp->evaluate(data_train->get_allinput(),output,total,CONF_X_dim,CONF_Y_dim);

  ofstream fout("score_main3_out.list");
  fout << total << endl;
  for(int i=0;i<total;i++)
	  fout << output[i] << endl;
  fout.close();
  return 0;
}
