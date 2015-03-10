#include "CallManager.h"

using namespace std;

int main(int argc, char** argv){
	if(argc <=2){
		cout << "Please specify filename of .dex file to parse !" << endl;
	}

	string dex_file = argv[1];
	string out_put_folder = argv[2];

	sail::TranslationUnit* translation_unit = dex_to_sail(dex_file);
	vector<sail::Function*> funs = translation_unit->get_functions();

	string begin_time = get_time();

	CallManager callm(&funs);

	callm.doing_analysis(out_put_folder);

	cerr << "Start time (pointer_analysis) :: " << begin_time << endl;
	cerr << "End time (pointer_analysis):: " << get_time() << endl;


	return 0;
}




