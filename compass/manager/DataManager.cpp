/*
 * DataManager.cpp
 *
 *  Created on: Aug 17, 2008
 *      Author: tdillig
 */

#include "DataManager.h"
#include <fstream>
#include <malloc.h>
#include "sail/Function.h"
#include "sail/Serializer.h"
#include "Serializable.h"
#include "SummaryUnit.h"
#include <dirent.h>
#include <errno.h>

//#include "sail-serialization.h"
#include "compass-serialization.h"
#include "serialization-hooks.h"




void remove_file(string file, const string & base_dir)
{
	while(true)
	{
		if(file.size()<= base_dir.size()-1) break;
		remove(file.c_str());
		size_t slash_pos = file.rfind('/');
		if(slash_pos == string::npos)
			break;
		file = file.substr(0, slash_pos);

	}
}

/*
 * Controls after how many requests the memory check thread is run
 */
#define NUM_MEMCHECK_THREAD 100

#define VERBOSE false


using namespace sail;

DataManager::DataManager(int MB, map<sum_data_type, string> & types_to_dir)
{
	for(int dt = SAIL_FUNCTION; dt < SUM_END; dt++)
	{
		if(types_to_dir.count((sum_data_type)dt) == 0) {
			cerr << "Missing directory for summary type " <<
					file_extensions[dt] << endl;
			assert(false);
		}
		string cur = types_to_dir[(sum_data_type)dt];
		if(cur.size() == 0 || cur[cur.size()-1] != '/')
			cur+='/';
		sum_type_directories[dt] = cur;
	}


	count = 1;
	terminated = false;
	prefetch_terminated = false;
	monitor_terminated = false;
	num_data_load = 0;
	num_cache_hits = 0;
	num_mem_emergency = 0;

	mem_limit = MB;
	prefetch_enabled.signal();
	prefetch_thread = thread(bind(&DataManager::prefetch_main, this));
	commit_thread = thread(bind(&DataManager::commit_main, this));
	memory_monitor_thread = thread(bind(&DataManager::memory_monitor_main, this));
	data_load_semaphore.signal();
	process_new_data.signal();
	commit_queue_empty.signal();


}

void DataManager::erase_all_data_with_prefix(const string & prefix)
{
	unique_lock<recursive_mutex> lock(memory_mutex);
	unique_lock<recursive_mutex> lock2(disk_mutex);

	map<pair<string, sum_data_type>, Serializable*>::iterator it =
			in_memory.begin();
	vector<pair<string, sum_data_type> > to_delete;
	//cout << "* prefix " << prefix << endl;
	for(; it!=in_memory.end(); it++)
	{
		const string& key = it->first.first;
		//cout << "key " << key << endl;
		//cout << "find: "<< key.find(prefix) << endl;
		if(key.find(prefix) == 0)
		{
			delete it->second;
			to_delete.push_back(it->first);
		}

	}
	for(unsigned int i=0; i < to_delete.size(); i++)
	{
		in_memory.erase(to_delete[i]);
	}

	set<string> all_files;
	for(int i=SAIL_FUNCTION; i < SUM_END; i++)
		get_serializable_ids(all_files, (sum_data_type)i);

	set<string>::iterator it2 = all_files.begin();
	for(; it2 != all_files.end(); it2++)
	{
		const string& file = *it2;

		if(file.find(prefix) == 0)
		{
			for(int i=SAIL_FUNCTION; i < SUM_END; i++)
			{
				string name = get_filename(file, (sum_data_type)i);
				remove_file(name, sum_type_directories[i]);

			}
		}
	}




}

