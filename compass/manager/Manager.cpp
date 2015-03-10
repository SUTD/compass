/*
 * Manager.cpp
 *
 *  Created on: Aug 14, 2008
 *      Author: tdillig
 */

#include "Manager.h"
#include "sail/Function.h"
#include "Summary.h"
#include "SummaryRetriever.h"
#include "util.h"
#include "Callgraph.h"
#include "DataManager.h"
#include "GenericSummary.h"
#include "SummaryStream.h"
#include "SummaryFetcher.h"
#include "sail/SummaryUnit.h"
#include "sail/SuperBlock.h"
#include "sail/Cfg.h"
#include "SummaryGraph.h"
#include "compass-serialization.h"
#include "AccessPath.h"
#include "Identifier.h"

#include <sys/prctl.h>

#include "AnalysisResult.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

using namespace sail;

#define VERBOSE false

#define CANCEL_EXIT_STATUS 9

int read_restart(int fd, void *buf, size_t count)
{
	while(true)
	{
		int res = read(fd, buf, count);
		if(res>= 0) return res;
		assert(errno == 4);
	}

}


class Compass;

bool CompareCGNode::operator()(const CGNode *c1, const CGNode* c2) const
{

	return (c1->cg_id < c2->cg_id);

}

const map<Identifier, AnalysisResult*>& Manager::get_results()
{
	return analysis_results;
}

const vector<Error*> & Manager::get_errors()
{
	return reported_errors;
}


Manager::Manager(Callgraph* cg, bool fixpoint,
		Analysis *a, DataManager* dm, int num_threads, int timeout,
		const Identifier & entry_point, set<Identifier>* excluded_functions,
		void (*start_func)(const Identifier & id),
		void (*end_func)(const Identifier & id, AnalysisResult * ar))
{
	start(cg, fixpoint, a,  dm, num_threads, timeout, entry_point,
			excluded_functions, start_func, end_func);
}

void Manager::start(Callgraph* cg, bool fixpoint,
		Analysis *a, DataManager* dm, int num_threads, int timeout,
		const Identifier & entry_point,
		set<Identifier>* excluded_functions,
		void (*start_func)(const Identifier & id),
		void (*end_func)(const Identifier & id, AnalysisResult * ar))
{
	this->canceled = false;
	this->start_func = start_func;
	this->end_func = end_func;
	this->timeout = timeout;
	this->entry_point = entry_point;
	this->excluded_functions = excluded_functions;
	this->cg = cg;
	this->dm = dm;
	this->fixpoint = fixpoint;
	this->num_threads = num_threads;
	this->analysis = a;


	analysis->register_summary_callbacks();
	progress_queue_empty.signal();

	bottom_up = analysis->get_order() == BOTTOM_UP;
	init_worklists();
	init_worker_to_function_array();
	init_num_analyzed_map();
	this->function_ready_cond = new Semaphore[num_threads];
	spawn_workers();

}

void Manager::init_worklists()
{

	bool entry_point_specified = !entry_point.is_empty();
	set<CGNode*> reachable_nodes;
	if(entry_point_specified) {
		CGNode* n = cg->get_node(entry_point);
		cg->get_transitive_callees(n, reachable_nodes);
	}

	map<Identifier, CGNode*>::iterator it = cg->get_nodes().begin();
	for(; it!= cg->get_nodes().end(); it++)
	{
		CGNode* cur = it->second;
		const Identifier& cur_id = it->first;
		if(entry_point_specified && reachable_nodes.count(cur) == 0)
			continue;

		if(this->excluded_functions != NULL &&
				excluded_functions->count(cur_id) > 0)
			continue;

		if(bottom_up && dependencies_met_bottomup(cur, true)){
			resolved_dependencies_queue.insert(cur);
			function_available.signal();
		}
		else if(!bottom_up && dependencies_met_topdown(cur, true)){
			resolved_dependencies_queue.insert(cur);
			function_available.signal();
		}
		else {
			unresolved_dependencies_queue.insert(cur);
		}
	}
}

