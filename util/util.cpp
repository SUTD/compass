/*
 * util.cpp
 *
 *  Created on: Jan 6, 2013
 *      Author: isil
 */

#include <string>
#include <sstream>
using namespace std;

string int_to_string(long i)
{

	char temp[100];
	sprintf(temp, "%ld",  i);
	string res = temp;
	return res;
}

long int string_to_int(string s)
{
	stringstream ss;
	ss<< s;
	long int res;
	ss >> res;
	return res;
}

string float_to_string(float i)
{
	stringstream s;
	s << i;
	string res;
	s >> res;
	return res;
}



void  address_to_string(string & res, const void* v)
{
	char temp[100];
	temp[0] = 'a';
	sprintf(temp+1, "%lu", (unsigned long int) v);
	res.append(temp);
}







string escape_string(string s)
{

	string res;
	for(unsigned int i=0; i<s.size(); i++)
	{
		if(s[i] == '\n') res += "\\n";
		else if(s[i] == '\r') res += "\\r";
		else if(s[i] == '\t') res += "\\t";
		else if(s[i] == 0) res+="0";
		else if(s[i] == '\"') res+="\\\"";
		else res += s[i];
	}
	return res;
}

string escape_string_nonewline(string s)
{
	string res;
	for(unsigned int i=0; i<s.size(); i++)
	{
		if(s[i] == '\r') res += "\\r";
		else if(s[i] == 0) res+="0";
		else if(s[i] == '\"') res+="\\\"";
		else res += s[i];
	}
	return res;
}

string line_wrap(string s, int chars_per_line)
{
	string res = "";
	int count = 0;
	for(unsigned int i=0; i<s.size(); i++)
	{
		if(s[i] == '\n'){
			count = 0;
		}
		else{
			count++;
			if(count >= chars_per_line) {
				res += "\n   ";
				count = 3;
			}
		}

		res+=s[i];
	}
	return res;
}

bool is_whitespace(char c)
{
	if(c=='\t') return true;
	if(c == '\n') return true;
	if(c == ' ') return true;
	return false;
}

string trim_whitespaces(const string& s)
{
	string trimmed = "";
	for(int i=0; i<(int)s.size(); i++)
	{
		char cur = s[i];
		if(is_whitespace(cur)) continue;
		trimmed += cur;
	}
	return trimmed;
}


char* get_time(){
	time_t now = time(0);
	char* dt = ctime(&now);
	return dt;
}