void DataManager::erase_all_data(sum_data_type sdt)
{
	unique_lock<recursive_mutex> lock(memory_mutex);
	unique_lock<recursive_mutex> lock2(disk_mutex);

	map<pair<string, sum_data_type>, Serializable*>::iterator it =
			in_memory.begin();
	vector<pair<string, sum_data_type> > to_delete;
	//cout << "* prefix " << prefix << endl;
	for(; it!=in_memory.end(); it++)
	{
		if(it->first.second != sdt) continue;
		//cout << "key " << key << endl;
		//cout << "find: "<< key.find(prefix) << endl;

		delete it->second;
		to_delete.push_back(it->first);


	}
	for(unsigned int i=0; i < to_delete.size(); i++)
	{
		in_memory.erase(to_delete[i]);
	}

	set<string> all_files;

	get_serializable_ids(all_files, sdt);

	set<string>::iterator it2 = all_files.begin();
	for(; it2 != all_files.end(); it2++)
	{
		const string& file = *it2;


		string name = get_filename(file, sdt);
		remove_file(name, sum_type_directories[(int)sdt]);



	}




}


void collect_all_files_with_extension(set<string> & reg_paths, string prefix,
		string ext, string file_prefix)
{
	 DIR *dp =  opendir(prefix.c_str());
	 if(dp  == NULL) {
		 return;
	 }
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
	     	string name = string(dirp->d_name);
	     	if(name == "." || name == "..")
	     		continue;
	     	string new_prefix = prefix + name  + "/";
	     	DIR *cur_dp = opendir(new_prefix.c_str());

	     	if(cur_dp  != NULL)
	     	{
	     		closedir(cur_dp);
	     		collect_all_files_with_extension(reg_paths, new_prefix, ext,
	     				file_prefix + name);
	     		continue;
	     	}

	     	if(name.size() <= ext.size()) continue;
	     	string ss = name.substr(name.size()-ext.size());
	     	if(ss == ext)
	     	{
	     		reg_paths.insert(file_prefix +
	     				name.substr(0, name.size()-ext.size()));

	     	}


	 }
	 closedir(dp);
}


void DataManager::get_serializable_ids(set<string> & data, sum_data_type kind)
{
	unique_lock<recursive_mutex> lock(memory_mutex);
	map<pair<string, sum_data_type>, Serializable*>::iterator it =
			in_memory.begin();
	for(; it!=in_memory.end(); it++)
	{
		if(it->first.second != kind) continue;
		const string& key = it->first.first;
		data.insert(key);
	}

	/*
	 * Now, check disk
	 */
	string ext = file_extensions[kind];
	collect_all_files_with_extension(data,sum_type_directories[kind], ext, "");

}

Serializable *DataManager::get_data(string id, sum_data_type kind)
{

	stat_record_load();
	mark_used(id, kind);
	data_load_semaphore.down();

	Serializable* s = get_data_internal(id, kind, false);


	data_load_semaphore.signal();
	return s;
}

Serializable *DataManager::get_data_internal(string id, sum_data_type kind,
		bool is_prefetch)
{

	// If already in memory, return what's in the map.
	Serializable* s = fetch_from_memory(id, kind);
	//cout << "fetched: "<< s << " for " << id << " kind: " << kind << endl;
	if(s!=NULL) {
		if(!is_prefetch) stat_record_cache_hit();
		return s;
	}


	process_new_data.wait();
	process_new_data.signal();
	{
		unique_lock<recursive_mutex> lock(disk_mutex);

		// Try memory again in case someone prefetched it
		// by the time we acquired the lock.
		s = fetch_from_memory(id, kind);
		if(s!=NULL){
			if(!is_prefetch) stat_record_cache_hit();
			return s;
		}


		s = load_object(id, kind);
		if(s == NULL) return NULL;
		update_memory(id, kind, s);
	}
	/*
	 * This races on count, but it is not important if we
	 * miss a few updates.
	 */
	count++;
	if(count % NUM_MEMCHECK_THREAD == 0)
		memory_increased.signal();
	return s;
}

