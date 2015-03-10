/*
 * CallingContext.h
 *
 *  Created on: Jun 18, 2012
 *      Author: boyang
 */

#ifndef CALLINGCONTEXT_H_
#define CALLINGCONTEXT_H_


#include <iostream>
#include <vector>
#include <string.h>
#include "Function.h"
#include "Term.h"


using namespace std;
using namespace sail;
using namespace il;


class CallingContext {

private:
	vector <pair<Function *, int> > context;


public:

	CallingContext();

    ~CallingContext();

	CallingContext(const CallingContext& other);

	void add_context(Function *, int);

	void pop_back_context();

	virtual string to_string() const;

	int get_context_size();

	int get_back_line_num();

	string get_caller_name();

	/*
	 * get the function name based on the context
	 * if it returns null, that means entry function
	 */
	string get_curfunc_name();


	void get_curfunc_args(vector<Term*>& v_return);

	bool operator==(const CallingContext &  other) const;
};

#endif /* CALLINGCONTEXT_H_ */
