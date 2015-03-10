/*
 * SailGenerator.h
 *
 *  Created on: Feb 25, 2010
 *      Author: tdillig
 */

#ifndef SAILGENERATOR_H_
#define SAILGENERATOR_H_

#include <string>
#include <set>
class Error;
using namespace std;

/*
 * Compiles the files in the specified path and emits
 * all SAIL files in the specified folder.
 * If the top level directory contains a file called
 * "Makefile", this is used to compile.
 *
 * Notes on Makefiles:
 * SailGenerator passes the following variables to the Makefile:
 *
 * SAIL_GCC: The SAIL gcc compiler.
 * SAIL_CFLAG: --sail=correct_dir -D.. (ADT macro defs) -include=runtime_file
 *
 * The makefile needs to use these variables (as in $(SAIL_GCC) )
 * for the SailGenerator to be able to use them. This needs to be done
 * by hand, since Makefiles vary too much to make anything else practical.
 *
 * Also, be *sure* to disable any optimizations (e.g. pass -o0).
 */

class SailGenerator
{
private:
	set<Error*> & errors;
	const string & compile_dir;
	const string & sail_out_dir;

	set<string> compiled_files;
	int num_lines;

	static string runtime_include_dir;

	bool compile_all;
public:
	SailGenerator(const string & compile_dir,
			const string & sail_out_dir, set<Error*> & errors,
			bool compile_all = false);
	void compile_without_makefile();
	void compile_with_makefile();
	void compile_android_project();
	void compile_java_project(set<string> & files);

	int get_total_lines();
	bool has_makefile();
	const set<string> & get_compiled_files();


	~SailGenerator();
private:
	Error* make_compilation_error(const string & message);
	bool is_number(string s);
	bool is_empty_string(string s);


};

#endif /* SAILGENERATOR_H_ */
