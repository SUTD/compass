/*
 * UserQuery.h
 *
 *  Created on: Sep 15, 2011
 *      Author: isil
 */

#ifndef USERQUERY_H_
#define USERQUERY_H_

#include "Constraint.h"
#include "FileIdentifier.h"

#include <map>
#include <string>
using namespace std;

class AccessPath;
class Term;

enum query_type {
	INVARIANT_QUERY,
	WITNESS_QUERY,
	RESTRICTION_QUERY
};

class UserQuery {

	/*
	 * A query to be presented to the user
	 */
	string query;

	/*
	 * Lines in the source code to be highlighted
	 */
	set<pair<string, int> > source_info;

	query_type qt;

public:
	UserQuery(string query, const set<pair<string, int> >& source_info,
			query_type qt);
	string to_string();
	~UserQuery();
};

#endif /* USERQUERY_H_ */