void Manager::add_to_resolved_dependencies_queue(CGNode* node)
{
	unique_lock<mutex> queue_lock(queue_mutex);
	unresolved_dependencies_queue.erase(node);
	resolved_dependencies_queue.insert(node);
	function_available.signal();
}

void Manager::init_worker_to_function_array()
{
	this->worker_to_function_array = new CGNode*[num_threads];
	for(unsigned int i=0; i<num_threads; i++){
		worker_to_function_array[i] = NULL;
	}
}


void Manager::cancel()
{

	unique_lock<mutex> lock(cancel_mutex);
	canceled = true;
	set<int>::iterator it = current_pids.begin();
	for(;it != current_pids.end(); it++)
	{
		int p_id = *it;
		kill(p_id, CANCEL_EXIT_STATUS);
	}
}

void Manager::spawn_workers()
{
	thread server = thread(bind(&Manager::server_main, this));
	for(int i=0; i<(int)num_threads; i++)
	{
		workers[i] = thread(bind(&Manager::worker_main, this, i));
	}


	server.join();

	/*
	 * Signal all workers to quit by adding NULL
	 * as their CGNode.
	 */
	for(unsigned int i=0; i<num_threads; i++){
		{
			unique_lock<mutex> lock(worker_to_function_mutex);
			worker_to_function_array[i] = NULL;
		}

		function_ready_cond[i].signal();
	}

	/*
	 * Wait for all workers to be done.
	 */
	for(int i=0; i<(int)num_threads; i++)
	{
		workers[i].join();
	}


}

CGNode* Manager::get_next_node()
{

	bool wait = false;
	{
		unique_lock<mutex> lock(queue_mutex);
		if(resolved_dependencies_queue.size() == 0 &&
				unresolved_dependencies_queue.size() == 0 &&
				in_progress_queue.size() == 0
				) return NULL;
		if(resolved_dependencies_queue.size() == 0 &&
				unresolved_dependencies_queue.size() == 0 &&
				in_progress_queue.size() != 0) wait = true;
	}
	if(wait){
		progress_queue_empty.wait();
		unique_lock<mutex> lock(queue_mutex);
		if(resolved_dependencies_queue.size()==0) return NULL;
		progress_queue_empty.signal();
	}


	/*
	 * Stall until there is a function whose dependencies have been
	 * processed.
	 */
	if(VERBOSE && resolved_dependencies_queue.size()==0 &&
			unresolved_dependencies_queue.size() >0) {
		CGNode* n = (*unresolved_dependencies_queue.begin());
		cerr<< n->id << endl;
		cerr << "****Callees : " << endl;
		map<CGNode*, cg_edge_id>::iterator it = n->callees.begin();
		for(; it!= n->callees.end(); it++){
			cerr << "\t " << (it->first)->id << endl;
		}
	}

	function_available.wait();

	unique_lock<mutex> lock(queue_mutex);
	assert(resolved_dependencies_queue.size() > 0);
	CGNode* to_process = NULL;

	if(bottom_up) {
		to_process = *resolved_dependencies_queue.begin();
	}
	else{
		to_process = *resolved_dependencies_queue.rbegin();
	}
	if(VERBOSE)
		cerr << "Enquing " << to_process->id << "..." << endl;

	in_progress_queue.insert(to_process);
	progress_queue_empty.down();
	resolved_dependencies_queue.erase(to_process);
	return to_process;


}

void Manager::server_main()
{
	while(true){
		{
			unique_lock<mutex> lock(cancel_mutex);
			if(canceled) break;
		}
		int thread_id;

		{
			CGNode* to_process = get_next_node();

			if(to_process == NULL) break; // we are done
			worker_ready_cond.wait();


			unique_lock<mutex> lock(worker_to_function_mutex);
			thread_id = find_ready_worker();
			worker_to_function_array[thread_id] = to_process;
		}
		function_ready_cond[thread_id].signal();
	}

	for(unsigned int i=0; i <num_threads; i++)
	{
		worker_ready_cond.wait();
	}

}

