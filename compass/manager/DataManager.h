/*
 * DataManager.h
 *
 *  Created on: Aug 17, 2008
 *      Author: tdillig
 */

#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_


#define STATS true

#include <boost/thread.hpp>
#include "Semaphore.h"
#include "Serializable.h"
#include <list>

#include "serializable_types.h"

#define MEM_HIGH_THRESHOLD 0.5
#define MEM_CONSTRAINED_THRESHOLD 0.85
#define MEM_LOW_THRESHOLD 0.9
#define MEM_EMERGENCY_THRESHOLD 0.99

using namespace boost;





// Do not change the order of this enum fields! There are checks like
// mi >= MEM_LOW etc.
enum memory_info {
	MEM_HIGH, //don't swap anything
	MEM_CONSTRAINED, // only swap finalized data
	MEM_LOW, // stop prefetching
	MEM_EMERGENCY, //swap everything except the working set
	MEM_EXHAUSTED // there isn't even space for working set
				  // -- throw memory low exception
};

class DataManager {
public:
	/*
	 * MBs of data the DataManager keeps in memory before
	 * writing things to disk.
	 */
	DataManager(int MB, map<sum_data_type, string> & types_to_dir);

	/*
	 * Returns the Serializable* associated with the given
	 * identifier and data type, NULL if non-existent.
	 * This blocks on IO if the data is not in memory.
	 * It also has the side effect of adding the specified
	 * id to the working_list so that the returned Serializable*
	 * object won't be swapped out to disk until mark_unused
	 * is explicitly invoked.
	 */

	Serializable *get_data(string id, sum_data_type kind);

	/*
	 * Writes the Serializable object associated with the identifier
	 * and data type in memory (and if necessary, disk). The data
	 * is only guaranteed to be written to disk after either
	 * flush() is explicitly called or the destructor
	 * is invoked.
	 * Important: If this (id, kind) is already in the map,
	 * this function will delete the previous binding.
	 * Thus, anyone who calls this function will need
	 * to make a deep copy of the previous binding
	 * if the previous binding is to be modified.
	 */
	void write_data(string id, sum_data_type kind, Serializable *s);

	/*
	 * Enqueues a request to preload the Serializable object specified
	 * by the identifier and kind into memory. This request may be ignored
	 * if memory is low. This function does not block. This has no effect
	 * if the object is not present on disk.
	 */
	void prefetch_data(string id, sum_data_type kind);

	/*
	 * The specified Serializable id is finalized and
	 * hence a good candidate for commiting to disk
	 * when memory is low.
	 */
	void data_finalized(string id, sum_data_type kind);

	/*
	 * The specified id should be removed from working set.
	 * If memory is very low, any unused element
	 * may be swapped out.
	 */
	void mark_unused(string id, sum_data_type kind);

	void mark_everything_unused();


	/*
	 * Flushes write requests to disk. This function is non-blocking.
	 */
	void flush();

	/*
	 * Flushes write requests to disk. This blocks until all data is written.
	 */
	void flush_syncronized();

	/*
	 * Returns a set with the keys of all available data with
	 * type kind. Note that this data my be either on disk or in
	 * memory.
	 */
    void get_serializable_ids(set<string> & data, sum_data_type kind);

    /*
     * Erases all data matching the prefix from memory.
     */
    void erase_all_data_with_prefix(const string & prefix);

    /*
      * Erases all data matching of the given type.
      */
    void erase_all_data(sum_data_type sdt);


	virtual ~DataManager();

private:

	/*
	 * The directory in which each sum_type is stored.
	 */
	string sum_type_directories[SUM_END];

	int mem_limit;


	map<pair<string, sum_data_type>, Serializable*> in_memory;
	recursive_mutex memory_mutex;


	// Fine-grained recursive_mutex for individual elements in the map so that
	// no one accesses this element if it is being swapped out
	map<pair<string, sum_data_type>, recursive_mutex*> elem_to_mutex_map;
	recursive_mutex elem_to_mutex_map_mutex;


	/*
	 * The set of id's currently in use and the number of users
	 * if num_users>=1 the object cannot be
	 * flushed to disk at this moment.
	 */
	map<pair<string, sum_data_type> , int > working_set;
	recursive_mutex working_set_mutex;

	recursive_mutex disk_mutex;
	list<pair<string, sum_data_type> > prefetch_requests;
	recursive_mutex prefetch_queue_mutex;
	thread prefetch_thread;
	Semaphore prefetch_queue_size;
	Semaphore prefetch_enabled;

	thread commit_thread;
	list<pair<string, sum_data_type> > commit_requests;
	recursive_mutex commit_queue_mutex;
	Semaphore commit_queue_size;

	thread memory_monitor_thread;
	Semaphore memory_increased;

	Semaphore process_new_data;

	Semaphore commit_queue_empty;


	Semaphore data_load_semaphore;
	set<pair<string, sum_data_type> > modified_objects;
	recursive_mutex modified_objects_mutex;
	set<pair<string, sum_data_type> > finalized_objects;
	recursive_mutex finalized_mutex;

	volatile bool terminated;
	volatile bool prefetch_terminated;
	volatile bool monitor_terminated;

	/*
	 * Count for running the memory check thread
	 */
	volatile int count;

#ifdef STATS
	recursive_mutex stats_mutex;
	int num_data_load;
	int num_cache_hits;
	int num_mem_emergency;
#endif
	void print_stats();


private:


	memory_info get_memory_usage();
	inline string get_filename(string id, sum_data_type kind);
	// The given element is being written out and we should
	// assert it's in working set.
	void assert_used(string id, sum_data_type kind);
	inline Serializable* fetch_from_memory(string id, sum_data_type kind,
			bool acquire_elem_lock = true);
	void update_memory(string id, sum_data_type kind, Serializable* s);


	Serializable* load_object(string id, sum_data_type kind);
	void save_object(Serializable* s, string id, sum_data_type kind);
	void prefetch_main();
	void commit_main();
	void memory_monitor_main();
	Serializable *get_data_internal(string id, sum_data_type kind,
			bool is_prefetch=true);
	void mark_dirty(string id, sum_data_type kind);
	bool is_dirty(string id, sum_data_type kind);
	void commit(string id, sum_data_type kind);
	void enqueue_commit(string id, sum_data_type kind);
	recursive_mutex* get_elem_lock(string id, sum_data_type kind);
	void mark_used(string id, sum_data_type kind);
	void write_objects_to_disk(bool all = false);
	void remove_from_memory(string id, sum_data_type kind);
	bool is_used(string id, sum_data_type kind);
	int get_mb_used();
	int get_bytes_used();
	void clear_elem_to_mutex_map();;


	inline void stat_record_load();
	inline void stat_record_cache_hit();
	inline void stat_record_mem_emergency();




};

#endif /* DATAMANAGER_H_ */
