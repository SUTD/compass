/*
 * Error.h
 *
 *  Created on: Oct 11, 2008
 *      Author: tdillig
 */

#ifndef ERROR_H_
#define ERROR_H_

#include "Identifier.h"

enum error_code_type {
	ERROR_UNMODELED_STORE,
	ERROR_ILLEGAL_CAST,
	ERROR_INVALID_POINTER_ARITHMETIC,
	ERROR_STATIC_ASSERT,
	ERROR_UNINITIALIZED_READ,
	ERROR_NULL_DEREF,
	ERROR_BUFFER_OVERRUN,
	ERROR_BUFFER_UNDERRUN,
	ERROR_STACK_RETURN,
	ERROR_INCONSISTENT_ELEM_SIZE,
	ERROR_COMPILATION,
	ERROR_CRASH,
	ERROR_SPEC_MISMATCH,
	ERROR_ILLEGAL_ADT_FUNCTION,
	ERROR_UNRESOLVED_CALL_ID,
	ERROR_FUNCTION_POINTER_WITH_NO_TARGET,
	ERROR_DOUBLE_DELETE,
	ERROR_MEMORY_LEAK,
	ERROR_DELETED_ACCESS,
	ERROR_CLIENT_ANALYSIS
};

static const char* error_names[] = {
	"Store into unmodeled value",
	"Illegal cast",
	"Invalid pointer arithmetic",
	"Static assert failed",
	"Use of uninitialized value",
	"Null dereference error",
	"Buffer overrun error",
	"Buffer underrun error",
	"Callee leaks stack allocation",
	"Access to array with inconsistent element sizes",
	"Compilation failed",
	"Analysis crashed",
	"Specification of function has incompatible type",
	"Illegal use of abstract data type interface",
	"Multiple functions defined with same name and signature",
	"Cannot find any targets for function pointer call"
	"Double delete error",
	"Memory leak detected",
	"Access to deleted memory",
	"(client) "
};

#include <string>
#include <boost/serialization/access.hpp>
using namespace std;

class Error {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & line;
		ar & file;
		ar & error;
		ar & message;
		ar & id;
	}
	Error(){};
public:
	int line;
	string file;
	error_code_type error;
	string message;
	Identifier id;

public:
	Error(int line, const string & file, error_code_type error, string message,
			const Identifier & id);
	string to_string() const;
	virtual ~Error();
	bool operator==(const Error& other) const;
	bool operator<(const Error & other) const;
};

#endif /* ERROR_H_ */
