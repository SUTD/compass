/*
 * util.h
 *
 *  Created on: Jan 6, 2013
 *      Author: isil
 */

#ifndef LIB_UTIL_H_
#define LIB_UTIL_H_

#include <sstream>
#include <fstream>
#include <ctime>
using namespace std;


char* get_time();
string int_to_string(long i);
long int string_to_int(string s);
string float_to_string(float f);

void  address_to_string(string & res, const void* v);



string escape_string(string s);
string escape_string_nonewline(string s);
string line_wrap(string s, int chars_per_line);
string trim_whitespaces(const string& s);
bool is_whitespace(char c);




#endif /* LIB_UTIL_H_ */