void DataManager::mark_used(string id, sum_data_type kind)
{


	pair<string, sum_data_type> map_key(id, kind);
	unique_lock<recursive_mutex> lock(working_set_mutex);
	if(working_set.count(map_key) == 0)
	{
		working_set[map_key] = 1;
		return;
	}
	int count =  working_set[map_key] + 1;
	working_set[map_key] = count;



}

void DataManager::mark_unused(string id, sum_data_type kind)
{


	pair<string, sum_data_type> map_key(id, kind);
	unique_lock<recursive_mutex> lock(working_set_mutex);
	if(working_set.count(map_key) == 0) return;

	int count =  working_set[map_key] - 1;
	if(count == 0)
		working_set.erase(map_key);
	else
		working_set[map_key] = count;
}

bool DataManager::is_used(string id, sum_data_type kind)
{
	pair<string, sum_data_type> map_key(id, kind);
	unique_lock<recursive_mutex> lock(working_set_mutex);

	if(working_set.count(map_key) == 0)
		return false;
	return working_set[map_key] > 0;
}

void DataManager::assert_used(string id, sum_data_type kind)
{
	unique_lock<recursive_mutex> lock(working_set_mutex);

	assert(working_set.count(pair<string, sum_data_type>(id, kind))>0);
}



void DataManager::update_memory(string id, sum_data_type kind, Serializable* s)
{

	process_new_data.wait();
	process_new_data.signal();
	pair<string, sum_data_type> map_key(id, kind);
	{
		unique_lock<recursive_mutex> map_lock(elem_to_mutex_map_mutex);
		if(elem_to_mutex_map.count(map_key)==0){
			elem_to_mutex_map[map_key] = new recursive_mutex();
		}
	}

	{
		//should not be necessary
		recursive_mutex* elem_recursive_mutex = get_elem_lock(id, kind);
		unique_lock<recursive_mutex> elem_lock(*elem_recursive_mutex);
		unique_lock<recursive_mutex> lock(memory_mutex);
		if(in_memory.count(map_key) > 0){
			Serializable* old_value = in_memory[map_key];
			/*
			 * It is an error to call write_data if the object has
			 * not been marked as unused.
			 */
			if(is_used(id, kind))
			{
				cout << "ERROR: Trying to update object in use: " <<
						id << " kind: " << file_extensions[kind] << endl;
				assert(false);
			}
			if(s != old_value)
				delete old_value;
		}
		in_memory[map_key] = s;
	}
	/*
	 * This races on count, but it is not important if we
	 * miss a few updates.
	 */
	count++;
	if(count % NUM_MEMCHECK_THREAD == 0)
		memory_increased.signal();
}





/*
 * Assumes whoever is calling this function holds the disk lock!
 */
Serializable* DataManager::load_object(string id, sum_data_type kind)
{
	string filename = get_filename(id, kind);
	ifstream infile;
	infile.open(filename.c_str(), ios::binary);
	if(!infile.is_open())
		return NULL;


	//begin_serialization();
	Serializable * s = load_serializable(infile, kind == SAIL_FUNCTION);



	//IN_ARCHIEVE ia(infile);
	//ia >> s;
	//end_serialization();
	assert(s != NULL);

	return s;


}

void DataManager::save_object(Serializable* s, string id, sum_data_type kind)
{
	string filename = get_filename(id, kind);
	ofstream ofile;
	ofile.open(filename.c_str(), ios::binary | ios::trunc);
	assert(ofile.is_open());
	write_serializable(ofile, s, kind == SAIL_FUNCTION);
	//OUT_ARCHIEVE oa(ofile);
	//oa << s;

	ofile.close();
}

Serializable* DataManager::fetch_from_memory(string id, sum_data_type kind,
			bool acquire_elem_lock)
{
	pair<string, sum_data_type> map_key(id, kind);
	{
		unique_lock<recursive_mutex> lock(memory_mutex);
		if(in_memory.count(map_key) == 0) return NULL;
	}
	if(acquire_elem_lock)
	{

		recursive_mutex* elem_recursive_mutex = get_elem_lock(id, kind);
		assert(elem_recursive_mutex != NULL);
		unique_lock<recursive_mutex> elem_lock(*elem_recursive_mutex);
		unique_lock<recursive_mutex> lock(memory_mutex);
		return in_memory[map_key];
	}
	else {
		unique_lock<recursive_mutex> lock(memory_mutex);
		return in_memory[map_key];
	}

}

