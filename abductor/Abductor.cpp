#include "Abductor.h"
#include <typeinfo>
#include "util.h"
#include "il/binop_expression.h"
#include "POE.h"
#include "Verify.h"
#include "fileops.h"


/*
 * Spurious Mistral dependencies -- fixme
 *
 */

Term* _make_ap(long int c)
{
	ConstantTerm* ct;
	ct = new ConstantTerm(c);
	return ct;
}

Term*_make_ap(const string & name)
{
	VariableTerm* vt = new VariableTerm(CNode::get_varmap().get_id(name));
	return vt;
}

Term* _make_ap(const map<Term*, long int>& elems,long int constant)
{
	ArithmeticTerm* at;
	at = new ArithmeticTerm(elems, constant);
	return at;
}

Term* _make_ap(const string & name, vector<Term*>& args, bool invertible)
{
	FunctionTerm* fv;
	fv = new FunctionTerm(CNode::get_varmap().get_id(name), args, invertible);
	return fv;
}

#include <map>
#include "LoopTree.h"

#define SAIL_GCC "~/research/gcc/bin/gcc"

double to_time(int ticks)
{
	return ((double)(ticks))/((double)CLOCKS_PER_SEC);
}


bool run_verify(string folder, string main_fn, double & pre_time,
		int & num_tries, double & init_time, double & unroll_time)
{

	int t1 = clock();
	Verify ver((char*)folder.c_str(), (char*)main_fn.c_str());



	LoopTree lt;
	ver.getLoopTree(lt);
	list<POEKey*> list_0;


	//cout << "\n\nVerification loop tree " << lt.to_string() << endl;

	lt.getAllLoopList(list_0);

	//map<POEKey* , Constraint*> invariant_map;
	map<POEKey* , map<int, Constraint*>* > invariant_map;

	list<POEKey*>::iterator list_riter;
	for(list_riter = list_0.begin(); list_riter != list_0.end(); list_riter++)
	{
		//cout << "list :" << (*list_riter)->to_string() << "\n*****\n";
		map<int, Constraint*>* temp_map = new map<int, Constraint*> ();
		temp_map->insert(pair<int , Constraint*>(1 , new Constraint(true)));
		invariant_map.insert(pair<POEKey *, map<int, Constraint*>* >((*list_riter), temp_map));
	}

	pre_time = to_time(clock()-t1);



	bool result = ver.do_verification(list_0, invariant_map);
	num_tries = ver.get_num_tries();

	int t_init = ver.get_init_po_time();
	int t_unroll = ver.get_loop_unroll_time();
	init_time = to_time(t_init);
	unroll_time = to_time(t_unroll);



	if(result == true) {
		map<POEKey* , map<int, Constraint*>* >::iterator map_iter = invariant_map.begin();
		for(; map_iter != invariant_map.end(); map_iter++)
		{

			cout << "Block ID :" << map_iter->first->to_string();
			map<int, Constraint*>::iterator sec = (*map_iter).second->begin();
			cout << "\n[<< loop invariant >>]  : " << sec->second->to_string() << endl;
			cout << "\n"<< endl;

		}
		cout << "It's safe. :-)" << endl;
	}else {
		cout << "It's not safe. :-(" << endl;
	}
	return result;
}
string dd_to_string(double i)
{
	stringstream s;
	s << i;
	string res;
	s >> res;
	return res;
}

#define COL_SIZE 15

string get_whitespace(int k)
{
	string res;
	for(int i=0; i < k; i++)
		res+= " ";
	return res;
}

struct info
{


	static string to_header()
	{
		string res = "| Verified \t | Time \t | Pre Time \t | PO Pre Time \t "
				"|Unroll Time \t |# Tries \t |";
		return res;
	}
	double time;
	double pre_time;
	bool verified;
	int num_tries;
	double po_pre_time;
	double loop_unroll_time;
	info(bool v, double t, double pre_t, int n, double po_time,
			double unroll_time)
	{
		this->time = t;
		this->verified = v;
		this->pre_time = pre_t;
		this->num_tries = n;
		this->po_pre_time = po_time;
		this->loop_unroll_time = unroll_time;
	}
	string to_string()
	{
		string res = "| ";
		if(verified) res += "yes ";
		else res += "no  ";
		string t1 = dd_to_string(time);
		res += "\t | " + t1 + get_whitespace(COL_SIZE- t1.size());
		string t2 = dd_to_string(pre_time);
		res += " | " + t2 + get_whitespace(COL_SIZE- t2.size());

		string t3 = dd_to_string(po_pre_time);
		res += " | " + t3 + get_whitespace(COL_SIZE- t3.size());

		string t4 = dd_to_string(loop_unroll_time);
		res += " | " + t4 + get_whitespace(COL_SIZE- t4.size());

		res += " | " + int_to_string(num_tries) + " \t |";

		return res;
	}

};



/*
 * Run all *.c files in folder one by one, assuming that the function is called
 * main.
 */
void run_folder()
{
	map<string, info> results;


	set<string> c_files;
	int length = 0;
	collect_all_files_with_extension(c_files, "./", ".c");
	for(auto it = c_files.begin(); it != c_files.end(); it++) {
		string file = *it;
		if((int)file.size() > length) length = file.size();
		int x = system("rm -rf *.sail");
		string cmd = SAIL_GCC + string(" --sail=. ") + *it;
		x = system(cmd.c_str());
		int t1 = clock();
		double pre_time = 0.0;
		int num_tries;
		double po_pre;
		double loop_unroll;
		bool res = run_verify(".", "main", pre_time, num_tries, po_pre,
				loop_unroll);
		int t2 = clock();
		double time = ((double)(t2-t1))/((double)CLOCKS_PER_SEC);
		results.insert(make_pair(*it, info(res, time, pre_time, num_tries,
				po_pre, loop_unroll)));
		x = system("rm -rf *.sail");
		assert(x==x);
	}


	cout << "************** RESULTS********************" << endl;
	cout << "| Name" << get_whitespace(length-4) << info::to_header() << endl;
	for(auto it = results.begin(); it != results.end(); it++)
	{
		cout << "|" << it->first << get_whitespace(length- it->first.size()+1)
						<< it->second.to_string() << endl;
	}
	cout << "******************************************" << endl;

}


int main(int argc, char** argv) {
	if(argc >1)
	{
		string arg = argv[1];
		if(arg == "--folder"){
			run_folder();
			return 0;
		}
	}

	if (argc < 2) {
		cout << "No .sail file provided. " << endl;
		exit(1);
	}



	if(argc != 3)
	{
		cout << "[ERROR] Please provide 2 arguments. " << endl;
		cout << "The first argument indicates the " <<
				"folder which contains all sail files." << endl;
		cout << "The second argument indicates the main function." << endl;
		exit(EXIT_FAILURE);
	}
	double d;
	int i;
	run_verify(argv[1], argv[2], d, i, d, d);
	return 0;




}
