/*
 * SummaryUnit.h
 *
 *  Created on: Mar 6, 2009
 *      Author: isil
 */

#ifndef SUMMARYUNIT_H_
#define SUMMARYUNIT_H_

#include <string>
#include "../Identifier.h"
using namespace std;




namespace sail{
	class BasicBlock;
	class Variable;

/**
 * \brief A summary unit is either a function or a super block, i.e.
 * any piece of code for which an analysis generates a summary.
 */
class SummaryUnit {
	    friend class boost::serialization::access;

	    template<class Archive>
	    void serialize(Archive & ar, const unsigned int version)
	    {
	    }
public:
	SummaryUnit();
	virtual ~SummaryUnit();

	virtual Identifier get_identifier() = 0;



	virtual bool is_function() = 0;
	virtual bool is_superblock() = 0;

	virtual BasicBlock* get_entry_block() = 0;
	virtual BasicBlock* get_exit_block() = 0;
	virtual BasicBlock* get_exception_block() = 0;


	/*
	 * Is v declared inside this summary unit?
	 */
	bool is_local(sail::Variable* v);

};
}

#endif /* SUMMARYUNIT_H_ */
