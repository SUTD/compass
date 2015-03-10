/*
 * PreciseTypeVarStat.h
 *
 *  Created on: Sep 27, 2012
 *      Author: boyang
 */

#ifndef PRECISETYPEVARSTAT_H_
#define PRECISETYPEVARSTAT_H_


#include <set>
#include <Variable.h>
#include <Instruction.h>



using namespace std;
using namespace sail;
using namespace il;



namespace sail {


class PreciseTypeVarStat {

private:
	/*
	 * the variable of PreciseTypeVarStat.
	 */
	Variable* var;

	/*
	 * current type of PreciseTypeVarStat.
	 */
	type* cur_type;

public:
	PreciseTypeVarStat();

	~PreciseTypeVarStat();

	PreciseTypeVarStat(Variable* var, type* type);

	PreciseTypeVarStat(const PreciseTypeVarStat& other);

	/*
	 * get superclass of rt_cur
	 */
	record_type* get_base(record_type* rt_cur);

	/*
	 * merge two PreciseTypeVarStat classes.
	 */
	void merge(PreciseTypeVarStat* other);


	/*
	 * get variable of this state.
	 */
	Variable* get_var();

	/*
	 * get cur_type of this state.
	 */
	type* get_type();

	string to_string() const;

	bool operator==(const PreciseTypeVarStat &  other) const;


};

}


#endif /* PRECISETYPEVARSTAT_H_ */