int Manager::find_ready_worker()
{
	for(int i=0; i<(int)num_threads; i++)
	{
		CGNode* node = worker_to_function_array[i];
		if(node==NULL) return i;
	}
	assert(false);
}

void Manager::worker_main(int thread_id)
{

	mutex m;
	worker_ready_cond.signal();

	if(VERBOSE) cout << "Thread started; id: " << thread_id << endl;
	while(true)
	{
		{

			unique_lock<mutex> lock(m);
			// Wait for a function to analyze
			function_ready_cond[thread_id].wait();
		}
		CGNode* node;
		{
			unique_lock<mutex> lock(worker_to_function_mutex);
			// Node is the function to analyze
			 node = worker_to_function_array[thread_id];
		}

		/*
		 * Are we told to exit ?
		 */
		if(node == NULL)
			return;
		if(VERBOSE)
			cerr << "Worker " << thread_id << " is processing function " <<
			node->id<< endl;
		int num_analyzed;
		if(this->fixpoint)
		{
			unique_lock<mutex> lock(num_analyzed_mutex);
			num_analyzed = num_analyzed_map[node];
			num_analyzed_map[node] = ++num_analyzed;
		}



		CGNode* fun_cg_node = cg->get_node(node->id.get_function_identifier());


		Function* f= (Function*) dm->get_data(node->get_function_serial_id(),
				SAIL_FUNCTION);
		assert(f != NULL);
		SummaryRetriever* sr = new SummaryRetriever(dm, node, analysis);
		SummaryUnit* su;
		if(node->is_function())
			su = f;
		else
			su = f->get_cfg()->get_superblock(node->id);




		SummaryStream* s = start_analysis_process(su, sr, !node->has_callers() ||
				node->id == entry_point,
				node, fun_cg_node->cg_id);

		if(VERBOSE)
			cerr << "Worker " << thread_id << " finished function " <<
			node->id  <<  endl;


		dm->mark_unused(node->get_function_serial_id(), SAIL_FUNCTION);
		delete sr;

		sum_data_type dt = analysis->get_summary_type();
		if(!fixpoint)
		{

			dm->write_data(node->get_serial_id(), dt, s);
			if(node->is_function())
				dm->data_finalized(node->get_function_serial_id(), SAIL_FUNCTION);
			unique_lock<mutex> lock(queue_mutex);
			in_progress_queue.erase(node);
			progress_queue_empty.signal();
			move_resolved_nodes(node);
		}
		else
		{
			bool sum_changed = false;
			if(dt != NO_DATA){
				sum_changed = false;
				dm->mark_unused(node->get_serial_id(), dt);
			}
			{
				unique_lock<mutex> lock(queue_mutex);
				bool cur_done = false;



				if(sum_changed)
				{
					cur_done = bottom_up ? enqueue_callers(node) :
							enqueue_callees(node);
				}
				in_progress_queue.erase(node);
				progress_queue_empty.signal();

				if(!sum_changed || cur_done) {
					move_resolved_nodes(node);
				}

			}

			if(sum_changed)
				dm->write_data(node->get_serial_id(), dt, s);
			else
			{
				if(node->is_function()) {
					dm->data_finalized(node->get_function_serial_id(),
							SAIL_FUNCTION);
				}
				delete s;
			}


		}

		if(dt != NO_DATA){
			if(bottom_up){
				finalize_callees(node);
				if(node->callers.size() == 0) {
					dm->mark_unused(node->get_serial_id(),
						analysis->get_summary_type());
					dm->data_finalized(node->get_serial_id(),
						analysis->get_summary_type());
				}
			}
			else {
				finalize_callers(node);
				if(node->callees.size() == 0) {
					dm->mark_unused(node->get_serial_id(),
						analysis->get_summary_type());
					dm->data_finalized(node->get_serial_id(),
						analysis->get_summary_type());
				}
			}
		}
		unique_lock<mutex> lock(worker_to_function_mutex);
		worker_to_function_array[thread_id] = NULL;
		worker_ready_cond.signal();

	}

}



