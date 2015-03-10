/*
 * Manager.h
 *
 *  Created on: Aug 14, 2008
 *      Author: tdillig
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <boost/thread.hpp>
#include <vector>
#include <set>
using namespace std;
namespace sail{
	class Function;
	class SummaryUnit;
}

class SummaryRetriever;

#include "Semaphore.h"
#include "Analysis.h"
#include "Identifier.h"
class Compass;

using namespace std;
using namespace boost;

#define DEFAULT_NUM_THREADS 8
#define MAX_NUM_THREADS 50

class Callgraph;
class DataManager;
class CGNode;
class SummaryStream;
class Error;
class AnalysisResult;
class Identifier;

enum analysis_status_type
{
	STATUS_ERROR,
	STATUS_SUMMARY_REQUEST,
	STATUS_DONE
};

class CompareCGNode:public binary_function<CGNode*, CGNode*, bool> {

public:
	bool operator()(const CGNode* f1, const CGNode* f2) const;
};

class Manager {
private:
	bool fixpoint;

	mutex cancel_mutex;
	volatile bool canceled;
	set<int> current_pids;


	unsigned int num_threads;

	Analysis* analysis;
	thread workers[MAX_NUM_THREADS];
	bool bottom_up;

	Callgraph *cg;
	DataManager* dm;

	/*
	 * The reported_errors and analysis_results
	 * are protected by the result_mutex.
	 */
	vector<Error*> reported_errors;
	map<Identifier, AnalysisResult*> analysis_results;
	mutex result_mutex;

	mutex tt;






	/*
	 * A CGNode N is in this set if it needs to be analyzed but has unmet
	 * dependencies. For example, if foo needs to be analyzed, but it needs to
	 * use bar's summary which we don't have yet, then foo is in the
	 * unresolved dependencies queue.
	 */
	set<CGNode*, CompareCGNode> unresolved_dependencies_queue;

	/*
	 * A CGNode N is in this queue if it needs to be analyzed and is
	 * ready to be analyzed, i.e., has all the summaries etc. it needs.
	 */
	set<CGNode*, CompareCGNode> resolved_dependencies_queue;

	/*
	 * The CGNode's that are currently being analyzed.
	 */
	set<CGNode*, CompareCGNode> in_progress_queue;


	Semaphore function_available;
	mutex queue_mutex;

	/*
	 * Semaphore to tell the server thread that worker is ready
	 */
	Semaphore worker_ready_cond;

	Semaphore progress_queue_empty;

	/*
	 * The server places the function a given worker should analyze
	 * in this array; worker_to_function_mutex is used to
	 * prevent data races on this array.
	 */
	CGNode** worker_to_function_array;
	mutex worker_to_function_mutex;

	map<CGNode*, unsigned short> num_analyzed_map;
	mutex num_analyzed_mutex;

	/*
	 * An array of condition_variables to signal to
	 * the corresponding thread that its Function* is ready.
	 */
	Semaphore* function_ready_cond;


	Identifier entry_point;
	set<Identifier>* excluded_functions;

	int timeout;



	void (*start_func)(const Identifier & id);
	void (*end_func)(const Identifier & id, AnalysisResult * ar);




public:
	Manager(){}
	void start(Callgraph* cg, bool fixpoint,
			Analysis *a, DataManager* dm, int num_threads,
			int timeout, const Identifier & entry_point,
			set<Identifier>* excluded_functions,
			void (*start_func)(const Identifier & id) = NULL,
			void (*end_func)(const Identifier & id, AnalysisResult * ar) = NULL	);

	Manager(Callgraph* cg, bool fixpoint,
			Analysis *a, DataManager* dm, int num_threads,
			int timeout, const Identifier & entry_point,
			set<Identifier>* excluded_functions,
			void (*start_func)(const Identifier & id) = NULL,
			void (*end_func)(const Identifier & id, AnalysisResult * ar) = NULL	);



	/*
	 * Cancels the current analysis. Returns once all manager threads/processes
	 * are killed.
	 */
	void cancel();

	virtual ~Manager();

	/*
	 * These methods are only safe to call once the analysis has terminated.
	 */
	const map<Identifier, AnalysisResult*>& get_results();
	const vector<Error*> & get_errors();
private:
	void spawn_workers();
	void worker_main(int thread_id);
	void server_main();
	inline int find_ready_worker();
	void init_worker_to_function_array();
	void init_num_analyzed_map();
	bool summary_changed(Summary* old_sum, Summary* new_sum);
	void init_worklists();
	CGNode* get_next_node();
	/* If this function returns true, it means that this function has not
	 * been reenqueued and its callers num_dependents_analyzed can be
	 * incremented */
	bool enqueue_callers(CGNode* node);
	bool enqueue_callees(CGNode* node);

	bool dependencies_met_bottomup(CGNode* node, bool init=false);
	bool dependencies_met_topdown(CGNode* node, bool init=false);

	void add_to_resolved_dependencies_queue(CGNode* node);
	void move_resolved_nodes(CGNode* node);

	bool is_in_queue(CGNode* node, bool acquire_lock = true);
	bool in_progress(CGNode* node, bool acquire_lock = true);


	SummaryStream* start_analysis_process(sail::SummaryUnit* f,
			SummaryRetriever* sr, bool report_errors, CGNode* node,
			int fun_cg_id);

	void run_analysis_process(sail::SummaryUnit* f,
			int* status_pipe, int *comm_pipe, int* sum_requested_pipe,
			bool report_errors, SummaryFetcher& sf, int cg_id);

	void crash_handler_main(int p_id, int* status_pipe);

	/*
	 * If we are done with the current CGNode, then:
	 * 1) For a bottom-up analysis, if f is a callee of this CGNode,
	 * and all callers of f are also done, then we can write summary to disk.
	 * 2) Similarly, for a top-down analysis, we can write the caller's summary
	 * to disk.
	 */
	void finalize_callees(CGNode* node);
	void finalize_callers(CGNode* node);

	/*
	 * Are we all done with the callers (resp. callees)
	 * of this CGNode? (Assumes we are already done with this node!!)
	 */
	bool callers_done(CGNode* node);
	bool callees_done(CGNode* node);
};

#endif /* MANAGER_H_ */