recursive_mutex* DataManager::get_elem_lock(string id, sum_data_type kind)
{
	pair<string, sum_data_type> map_key(id, kind);
	recursive_mutex* elem_recursive_mutex;
	unique_lock<recursive_mutex> map_lock(elem_to_mutex_map_mutex);
	elem_recursive_mutex = elem_to_mutex_map[map_key];
	return elem_recursive_mutex;
}

void DataManager::write_data(string id, sum_data_type kind, Serializable *s)
{
	mark_dirty(id, kind);
	update_memory(id, kind, s);
}

void DataManager::mark_dirty(string id, sum_data_type kind)
{
	unique_lock<recursive_mutex> lock(modified_objects_mutex);
	modified_objects.insert(pair<string, sum_data_type>(id, kind));
}

bool DataManager::is_dirty(string id, sum_data_type kind)
{
	unique_lock<recursive_mutex> lock(modified_objects_mutex);
	return (modified_objects.count(pair<string, sum_data_type>(id, kind))>0);
}


void DataManager::prefetch_data(string id, sum_data_type kind)
{

	// If already in memory, don't enqueue a prefetch request.
	if(fetch_from_memory(id, kind) != NULL) return;

	{
		unique_lock<recursive_mutex> lock(prefetch_queue_mutex);
		prefetch_requests.push_back(pair<string, sum_data_type>(id, kind));
	}
	prefetch_queue_size.signal();

}

void DataManager::prefetch_main()
{
	while(true) {
		prefetch_enabled.wait();
		prefetch_enabled.signal();
		// wait until someone issues a prefetch request
		prefetch_queue_size.wait();

		if(prefetch_terminated)
			return;
		// Get first prefetch request in queue
		pair<string, sum_data_type> request_id;
		{
			unique_lock<recursive_mutex> lock(prefetch_queue_mutex);
			request_id = prefetch_requests.front();
			prefetch_requests.pop_front();
		}
		// Yield to any actual "get_data" requests.
		data_load_semaphore.wait();
		if(VERBOSE)
			cerr << "Prefetching " << request_id.first << endl;

		get_data_internal(request_id.first, request_id.second);
		data_load_semaphore.signal();
	}
}