SummaryStream* Manager::start_analysis_process(SummaryUnit* f,
		SummaryRetriever* sr, bool report_errors, CGNode* node, int fun_cg_id)
{
	int comm_pipe[2];
	int status_pipe[2];
	int sum_requested_pipe[2];


	int pipe_status = pipe(comm_pipe);
	assert(pipe_status == 0);
	int status_status = pipe(status_pipe);
	assert(status_status == 0);
	int sum_requested = pipe(sum_requested_pipe);
	assert(sum_requested == 0);

	if(start_func != NULL)
		(*start_func)(f->get_identifier());





	map<call_id, SummaryStream*> summaries;
	set<call_id> init_ids;


	const map<CGNode*, cg_edge_id> & callees = node->get_callees();
	map<CGNode*, cg_edge_id>::const_iterator it = callees.begin();
	for(; it!= callees.end(); it++)
	{
		if(!fixpoint && it->second.is_backedge()) continue;
		CGNode* key = it->first;
		call_id ci(key->id);
		if(cg->get_init_functions().count(key) > 0) {
			init_ids.insert(ci);
		}
		SummaryStream* ss = sr->get_summary(key->get_serial_id(),
				analysis->get_summary_type());
		summaries[ci] = ss;

	}
		SummaryFetcher* sf = new SummaryFetcher(&summaries, init_ids,
				cg->get_stubs());



	int p_id;
	{
		unique_lock<mutex> lock(cancel_mutex);
		if(canceled){
			close(comm_pipe[0]);
			close(comm_pipe[1]);

			close(status_pipe[0]);
			close(status_pipe[1]);

			close(sum_requested_pipe[0]);
			close(sum_requested_pipe[1]);
			return NULL;
		}
		p_id = fork();
		if(p_id != 0) current_pids.insert(p_id);
	}
	if(p_id == 0)
	{
		//make sure we die if parent dies
		prctl(PR_SET_PDEATHSIG, SIGHUP);
		AccessPath::clear();

		run_analysis_process(f, status_pipe, comm_pipe, sum_requested_pipe,
				report_errors, *sf, fun_cg_id);


		_exit(0);
	}
	delete sf;

	thread crash_handler = thread(bind(&Manager::crash_handler_main, this, p_id,
					status_pipe));




	SummaryStream* res = NULL;
	AnalysisResult* ar = NULL;



	while(true)
	{
		analysis_status_type cur_status;
		int t = read_restart(status_pipe[0],
				&cur_status, sizeof(analysis_status_type));

		if(t < 0 || cur_status == STATUS_ERROR)
		{
			if(t<0)
				{cerr << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT < 0 "<<
					f->get_identifier().to_string() <<  "   " <<
					strerror(errno) << endl;
				assert(false);

				}
			/*
			 * Was this crash the result of a cancel terminating the
			 * process? In this case, we do not report an error.
			 */
			{
				unique_lock<mutex> lock(cancel_mutex);
				if(canceled) break;
			}

			int error_code;
			assert(read_restart(status_pipe[0], &error_code, sizeof(int))!=-1);
			ar = new AnalysisResult();
			if (t< 0) error_code = -1;
			ar->termination_value = error_code;
			string res = f->get_identifier().to_string() + " terminated with code " +
					int_to_string(error_code);
			ar->errors.push_back(new Error(-1,
					f->get_identifier().get_file_id().to_string(),
					ERROR_CRASH, res, Identifier()));
			break;
		}

		else if(cur_status == STATUS_SUMMARY_REQUEST)
		{
			string requested_function_id;
			while(true)
			{
				char c;
				assert(read_restart(comm_pipe[0], &c, sizeof(char))!=-1);
				if(c == '\0') break;
				requested_function_id += c;
			}
			SummaryStream *s = sr->get_summary(requested_function_id,
					analysis->get_summary_type());

			if(s == NULL)
			{
				int size = 0;
				assert(write(comm_pipe[1], &size, sizeof(int))!=-1);

			}
			else
			{
				stringstream& ss = s->get_data();
				int size = ss.str().size();
				const string & data = ss.str();
				assert(write(comm_pipe[1], &size, sizeof(int))!=-1);
				assert(write(comm_pipe[1], data.c_str(), size)!=-1);
			}

			/*
			 * Notify the SummaryFetcher that it is time to pick up
			 * the written data.
			 */
			int dummy = 0;
			assert(write(sum_requested_pipe[1], &dummy, sizeof(int)) != -1);
		}
		else if(cur_status == STATUS_DONE)
		{
			cout << "DONE WITH " << f->get_identifier().to_string() << endl;
			/*
			 * First item in pipe is the summary
			 */

			int size;
			assert(read_restart(comm_pipe[0], &size, sizeof(int))!=-1);
			res = new SummaryStream();
			stringstream & ss = res->get_data();
			char buff[size];
			int filled = read_restart(comm_pipe[0], buff, size);
			assert(filled == size);
			for(int i = 0; i < size; i++)
			{
				ss << buff[i];
			}
			/*
			 * Second, there should be a serialized AnalysisResult;
			 */
			assert(read_restart(comm_pipe[0], &size, sizeof(int))!=-1);
			stringstream res_ss;
			char res_buff[size];
			filled = read_restart(comm_pipe[0], res_buff, size);
			assert(filled == size);
			for(int i = 0; i < size; i++)
			{
				res_ss << res_buff[i];
			}
			ar = load_analysis_result(res_ss);
			break;
		}
	}
	if(ar != NULL){
		unique_lock<mutex> lock(result_mutex);
		if(analysis_results.count(f->get_identifier()) > 0)
		{
			delete analysis_results[f->get_identifier()];
		}


		analysis_results[f->get_identifier()] = ar;
		reported_errors.insert(reported_errors.begin(), ar->errors.begin(),
				ar->errors.end());

		if(end_func != NULL){
			(*end_func)(f->get_identifier(), ar);
		}
	}
	{
		unique_lock<mutex> lock(cancel_mutex);
		current_pids.erase(p_id);
	}
	crash_handler.join();


	close(comm_pipe[0]);
	close(comm_pipe[1]);

	close(status_pipe[0]);
	close(status_pipe[1]);

	close(sum_requested_pipe[0]);
	close(sum_requested_pipe[1]);

	return res;
}


