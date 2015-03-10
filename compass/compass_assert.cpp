/*
 * compass_assert.cpp
 *
 *  Created on: Mar 9, 2010
 *      Author: tdillig
 */
#include <vector>
#include <string>
#include <iostream>
#include "compass_assert.h"

using namespace std;


static vector<string> context;

static void print_context()
{
	if(context.size() == 0) return;
	cerr << "Context: " << endl;
	for(int i= context.size()-1; i>=0; i--)
	{
		cerr << "\t" << context[i] << endl;
	}

}

extern void __compass_assert_fail(const char* exp, const char* file, int line,
		const char* fn)
{

	cerr << file << ":" << line << " " << fn << ": Assertion \"" << exp
			<< "\" failed." << endl;
	print_context();

	abort();
}



extern void __compass_assert_warn(const char* exp, const char* file, int line,
		const char* fn)
{

	cerr << "Warning: " << file << ":" << line << " " << fn <<
			": Assertion \"" << exp << "\" failed." << endl;
	print_context();

}





void push_context(const std::string & ctx)
{
	context.push_back(ctx);
}

extern void pop_context()
{
	if(context.size() == 0) {
		cerr << "Cannot pop empty context." << endl;
		return;
	}
	context.pop_back();
}

extern void clear_context()
{
	context.clear();
}