void DataManager::commit_main()
{
	while(true)
	{
		commit_queue_size.wait();
		if(terminated){
			unique_lock<recursive_mutex> lock(commit_queue_mutex);
			assert(commit_requests.size() == 0);
			return;
		}
		pair<string, sum_data_type> request_id;
		{
			unique_lock<recursive_mutex> lock(commit_queue_mutex);
			request_id = commit_requests.front();
			if(VERBOSE)
				cerr << "commit thread: queue size " << commit_requests.size()
				<< ": commit on " << request_id.first << endl;

			commit_requests.pop_front();

		}
		commit(request_id.first, request_id.second);
		commit_queue_empty.signal();

	}
}
// TODO: If memory is low (or worse), force commit queue to to be flushed
// so we don't do things unnecessarily.
void DataManager::memory_monitor_main()
{
	bool prefetch_option = true;
	while(!monitor_terminated)
	{
		memory_increased.wait_and_clear();


		memory_info mi = get_memory_usage();
		if(VERBOSE)
			cerr << "Memory monitor running... " << endl;
		// If memory is high or constrained, we can enable
		// prefetching if it was switched off.
		if(mi <= MEM_LOW && !prefetch_option){
			prefetch_enabled.signal();
			prefetch_option = true;
		}
		// If memory is constrained, commit all finalized data to disk.
		if(mi >= MEM_CONSTRAINED){
			if(VERBOSE)
				cerr << "Memory constrained " << endl;
			unique_lock<recursive_mutex> finalized_objs_lock(finalized_mutex);
			set<pair<string, sum_data_type> >::iterator it = finalized_objects.begin();
			for(; it!= finalized_objects.end(); it++){
				enqueue_commit(it->first, it->second);
			}
		}
		// If mem is low, disable prefetch as well as committing finalized data.
		if(mi >= MEM_LOW){
			if(VERBOSE)
					cerr << "Memory low " << endl;
			if(prefetch_option) {
				prefetch_enabled.down();
				prefetch_option = false;
			}
			process_new_data.down();
			commit_queue_empty.wait();
			commit_queue_empty.signal();
			process_new_data.signal();
			mi = get_memory_usage();

		}

		if(mi >= MEM_EMERGENCY){
			stat_record_mem_emergency();
			if(VERBOSE)
					cerr << "Memory emergency " << endl;
			write_objects_to_disk();
			cerr << "EMERGENCY Mem before " << get_bytes_used() <<
					"commit queue size " << commit_requests.size() << endl;
			process_new_data.down();
			commit_queue_empty.wait();
			commit_queue_empty.signal();
			cerr << "EMERGENCY Mem after " << get_bytes_used() << endl;
			process_new_data.signal();
			mi = get_memory_usage();

		}

		if(mi >= MEM_EXHAUSTED){
			cerr << "Out of memory: Memory limit of " << mem_limit << " megabytes"
					" exceeded..." << endl;
			exit(1);
		}

	}
}

void DataManager::write_objects_to_disk(bool all)
{
	unique_lock<recursive_mutex> mem_map_lock(memory_mutex);
	map<pair<string, sum_data_type>, Serializable*>::iterator it =
			in_memory.begin();
	for(; it!= in_memory.end(); it++)
	{
		pair<string, sum_data_type> key = it->first;
		{
			bool used = is_used(key.first, key.second);
			if(!all && used) continue;
		}
		enqueue_commit(key.first, key.second);
	}
}



void DataManager::enqueue_commit(string id, sum_data_type kind)
{
	pair<string, sum_data_type> req(id, kind);
	unique_lock<recursive_mutex> commit_queue_lock(commit_queue_mutex);
	commit_queue_empty.down();
	commit_requests.push_back(req);
	commit_queue_size.signal();
}

/*
 * Commits the specified serializable object to disk.
 */
void DataManager::commit(string id, sum_data_type kind)
{

	if(kind == SAIL_FUNCTION) return;

	if(VERBOSE)
		cerr << " commit called " << id <<"  " << file_extensions[kind] << endl;
	pair<string, sum_data_type> key(id, kind);
	recursive_mutex* elem_recursive_mutex = get_elem_lock(id, kind);


	unique_lock<recursive_mutex> elem_lock(*elem_recursive_mutex);

	{
		// refuse to commit anything that's currently in use.
		if(is_used(id, kind)){
			return;
		}
	}
	if(VERBOSE)
				cerr << " commit : not in working set " << endl;


	Serializable* s = fetch_from_memory(id, kind, false);
	//cout << "~~~~~ fetching " << id << " " << kind << " from mem" << endl;
	//cout << "RES: " << s << endl;
	/*
	 * This object was already committed.
	 */
	if(s == NULL)
		return;

	//cout << "still in memory" << endl;
	remove_from_memory(id, kind);


	if(is_dirty(id, kind)){
	//	cout << "is dirty...write " << id << endl;
		unique_lock<recursive_mutex> disk_lock(disk_mutex);
		save_object(s, id, kind);
	}

	delete s;

}

void DataManager::remove_from_memory(string id, sum_data_type kind)
{

	unique_lock<recursive_mutex> lock(memory_mutex);
	in_memory.erase(pair<string, sum_data_type>(id, kind));
}