/*
 * Handle crashes in the child process by waiting for the child to terminate,
 * and writing an error code to the pipe if something goes
 * wrong in the child process.
 */
void Manager::crash_handler_main(int p_id, int* status_pipe)
{
	int child_status;

	waitpid(p_id, &child_status, WUNTRACED);


	if(child_status == 0) return;
	analysis_status_type ast = STATUS_ERROR;
	assert(write(status_pipe[1], &ast, sizeof(analysis_status_type))!=-1);
	assert(write(status_pipe[1], &child_status, sizeof(int))!=-1);

}



void Manager::run_analysis_process(SummaryUnit* f, int* status_pipe,
		int *comm_pipe, int* sum_requested_pipe, bool report_errors,
		SummaryFetcher& sf, int cg_id)
{
	//Set timeout for analysis

	alarm(timeout);
	AnalysisResult ar;
	SummaryGraph* sg = analysis->do_analysis(f, report_errors, &ar, &sf, cg_id,
			&cg->get_signature_to_callid_map(), cg->track_rtti());

	/*
	 * First, send the summary stream over the comm_pipe.
	 */
	SummaryStream sum_stream(sg);



	stringstream & ss = sum_stream.get_data();

	int size = ss.str().size();

	const string & data = ss.str();


	assert(write(comm_pipe[1], &size, sizeof(int))!=-1);


	analysis_status_type done = STATUS_DONE;
	assert(write(status_pipe[1], &done, sizeof(analysis_status_type))!=-1);

	assert(write(comm_pipe[1], data.c_str(), size)!=-1);

	/*
	 * Now, send the AnalysisResult over the same pipe, size fist.
	 */

	stringstream res_ss;

	write_analysis_results(res_ss, &ar);



	const string & data_res = res_ss.str();

	size = res_ss.str().size();
	assert(write(comm_pipe[1], &size, sizeof(int))!=-1);
	assert(write(comm_pipe[1], data_res.c_str(), size)!=-1);


}

