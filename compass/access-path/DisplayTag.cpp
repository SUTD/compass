/*
 * DisplayTag.cpp
 *
 *  Created on: Sep 14, 2011
 *      Author: isil
 */

#include "DisplayTag.h"
#include "util.h"

DisplayTag::DisplayTag(string readable_exp, int line, string file) {
	this->description = readable_exp;
	this->line = line;
	this->file = file;

}

DisplayTag::DisplayTag(const DisplayTag& other)
{
	this->description = other.description;
	this->line = other.line;
	this->file = other.file;
}

DisplayTag::DisplayTag()
{
	this->description = "<unknown>";
	this->line = -1;
	this->file = "<unknown>";
}

string DisplayTag::to_string()
{
	string res = description;
	res += " at line " + int_to_string(line);
	res += " in file " + file;
	return res;
}
string DisplayTag::get_description() const
{
	return this->description;
}
int DisplayTag::get_line() const
{
	return this->line;
}
string DisplayTag::get_file() const
{
	return this->file;
}

DisplayTag::~DisplayTag() {

}