void DataManager::flush()
{
	write_objects_to_disk(true);
}

void DataManager::flush_syncronized()
{
	process_new_data.down();
	write_objects_to_disk(true);
	commit_queue_empty.wait();
	commit_queue_empty.signal();
	process_new_data.signal();

}

void DataManager::data_finalized(string id, sum_data_type kind)
{
	unique_lock<recursive_mutex> lock(finalized_mutex);
	finalized_objects.insert(pair<string, sum_data_type>(id, kind));

}





string DataManager::get_filename(string id, sum_data_type kind)
{
	string dir = sum_type_directories[kind];
	string filename = id+ file_extensions[kind];
	string res = get_legal_filename(dir, filename);
	return res;
}

memory_info DataManager::get_memory_usage()
{
	int used_mb = get_mb_used();
	float ratio = used_mb/mem_limit;
	if(ratio <= MEM_HIGH_THRESHOLD) return MEM_HIGH;
	if(ratio <= MEM_CONSTRAINED_THRESHOLD) return MEM_CONSTRAINED;
	if(ratio <= MEM_LOW_THRESHOLD) return MEM_LOW;
	if(ratio <= MEM_EMERGENCY_THRESHOLD) return MEM_EMERGENCY;
	return MEM_EXHAUSTED;

}

int DataManager::get_mb_used()
{
	struct mallinfo mi = mallinfo();
	int used_kb = mi.uordblks/1024L;
	int used_mb = used_kb/1024;
	return used_mb;
}

int DataManager::get_bytes_used()
{
	struct mallinfo mi = mallinfo();
	return mi.uordblks;
}


DataManager::~DataManager() {


	monitor_terminated = true;
	memory_monitor_thread.interrupt();
	memory_monitor_thread.join();


	prefetch_terminated = true;
	prefetch_enabled.signal();
	prefetch_queue_size.signal();
	prefetch_thread.join();

	assert(commit_requests.size() == 0);

	/*
	{
		unique_lock<recursive_mutex> lock(memory_mutex);
		assert(in_memory.size() == 0);
	}
	*/







	terminated = true;

	commit_queue_size.signal();


	commit_thread.join();
	/*
	 * Interrupt since this thread can sleep an arbitrary time
	 */

	clear_elem_to_mutex_map();

	//print_stats();
}

void DataManager::mark_everything_unused()
{
	{
		unique_lock<recursive_mutex> lock(working_set_mutex);
		working_set.clear();
	}

	flush_syncronized();
}

void DataManager::clear_elem_to_mutex_map()
{
	map<pair<string, sum_data_type>, recursive_mutex*>::iterator it =
		elem_to_mutex_map.begin();
	for(; it!= elem_to_mutex_map.end(); it++)
	{
		delete it->second;
	}
}





void DataManager::stat_record_load()
{
#ifdef STATS
	unique_lock<recursive_mutex> lock(stats_mutex);
	num_data_load++;
#endif
}
void DataManager::stat_record_cache_hit()
{
#ifdef STATS
	unique_lock<recursive_mutex> lock(stats_mutex);
	num_cache_hits++;
#endif
}
void DataManager::stat_record_mem_emergency()
{
#ifdef STATS
	unique_lock<recursive_mutex> lock(stats_mutex);
	num_mem_emergency++;
#endif
}

void DataManager::print_stats()
{
	if(!STATS)
	{
		cout << "STATS collection not enabled " << endl;
		return;
	}
	unique_lock<recursive_mutex> lock(stats_mutex);
	cout << "Stats for current run: " << endl;
	cout << "Num data loads: " << num_data_load << endl;
	cout << "Num cache hits: " << num_cache_hits << endl;
	cout << "Percent cache hits: " <<
	((double)num_cache_hits)/((double)num_data_load) * 100.0 << endl;
	cout << "Num memory emergencies: " << num_mem_emergency << endl;

}