void Manager::finalize_callees(CGNode* node)
{
	const map<CGNode*, cg_edge_id>& callees = node->get_callees();
	map<CGNode*, cg_edge_id>::const_iterator it = callees.begin();
	for(; it!= callees.end(); it++)
	{
		CGNode* callee = it->first;
		if(it->second.is_backedge()) continue;
		if(callers_done(callee)) {
			dm->mark_unused(callee->get_serial_id(),
								analysis->get_summary_type());
			dm->data_finalized(callee->get_serial_id(),
					analysis->get_summary_type());
		}
	}
}


void Manager::finalize_callers(CGNode* node)
{
	const map<CGNode*, cg_edge_id>& callers = node->get_callers();
	map<CGNode*, cg_edge_id>::const_iterator it = callers.begin();
	for(; it!= callers.end(); it++)
	{
		CGNode* caller = it->first;
		if(it->second.is_backedge()) continue;
		if(callees_done(caller)) {
			dm->mark_unused(caller->get_serial_id(),
								analysis->get_summary_type());
			dm->data_finalized(caller->get_serial_id(),
					analysis->get_summary_type());
		}
	}
}

/*
 * Move any nodes whose dependencies have been met from the unresolved queue
 * to the resolved queue.
 */
void Manager::move_resolved_nodes(CGNode* node)
{
	if(VERBOSE)
		cerr << "CHECKING IF CALLERS OF " << node->id.to_string() <<
		" ARE RESOLVED..." << endl;
	map<CGNode*, cg_edge_id>& dependents = bottom_up ?node->callers : node->callees;
	map<CGNode*, cg_edge_id>::iterator it = dependents.begin();
	for(; it!=dependents.end(); it++)
	{
		CGNode* cur = it->first;
		if(unresolved_dependencies_queue.count(cur) == 0) continue;
		if(resolved_dependencies_queue.count(cur)>0) continue;
		bool move = bottom_up? dependencies_met_bottomup(cur) :
			dependencies_met_topdown(cur);
		if(!move) {
			if(VERBOSE)
				cerr << "\t " << cur->id << " not resolved.." << endl;
			continue;
		}
		unresolved_dependencies_queue.erase(cur);
		resolved_dependencies_queue.insert(cur);
		if(VERBOSE)
			cerr << "\t " << cur->id << " resolved.."<< endl;
		function_available.signal();
	}

}

bool Manager::is_in_queue(CGNode* node, bool acquire_lock)
{
	if(acquire_lock)
			unique_lock<mutex> lock(queue_mutex);
	return(unresolved_dependencies_queue.count(node)>0 ||
	resolved_dependencies_queue.count(node)>0);
}

bool Manager::in_progress(CGNode* node, bool acquire_lock)
{
	if(acquire_lock)
			unique_lock<mutex> lock(queue_mutex);
	return(in_progress_queue.count(node)>0);
}



/*
 * Assumes the caller is holding the queue lock
 */
bool Manager::dependencies_met_bottomup(CGNode* node, bool init)
{

	map<CGNode*, cg_edge_id>::iterator it = node->callees.begin();
	for(; it!=node->callees.end(); it++)
	{
		CGNode* callee = it->first;
		bool backedge = it->second.is_backedge();
		if(backedge){
			continue;
		}
		if(init) return false;
		if(is_in_queue(callee, false) || in_progress(callee, false))
			return false;
	}

	return true;
}

