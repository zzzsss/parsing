#include"CONLLWriter.h"

CONLLWriter::CONLLWriter(){
}

void CONLLWriter::startWriting(const char* file){
	writer = fopen(file, "w");
}

void CONLLWriter::finishWriting(){
	fclose(writer);
}

void CONLLWriter::write(vector<string*> &forms,vector<int> &heads){
	int length = (int)(forms.size());
	string* str;
	for(int i = 0; i<length; ++i){
		fprintf(writer, "%d\t", i + 1);

		str = forms[i];
		fprintf(writer, "%s\t_\t", str->c_str());

		fprintf(writer, "_\t_\t_\t%d\t",  heads[i]);

		fprintf(writer, "_\n");
	}
	fputc('\n', writer);
}

