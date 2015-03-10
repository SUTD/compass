/*
 * Error.cpp
 *
 *  Created on: Oct 11, 2008
 *      Author: tdillig
 */

#include "Error.h"
#include "util.h"

Error::Error(int line, const string & file, error_code_type error,
		string message, const Identifier & id) {
	this->line = line;
	this->file = file;
	this->error = error;
	this->message = message;
	this->id = id;
}

Error::~Error() {

}

string Error::to_string() const
{
	string res = "Error: ";
	if(message == "")
	{
		res +=  string(error_names[error]);
		if(line != -1) res+= " at line " + int_to_string(line);
	}

	else res +=  message;
	return res;
}

bool Error::operator==(const Error& other) const
{
	return line == other.line && error == other.error && file == other.file;
}
bool Error::operator<(const Error & other) const
{
	if(line < other.line) return true;
	if(line > other.line) return false;
	if(file < other.file) return true;
	if(file > other.file) return false;
	return error < other.error;
}

