/*
 * GlobalAnalysisState.h
 *
 *  Created on: Apr 9, 2009
 *      Author: tdillig
 */

#ifndef GLOBALANALYSISSTATE_H_
#define GLOBALANALYSISSTATE_H_
#include <string>
using namespace std;

class CGNode;
class DataManager;

enum check_type
{
	CHECK_NONE = 0,
	CHECK_BUFFER = 1,
	CHECK_NULL = 2,
	CHECK_UNINIT = 4,
	CHECK_CAST = 8,
	CHECK_MEMORY_LEAK = 16,
	CHECK_DOUBLE_DELETE = 32,
	CHECK_DELETED_ACCESS = 64
};

class GlobalAnalysisState {
public:
	static check_type checks;
	static check_type disabled_checks;

	static void add_check_type(check_type ct);
	static bool check_buffer();
	static bool check_null();
	static bool check_uninit();
	static bool check_casts();
	static bool check_memory_leaks();
	static bool check_double_deletes();
	static bool check_deleted_access();
	static bool track_delete();

	static string get_analysis_options();
	static void disable_checks();
	static void enable_checks();
	static void set_analysis_options(CGNode* check_fn,
			DataManager & dm);

	static void enable_buffer_check();
	static void enable_null_check();
	static void enable_uninit_check();
	static void enable_cast_check();
	static void enable_memory_leak_check();
	static void enable_double_delete_check();
	static void enable_deleted_access_check();


public:
	static void clear();
};

#endif /* GLOBALANALYSISSTATE_H_ */

