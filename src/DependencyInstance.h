#ifndef DependencyInstance_H
#define DependencyInstance_H
#include<vector>
#include<string>
using namespace std;

/* An instance means one sentence
 * 	--- also simpilfied version
 */

class DependencyInstance{
private:
	void init();
public:
	//FeatureVector* fv;
	//string* actParserTree;
	vector<string*>* forms;
	//vector<string*>* lemmas;
	//vector<string*>* cpostags;
	//vector<string*>* postags;
	//vector<vector<string*>*>* feats;
	vector<int>* heads;
	//vector<string*>* deprels;
	//vector<RelationalFeature*>* relFeats;
	DependencyInstance();
	DependencyInstance(vector<string*>* forms, vector<int>* heads);
	~DependencyInstance();
	//void setFeatureVector(FeatureVector* fv);
	int length();
	string* toString();
	//void writeObject(ObjectWriter &out);
	//void readObject(ObjectReader &in);

};
#endif
