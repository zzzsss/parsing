/*
 * Data.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: zzs
 */

//really simplified version of the original Data and other classes
#include <fstream>
#include "Data.h"
using namespace std;

#define DATA_LINE_LEN 10000
#define OP_X "x"	//x file
#define OP_Y "y"	//y file
#define OP_IN "in-dim"
#define OP_OUT "out-dim"
#define OP_TOTAL "total"

Data::Data(const char *p_fname, Data *other_data)
{
	fname = p_fname;
	//5 events in data description file
	string p_x="",p_y="";
	int in=-1,out=-1,total=-1;
	cout << "Opening data description '" << fname << "'" << endl;
	ifstream ifs;
	ifs.open(fname,ios::in);
	CHECK_FILE(ifs,fname);
	while (!ifs.eof()){
		string buf;
		char line[DATA_LINE_LEN];
		ifs >> buf;
		if (buf[0]=='#') {ifs.getline(line, DATA_LINE_LEN); continue;} // skip comments
		if (buf=="") continue; // HACK
		if(buf==OP_X)
			ifs >> p_x;
		else if(buf==OP_Y)
			ifs >> p_y;
		else if(buf==OP_IN)
			ifs >> in;
		else if(buf==OP_OUT)
			ifs >> out;
		else if(buf==OP_TOTAL)
			ifs >> total;
		else{ifs.getline(line, DATA_LINE_LEN); continue;}
	}
	//if ok
	if(in>0 && out>0 && total>0 && p_x.size()>0 && p_y.size()>0){
		read_data(p_x.c_str(),p_y.c_str(),in,out,total);
	}
	else{
		Error("Bad data des file.");
	}
}

void Data::read_data(const char *p_x, const char *p_y,int in,int out,int total)
{
	//init
	fx=p_x;fy=p_y;idim=in;odim=out;idx=-1;nb_totl=total;
	//binary files
	ifstream x,y;
	x.open(fx,ios::binary |ios::in);
	y.open(fy,ios::binary |ios::in);
	if(x.fail() || y.fail()){
		Error("File opened fail, can't read Data");
	}
	//read header and num
	double magic;
	int total1,total2;
	int din,dout;
	//-magic
	x.read((char*)&magic,sizeof(magic));
	if(magic != MAGIC_HEADER){
		ErrorN("File %s no magic.",fx);
	}
	y.read((char*)&magic,sizeof(magic));
	if(magic != MAGIC_HEADER){
		ErrorN("File %s no magic.",fy);
	}
	//-dim
	x.read((char*)&din,sizeof(din));
	y.read((char*)&dout,sizeof(dout));
	if(din != idim || dout != odim){
		Error("X or Y does not match in dimension.");
	}
	//-total
	x.read((char*)&total1,sizeof(total1));
	y.read((char*)&total2,sizeof(total2));
	if(total1 != nb_totl || total2 != nb_totl){
		Error("X or Y does not match in num.");
	}

	//now read datas
	mem_inp = new REAL[idim*nb_totl];
	mem_trg = new REAL[odim*nb_totl];
	x.read((char*)mem_inp,idim*nb_totl);
	y.read((char*)mem_trg,odim*nb_totl);

	//ok
	cout << "Init Data ok..." << endl;
	x.close();
	y.close();
}

Data::~Data()
{
	delete []mem_inp;
	delete []mem_trg;
}

void Data::Shuffle()
{//not implemented
}

void Data::Rewind()
{
	Shuffle();
	idx=-1;
	input = target = 0;
}

bool Data::Next()
{
	if (idx >= nb_totl-1) return false;
	idx++;
    // just advance to next data in memory
	input = &mem_inp[idx*idim];
	if (odim>0) target = &mem_trg[idx*odim];
	//printf("DATA:"); for (int i =0; i<idim; i++) printf(" %5.2f", input[i]); printf("\n");
	return true;
}

//in fact, this function is just for writing, not for this class
void Data::write_array(const char *f,int total,int dim,REAL*a)
{
	ofstream x;
	x.open(f,ios::binary |ios::out|ios::trunc);
	if(x.fail()){
		Error("File opened fail, can't write Data");
	}
	double magic = MAGIC_HEADER;
	x.write((char*)&magic,sizeof(magic));
	x.write((char*)&dim,sizeof(dim));
	x.write((char*)&total,sizeof(total));
	x.write((char*)a,dim*total);
	x.close();
}

