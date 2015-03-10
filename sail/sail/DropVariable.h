/*
 * DropTemporary.h
 *
 *  Created on: Oct 6, 2008
 *      Author: tdillig
 */

#ifndef DROPVARIABLE_H_
#define DROPVARIABLE_H_
#include "Instruction.h"

namespace sail{

	/**
	 * \brief This instruction is optional and marks the last use of temporaries
	 * introduced by SAIL.
	 * */
	 /** Since SAIL often introduces a very large number of temporaries,
	 * cleaning the temporaries up after their last use is often a good idea.
	 * DropVariable instructions are generated only if
	 * ADD_DROP_TEMP_INSTRUCTIONS defined in Function.h is set to true.
	 */
	class DropVariable: public Instruction {

	    friend class boost::serialization::access;

	    template<class Archive>
	    void serialize(Archive & ar, const unsigned int version)
	    {
	    	ar & boost::serialization::base_object<sail::Instruction>(*this);
	        ar & var;
	        ar & temp;
	    }
	private:
		Variable* var;

		/*
		 * Is this a temporary introduced by sail or
		 * a real program variable going out of scope?
		 */
		bool temp;

	public:
		DropVariable();
		DropVariable(Variable* temp, bool is_temp);
		Variable* get_var();
		bool is_temp();
		virtual bool is_removable();
		virtual string to_string() const;
		virtual string to_string(bool pretty_print) const;
		virtual ~DropVariable();
	};
}

#endif /* DROPVARIABLE_H_ */
