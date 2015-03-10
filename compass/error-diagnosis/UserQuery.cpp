/*
 * UserQuery.cpp
 *
 *  Created on: Sep 15, 2011
 *      Author: isil
 */

#include "UserQuery.h"

UserQuery::UserQuery(string query, const set<pair<string, int> >& source_info,
		query_type qt)
{
	this->query = query;
	this->source_info = source_info;
	this->qt = qt;

}

string UserQuery::to_string()
{
	string pre;
	if(qt == INVARIANT_QUERY) pre = "Is the following always true?\n";
	else if(qt == WITNESS_QUERY) pre = "Is the following possible?\n";
	else if(qt == RESTRICTION_QUERY) pre = "Are there any restrictions on the"
			"following value?\n";
	return pre + query;
}

UserQuery::~UserQuery() {

}
