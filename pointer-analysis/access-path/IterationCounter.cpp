/*
 * IterationCounter.cpp
 *
 *  Created on: Oct 14, 2009
 *      Author: tdillig
 */

#include "IterationCounter.h"
#include "util.h"

IterationCounter::IterationCounter(int id, bool is_parametric,
		const string & context, int num_contexts, int num_total_contexts):
		VariableTerm(CNode::get_varmap().get_id("it_counter" + int_to_string(id)+
				context),
				(is_parametric? ATTRIB_ITERATION_COUNTER:
				ATTRIB_LOOP_TERMINATION_VAR))
{
	this->id = id;
	this->parametric = is_parametric;
	this->context = context;
	this->num_contexts = num_contexts;
	this->num_total_contexts = num_total_contexts;
	this->t = il::get_unsigned_integer_type();
	this->apt = AP_COUNTER;
	this->specialization_type = AP_COUNTER;
	this->has_index_var = false;
	add_signedness_attribute();
}

IterationCounter* IterationCounter::make_parametric(int id)
{
	IterationCounter* ic = new IterationCounter(id, true, "", 0, 0);
	return (IterationCounter*)Term::get_term(ic);
}
IterationCounter* IterationCounter::make_last(int id)
{
	IterationCounter* ic = new IterationCounter(id, false, "", 0, 0);
	return  (IterationCounter*)Term::get_term(ic);
}
IterationCounter* IterationCounter::make_parametric(IterationCounter* v)
{
	IterationCounter* ic = new IterationCounter(v->get_id(), true,
			v->get_context(), v->get_num_contexts(), v->get_num_total_contexts());
	return  (IterationCounter*)Term::get_term(ic);
}
IterationCounter* IterationCounter::make_last(IterationCounter* v)
{
	IterationCounter* ic = new IterationCounter(v->get_id(), false,
			v->get_context(), v->get_num_contexts(),
			v->get_num_total_contexts());
	return  (IterationCounter*)Term::get_term(ic);
}
IterationCounter* IterationCounter::add_context(IterationCounter* ic,
		int inst_id, const string & context, bool inc_num_contexts)
{
	string new_context = ic->get_context() + ":" +int_to_string(inst_id) + ":" +
			context;
	int num_contexts = (inc_num_contexts? ic->get_num_contexts()+1 :
		ic->get_num_contexts());
	IterationCounter* new_ic = new IterationCounter(ic->get_id(),
			ic->is_parametric(), new_context, num_contexts,
			ic->num_total_contexts+1);
	return  (IterationCounter*)Term::get_term(new_ic);
}

bool IterationCounter::is_parametric()
{
	return parametric;
}
bool IterationCounter::is_last()
{
	return !parametric;
}
string IterationCounter::to_string()
{
	if(PRINT_AS_TERM) {
		return VariableTerm::to_string();
	}

	string suffix = int_to_string(id) + context;
	if(parametric) return "k"+suffix;
	else return "N"+suffix;
}
int IterationCounter::get_id()
{
	return id;
}
string IterationCounter::get_context()
{
	return context;
}
int IterationCounter::get_num_contexts()
{
	return num_contexts;
}
int IterationCounter::get_num_total_contexts()
{
	return num_total_contexts;
}
Term* IterationCounter::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}

Term* IterationCounter::substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}


/*
 * Makes an iteration counter that is identical to c, but its id is 0.
 */
IterationCounter* IterationCounter::normalize(IterationCounter* ic)
{

	IterationCounter* nic = new IterationCounter(0,
			ic->is_parametric(),
			ic->get_context(), ic->get_num_contexts(),
			ic->get_num_total_contexts());

	return (IterationCounter*)Term::get_term(nic);

}

IterationCounter::~IterationCounter() {

}
