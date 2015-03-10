/*
 * sail-test.cpp
 *
 *  Created on: Apr 11, 2012
 *      Author: tdillig
 */

 // All of the high-level intermediate language headers are combined in sail.h
#include "sail.h"

// All of the low-level intermediate language headers are combined in sail.h
#include "il.h"

using namespace sail; //namespace for high-level il
using namespace il; //namespace for low-levl il
#include <iostream>

#include "Serializer.h"
#include "Function.h"
#include "Cfg.h"
#include "../dex-parser/dex-parser.h"

using namespace std;
int main(int argc, char** argv)
{
	if(argc < 2)
	{
		cout << "No .sail file provided. " << endl;
		exit(1);
	}
	sail::Function* f = sail::Serializer::load_file(argv[1]);
	/*
	il::function_type* f_t = f->get_signature();
	cout << "f_t :::::::::" << f_t->to_string()<< endl;
	il::type* ret = f_t->get_return_type();
	if(ret != NULL)
	{
	    cout << "RET type " << ret->to_string()<<endl;
	    if(ret->is_pointer_type())
	    {
		if(ret->get_inner_type()->is_record_type())
		{
		    il::record_type* r_t = static_cast<il::record_type*>(ret->get_inner_type());
		    map<int,il::record_type*> m = r_t->get_bases();
		    map<int, il::record_type*>::iterator it = m.begin();
		    if(m.size()!= 0)
		    {
			for(; it != m.end(); it++)
			{
			    cout << "RET base t " << (*it).first <<" == " << (*it).second->to_string()<<endl;
			} 
		    }
		    vector<il::record_type*> deriv = r_t->get_derivatives();
		    if(deriv.size()!= 0)
		    {
			for(int u = 0; u < deriv.size(); u++)
			{
			    
			    cout << "RET derivative:: "  << deriv.at(u) <<endl;
			    
			} 
		    }
		}
	    }
	}

	vector<il::type*> arg_ts = f_t->get_arg_types();
	for(int b = 0; b < arg_ts.size(); b++)
	{
	    il::type* cur_para = arg_ts.at(b);
	    if( cur_para!= NULL)
	    {
		cout << "cur para type " << ret->to_string()<<endl;
		if(cur_para->is_pointer_type())
		{
		    if(cur_para->get_inner_type()->is_record_type())
		    {
			il::record_type* r_t = static_cast<il::record_type*>(ret->get_inner_type());
			map<int,il::record_type*> m = r_t->get_bases();
			map<int, il::record_type*>::iterator it = m.begin();
			if(m.size()!= 0)
			{
			    for(; it != m.end(); it++)
			    {
				cout << "Para base t " << (*it).first <<" == " << (*it).second->to_string()<<endl;
			    } 
			}
			vector<il::record_type*> deriv = r_t->get_derivatives();
			if(deriv.size()!= 0)
			{
			    for(int u = 0; u < deriv.size(); u++)
			    {
				cout << "para derivative:: "  << deriv.at(u) <<endl;
			    } 
			}
		    }
		}
	    }
	}*/
	assert(f != NULL);
	cout << "CFG" << endl << f->get_cfg()->to_dotty(false) << endl;

	/*
	vector<Instruction*>* body = f->get_body();
	for(auto it = body->begin(); it != body->end(); it++) {
		Instruction* i = *it;
		cout << "Inst:" << i->to_string(false) << endl;
		if(i->get_instruction_id() == ASSIGNMENT){
			sail::Assignment* a = static_cast<sail::Assignment*>(i);
			sail::Variable* v = a->get_lhs();
			cout << "Var 1: " << v->to_string(true) << endl;
			cout << "Var 2: " << v->to_string(false) << endl;
			cout << v->get_type()->to_string() << endl;
		}
	}
	*/

}
