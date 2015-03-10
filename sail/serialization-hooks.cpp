/*
 * serialization-hooks.cpp
 *
 *  Created on: Apr 10, 2010
 *      Author: tdillig
 */
#include "serialization-hooks.h"

#include <set>
#include <iostream>
using namespace std;

#include "namespace_context.h"
#include "type.h"

set<callback_t>* callbacks_begin;
set<callback_t>* callbacks_end;

bool active = false;

void register_begin_callback(callback_t fn)
{

	/*
	static bool first = true;
	if(first){
		callbacks_begin = new set<callback_t>();
		first = false;
	}

	callbacks_begin->insert(fn);
	*/
}
void register_end_callback(callback_t fn)
{
	/*
	cout << "REGISTERING>>>" << endl;

	static bool first = true;
	if(first){
		cout << "FIRST " << endl;
		callbacks_end = new set<callback_t>();
		first = false;
	}


	callbacks_end->insert(fn);
	cout << "NEW SIZE OF SET" << callbacks_end->size() << endl;
	*/
}

bool in_serialization()
{
	return active;
}


void begin_serialization()
{
	active = true;
	/*active = true;
	set<callback_t>::iterator it = callbacks_begin->begin();
	for(; it != callbacks_begin->end(); it++)
	{
		(**it)();
	}
	*/
}
void end_serialization()
{


	il::type::uniquify_loaded_types();
	il::_namespace_context::delete_unused_instances();
	active = false;

	/*
	cout << "EENNDD" << endl;
	cout << "size: " << callbacks_end->size() << endl;
	set<callback_t>::iterator it = callbacks_end->begin();
	for(; it != callbacks_end->end(); it++)
	{
		(**it)();
	}
	active = false;
	*/
}
