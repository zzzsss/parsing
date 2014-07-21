#ifndef DependencyEvaluator_H
#define DependencyEvaluator_H
#include <string>
using std::string;

class DependencyEvaluator{
public:
	static void evaluate(string &act_file, string &pred_file, string &format, FILE *log_fp, bool labeled);
	static void evaluate(string &act_file, string &pred_file, string &format, bool labeled);
};
#endif
