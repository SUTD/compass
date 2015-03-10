/*
 * Summary.h
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#ifndef SUMMARY_H_
#define SUMMARY_H_

#include "Serializable.h"
#include "DataManager.h"

#define GENERIC_SERIAL_ID 1000

class Summary: public Serializable {
public:
	Summary();
	/*
	 * The == operator is used to determine whether
	 * the summary associated with a function has changed.
	 */
	virtual bool operator==(const Summary& other) = 0;
	virtual bool is_externally_managed();
	/*
	 * Anyone extending Summary has to implement a proper destructor.
	 */
	virtual ~Summary();

};

#endif /* SUMMARY_H_ */
