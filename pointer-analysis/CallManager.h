#ifndef _CALL_MANAGER_H
#define _CALL_MANAGER_H

#include <set>

#include "MtdInstsProcessor.h"

#include <map>

#include "CircleSummaryUnit.h"


using namespace std;
class SummaryUnit;

class CircleSummaryUnit;
class MtdInstsProcessor;
class CallManager {
private:

	vector<sail::Function*>* funs;

	//built after first round analysis of methods;
	map<call_id, MtdInstsProcessor*> ci_mp;

	set<call_id> call_ids; //**



	map<call_id, set<call_id>*> call_graph_hierarchy; //**

	map<call_id, set<call_id>*> worklist; //**/

	map<call_id, SummaryUnit*> unresolved_callid_su; //**

	map<call_id, SummaryUnit*> resolved_callid_su;

	static lang_type cur_lang;

	map<call_id, int> duplicated_ids; //used to keep the latest index that duplicated id use;
	map<call_id, set<call_id>> dummy_duplicated_ids;
	map<int, call_id> dummy_ids; //index is the id in funs vector;

	set<info_item> alloc_site;
	//map<int, call_id> irreducible_method; //which treated as empty method;
	set<call_id> irreducible_method;

public:
	ofstream ofs;

	CallManager(vector<sail::Function*>* fuc) {
		funs = fuc;
		open_file(ofs, "/scratch/test");
	}

	~CallManager() {
		ofs.close();
	}

	void doing_analysis(const string& out_put_folder);


	void collect_call_id_summary_unit(const call_id&, SummaryUnit* su);

	bool is_dummy_id(int f_idx);

	call_id& get_dummy_id(int f_idx);

	bool is_dummy_callid(call_id& callid);
	set<call_id>& get_dummy_duplicated_ids(call_id& callid);

	bool inside_collect_call_ids(call_id & call_id) const;

	void collect_ci_mp(const call_id&,
			MtdInstsProcessor*);

	bool is_resolved(const call_id& call_id) const{
		return (resolved_callid_su.count(call_id) >0) ;
	}

	SummaryUnit* get_resolved_summary_unit(const call_id& call_id){
		return resolved_callid_su[call_id];
	}

	//each time process instruction that alloc a site,
	//we need a different allo_site for it;
	void generate_different_allocsite_id(int line_number, int instruction_idx,
				Identifier& id, alloc_info& alloc_info);

	const bool is_irreducible_method(const call_id& call_id){
		return irreducible_method.count(call_id) > 0;
	}

private:


	void maps_init();
	void print_call_graph_hierarchy()const;
	void print_work_list() const;
	void first_round_functions_analysis(const string& output_folder);

	void interprocedural_analysis(const string& output_folder);

	void delete_callee_from_caller_for_circle(map<call_id, call_id>&
			caller_callee);

	unsigned int get_method_idx_from_call_id(const call_id& call_id);
	string to_string(const call_id& call_id);



	/**************
	 * para: vector<call_id>& terminals hold terminals;
	 * bool circle_t denote if those terminals produce circle;
	 *
	 * Functionality: move terminals from unresolved map to resolved_map;
	 */
	void terminals_to_resolved_map(vector<call_id>& terminals,
			bool circle_t, const string& output_f);






	/********
	 * parameter: vector<call_id>& terminals
	 *
	 * Functionality: find terminals in call_graph and put them into vector of terminals;
	 */
	void collect_leaves_in_call_graph(vector<call_id>& terminals);






	/******
	 * para: vector<call_id>& terminals have information of current terminals,
	 * para: vector<call_id>& newest_terms, need to hold the new generated terminals
	 * para: denote if first vector's elements  is circle
	 *
	 *
	 * Functionality: update all caller's of terminals
	 * (except for those in the circle(they are updated in fix-point stage))
	 * build the element of second paramter;
	 */
	void terminal_update_callers_and_collect_new_terminals(
			vector<call_id>& terminals, vector<call_id>& newest_terms,
			bool circle, const string& output_folder, int& idx);






	/**************************************
	 * para: callerid
	 * para: calleeid
	 *
	 * functionality::calleeid corresponding summaryUnit to
	 * update callerid's correponding summaryUnit
	 **************************************/
	void update_caller_with_callee(const call_id& callerid,
			const call_id& calleeid, const bool inside_circle = false);






	/******
	 * para: circle_ids
	 *
	 *
	 * functionality: filled circle_ids based on current worklist
	 ***********/
	void find_one_circle_from_worklist(vector<call_id>& circle_ids);









	/*********
	 * para: call_id& cur_callid :: current_point when traversing
	 * para: call_id& start_callid :: start_point
	 *
	 * para: tracked_ids (output), tracked_ids contain the traversing path;
	 *
	 * para: circle_flag used to track if return by the branch of find target
	 *
	 * Functionality: recursive function,
	 * can traversing start_callid produce a circle?
	 */
	void find_circle_in_term_of_startpoint(const call_id& cur_callid,
			vector<call_id>& tracked_ids,
			bool & circle_flag);









	/************
	 *  delete "callee_id" from "caller_id", then see if caller_id is suit for
	 *  being added to newest_terms;
	 *
	 *  caller_in_circle denotes if caller_id also build circle with callee?
	 */
	void update_call_graph_hierarchy(const call_id& caller_id,
			const call_id& callee_id, vector<call_id>& newest_terms,
			bool caller_in_circle);




	void first_round_fun_analysis(const unsigned int& i,
			const string& output_f);

	void collect_all_ids();

	void collect_call_graph_hierarchy();

	void circle_summary_units_init(CircleSummaryUnit& csu,
			vector<call_id>& circle_ids);

	string all_ids_to_string()const;
	string call_graph_hierarchy_to_string() const;

	string worklist_to_string() const;

	const string unresolved_summary_unit_to_string() const;
	const string resolved_summary_unit_to_string() const;

	const string unresolved_fc_for_each_method_to_string()const;

	const string duplicated_dummy_ids_to_string() const;
	const string dummy_ids_to_string()const;




};

#endif