/*
 * Assumes the caller is holding the queue lock
 */
bool Manager::dependencies_met_topdown(CGNode* node, bool init)
{

	map<CGNode*, cg_edge_id>::iterator it = node->callers.begin();
	for(; it!=node->callers.end(); it++)
	{
		CGNode* caller = it->first;
		bool backedge = it->second.is_backedge();
		if(backedge) continue;
		if(init) return false;
		if(is_in_queue(caller, false) || in_progress(caller, false))
			return false;
	}
	return true;
}

bool Manager::enqueue_callers(CGNode* node)
{
	bool node_done = true;

	map<CGNode*, cg_edge_id>::iterator it = node->callers.begin();
	for(; it!= node->callers.end(); it++)
	{
		CGNode* caller = it->first;
		if(caller == node) node_done = false;

		if(dependencies_met_bottomup(caller)){
			unresolved_dependencies_queue.erase(caller);
			resolved_dependencies_queue.insert(caller);
			function_available.signal();
		}
		else if(!is_in_queue(caller, false)){
			unresolved_dependencies_queue.insert(caller);
			if(VERBOSE)
				cout << "-> Inserting into unresolved queue: " <<
					caller->id << endl;

		}
	}
	return node_done;

}



bool Manager::enqueue_callees(CGNode* node)
{
	bool node_done = true;
	map<CGNode*, cg_edge_id>::iterator it = node->callees.begin();
	for(; it!= node->callees.end(); it++)
	{
		CGNode* callee = it->first;
		if(callee == node) node_done = false;
		if(dependencies_met_topdown(callee)){
			unresolved_dependencies_queue.erase(callee);
			resolved_dependencies_queue.insert(callee);
			function_available.signal();
		}
		else if(!is_in_queue(callee, false)){
			unresolved_dependencies_queue.insert(callee);

		}
	}
	return node_done;
}


bool Manager::summary_changed(Summary* old_sum, Summary* new_sum)
{
	return (fixpoint && (old_sum == NULL || !(*old_sum == *new_sum)));
}


void Manager::init_num_analyzed_map()
{
	map<Identifier, CGNode*>::iterator it = cg->get_nodes().begin();
	for(; it!= cg->get_nodes().end(); it++){
		CGNode* node = it->second;
		num_analyzed_map[node]=0;
	}
}

/*
 * Are we all done with the callers (resp. callees)
 * of this CGNode?
 */
bool Manager::callers_done(CGNode* node)
{

	const map<CGNode*, cg_edge_id>& callers =  node->get_callers();
	map<CGNode*, cg_edge_id>::const_iterator it = callers.begin();


	unique_lock<mutex> lock(queue_mutex);

	for(; it!= callers.end(); it++)
	{
		CGNode* p = it->first;
		if(it->second.is_backedge()) continue; // assume we are done with this one
		if(in_progress_queue.count(p) > 0) return false;
		if(unresolved_dependencies_queue.count(p) > 0) return false;
		if(resolved_dependencies_queue.count(p) > 0) return false;

	}

	return true;


}
bool Manager::callees_done(CGNode* node)
{
	const map<CGNode*, cg_edge_id>& callees =  node->get_callees();
	map<CGNode*, cg_edge_id>::const_iterator it = callees.begin();


	unique_lock<mutex> lock(queue_mutex);

	for(; it!= callees.end(); it++)
	{
		CGNode* p = it->first;
		if(it->second.is_backedge()) continue; // assume we are done with this one
		if(in_progress_queue.count(p) > 0) return false;
		if(unresolved_dependencies_queue.count(p) > 0) return false;
		if(resolved_dependencies_queue.count(p) > 0) return false;

	}

	return true;
}



Manager::~Manager()
{
	delete[] function_ready_cond;
	delete[] worker_to_function_array;

}
